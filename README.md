# libqpsd

PSD (Photoshop Document) & PSB (Photoshop Big) Plugin for Qt/C++ (Qt4/Qt5)

This project is based on the [Adobe Photoshop File Formats Specification](http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm)

## Support
If you like `libqpsd` or if it is useful to you, show your support by sponsoring my projects.

[![Github Sponsors](https://img.shields.io/github/sponsors/roniemartinez?label=github%20sponsors&logo=github%20sponsors&style=for-the-badge)](https://github.com/sponsors/roniemartinez)

## FEATURES

### Supported Formats

- [X] PSD
- [X] PSB

### Supported Compressions

- [X] Raw
- [X] RLE
- [ ] Zip without Prediction
- [ ] Zip with Prediction

### Supported Color Modes

- [X] Bitmap
- [X] Grayscale
- [X] Indexed
- [X] RGB
- [X] CMYK
- [X] Multichannel
- [X] Duotone
- [X] Lab

### Supported Depths

- [X] 1-bit (Bitmap)
- [X] 8-bits
- [X] 16-bits (Note: 16-bit depth is scaled down to 8-bits)
- [ ] 32-bits (Tonemapping Algorithm, needed help!)


## CONTRIBUTE

- If you have an existing PSD/PSB file with different color mode or compression. Feel free to send me an [email](mailto:ronmarti18@gmail.com). I need samples with zip-compressed layers (zip w/ or w/o prediction).
- If you want to contribute to the code, just fork the project and pull requests.

## AUTHOR

- [Ronie Martinez](ronmarti18@gmail.com)

## CONTRIBUTORS

- [Yuezhao Huang](https://github.com/yuezhao)
- [Markus Diem](https://github.com/diemmarkus)
- [stepanp](https://github.com/stepanp)
- [Eli Schwartz](https://github.com/eli-schwartz)
- username [asgohtals](http://qt-project.org/member/136052) contributed PSB files for testing: 
    - berlin-cmyk.psb
    - berlin-quadtone.psb
    - wall-small-Lab.psb

## LIST OF SOFTWARE USING libqpsd

- [nomacs](http://nomacs.org/)
- [EzViewer](https://github.com/yuezhao/ezviewer)
- [PhotoQt](http://photoqt.org/)
- [Seer](http://www.1218.io/)
- *for applications not included in the list, shoot me an email*

## REFERENCES

- [Adobe Photoshop File Formats Specification](http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm)
- [RGB/XYZ Matrices](http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html)
- [ADOBE® PHOTOSHOP Help and tutorials](http://help.adobe.com/en_US/photoshop/cs/using/WSfd1234e1c4b69f30ea53e41001031ab64-73eea.html#WSfd1234e1c4b69f30ea53e41001031ab64-73e5a)
- [Adobe® RGB (1998) Color Image Encoding](http://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf)
