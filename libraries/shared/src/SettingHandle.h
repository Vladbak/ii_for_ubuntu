//
//  SettingHandle.h
//
//
//  Created by Clement on 1/18/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SettingHandle_h
#define hifi_SettingHandle_h

#include <type_traits>

#include <QtCore/QStack>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QReadWriteLock>
#include <QtCore/QDebug>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "SettingInterface.h"


// TODO: remove
class Settings {
public:
    static const QString firstRun;
    Settings();

    QString fileName() const;

    void remove(const QString& key);
    QStringList childGroups() const;
    QStringList childKeys() const;
    QStringList allKeys() const;
    bool contains(const QString& key) const;
    int	beginReadArray(const QString & prefix);
    void beginWriteArray(const QString& prefix, int size = -1);
    void endArray();
    void setArrayIndex(int i);

    void beginGroup(const QString& prefix);
    void endGroup();

    void setValue(const QString& name, const QVariant& value);
    QVariant value(const QString& name, const QVariant& defaultValue = QVariant()) const;

    void getFloatValueIfValid(const QString& name, float& floatValue);
    void getBoolValue(const QString& name, bool& boolValue);

    void setVec3Value(const QString& name, const glm::vec3& vecValue);
    void getVec3ValueIfValid(const QString& name, glm::vec3& vecValue);

    void setQuatValue(const QString& name, const glm::quat& quatValue);
    void getQuatValueIfValid(const QString& name, glm::quat& quatValue);

private:
    QSharedPointer<Setting::Manager> _manager;
};

namespace Setting {
    template <typename T>
    class Handle : public Interface {
    public:
        Handle(const QString& key) : Interface(key) {}
        Handle(const QStringList& path) : Interface(path.join("/")) {}

        Handle(const QString& key, const T& defaultValue) : Interface(key), _defaultValue(defaultValue) {}
        Handle(const QStringList& path, const T& defaultValue) : Handle(path.join("/"), defaultValue) {}

        static Handle Deprecated(const QString& key) {
            Handle handle = Handle(key);
            handle.deprecate();
            return handle;
        }
        static Handle Deprecated(const QStringList& path) {
            return Deprecated(path.join("/"));
        }

        static Handle Deprecated(const QString& key, const T& defaultValue) {
            Handle handle = Handle(key, defaultValue);
            handle.deprecate();
            return handle;
        }
        static Handle Deprecated(const QStringList& path, const T& defaultValue) {
            return Deprecated(path.join("/"), defaultValue);
        }

        virtual ~Handle() {
            deinit();
        }

        // Returns setting value, returns its default value if not found
        T get() const {
            return get(_defaultValue);
        }

        // Returns setting value, returns other if not found
        T get(const T& other) const {
            maybeInit();
            return (_isSet) ? _value : other;
        }

        const T& getDefault() const {
            return _defaultValue;
        }

        void reset() {
            set(_defaultValue);
        }

        void set(const T& value) {
            maybeInit();
            if ((!_isSet && (value != _defaultValue)) || _value != value) {
                _value = value;
                _isSet = true;
                save();
            }
            if (_isDeprecated) {
                deprecate();
            }
        }

        void remove() {
            maybeInit();
            if (_isSet) {
                _isSet = false;
                save();
            }
        }

    protected:
        virtual void setVariant(const QVariant& variant) override;
        virtual QVariant getVariant() override { return QVariant::fromValue(get()); }

    private:
        void deprecate() {
            if (_isSet) {
                if (get() != getDefault()) {
                    qInfo().nospace() << "[DEPRECATION NOTICE] " << _key << "(" << get() << ") has been deprecated, and has no effect";
                } else {
                    remove();
                }
            }
            _isDeprecated = true;
        }

        T _value;
        const T _defaultValue;
        bool _isDeprecated{ false };
    };

    template <typename T>
    void Handle<T>::setVariant(const QVariant& variant) {
        if (variant.canConvert<T>() || std::is_same<T, QVariant>::value) {
            set(variant.value<T>());
        }
    }
}

#endif // hifi_SettingHandle_h
