//
//  main.cpp
//  tests/gpu-test/src
//
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <unordered_map>
#include <memory>
#include <cstdio>
#include <mutex>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QLoggingCategory>

#include <QtGui/QResizeEvent>
#include <QtGui/QWindow>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>

#include <QtQuick/QQuickItem>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include <plugins/Plugin.h>
#include <plugins/PluginContainer.h>
#include <plugins/PluginManager.h>
#include <input-plugins/InputPlugin.h>
#include <input-plugins/KeyboardMouseDevice.h>
#include <controllers/ScriptingInterface.h>

#include <DependencyManager.h>
#include <controllers/UserInputMapper.h>

const QString& getResourcesDir() {
    static QString dir;
    if (dir.isEmpty()) {
        QDir path(__FILE__);
        path.cdUp();
        dir = path.cleanPath(path.absoluteFilePath("../../../interface/resources/")) + "/";
        qDebug() << "Resources Path: " << dir;
    }
    return dir;
}

const QString& getQmlDir() {
    static QString dir;
    if (dir.isEmpty()) {
        dir = getResourcesDir() + "qml/";
        qDebug() << "Qml Path: " << dir;
    }
    return dir;
}

const QString& getTestQmlDir() {
    static QString dir;
    if (dir.isEmpty()) {
        QDir path(__FILE__);
        path.cdUp();
        dir = path.cleanPath(path.absoluteFilePath("../")) + "/qml/";
        qDebug() << "Qml Test Path: " << dir;
    }
    return dir;
}

using namespace controller;


class PluginContainerProxy : public QObject, PluginContainer {
    Q_OBJECT
public:
    virtual ~PluginContainerProxy() {}
    virtual void addMenu(const QString& menuName) override {}
    virtual void removeMenu(const QString& menuName) override {}
    virtual QAction* addMenuItem(PluginType type, const QString& path, const QString& name, std::function<void(bool)> onClicked, bool checkable = false, bool checked = false, const QString& groupName = "") override { return nullptr;  }
    virtual void removeMenuItem(const QString& menuName, const QString& menuItem) override {}
    virtual bool isOptionChecked(const QString& name) override { return false;  }
    virtual void setIsOptionChecked(const QString& path, bool checked) override {}
    virtual void setFullscreen(const QScreen* targetScreen, bool hideMenu = true) override {}
    virtual void unsetFullscreen(const QScreen* avoidScreen = nullptr) override {}
    virtual void showDisplayPluginsTools() override {}
    virtual void requestReset() override {}
    virtual bool makeRenderingContextCurrent() override { return true; }
    virtual void releaseSceneTexture(uint32_t texture) override {}
    virtual void releaseOverlayTexture(uint32_t texture) override {}
    virtual GLWidget* getPrimaryWidget() override { return nullptr; }
    virtual QWindow* getPrimaryWindow() override { return nullptr; }
    virtual QOpenGLContext* getPrimaryContext() override { return nullptr; }
    virtual bool isForeground() override { return true;  }
    virtual const DisplayPlugin* getActiveDisplayPlugin() const override { return nullptr;  }
    virtual bool getBoolSetting(const QString& settingName, bool defaultValue) override { return defaultValue; }
    virtual void setBoolSetting(const QString& settingName, bool value) override { }
};

class MyControllerScriptingInterface : public controller::ScriptingInterface {
public:
    virtual void registerControllerTypes(QScriptEngine* engine) {};
};


int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    auto rootContext = engine.rootContext();
    new PluginContainerProxy();

    // Simulate our application idle loop
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [] {
        static float last = secTimestampNow();
        float now = secTimestampNow();
        float delta = now - last;
        last = now;

        InputCalibrationData calibrationData = {
            glm::mat4(),
            glm::mat4(),
            glm::mat4()
        };

        foreach(auto inputPlugin, PluginManager::getInstance()->getInputPlugins()) {
            inputPlugin->pluginUpdate(delta, calibrationData, false);
        }

        auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
        userInputMapper->update(delta);
    });
    timer.start(50);

    {
        InputCalibrationData calibrationData = {
            glm::mat4(),
            glm::mat4(),
            glm::mat4()
        };

        DependencyManager::set<controller::UserInputMapper>();
        foreach(auto inputPlugin, PluginManager::getInstance()->getInputPlugins()) {
            QString name = inputPlugin->getName();
            inputPlugin->activate();
            auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
            if (name == KeyboardMouseDevice::NAME) {
                userInputMapper->registerDevice(std::dynamic_pointer_cast<KeyboardMouseDevice>(inputPlugin)->getInputDevice());
            }
            inputPlugin->pluginUpdate(0, calibrationData, false);
        }
        rootContext->setContextProperty("Controllers", new MyControllerScriptingInterface());
    }
    qDebug() << getQmlDir();
    rootContext->setContextProperty("ResourcePath", getQmlDir());
    engine.setBaseUrl(QUrl::fromLocalFile(getQmlDir()));
    engine.addImportPath(getQmlDir());
    engine.load(getTestQmlDir() + "main.qml");
    for (auto pathItem : engine.importPathList()) {
        qDebug() << pathItem;
    }
    app.exec();
    return 0;
}

#include "main.moc"

