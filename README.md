libqpsd
=======

PSD (Photoshop Document) & PSB (Photoshop Big) Plugin for Qt/C++ (Qt4/Qt5)


This project is based on the Photoshop File Format Specification found in http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm

Format:
- [X] PSD
- [X] PSB
	
Compression:
- [X] Raw
- [X] RLE
- [ ] Zip without Prediction
- [ ] Zip with Prediction
	
Color Mode
- [X] Bitmap
- [X] Grayscale
- [X] Indexed
- [X] RGB
- [X] CMYK
- [X] Multichannel
- [X] Duotone
- [X] Lab
	
Depth
- [X] 1-bit (Bitmap)
- [X] 8-bits
- [X] 16-bits
- [ ] 32-bits (Tonemapping Algorithm, needed help!)
- *16-bit depth is scaled down to 8-bits*


CONTRIBUTE

- If you have an existing PSD/PSB file with different color mode or compression. Feel free to send me an [email](mailto:ronmarti18@gmail.com). I need samples with zip-compressed layers (zip w/ or w/o prediction).
- If you want to contribute to the code, just fork the project and pull requests.
- If you want to keep this project alive you can send your donations to:
	- Bitcoin: 3A23hHJF8q8hNPz3sedqD9T7g25ELPcvg2
	- Paypal: https://www.paypal.me/RonieMartinez

CONTRIBUTORS

- Ronie Martinez aka Code ReaQtor (ronmarti18@gmail.com)
- Yuezhao Huang (huangezhao@gmail.com)
- username "asgohtals" (http://qt-project.org/member/136052) - contributed PSB files for testing: 
	- berlin-cmyk.psb
	- berlin-quadtone.psb
	- wall-small-Lab.psb

LIST OF SOFTWARE USING libqpsd
- [nomacs](http://nomacs.org/)
- [EzViewer](https://github.com/yuezhao/ezviewer)
- [PhotoQt](http://photoqt.org/)
- [Seer](http://www.1218.io/)
- *for applications not included in the list, shoot me an email*
