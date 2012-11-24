#include "qpsdplugin.h"
#include <QRgb>

qpsdPlugn::qpsdPlugn(QObject *parent) :
    QImageIOPlugin(parent)
{
}

qpsdPlugn::~qpsdPlugn()
{

}


QStringList qpsdPlugn::keys() const
{
    return QStringList() << "psd";//TO-DO: add PSB (Photoshop Big)
}

QImageIOPlugin::Capabilities qpsdPlugn::capabilities(
    QIODevice *device, const QByteArray &format) const
{
    if (format == "psd")
        return Capabilities(CanRead);//TO-DO: add CanWrite
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
    //reserved bytes should be 6-byte in size
    //there are no standard 6-byte datatype
    quint32 reserved1;  quint16 reserved2;
    input.setByteOrder(QDataStream::BigEndian);
    input >> signature >> version >> reserved1 >>reserved2 >> channels >> height >> width >> depth >> colorMode;
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
            case 3:
                quint8 red,green,blue;
                QDataStream planar(decompressed);

                int totalBytes = width*height; quint8 byte;
                QByteArray channel1, channel2, channel3, data;
                for(int j=0; j<3; j++)
                {
                    for(int i=0; i <totalBytes ; i++)
                    {
                        planar >> byte;
                        switch(j)
                        {
                        case 0: channel1.append(byte);
                            break;
                        case 1: channel2.append(byte);
                            break;
                        case 2: channel3.append(byte);
                            break;
                        }

                    }
                }
                for( int i=0; i <totalBytes ; i++)
                {
                    data.append(channel1.at(i));
                    data.append(channel2.at(i));
                    data.append(channel3.at(i));
                }
                QDataStream imageData(data);
                QImage result(width,height,QImage::Format_RGB32);

                for(quint32 i=0;i < height;i++)
                {
                    for(quint32 j=0;j<width;j++)
                    {
                        QRgb value;
                        imageData >> red >> green >> blue;
                        value = qRgb(red, green, blue);
                        result.setPixel(j,i,value);
                    }
                }
                if (imageData.status() == QDataStream::Ok)
                    *image = result;
                break;
            }

            break;
        case 16:
            break;
        case 32:
            break;
        }
        break;
    case 4: /*CMYK - UNIMPLEMENTED*/
        break;
    case 7: /*MULTICHANNEL - UNIMPLEMENTED*/
        break;
    case 8: /*DUOTONE - UNIMPLEMENTED*/
        break;
    case 9: /*LAB - UNIMPLEMENTED*/
        break;
    }

    /*
    QImage result(width,height,QImage::Format_RGB32);
    for(quint32 i=0;i<height;i++)
    {
        for(quint32 j=0;j<width;j++)
        {
            QRgb value;
            value = qRgb(122, 163, 39); // 0xff7aa327
            result.setPixel(j,i,value);
        }
    }*/
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
        //reserved bytes should be 6-byte in size
        //there are no standard 6-byte datatype
        quint32 reserved1;  quint16 reserved2;
        input.setByteOrder(QDataStream::BigEndian);
        input >> signature >> version >> reserved1 >>reserved2 >> channels >> height >> width >> depth >> colorMode;
        if (input.status() == QDataStream::Ok && signature == 0x38425053 && version == 0x0001)
            return QSize(width, height);
    }
    return QVariant();
}
