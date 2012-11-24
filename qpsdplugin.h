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

#ifndef QPSDPLUGIN_H
#define QPSDPLUGIN_H

#include <QtGui/QImageIOPlugin>
#include <QtGui/QImageIOHandler>
#include <QtGui/QImage>
#include <QtCore/QVariant>
#include <QByteArray>


class qpsdPlugn : public QImageIOPlugin {
    Q_OBJECT
public:
    qpsdPlugn(QObject *parent = 0);
    ~qpsdPlugn();
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device,
                    const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device,
                               const QByteArray &format = QByteArray()) const;
};

class qpsdHandler : public QImageIOHandler
{
public:
    qpsdHandler();
    ~qpsdHandler();

    bool canRead() const;
    bool read(QImage *image);
    //bool write(const QImage &image);

    //QByteArray name() const;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const;
    //void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;
};

#endif // QPSDPLUGIN_H
