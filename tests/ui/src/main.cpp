#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtWebEngine>
#include <QFileSystemModel>

#include "../../../libraries/ui/src/FileDialogHelper.h"


class Preference : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString category READ getCategory() CONSTANT)
    Q_PROPERTY(QString name READ getName() CONSTANT)
    Q_PROPERTY(Type type READ getType() CONSTANT)
    Q_ENUMS(Type)
public:
    enum Type {
        Editable,
        Browsable,
        Spinner,
        Checkbox,
    };

    Preference(QObject* parent = nullptr) : QObject(parent) {}

    Preference(const QString& category, const QString& name, QObject* parent = nullptr)
        : QObject(parent), _category(category), _name(name) { }
    const QString& getCategory() const { return _category; }
    const QString& getName() const { return _name; }
    virtual Type getType() { return Editable; }

protected:
    const QString _category;
    const QString _name;
};


QString getRelativeDir(const QString& relativePath = ".") {
    QDir path(__FILE__); path.cdUp();
    auto result = path.absoluteFilePath(relativePath);
    result = path.cleanPath(result) + "/";
    return result;
}

QString getTestQmlDir() {
    return getRelativeDir("../qml");
}

QString getInterfaceQmlDir() {
    return getRelativeDir("/");
}


void setChild(QQmlApplicationEngine& engine, const char* name) {
  for (auto obj : engine.rootObjects()) {
    auto child = obj->findChild<QObject*>(QString(name));
    if (child) {
      engine.rootContext()->setContextProperty(name, child);
      return;
    }
  }
  qWarning() << "Could not find object named " << name;
}

void addImportPath(QQmlApplicationEngine& engine, const QString& relativePath) {
    QString resolvedPath = getRelativeDir("../qml");
    QUrl resolvedUrl = QUrl::fromLocalFile(resolvedPath);
    resolvedPath = resolvedUrl.toString();
    engine.addImportPath(resolvedPath);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("Some Company");
    app.setOrganizationDomain("somecompany.com");
    app.setApplicationName("Amazing Application");
    QDir::setCurrent(getRelativeDir(".."));

    QtWebEngine::initialize();
    qmlRegisterType<Preference>("Hifi", 1, 0, "Preference");

    QQmlApplicationEngine engine;
    addImportPath(engine, "../qml");
    addImportPath(engine, "../../../interface/resources/qml");
    engine.load(QUrl(QStringLiteral("qml/Stubs.qml")));

    setChild(engine, "offscreenFlags");
    setChild(engine, "Account");
    setChild(engine, "ApplicationCompositor");
    setChild(engine, "Desktop");
    setChild(engine, "ScriptDiscoveryService");
    setChild(engine, "MenuHelper");
    setChild(engine, "Preferences");
    setChild(engine, "urlHandler");
    engine.rootContext()->setContextProperty("DebugQML", true);
    engine.rootContext()->setContextProperty("fileDialogHelper", new FileDialogHelper());

    //engine.load(QUrl(QStringLiteral("qrc:/qml/gallery/main.qml")));
    engine.load(QUrl(QStringLiteral("qml/main.qml")));
    return app.exec();
}

#include "main.moc"
