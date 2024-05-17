#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <linux/fb.h>

struct drm_device {
    uint8_t *fb;

    uint32_t connector_id;
    uint32_t encoder_id;
    uint32_t crtc_id;
    uint32_t fb_id;
    
    uint32_t width;
    uint32_t height;

    uint32_t pitch;
    uint32_t size;
    uint32_t handle;

    drmModeModeInfo mode;
    drmModeCrtc *crtc;

    struct drm_device *next;
};

void drm_init_fb(int fd, struct drm_device *dev)
{
    int ret;
    struct drm_mode_create_dumb creq = {0};
    struct drm_mode_map_dumb mreq = {0};
    
    creq.width = dev->width;
    creq.height = dev->height;
    creq.bpp = 32;

    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
    if (ret < 0) {
        printf("failed to create dumb buffer\n");
        return;
    }

    dev->pitch = creq.pitch;
    dev->size = creq.size;
    dev->handle = creq.handle;

    printf("w %i h %i pitch %i\n", dev->width, dev->height, dev->pitch);
    ret = drmModeAddFB(fd, dev->width, dev->height, 24, 32, dev->pitch, dev->handle, &dev->fb_id);
    if (ret) {
        printf("Failed to add fb %i\n", ret);
        return;
    }

    mreq.handle = dev->handle;
    ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
    if (ret) {
        printf("failed to map dumb buffer\n");
        return;
    }

    dev->fb = mmap(0, dev->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);
    if (dev->fb == MAP_FAILED) {
        printf("failed to mmap\n");
        return;
    }

    dev->crtc = drmModeGetCrtc(fd, dev->crtc_id);
    ret = drmModeSetCrtc(fd, dev->crtc_id, dev->fb_id, 0, 0, &dev->connector_id, 1, &dev->mode);
    if (ret) {
        printf("failed to set crtc\n");
        return;
    }

}

struct drm_device *drm_get_devices(int fd)
{
    struct drm_device *head = NULL;

    drmModeRes *resources;
    drmModeConnector *connector;
    drmModeEncoder *encoder;

    resources = drmModeGetResources(fd);
    if (!resources) {
        printf("Couldnt get resources\n");
        return NULL;
    }

    for (uint32_t i = 0; i < resources->count_connectors; ++i) {
        connector = drmModeGetConnector(fd, resources->connectors[i]);
        if (!connector || connector->connection != DRM_MODE_CONNECTED || connector->count_modes <= 0) {
            printf("Bad connector!\n");
            free(connector);
            continue;
        }

        struct drm_device *device = calloc(1, sizeof(struct drm_device));
        device->next = NULL;
        device->connector_id = connector->connector_id;
        device->encoder_id = connector->encoder_id;
        printf("encoder %i\n", device->encoder_id); 
        memcpy(&device->mode, &connector->modes[0], sizeof(drmModeModeInfo));

        device->width = connector->modes[0].hdisplay;
        device->height = connector->modes[0].vdisplay;

        printf("Found display device! w %i h %i\n", device->width, device->height);
        
        free(connector);
        encoder = drmModeGetEncoder(fd, device->encoder_id);
        if (!encoder) {
            printf("could not get encoder\n");
            free(encoder);
            continue;
        }
        device->crtc_id = encoder->crtc_id;
        free(encoder);

        device->crtc = NULL;

        device->next = head;
        head = device;
    }
    free(resources);

    return head;
}

void draw_rect(struct drm_device *dev, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color)
{
    uint32_t *fb = (uint32_t *)dev->fb;
    uint32_t pitch = dev->pitch / 4;
    for (uint32_t x = x1; x < x2; ++x) {
        for (uint32_t y = y1; y < y2; ++y) {
            fb[y * pitch + x] = color;
        }
    }
}

int drm_open(const char *path)
{
    int fd;

    // For some reason it will always fail to get the encoder on the first open of the device
    // so we open and close and open the device again
    fd = open(path, O_RDWR | O_CLOEXEC);
    close(fd);    
    fd = open(path, O_RDWR | O_CLOEXEC);

    uint64_t has_dumb_buffer;
    drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb_buffer);
    if (!has_dumb_buffer) {
        printf("We're not dumb folks!\n");
        return -1;
    }
    return fd;
}

int main(int argc, char **argv)
{
    int fd;
    fd = drm_open("/dev/dri/card0");
    if (fd < 0) {
        printf("Failed to open drm device\n");
        return -1;
    }

    struct drm_device *dev = drm_get_devices(fd);
    if (!dev) {
        printf("Failed to get DRM device\n");
        close(fd);
        return -1;
    }

    drm_init_fb(fd, dev);
    printf("pitch %i\n", dev->pitch);
    draw_rect(dev, 0, 0, 128, 128, 0xffffffff);
    while (1) {
        draw_rect(dev, 0, 0, 100, 100, 0);
        drmModeClip s = {.x1 = 0, .x2 = 128, .y1 = 0, .y2 = 128};
        drmModeDirtyFB(fd, dev->fb_id, &s, 1); 
    }
    return 0;

}
