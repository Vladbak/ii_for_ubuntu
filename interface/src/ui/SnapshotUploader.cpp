//
//  SnapshotUploader.cpp
//  interface/src/ui
//
//  Created by Howard Stearns on 8/22/2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <AddressManager.h>
#include "scripting/WindowScriptingInterface.h"
#include "SnapshotUploader.h"

void SnapshotUploader::uploadSuccess(QNetworkReply& reply) {
    const QString STORY_UPLOAD_URL = "/api/v1/user_stories";
    static SnapshotUploader uploader;

    // parse the reply for the thumbnail_url
    QByteArray contents = reply.readAll();
    QJsonParseError jsonError;
    auto doc = QJsonDocument::fromJson(contents, &jsonError);
    if (jsonError.error == QJsonParseError::NoError) {
        auto dataObject = doc.object().value("data").toObject();
        QString thumbnailUrl = dataObject.value("thumbnail_url").toString();
        QString imageUrl = dataObject.value("image_url").toString();
        auto addressManager = DependencyManager::get<AddressManager>();
        QString placeName = addressManager->getPlaceName();
        if (placeName.isEmpty()) {
            placeName = addressManager->getHost();
        }
        QString currentPath = addressManager->currentPath(true);

        // create json post data
        QJsonObject rootObject;
        QJsonObject userStoryObject;
        QJsonObject detailsObject;
        detailsObject.insert("image_url", imageUrl);
        QString pickledDetails = QJsonDocument(detailsObject).toJson();
        userStoryObject.insert("details", pickledDetails);
        userStoryObject.insert("thumbnail_url", thumbnailUrl);
        userStoryObject.insert("place_name", placeName);
        userStoryObject.insert("path", currentPath);
        userStoryObject.insert("action", "snapshot");
        rootObject.insert("user_story", userStoryObject);

        auto accountManager = DependencyManager::get<AccountManager>();
        JSONCallbackParameters callbackParams(&uploader, "createStorySuccess", &uploader, "createStoryFailure");

        accountManager->sendRequest(STORY_UPLOAD_URL,
            AccountManagerAuth::Required,
            QNetworkAccessManager::PostOperation,
            callbackParams,
            QJsonDocument(rootObject).toJson());

    }
    else {
        emit DependencyManager::get<WindowScriptingInterface>()->snapshotShared(contents);
    }
}

void SnapshotUploader::uploadFailure(QNetworkReply& reply) {
    emit DependencyManager::get<WindowScriptingInterface>()->snapshotShared(reply.readAll());
}

void SnapshotUploader::createStorySuccess(QNetworkReply& reply) {
    emit DependencyManager::get<WindowScriptingInterface>()->snapshotShared(QString());
}

void SnapshotUploader::createStoryFailure(QNetworkReply& reply) {
    emit DependencyManager::get<WindowScriptingInterface>()->snapshotShared(reply.readAll());
}