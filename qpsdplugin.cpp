/*
Copyright (c) 2012, Ronie Martinez
ronmarti18@gmail.com
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301  USA
*/

#include "qpsdplugin.h"
#include <QRgb>
#include <QColor>
#include <qmath.h>

qpsdPlugn::qpsdPlugn(QObject *parent) :
    QImageIOPlugin(parent)
{
}

qpsdPlugn::~qpsdPlugn()
{

}


QStringList qpsdPlugn::keys() const
{
    return QStringList() << "psd";//TODO: add PSB (Photoshop Big) support
}

QImageIOPlugin::Capabilities qpsdPlugn::capabilities(
    QIODevice *device, const QByteArray &format) const
{
    if (format == "psd")
        return Capabilities(CanRead);//TODO: add CanWrite support
    if (!(format.isEmpty() && device->isOpen()))
        return 0;

    Capabilities cap;
    if (device->isReadable() && qpsdHandler::canRead(device))
        cap |= CanRead;
    // if (device->isWritable())
    //     cap |= CanWrite;
    return cap;
}

QImageIOHandler *qpsdPlugn::create(
    QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new qpsdHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

//Q_EXPORT_STATIC_PLUGIN(qpsdPlugn)
Q_EXPORT_PLUGIN2(qpsd, qpsdPlugn)

qpsdHandler::qpsdHandler()
{

}

qpsdHandler::~qpsdHandler()
{

}

bool qpsdHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("psd");
        return true;
    }
    return false;
}

bool qpsdHandler::canRead(QIODevice *device)
{
    return device->peek(4) == QByteArray("\x38\x42\x50\x53"); //8BPS
}

