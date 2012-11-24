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
