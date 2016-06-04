//
//  DataServerAccountInfo.cpp
//  libraries/networking/src
//
//  Created by Stephen Birarda on 2/18/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <openssl/rsa.h>
#include <openssl/x509.h>

#include <qjsondocument.h>
#include <QtCore/QDebug>
#include <QtCore/QDataStream>

#include <UUID.h>

#include "NetworkLogging.h"
#include "DataServerAccountInfo.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

DataServerAccountInfo::DataServerAccountInfo(const DataServerAccountInfo& otherInfo) : QObject() {
    _accessToken = otherInfo._accessToken;
    _username = otherInfo._username;
    _xmppPassword = otherInfo._xmppPassword;
    _discourseApiKey = otherInfo._discourseApiKey;
    _walletID = otherInfo._walletID;
    _privateKey = otherInfo._privateKey;
    _domainID = otherInfo._domainID;
}

DataServerAccountInfo& DataServerAccountInfo::operator=(const DataServerAccountInfo& otherInfo) {
    DataServerAccountInfo temp(otherInfo);
    swap(temp);
    return *this;
}

void DataServerAccountInfo::swap(DataServerAccountInfo& otherInfo) {
    using std::swap;

    swap(_accessToken, otherInfo._accessToken);
    swap(_username, otherInfo._username);
    swap(_xmppPassword, otherInfo._xmppPassword);
    swap(_discourseApiKey, otherInfo._discourseApiKey);
    swap(_walletID, otherInfo._walletID);
    swap(_privateKey, otherInfo._privateKey);
    swap(_domainID, otherInfo._domainID);
}

void DataServerAccountInfo::setAccessTokenFromJSON(const QJsonObject& jsonObject) {
    _accessToken = OAuthAccessToken(jsonObject);
}

void DataServerAccountInfo::setUsername(const QString& username) {
    if (_username != username) {
        _username = username;
        
        qCDebug(networking) << "Username changed to" << username;
    }
}

void DataServerAccountInfo::setRole(const QString& role) {
    if (_role != role) {
        _role = role;

        qCDebug(networking) << "Role changed to" << role;
    }
}

void DataServerAccountInfo::setXMPPPassword(const QString& xmppPassword) {
     if (_xmppPassword != xmppPassword) {
         _xmppPassword = xmppPassword;
     }
}

void DataServerAccountInfo::setDiscourseApiKey(const QString& discourseApiKey) {
    if (_discourseApiKey != discourseApiKey) {
        _discourseApiKey = discourseApiKey;
    }
}

void DataServerAccountInfo::setWalletID(const QUuid& walletID) {
    if (_walletID != walletID) {
        _walletID = walletID;
    }
}

bool DataServerAccountInfo::hasProfile() const {
    return _username.length() > 0;
}

void DataServerAccountInfo::setProfileInfoFromJSON(const QJsonObject& jsonObject) {
    QJsonObject user = jsonObject["data"].toObject()["user"].toObject();
    setUsername(user["username"].toString());
    setRole(user["role"].toString());
    //UTII We dont need this:
    //setXMPPPassword(user["xmpp_password"].toString());
    //setDiscourseApiKey(user["discourse_api_key"].toString());
    //setWalletID(QUuid(user["wallet_id"].toString()));
}

QByteArray DataServerAccountInfo::getUsernameSignature(const QUuid& connectionToken) {
    auto lowercaseUsername = _username.toLower().toUtf8();
    auto plaintext = lowercaseUsername.append(connectionToken.toRfc4122());

    auto signature = signPlaintext(plaintext);
    if (!signature.isEmpty()) {
        qDebug(networking) << "Returning username" << _username
            << "signed with connection UUID" << uuidStringWithoutCurlyBraces(connectionToken);
    } else {
        qCDebug(networking) << "Error signing username with connection token";
        qCDebug(networking) << "Will re-attempt on next domain-server check in.";
    }

    return signature;
}

QByteArray DataServerAccountInfo::signPlaintext(const QByteArray& plaintext) {
    if (!_privateKey.isEmpty()) {
        const char* privateKeyData = _privateKey.constData();
        RSA* rsaPrivateKey = d2i_RSAPrivateKey(NULL,
                                               reinterpret_cast<const unsigned char**>(&privateKeyData),
                                               _privateKey.size());
        if (rsaPrivateKey) {
            QByteArray signature(RSA_size(rsaPrivateKey), 0);
            unsigned int signatureBytes = 0;

            QByteArray hashedPlaintext = QCryptographicHash::hash(plaintext, QCryptographicHash::Sha256);

            int encryptReturn = RSA_sign(NID_sha256,
                                         reinterpret_cast<const unsigned char*>(hashedPlaintext.constData()),
                                         hashedPlaintext.size(),
                                         reinterpret_cast<unsigned char*>(signature.data()),
                                         &signatureBytes,
                                         rsaPrivateKey);

            // free the private key RSA struct now that we are done with it
            RSA_free(rsaPrivateKey);

            if (encryptReturn != -1) {
                return signature;
            }
        } else {
            qCDebug(networking) << "Could not create RSA struct from QByteArray private key.";
        }
    }
    return QByteArray();
}

QDataStream& operator<<(QDataStream &out, const DataServerAccountInfo& info) {
    out << info._accessToken << info._username << info._xmppPassword << info._discourseApiKey
        << info._walletID << info._privateKey << info._domainID;

    return out;
}

QDataStream& operator>>(QDataStream &in, DataServerAccountInfo& info) {
    in >> info._accessToken >> info._username >> info._xmppPassword >> info._discourseApiKey
        >> info._walletID >> info._privateKey >> info._domainID;
    return in;
}
