#include <QtWebKit>
#include <QDebug>
#include <QRegExp>

#include "interceptormanager.h"
#include "networkreplyproxy.h"

class InterceptorAccessManagerPrivate
{
public:
    InterceptorAccessManagerPrivate()
    {
        urlRegExp.setPatternSyntax(QRegExp::Wildcard);
        urlRegExp.setPattern("*");

        dataRegExp.setPatternSyntax(QRegExp::Wildcard);
        dataRegExp.setPattern("");
    }

    ~InterceptorAccessManagerPrivate() {}

    QRegExp urlRegExp;
    QRegExp dataRegExp;
};

InterceptorAccessManager::InterceptorAccessManager(QObject *parent) :
    QNetworkAccessManager(parent),
    d_ptr(new InterceptorAccessManagerPrivate)
{
}

InterceptorAccessManager::~InterceptorAccessManager()
{
    delete d_ptr;
}

QRegExp InterceptorAccessManager::urlMask() const
{
    Q_D(const InterceptorAccessManager);
    return d->urlRegExp;
}

void InterceptorAccessManager::setUrlMask(const QRegExp &mask)
{
    Q_D(InterceptorAccessManager);
    d->urlRegExp = mask;
}

QRegExp InterceptorAccessManager::dataMask() const
{
    Q_D(const InterceptorAccessManager);
    return d->dataRegExp;
}

void InterceptorAccessManager::setDataMask(const QRegExp &mask)
{
    Q_D(InterceptorAccessManager);
    d->dataRegExp = mask;

}

bool InterceptorAccessManager::isResourceNeeded(const QNetworkRequest &request, const QByteArray &data)
{
    Q_D(InterceptorAccessManager);

    if (d->urlRegExp.exactMatch(request.url().toString()))
        return true;

    if (!data.isEmpty() && d->dataRegExp.exactMatch(QString(data)))
        return true;

    return false;
}

QNetworkReply *InterceptorAccessManager::createRequest(Operation op, const QNetworkRequest &request,
                                                       QIODevice *outgoingData)
{
    QNetworkRequest req(request);
//    req.setRawHeader("Connection", "close");

    QByteArray iod = QByteArray("");
    QByteArray copyIod = QByteArray("");
    QBuffer *buf = new QBuffer(this);
    if (outgoingData)
    {
        iod = outgoingData->readAll();
        copyIod.replace(0, iod.size(), iod);
        buf->setData(iod);
        buf->open(QIODevice::ReadOnly);

    }
//    qDebug() << "POST: " << iod;

//    QNetworkReply *real = QNetworkAccessManager::createRequest(op, request, outgoingData);
    QNetworkReply *real = QNetworkAccessManager::createRequest(op, req, buf);
    NetworkReplyProxy *proxy = new NetworkReplyProxy(this, real);

    if (isResourceNeeded(request, copyIod)) {

        emit pendingReplyProxy(proxy);
        emit requestCatched(request, copyIod);

    }

    return proxy;
}
