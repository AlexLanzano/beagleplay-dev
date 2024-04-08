#include "linux/kern_levels.h"
#include "linux/printk.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_damage_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_fb_dma_helper.h>
#include <drm/drm_fbdev_generic.h>
#include <drm/drm_format_helper.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_dma_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_managed.h>
#include <drm/drm_modes.h>
#include <drm/drm_rect.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_simple_kms_helper.h>

#define SHARP_MODE_PERIOD 8
#define SHARP_ADDR_PERIOD 8
#define SHARP_DUMMY_PERIOD 8

static unsigned char lookup[16] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

enum sharp_model {
    LS013B7DH03 = 1,
};

struct sharp_device {
    struct drm_device drm;
    struct drm_simple_display_pipe pipe;
    const struct drm_display_mode *mode;
    struct drm_connector connector;

    struct spi_device *spi;

    uint32_t pitch;
    uint32_t tx_buffer_size;
    uint8_t *tx_buffer;
};

static inline struct sharp_device *drm_to_sharp_device(struct drm_device *drm)
{
    return container_of(drm, struct sharp_device, drm);
}

static uint8_t reverse_byte(uint8_t n) {
   // Reverse the top and bottom nibble then swap them.
   return (lookup[n&0b1111] << 4) | lookup[n>>4];
}

static void clear_display(struct sharp_device *sharp_dev)
{
    uint8_t *tx_buffer = sharp_dev->tx_buffer;
    tx_buffer[0] = reverse_byte(0x4);
    spi_write(sharp_dev->spi, tx_buffer, 2);
}

DEFINE_DRM_GEM_DMA_FOPS(sharp_fops);

static const struct drm_driver sharp_drm_driver = {
	.driver_features	= DRIVER_GEM | DRIVER_MODESET | DRIVER_ATOMIC,
    .fops			= &sharp_fops,
	DRM_GEM_DMA_DRIVER_OPS_VMAP,
	.name			= "sharp_display",
	.desc			= "Sharp Display Memory LCD",
	.date			= "20231129",
	.major			= 1,
	.minor			= 0,
};

static enum drm_mode_status sharp_pipe_mode_valid(struct drm_simple_display_pipe *pipe,
                                                  const struct drm_display_mode *mode)
{
    struct drm_crtc *crtc = &pipe->crtc;
    struct sharp_device *sharp_device = drm_to_sharp_device(crtc->dev);

    return drm_crtc_helper_mode_valid_fixed(crtc, mode, sharp_device->mode);
}

static void sharp_pipe_enable(struct drm_simple_display_pipe *pipe,
                              struct drm_crtc_state *crtc_state,
                              struct drm_plane_state *plane_state)
{
    struct sharp_device *sharp_device = drm_to_sharp_device(pipe->crtc.dev);
    clear_display(sharp_device);
}

static void sharp_pipe_disable(struct drm_simple_display_pipe *pipe)
{

}

static void set_tx_buffer_addresses(uint8_t *buffer, struct drm_rect clip, uint32_t pitch)
{
    for (uint32_t line = 0; line < clip.y2; ++line) {
        buffer[line * pitch] = reverse_byte(line+1);
    }
}

static void set_tx_buffer_data(uint8_t *buffer, struct drm_framebuffer *fb, struct drm_rect clip, uint32_t pitch)
{
    int ret;
    struct iosys_map dst, vmap;
    struct drm_gem_dma_object *dma_obj = drm_fb_dma_get_gem_obj(fb, 0);

    ret = drm_gem_fb_begin_cpu_access(fb, DMA_FROM_DEVICE);
    if (ret) {
        printk(KERN_ERR "Failed to begin dma access\n");
        return;
    }

    iosys_map_set_vaddr(&dst, buffer);
    iosys_map_set_vaddr(&vmap, dma_obj->vaddr);

    drm_fb_xrgb8888_to_mono(&dst, &pitch, &vmap, fb, &clip);

    drm_gem_fb_end_cpu_access(fb, DMA_FROM_DEVICE);

    for (uint32_t y = clip.y1; y < clip.y2; ++y) {
        for (uint32_t x = clip.x1/8; x < clip.x2/8; ++x) {
            
            uint8_t temp_val = buffer[y * pitch + x];

            buffer[y * pitch + x] = reverse_byte(temp_val);

        }
    }
}

