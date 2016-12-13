//
//  WindowScriptingInterface.cpp
//  interface/src/scripting
//
//  Created by Ryan Huffman on 4/29/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_WindowScriptingInterface_h
#define hifi_WindowScriptingInterface_h

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtScript/QScriptValue>

class CustomPromptResult {
public:
    QVariant value;
};

Q_DECLARE_METATYPE(CustomPromptResult);

QScriptValue CustomPromptResultToScriptValue(QScriptEngine* engine, const CustomPromptResult& result);
void CustomPromptResultFromScriptValue(const QScriptValue& object, CustomPromptResult& result);


class WindowScriptingInterface : public QObject, public Dependency {
    Q_OBJECT
    Q_PROPERTY(int innerWidth READ getInnerWidth)
    Q_PROPERTY(int innerHeight READ getInnerHeight)
    Q_PROPERTY(int x READ getX)
    Q_PROPERTY(int y READ getY)
public:
    WindowScriptingInterface();
    int getInnerWidth();
    int getInnerHeight();
    int getX();
    int getY();

public slots:
    QScriptValue hasFocus();
    void setFocus();
    void raiseMainWindow();
    void alert(const QString& message = "");
    QScriptValue confirm(const QString& message = "");
    QScriptValue prompt(const QString& message = "", const QString& defaultText = "");
    CustomPromptResult customPrompt(const QVariant& config);
    QScriptValue browse(const QString& title = "", const QString& directory = "",  const QString& nameFilter = "");
    QScriptValue save(const QString& title = "", const QString& directory = "",  const QString& nameFilter = "");
    void showAssetServer(const QString& upload = "");
    void copyToClipboard(const QString& text);
    void takeSnapshot(bool notify = true, bool includeAnimated = false, float aspectRatio = 0.0f);
    void shareSnapshot(const QString& path, const QUrl& href = QUrl(""));
    bool isPhysicsEnabled();

signals:
    void domainChanged(const QString& domainHostname);
    void svoImportRequested(const QString& url);
    void domainConnectionRefused(const QString& reasonMessage, int reasonCode, const QString& extraInfo);
    void snapshotTaken(const QString& pathStillSnapshot, const QString& pathAnimatedSnapshot, bool notify);
    void snapshotShared(const QString& error);
    void processingGif();

private:
    QString getPreviousBrowseLocation() const;
    void setPreviousBrowseLocation(const QString& location);
};

#endif // hifi_WindowScriptingInterface_h
