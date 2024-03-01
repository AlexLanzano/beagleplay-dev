#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/spi/spi.h>

static void clear_display(struct spi_device *spi)
{
    for (uint8_t addr = 1; addr < 129; ++addr) {
        int ret;
        uint8_t buffer[19] = {0};
        buffer[0] = 0x1;
        buffer[1] = addr;
        memset(&buffer[2], 0xff, 16);
        printk(KERN_ERR "ALEX: writing...\n");
        ret = spi_write(spi, buffer, 19);
        if (ret) {
            printk(KERN_ERR "ALEX: failed to write\n");
        }
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
    
    clear_display(spi);

    return 0;
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
    .id_table = sharp_display_ids,
};
module_spi_driver(sharp_display_driver);

MODULE_AUTHOR("Alex Lanzano <lanzano.alex@gmail.com>");
MODULE_DESCRIPTION("SPI Protocol driver for the sharp display");
MODULE_LICENSE("GPL v2");
