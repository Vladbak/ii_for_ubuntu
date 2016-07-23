//
//  OffscreenGLCanvas.cpp
//  interface/src/renderer
//
//  Created by Bradley Austin Davis on 2014/04/09.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#include "OffscreenGLCanvas.h"

#include <QtCore/QProcessEnvironment>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLDebugLogger>
#include <QtGui/QOpenGLContext>

#include "GLHelpers.h"

#ifdef DEBUG
static bool enableDebugLogger = true;
#else
static const QString DEBUG_FLAG("HIFI_ENABLE_OPENGL_45");
static bool enableDebugLogger = QProcessEnvironment::systemEnvironment().contains(DEBUG_FLAG);
#endif


OffscreenGLCanvas::OffscreenGLCanvas() : _context(new QOpenGLContext), _offscreenSurface(new QOffscreenSurface){
}

OffscreenGLCanvas::~OffscreenGLCanvas() {
    if (_logger) {
        makeCurrent();
        delete _logger;
        _logger = nullptr;
    }
    
    _context->doneCurrent();
    delete _context;
    _context = nullptr;

    _offscreenSurface->destroy();
    delete _offscreenSurface;
    _offscreenSurface = nullptr;

}

bool OffscreenGLCanvas::create(QOpenGLContext* sharedContext) {
    if (nullptr != sharedContext) {
        sharedContext->doneCurrent();
        _context->setShareContext(sharedContext);
    }
    _context->setFormat(getDefaultOpenGLSurfaceFormat());

    if (_context->create()) {
        _offscreenSurface->setFormat(_context->format());
        _offscreenSurface->create();
        return _offscreenSurface->isValid();
    }
    qWarning("Failed to create OffscreenGLCanvas context");

    return false;
}

bool OffscreenGLCanvas::makeCurrent() {
    bool result = _context->makeCurrent(_offscreenSurface);
    Q_ASSERT(result);
    
    std::call_once(_reportOnce, []{
        qDebug() << "GL Version: " << QString((const char*) glGetString(GL_VERSION));
        qDebug() << "GL Shader Language Version: " << QString((const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));
        qDebug() << "GL Vendor: " << QString((const char*) glGetString(GL_VENDOR));
        qDebug() << "GL Renderer: " << QString((const char*) glGetString(GL_RENDERER));
    });


    if (result && !_logger) {
        _logger = new QOpenGLDebugLogger(this);
        if (_logger->initialize()) {
            connect(_logger, &QOpenGLDebugLogger::messageLogged, [](const QOpenGLDebugMessage& message) {
                qDebug() << message;
            });
            _logger->disableMessages(QOpenGLDebugMessage::AnySource, QOpenGLDebugMessage::AnyType, QOpenGLDebugMessage::NotificationSeverity);
            _logger->startLogging(QOpenGLDebugLogger::LoggingMode::SynchronousLogging);
        }
    }

    return result;
}

void OffscreenGLCanvas::doneCurrent() {
    _context->doneCurrent();
}

QObject* OffscreenGLCanvas::getContextObject() {
    return _context;
}

void OffscreenGLCanvas::moveToThreadWithContext(QThread* thread) {
    moveToThread(thread);
    _context->moveToThread(thread);
}