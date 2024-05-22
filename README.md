# Beagleplay-dev
This repo contains an OpenEmbedded build environment tailored to the Beagleplay development board.
I use the open source meta-ti, meta-arm, and openembedded-core layers. I have provided a custom meta-beagleplay that builds
upon the existing layers.

## How to build
First clone the project and all its submodules by doing the following:
```
git clone https://github.com/AlexLanzano/beagleplay-dev --recurse-submodules
```

Now you can kick off the build by doing:
```
source oe-init-env
bitbake core-image-minimal
```

## How to flash the sd card
After the build is done, insert your sd card and run the following:
```
make update-sdcard DEV=<path to your sd card device>
```
Ignore any errors that get spit out from this. Trust me.

## How to boot the image
Insert your sd card into your Beagleplay board. Hold down the USR button, 
then press and release the RESET button. This will signal the Beagleplay board
to boot from the sdcard.
