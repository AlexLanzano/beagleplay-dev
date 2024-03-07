#include "linux/kern_levels.h"
#include "linux/printk.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#include <drm/drm_drv.h>
#include <drm/drm_simple_kms_helper.h>

static struct gpio_desc *g_com;
static int g_com_value = 0;

static unsigned char lookup[16] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

uint8_t reverse_byte(uint8_t n) {
   // Reverse the top and bottom nibble then swap them.
   return (lookup[n&0b1111] << 4) | lookup[n>>4];
}


static void clear_display(struct spi_device *spi)
{
    int ret;
    g_com_value ^= 1;
    uint8_t buffer[2305] = {0};
    buffer[0] = reverse_byte(0x1) | (g_com_value << 6);
    for (uint8_t addr = 0; addr < 128; ++addr) {
        buffer[1 + (18*addr)] = reverse_byte(addr + 1);
        memset(&buffer[2 + (18*addr)], 0xff, 16);
        
    }    
    ret = spi_write(spi, buffer, 2305);
    if (ret) {
        printk(KERN_ERR "ALEX: failed to write\n");
    }

}

static int sharp_display_probe(struct spi_device *spi)
{
    printk(KERN_CRIT "ALEX: probe\n");
    int ret;
    ret = spi_setup(spi);
    if (ret < 0) {
        printk(KERN_ERR "ALEX: failed to setup spi\n");
        return ret;
    }

    g_com = devm_gpiod_get(&spi->dev, "com", GPIOD_OUT_LOW);
    if (!g_com) {
        printk(KERN_ERR "ALEX: GPIO FAILED\n");
    }
    
    //g_toggle_com_task = kthread_create(toggle_com_thread, NULL, "toggle_com_thread");
    //if (g_toggle_com_task)
    //{
    //    wake_up_process(g_toggle_com_task);
    //}

    clear_display(spi);

    return 0;
}

static const struct drm_simple_display_pipe_funcs sharp_pipe_funcs = {

};

static void sharp_display_remove(struct spi_device *spi) 
{
    //kthread_stop(g_toggle_com_task);
}

static const struct spi_device_id sharp_display_ids[] = {
    {"sharpdisplay",0},
    {},
};
MODULE_DEVICE_TABLE(spi, sharp_display_ids);

static const struct of_device_id sharp_display_of_match[] = {
    {.compatible = "sharpdisplay"},
    {},
};
MODULE_DEVICE_TABLE(of, sharp_display_of_match);

static struct spi_driver sharp_display_driver = {
    .driver = {
        .name = "sharpdisplay",
        .of_match_table = sharp_display_of_match,
    },
    .probe = sharp_display_probe,
    .remove = sharp_display_remove,
    .id_table = sharp_display_ids,
};
module_spi_driver(sharp_display_driver);

MODULE_AUTHOR("Alex Lanzano <lanzano.alex@gmail.com>");
MODULE_DESCRIPTION("SPI Protocol driver for the sharp display");
MODULE_LICENSE("GPL v2");
