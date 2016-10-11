//
//  AtpReply.h
//  libraries/networking/src
//
//  Created by Zander Otavka on 8/4/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AtpReply_h
#define hifi_AtpReply_h

#include <QtNetwork/QNetworkReply>
#include <QUrl>

#include "AssetResourceRequest.h"

class AtpReply : public QNetworkReply {
    Q_OBJECT
public:
    AtpReply(const QUrl& url, QObject* parent = Q_NULLPTR);
    ~AtpReply();
    qint64 bytesAvailable() const override;
    void abort() override { }
    bool isSequential() const override { return true; }

protected:
    qint64 readData(char* data, qint64 maxSize) override;

private:
    void handleRequestFinish();

    ResourceRequest* _resourceRequest { nullptr };
    QByteArray _content;
    qint64 _readOffset { 0 };
};

#endif // hifi_AtpReply_h
