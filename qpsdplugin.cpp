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
    input.setByteOrder(QDataStream::BigEndian);
    input >> signature >> version ;
    input.skipRawData(6);//reserved bytes should be 6-byte in size
    input >> channels >> height >> width >> depth >> colorMode;
    input >> colorModeDataLength;
    input.skipRawData(colorModeDataLength);
    input >> imageResourcesLength;
    input.skipRawData(imageResourcesLength);
    input >> layerAndMaskInformationLength;
    input.skipRawData(layerAndMaskInformationLength);
    input >> compression;

    if (input.status() != QDataStream::Ok || signature != 0x38425053 || version != 0x0001)
        return false;
    QByteArray decompressed;
    switch(compression)
    {
    case 0: /*RAW IMAGE DATA - UNIMPLEMENTED*/
        break;
    case 1: /*RLE COMPRESSED DATA*/
        // The RLE-compressed data is proceeded by a 2-byte data count for each row in the data,
        // which we're going to just skip.
        input.skipRawData(height*channels*2);

        quint8 byte,count;
        decompressed.clear();

        /*Code based on PackBits implementation which is primarily used by Photoshop for RLE encoding/decoding*/
        while(!input.atEnd())
        {
            input >> byte;
            if(byte > 128)
            {
                count=256-byte;
                input >>  byte;
                for(quint8 i=0;i<=count;i++)
                {
                    decompressed.append(byte);

                }
            }
            else if(byte < 128)
            {
                count=byte+1;
                for(quint8 i=0;i<count;i++)
                {
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
    if(decompressed.size() != channels * totalBytes) //used "channels" instead of only "3"
        return false;
        
    switch(colorMode)
    {
    case 0: /*BITMAP - UNIMPLEMENTED*/
        break;
    case 1: /*GRAYSCALE - UNIMPLEMENTED*/
        break;
    case 2: /*INDEXED - UNIMPLEMENTED*/
        break;
    case 3: /*RGB*/
        switch(depth)
        {
        case 1:
            break;
        case 8:
            switch(channels)
            {
            case 1:
                break;
            case 3:
                {
                    QImage result(width, height, QImage::Format_RGB32);
                    const char *data = decompressed.constData();
                    QRgb  *p, *end;
                    for (quint32 y = 0; y < height; ++y) {
                        p = (QRgb *)result.scanLine(y);
                        end = p + width;
                        while (p < end) {
                            *p++ = qRgb(data[0], data[totalBytes], data[totalBytes + totalBytes]);
                            ++data;
                        }
                    }

                    *image = result; 
                }
                break;
            case 4:
                {
                    QImage result(width, height, QImage::Format_ARGB32);
                    const char *data = decompressed.constData();
                    QRgb  *p, *end;
                    for (quint32 y = 0; y < height; ++y) {
                        p = (QRgb *)result.scanLine(y);
                        end = p + width;
                        while (p < end) {
                            *p++ = qRgba(data[0], data[totalBytes],
                                     data[totalBytes + totalBytes],
                                     data[totalBytes + totalBytes + totalBytes]);
                            ++data;
                        }
                    }

                    *image = result;
                }
                break;
            }

            break;
        case 16:
            break;
        }
        break;
    case 4: /*CMYK*/
        switch(depth)
        {
        case 8:
            switch(channels)
            {
            case 4:
                QImage result(width, height, QImage::Format_RGB32);
                const char *data = decompressed.constData();
                QRgb  *p, *end;
                quint8 cyan, magenta, yellow, key;
                for (quint32 y = 0; y < height; ++y) {
                    p = (QRgb *)result.scanLine(y);
                    end = p + width;
                    while (p < end) {
                        cyan = data[0];
                        magenta = data[totalBytes];
                        yellow = data[totalBytes + totalBytes];
                        key = data[totalBytes + totalBytes + totalBytes];
                        *p++ = QColor::fromCmyk(255-cyan, 255-magenta,255-yellow, 255-key).rgb();
                        ++data;
                    }
                }
                *image = result;
                break;
            }
        }
        break;
    case 7: /*MULTICHANNEL - UNIMPLEMENTED*/
        break;
    case 8: /*DUOTONE - UNIMPLEMENTED*/
        break;
    case 9: /*LAB - UNIMPLEMENTED*/
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
