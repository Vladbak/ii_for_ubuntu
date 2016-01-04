//
//  LoadingScreen.h
//  interface/src/ui
//
//  Created by Thijs Wenker on 2016/01/02
//  E-SPACES  
//

#pragma once

#ifndef hifi_LoadingScreen_h
#define hifi_LoadingScreen_h

#include <OffscreenQmlDialog.h>
#include "Application.h"

class LoadingScreen : public QQuickItem {
    Q_OBJECT
    HIFI_QML_DECL

    Q_PROPERTY(float percentage READ getPercentage)

public:
    LoadingScreen(QQuickItem* parent = nullptr);

    float getPercentage() const;

private:
    bool _downloading;
    bool _foundRequest;
    int _loadedResources;
    float _percentage;
    QTimer* _checkDownloadTimer;

private slots:
    void checkDownloadProgress();
    void nodeAdded(SharedNodePointer node);

};

#endif // hifi_LoadingScreen_h