bool qpsdHandler::read(QImage *image)
{
    QDataStream input(device());
    quint32 signature, height, width, colorModeDataLength, imageResourcesLength, layerAndMaskInformationLength;
    quint16 version, channels, depth, colorMode, compression;
    QByteArray colorData;
    input.setByteOrder(QDataStream::BigEndian);
    input >> signature >> version ;
    input.skipRawData(6);//reserved bytes should be 6-byte in size
    input >> channels >> height >> width >> depth >> colorMode;
    input >> colorModeDataLength;
    if (colorModeDataLength != 0) {
        quint8 byte;
        for(quint32 i=0; i<colorModeDataLength; ++i) {
            input >> byte;
            colorData.append(byte);
        }
    }

    input >> imageResourcesLength;
    input.skipRawData(imageResourcesLength);
    input >> layerAndMaskInformationLength;
    input.skipRawData(layerAndMaskInformationLength);
    input >> compression;

    if (input.status() != QDataStream::Ok || signature != 0x38425053 || version != 0x0001)
        return false;

    QByteArray decompressed;
    switch (compression) {
    case 0: /*RAW IMAGE DATA - UNIMPLEMENTED*/
        break;
    case 1: /*RLE COMPRESSED DATA*/
        // The RLE-compressed data is proceeded by a 2-byte data count for each row in the data,
        // which we're going to just skip.
        input.skipRawData(height*channels*2);

        quint8 byte,count;
        decompressed.clear();

        /*Code based on PackBits implementation which is primarily used by Photoshop for RLE encoding/decoding*/
        while (!input.atEnd()) {
            input >> byte;
            if (byte > 128) {
                count=256-byte;
                input >>  byte;
                for (quint8 i=0; i<=count; ++i) {
                    decompressed.append(byte);
                }
            } else if (byte < 128) {
                count = byte + 1;
                for(quint8 i=0; i<count; ++i) {
                    input >> byte;
                    decompressed.append(byte);
                }
            }
        }
        break;
    case 2:/*ZIP WITHOUT PREDICTION - UNIMPLEMENTED*/
        break;
    case 3:/*ZIP WITH PREDICTION - UNIMPLEMENTED*/
        break;
    }

    int totalBytes = width * height;

    //FIXME: find better alternative
    switch (colorMode) {
    case 0: //for bitmap
        if (decompressed.size() != (channels * totalBytes)/8)
            return false;
        break;
    default: //for non-bitmap
        if (decompressed.size() != channels * totalBytes)
            return false;

        break;
    }

    switch (colorMode) {
    case 0: /*BITMAP*/
        {
            QString head = QString("P4\n%1 %2\n").arg(width).arg(height);
            QByteArray buffer(head.toAscii());
            buffer.append(decompressed);
            QImage result = QImage::fromData(buffer);
            if (result.isNull())
                return false;
            else
                *image = result;
        }

        break;
    case 1: /*GRAYSCALE*/
        switch (depth) {
        case 8:
            switch (channels) {
            case 1:
                QImage result(width, height, QImage::Format_Indexed8);
                const int IndexCount = 256;
                for (int i = 0; i < IndexCount; ++i){
                    result.setColor(i, qRgb(i, i, i));
                }

                quint8 *data = (quint8*)decompressed.constData();
                for (quint32 i=0; i < height; ++i) {
                    for (quint32 j=0; j < width; ++j) {
                        result.setPixel(j,i, *data);
                        ++data;
                    }
                }

                *image = result;
                break;
            }
        }

        break;
    case 2: /*INDEXED*/
        switch (depth) {
        case 8:
            switch (channels) {
            case 1:
                QImage result(width, height, QImage::Format_Indexed8);
                int indexCount = colorData.size() / 3;
                Q_ASSERT(indexCount == 256);
                quint8 *red = (quint8*)colorData.constData();
                quint8 *green = red + indexCount;
                quint8 *blue = green + indexCount;
                for (int i=0; i < indexCount; ++i) {
                    /*
                     * reference https://github.com/OpenImageIO/oiio/blob/master/src/psd.imageio/psdinput.cpp
                     * function bool PSDInput::indexed_to_rgb (char *dst)
                     */
                    result.setColor(i, qRgb(*red, *green, *blue));
                    ++red; ++green; ++blue;
                }

                quint8 *data = (quint8*)decompressed.constData();
                for (quint32 i=0; i < height; ++i) {
                    for (quint32 j=0; j < width; ++j) {
                        result.setPixel(j,i,*data);
                        ++data;
                    }
                }
                *image=result;
                break;
            }
        }
        break;
    case 3: /*RGB*/
        switch (depth) {
        case 1:
            break;
        case 8:
            switch(channels) {
            case 1:
                break;
            case 3:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *red = (quint8*)decompressed.constData();
                quint8 *green = red + totalBytes;
                quint8 *blue = green + totalBytes;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = qRgb(*red, *green, *blue);
                        ++p; ++red; ++green; ++blue;
                    }
                }

                *image = result;
            }
                break;
            case 4:
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint8 *red = (quint8*)decompressed.constData();
                quint8 *green = red + totalBytes;
                quint8 *blue = green + totalBytes;
                quint8 *alpha = blue + totalBytes;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = qRgba(*red, *green, *blue, *alpha);
                        ++p; ++red; ++green; ++blue; ++alpha;
                    }
                }

                *image = result;
            }
                break;
            case 5:
                qDebug("5 channels of rgb mode");
                return false;
            }

            break;
        case 16:
            break;
        }
        break;
    case 4: /*CMYK*/
        switch (depth) {
        case 8:
            switch (channels) {
            case 4:
            {
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *cyan = (quint8*)decompressed.constData();
                quint8 *magenta = cyan + totalBytes;
                quint8 *yellow = magenta + totalBytes;
                quint8 *key = yellow + totalBytes;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = QColor::fromCmyk(255-*cyan, 255-*magenta,
                                              255-*yellow, 255-*key).rgb();
                        ++p; ++cyan; ++magenta; ++yellow; ++key;
                    }
                }
                *image = result;
            }
                break;
            case 5:
            {
                QImage result(width, height, QImage::Format_ARGB32);
                quint8 *alpha = (quint8*)decompressed.constData();
                quint8 *cyan = alpha + totalBytes;
                quint8 *magenta = cyan + totalBytes;
                quint8 *yellow = magenta + totalBytes;
                quint8 *key = yellow + totalBytes;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        *p = QColor::fromCmyk(255-*cyan, 255-*magenta,
                                              255-*yellow, 255-*key,
                                              *alpha).rgba();
                        ++p; ++alpha; ++cyan; ++magenta; ++yellow; ++key;
                    }
                }
                *image = result;
            }
                break;
            }
        }
        break;
    case 7: /*MULTICHANNEL - UNIMPLEMENTED*/
        return 0;
        break;
    case 8: /*DUOTONE*/
        switch (depth) {
        case 8:
            switch (channels) {
            case 1:
                /*
                 *Duotone images: color data contains the duotone specification
                 *(the format of which is not documented). Other applications that
                 *read Photoshop files can treat a duotone image as a gray image,
                 *and just preserve the contents of the duotone information when
                 *reading and writing the file.
                 *
                 *TODO: find a way to actually get the duotone, tritone, and quadtone colors
                 *Noticed the added "Curve" layer when using photoshop
                 */
                QImage result(width, height, QImage::Format_Indexed8);
                const int IndexCount = 256;
                for(int i = 0; i < IndexCount; ++i){
                    result.setColor(i, qRgb(i, i, i));
                }
                quint8 *data = (quint8*)decompressed.constData();
                for(quint32 i=0; i < height; ++i)
                {
                    for(quint32 j=0; j < width; ++j)
                    {
                        result.setPixel(j,i, *data);
                        ++data;
                    }
                }
                *image = result;
                break;
            }

            break;
        }
        break;
    case 9: /*LAB - UNDER TESTING*/
        switch (depth) {
        case 8:
            switch (channels) {
            case 3:
                //FIXME: something is wrong with the computation
                //overflow?
                QImage result(width, height, QImage::Format_RGB32);
                quint8 *L = (quint8*)decompressed.constData();
                quint8 *a = L + totalBytes;
                quint8 *b = a + totalBytes;

                qreal var_X, var_Y, var_Z, var_R, var_G, var_B, X, Y, Z;
                qreal red, green, blue;
                QRgb  *p, *end;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        //coversion from Lab to xyz
                        //http://www.easyrgb.com/index.php?X=MATH&H=08#text8
                        var_Y = (*L + 16) / 116;
                        var_X = *a / 500 + var_Y;
                        var_Z = var_Y - *b / 200;
                        if ( qPow(var_Y,3) > 0.008856 ) {
                            var_Y =  qPow(var_Y, 3);
                        } else {
                            var_Y = ( var_Y - 16 / 116 ) / 7.787;
                        }
                        if (  qPow(var_X, 3) > 0.008856 ) {
                            var_X =  qPow(var_X, 3);
                        } else {
                            var_X = ( var_X - 16 / 116 ) / 7.787;
                        }
                        if (  qPow(var_Z, 3) > 0.008856 ) {
                            var_Z =  qPow(var_Z, 3);
                        } else {
                            var_Z = ( var_Z - 16 / 116 ) / 7.787;
                        }
                        X = 95.047 * var_X;
                        Y = 100.000 * var_Y;
                        Z = 108.883 * var_Z;

                        //conversion from xyz to rgb
                        //http://www.easyrgb.com/index.php?X=MATH&H=01#text1
                        var_X = X / 100;        //X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
                        var_Y = Y / 100;        //Y from 0 to 100.000
                        var_Z = Z / 100;        //Z from 0 to 108.883
                        var_R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
                        var_G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415;
                        var_B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570;
                        if (  var_R > 0.0031308) {
                            var_R = 1.055 * ( qPow( var_R, 1 / 2.4) ) - 0.055;
                        } else {
                            var_R = 12.92 * (var_R);
                        }
                        if ( var_G > 0.0031308 ) {
                            var_G = 1.055 * ( qPow(var_G, 1 / 2.4 ) ) - 0.055;
                        } else {
                            var_G = 12.92 * (var_G);
                        }
                        if ( var_B > 0.0031308 ) {
                            var_B = 1.055 * ( qPow(var_B , 1 / 2.4 ) ) - 0.055;
                        } else {
                            var_B = 12.92 * (var_B);
                        }

                        red = var_R * 255;
                        green = var_G * 255;
                        blue = var_B * 255;

                        *p = qRgb(red, green, blue);
                        ++p; ++L; ++a; ++b;
                    }

                }
                *image = result;
                break;
            }
            break;
        }
        break;
    }

    return input.status() == QDataStream::Ok;
}

bool qpsdHandler::supportsOption(ImageOption option) const
{
    return option == Size;
}

QVariant qpsdHandler::option(ImageOption option) const
{
    if (option == Size) {
        QByteArray bytes = device()->peek(26);
        QDataStream input(bytes);
        quint32 signature, height, width;
        quint16 version, channels, depth, colorMode;
        input.setByteOrder(QDataStream::BigEndian);
        input >> signature >> version ;
        input.skipRawData(6);//reserved bytes should be 6-byte in size
        input >> channels >> height >> width >> depth >> colorMode;
        if (input.status() == QDataStream::Ok && signature == 0x38425053 && version == 0x0001)
            return QSize(width, height);
    }
    return QVariant();
}
