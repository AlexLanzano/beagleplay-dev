#include "linux/printk.h"
#include <linux/i2c.h>
#include <linux/iio/iio.h>
#include <linux/module.h>
#include <linux/regmap.h>

enum bmi270_registers {
    BMI270_REG_CHIP_ID = 0x00,
};

static struct regmap_config bmi270_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,
};

struct bmi270_device {
    struct regmap *regmap;
};

static const struct iio_info bmi270_info[] = {

};

static const struct iio_chan_spec bmi270_channels[] = {

};

static int bmi270_setup(struct bmi270_device *bmi270_device)
{
    int chip_id;
    regmap_read(bmi270_device->regmap, BMI270_REG_CHIP_ID, &chip_id);
    if (chip_id != 0x24) {
        printk(KERN_ERR "Failed to read chip id\n");
    } else {
        printk(KERN_INFO "successfully read the chip id\n");
    }
    return 0;
}

static int bmi270_i2c_probe(struct i2c_client *client)
{
    const struct i2c_device_id *id = i2c_client_get_device_id(client);
    struct regmap *regmap;
    const char *name;

    regmap = devm_regmap_init_i2c(client, &bmi270_regmap_config);
    if (IS_ERR(regmap)) {
        printk(KERN_ERR "Failed to init i2c\n");
        return PTR_ERR(regmap);
    }

    if (id) {
        name = id->name;
    } else {
        name = dev_name(&client->dev);
    }


    struct iio_dev *indio_dev;
    indio_dev = devm_iio_device_alloc(&client->dev, sizeof(struct bmi270_device *));
    if (!indio_dev) {
        return -ENOMEM;
    }
    
    struct bmi270_device *bmi270_device = iio_priv(indio_dev);
    bmi270_device->regmap = regmap;
    i2c_set_clientdata(client, indio_dev);
  
    bmi270_setup(bmi270_device);

    indio_dev->channels = bmi270_channels;
    indio_dev->num_channels = ARRAY_SIZE(bmi270_channels);
    indio_dev->name = name;
    indio_dev->modes = INDIO_DIRECT_MODE;
    indio_dev->info = bmi270_info;
    
    return devm_iio_device_register(&client->dev, indio_dev);

    return 0;
}

static const struct i2c_device_id bmi270_i2c_id[] = {
    {"bmi270", 0},
    {}
};

static const struct of_device_id bmi270_of_match[] = {
    {.compatible = "bosch,bmi270"},
    {},
};

static struct i2c_driver bmi270_i2c_driver = {
    .driver = {
        .name = "bmi270_i2c",
        .of_match_table = bmi270_of_match,
    },
    .probe = bmi270_i2c_probe,
    .id_table = bmi270_i2c_id,
};
module_i2c_driver(bmi270_i2c_driver);

MODULE_AUTHOR("Alex Lanzano");
MODULE_DESCRIPTION("BMI270 driver");
MODULE_LICENSE("GPL v2");