static void set_tx_buffer(struct sharp_device *sharp_dev, struct drm_framebuffer *fb, struct drm_rect clip)
{
    uint8_t *tx_buffer = sharp_dev->tx_buffer;
    tx_buffer[0] = reverse_byte(0x1);
    set_tx_buffer_addresses(&tx_buffer[1], clip, sharp_dev->pitch);
    set_tx_buffer_data(&tx_buffer[2], fb, clip, sharp_dev->pitch);
}

static void sharp_fb_dirty(struct drm_framebuffer *fb, struct drm_rect *rect)
{
    struct drm_rect clip;
    struct sharp_device *sharp_dev = drm_to_sharp_device(fb->dev);

    clip.x1 = 0;
    clip.x2 = fb->width;
    clip.y1 = rect->y1;
    clip.y2 = rect->y2; 

    set_tx_buffer(sharp_dev, fb, clip);
    spi_write(sharp_dev->spi, sharp_dev->tx_buffer, sharp_dev->tx_buffer_size);
}

static void sharp_pipe_update(struct drm_simple_display_pipe *pipe,
                              struct drm_plane_state *old_state)
{
    struct drm_plane_state *state = pipe->plane.state;
    struct drm_rect rect;

    if (!pipe->crtc.state->active)
    {
        return;
    }

    if (drm_atomic_helper_damage_merged(old_state, state, &rect)) {
        sharp_fb_dirty(state->fb, &rect);
    }
}

static const struct drm_simple_display_pipe_funcs sharp_pipe_funcs = {
    .mode_valid = sharp_pipe_mode_valid,
    .enable = sharp_pipe_enable,
    .disable = sharp_pipe_disable,
    .update = sharp_pipe_update,
};

static int sharp_connector_get_modes(struct drm_connector *connector)
{
    struct sharp_device *sharp_device = drm_to_sharp_device(connector->dev);

    return drm_connector_helper_get_modes_fixed(connector, sharp_device->mode);
}

static const struct drm_connector_helper_funcs sharp_connector_hfuncs = {
    .get_modes = sharp_connector_get_modes,
};

static const struct drm_connector_funcs sharp_connector_funcs = {
    .reset = drm_atomic_helper_connector_reset,
    .fill_modes = drm_helper_probe_single_connector_modes,
    .destroy = drm_connector_cleanup,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,

 };

