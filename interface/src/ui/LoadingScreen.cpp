//
//  LoadingScreen.cpp
//  interface/src/ui
//
//  Created by Thijs Wenker on 2016/01/02
//  E-SPACES  
//

#include "LoadingScreen.h"

#include <QDesktopServices>

#include "DependencyManager.h"

HIFI_QML_DEF(LoadingScreen)

LoadingScreen::LoadingScreen(QQuickItem *parent) : QQuickItem(parent)
{
    _percentage = 0.0f;
    _downloading = false;
    _checkDownloadTimer = new QTimer(this);
    connect(_checkDownloadTimer, &QTimer::timeout, this, &LoadingScreen::checkDownloadProgress);
    const int CHECK_DOWNLOAD_INTERVAL = MSECS_PER_SECOND / 2;
    _checkDownloadTimer->start(CHECK_DOWNLOAD_INTERVAL);

    auto nodeList = DependencyManager::get<NodeList>();
    connect(nodeList.data(), &NodeList::nodeAdded, this, &LoadingScreen::nodeAdded);
}

void LoadingScreen::nodeAdded(SharedNodePointer node) {
    if (node->getType() == NodeType::EntityServer) {
        _downloading = true;
    }
}

int LoadingScreen::getCompletedDownloads() {
    int result = 0;
    foreach(Resource* resource, _loadedResources) {
        if (resource->isLoaded()) {
            result++;
        }
    }
    return result;
}

void LoadingScreen::checkDownloadProgress() {
    float progress = 0.0f;
    if (_downloading && OctreeElement::getNodeCount() > 0) {
        QSharedPointer<TextureCache> modelCacheP = DependencyManager::get<TextureCache>();
        ResourceCache* modelCache = modelCacheP.data();
        int totalCount = 0;
        int loadedCount = 0;
        foreach (QSharedPointer<Resource> resource, modelCache->getAllResources()) {
            if (resource.isNull()) {
                continue;
            }
            totalCount++;
            Resource* myResource = resource.data();
            if (myResource->isLoaded()) {
                loadedCount++;
            }
        }
        float newPercentage = ((float)loadedCount / (float)totalCount);
        if (fabs(newPercentage - _percentage) >= 0.01f) {
            _percentage = newPercentage;
            emit percentageChanged();
        }
    }

    emit percentageChanged();
    if (_foundRequest && _percentage > 0.99f) {
        _checkDownloadTimer->stop();
        setParent(NULL);
        setParentItem(NULL);
        deleteLater();
    }
}

float LoadingScreen::getPercentage() const {
    return _percentage;
}
