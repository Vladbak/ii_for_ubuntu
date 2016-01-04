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
    _loadedResources = false;
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

void LoadingScreen::checkDownloadProgress() {
    float progress = 0.0f;
    if (_downloading) {
        foreach(Resource* resource, ResourceCache::getLoadingRequests()) {
            _foundRequest = true;
            progress += resource->getProgress();
        }
        int totalCount = ResourceCache::getPendingRequestCount() + ResourceCache::getLoadingRequests().count() + _loadedResources;
        _percentage = (_loadedResources + progress) / totalCount;
        
    }
    
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