static const struct drm_mode_config_funcs sharp_mode_config_funcs = {
	.fb_create = drm_gem_fb_create_with_dirty,
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

static void sharp_remove(struct spi_device *spi) 
{
    struct sharp_device *sharp_device = spi_get_drvdata(spi);

    drm_dev_unplug(&sharp_device->drm);
    drm_atomic_helper_shutdown(&sharp_device->drm);
}

static const struct spi_device_id sharp_ids[] = {
    {"ls013b7dh03", LS013B7DH03},
    {},
};
MODULE_DEVICE_TABLE(spi, sharp_ids);

static const struct of_device_id sharp_of_match[] = {
    {.compatible = "sharp,ls013b7dh03"},
    {},
};
MODULE_DEVICE_TABLE(of, sharp_of_match);

static const struct drm_display_mode sharp_ls013b7dh03_mode = {
	DRM_SIMPLE_MODE(128, 128, 23, 23),
};

static const uint32_t sharp_formats[] = {
	DRM_FORMAT_XRGB8888,
};

static inline enum sharp_model sharp_model_from_device_id(struct spi_device *spi)
{
    const struct spi_device_id *spi_id = spi_get_device_id(spi);
    if (spi_id) {
        return (enum sharp_model)spi_id->driver_data;
    }
    return 0;
}

static int sharp_probe(struct spi_device *spi)
{
    int ret;
    ret = spi_setup(spi);
    if (ret < 0) {
        printk(KERN_ERR "failed to setup spi\n");
        return ret;
    }
    printk(KERN_ERR "probe\n");
    struct device *dev = &spi->dev;
    if (!dev->coherent_dma_mask) {
        ret = dma_coerce_mask_and_coherent(dev, DMA_BIT_MASK(32));
        if (ret)
        {
            return dev_err_probe(dev, ret, "Failed to set dma mask\n");
        }
    }

    struct sharp_device *sharp_device = devm_drm_dev_alloc(&spi->dev, &sharp_drm_driver,
                                                           struct sharp_device, drm);

    spi_set_drvdata(spi, sharp_device);

    sharp_device->spi = spi;
    struct drm_device *drm = &sharp_device->drm;
    ret = drmm_mode_config_init(drm);
    if (ret) {
        return ret;
    }
    

    drm->mode_config.funcs = &sharp_mode_config_funcs;

    enum sharp_model model = sharp_model_from_device_id(spi);
    switch (model) {
        case LS013B7DH03:
            sharp_device->mode = &sharp_ls013b7dh03_mode;
            break;
    }

    sharp_device->pitch = (SHARP_ADDR_PERIOD + sharp_device->mode->hdisplay + SHARP_DUMMY_PERIOD) / 8;
    sharp_device->tx_buffer_size = 
        (SHARP_MODE_PERIOD + 
        (SHARP_ADDR_PERIOD + (sharp_device->mode->hdisplay) + SHARP_DUMMY_PERIOD) * 
        sharp_device->mode->vdisplay) / 8; 
    
    sharp_device->tx_buffer = devm_kzalloc(&spi->dev, sharp_device->tx_buffer_size, GFP_KERNEL);

    drm->mode_config.min_width = sharp_device->mode->hdisplay;
    drm->mode_config.max_width = sharp_device->mode->hdisplay;
    drm->mode_config.min_height = sharp_device->mode->vdisplay;
    drm->mode_config.max_height = sharp_device->mode->vdisplay;

    ret = drm_connector_init(drm, &sharp_device->connector,
                             &sharp_connector_funcs,
                             DRM_MODE_CONNECTOR_SPI);
    if (ret) {
        printk(KERN_ERR "failed to init connector\n");
        return ret;
    }

    drm_connector_helper_add(&sharp_device->connector,
                             &sharp_connector_hfuncs);

    ret = drm_simple_display_pipe_init(drm, &sharp_device->pipe,
                                       &sharp_pipe_funcs,
                                       sharp_formats,
                                       ARRAY_SIZE(sharp_formats),
                                       NULL,
                                       &sharp_device->connector);
    if (ret) {
        printk(KERN_ERR "failed to init simple display pipe\n");
        return ret;
    }

    drm_plane_enable_fb_damage_clips(&sharp_device->pipe.plane);
    drm_mode_config_reset(drm);
    ret = drm_dev_register(drm, 0);
    if (ret) {
        printk(KERN_ERR "failed to register drm device\n");
        return ret;
    } 

    clear_display(sharp_device);
    drm_fbdev_generic_setup(drm, 0);

    return 0;
}

static struct spi_driver sharp_spi_driver = {
    .driver = {
        .name = "sharp",
        .of_match_table = sharp_of_match,
    },
    .probe = sharp_probe,
    .remove = sharp_remove,
    .id_table = sharp_ids,
};
module_spi_driver(sharp_spi_driver);


MODULE_AUTHOR("Alex Lanzano <lanzano.alex@gmail.com>");
MODULE_DESCRIPTION("SPI Protocol driver for the sharp display");
MODULE_LICENSE("GPL v2");
