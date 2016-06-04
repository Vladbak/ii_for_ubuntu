#include "GLHelpers.h"

#include <mutex>

#include <QtGui/QSurfaceFormat>
#include <QtOpenGL/QGL>
#include <QOpenGLContext>

const QSurfaceFormat& getDefaultOpenGLSurfaceFormat() {
    static QSurfaceFormat format;
    static std::once_flag once;
    std::call_once(once, [] {
        // Qt Quick may need a depth and stencil buffer. Always make sure these are available.
        format.setDepthBufferSize(DEFAULT_GL_DEPTH_BUFFER_BITS);
        format.setStencilBufferSize(DEFAULT_GL_STENCIL_BUFFER_BITS);
        setGLFormatVersion(format);
#ifdef DEBUG
        format.setOption(QSurfaceFormat::DebugContext);
#endif
        format.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);
        QSurfaceFormat::setDefaultFormat(format);
    });
    return format;
}

const QGLFormat& getDefaultGLFormat() {
    // Specify an OpenGL 3.3 format using the Core profile.
    // That is, no old-school fixed pipeline functionality
    static QGLFormat glFormat;
    static std::once_flag once;
    std::call_once(once, [] {
        setGLFormatVersion(glFormat);
        glFormat.setProfile(QGLFormat::CoreProfile); // Requires >=Qt-4.8.0
        glFormat.setSampleBuffers(false);
        glFormat.setDepth(false);
        glFormat.setStencil(false);
        QGLFormat::setDefaultFormat(glFormat);
    });
    return glFormat;
}

QJsonObject getGLContextData() {
    if (!QOpenGLContext::currentContext()) {
        return QJsonObject();
    }

    QString glVersion = QString((const char*)glGetString(GL_VERSION));
    QString glslVersion = QString((const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));
    QString glVendor = QString((const char*) glGetString(GL_VENDOR));
    QString glRenderer = QString((const char*)glGetString(GL_RENDERER));

    return QJsonObject {
        { "version", glVersion },
        { "slVersion", glslVersion },
        { "vendor", glVendor },
        { "renderer", glRenderer },
    };
}
