//
//  EntityItemPropertiesMacros.h
//  libraries/entities/src
//
//  Created by Brad Hefta-Gaub on 9/10/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#ifndef hifi_EntityItemPropertiesMacros_h
#define hifi_EntityItemPropertiesMacros_h

#include <QDateTime>

#include "EntityItemID.h"
#include <RegisteredMetaTypes.h>

#define APPEND_ENTITY_PROPERTY(P,V) \
        if (requestedProperties.getHasProperty(P)) {                \
            LevelDetails propertyLevel = packetData->startLevel();  \
            successPropertyFits = packetData->appendValue(V);       \
            if (successPropertyFits) {                              \
                propertyFlags |= P;                                 \
                propertiesDidntFit -= P;                            \
                propertyCount++;                                    \
                packetData->endLevel(propertyLevel);                \
            } else {                                                \
                packetData->discardLevel(propertyLevel);            \
                appendState = OctreeElement::PARTIAL;               \
            }                                                       \
        } else {                                                    \
            propertiesDidntFit -= P;                                \
        }

#define READ_ENTITY_PROPERTY(P,T,S)                                                \
        if (propertyFlags.getHasProperty(P)) {                                     \
            T fromBuffer;                                                          \
            int bytes = OctreePacketData::unpackDataFromBytes(dataAt, fromBuffer); \
            dataAt += bytes;                                                       \
            bytesRead += bytes;                                                    \
            if (overwriteLocalData) {                                              \
                S(fromBuffer);                                                     \
            }                                                                      \
            somethingChanged = true;                                               \
        }

#define SKIP_ENTITY_PROPERTY(P,T)                                                  \
        if (propertyFlags.getHasProperty(P)) {                                     \
            T fromBuffer;                                                          \
            int bytes = OctreePacketData::unpackDataFromBytes(dataAt, fromBuffer); \
            dataAt += bytes;                                                       \
            bytesRead += bytes;                                                    \
        }

#define DECODE_GROUP_PROPERTY_HAS_CHANGED(P,N) \
        if (propertyFlags.getHasProperty(P)) {  \
            set##N##Changed(true); \
        }


#define READ_ENTITY_PROPERTY_TO_PROPERTIES(P,T,O)                                  \
        if (propertyFlags.getHasProperty(P)) {                                     \
            T fromBuffer;                                                          \
            int bytes = OctreePacketData::unpackDataFromBytes(dataAt, fromBuffer); \
            dataAt += bytes;                                                       \
            processedBytes += bytes;                                               \
            properties.O(fromBuffer);                                              \
        }

#define SET_ENTITY_PROPERTY_FROM_PROPERTIES(P,M)    \
    if (properties._##P##Changed) {    \
        M(properties._##P);                         \
        somethingChanged = true;                    \
    }

#define SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(G,P,p,M)  \
    if (properties.get##G().p##Changed()) {                 \
        M(properties.get##G().get##P());                    \
        somethingChanged = true;                            \
    }

#define SET_ENTITY_PROPERTY_FROM_PROPERTIES_GETTER(C,G,S)    \
    if (properties.C()) {    \
        S(properties.G());                         \
        somethingChanged = true;                    \
    }

#define COPY_ENTITY_PROPERTY_TO_PROPERTIES(P,M) \
    properties._##P = M();                      \
    properties._##P##Changed = false;

#define COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(G,P,M)  \
    properties.get##G().set##P(M());                     \
    properties.get##G().set##P##Changed(false);

#define CHECK_PROPERTY_CHANGE(P,M) \
    if (_##M##Changed) {           \
        changedProperties += P;    \
    }

inline QScriptValue convertScriptValue(QScriptEngine* e, const glm::vec3& v) { return vec3toScriptValue(e, v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, float v) { return QScriptValue(v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, int v) { return QScriptValue(v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, quint16 v) { return QScriptValue(v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, quint32 v) { return QScriptValue(v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, quint64 v) { return QScriptValue((qsreal)v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, const QString& v) { return QScriptValue(v); }

inline QScriptValue convertScriptValue(QScriptEngine* e, const xColor& v) { return xColorToScriptValue(e, v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, const glm::quat& v) { return quatToScriptValue(e, v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, const QScriptValue& v) { return v; }
inline QScriptValue convertScriptValue(QScriptEngine* e, const QVector<glm::vec3>& v) {return qVectorVec3ToScriptValue(e, v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, const QVector<glm::quat>& v) {return qVectorQuatToScriptValue(e, v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, const QVector<bool>& v) {return qVectorBoolToScriptValue(e, v); }
inline QScriptValue convertScriptValue(QScriptEngine* e, const QVector<float>& v) { return qVectorFloatToScriptValue(e, v); }

inline QScriptValue convertScriptValue(QScriptEngine* e, const QByteArray& v) {
    QByteArray b64 = v.toBase64();
    return QScriptValue(QString(b64));
}

inline QScriptValue convertScriptValue(QScriptEngine* e, const EntityItemID& v) { return QScriptValue(QUuid(v).toString()); }

inline QScriptValue convertScriptValue(QScriptEngine* e, const AACube& v) { return aaCubeToScriptValue(e, v); }



#define COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(X,G,g,P,p) \
    if ((desiredProperties.isEmpty() || desiredProperties.getHasProperty(X)) && \
        (!skipDefaults || defaultEntityProperties.get##G().get##P() != get##P())) { \
        QScriptValue groupProperties = properties.property(#g); \
        if (!groupProperties.isValid()) { \
            groupProperties = engine->newObject(); \
        } \
        QScriptValue V = convertScriptValue(engine, get##P()); \
        groupProperties.setProperty(#p, V); \
        properties.setProperty(#g, groupProperties); \
    }

#define COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE_GETTER(X,G,g,P,p,M)                       \
    if ((desiredProperties.isEmpty() || desiredProperties.getHasProperty(X)) &&       \
        (!skipDefaults || defaultEntityProperties.get##G().get##P() != get##P())) {   \
        QScriptValue groupProperties = properties.property(#g);                       \
        if (!groupProperties.isValid()) {                                             \
            groupProperties = engine->newObject();                                    \
        }                                                                             \
        QScriptValue V = convertScriptValue(engine, M());                             \
        groupProperties.setProperty(#p, V);                                           \
        properties.setProperty(#g, groupProperties);                                  \
    }

#define COPY_PROPERTY_TO_QSCRIPTVALUE(p,P) \
    if ((_desiredProperties.isEmpty() || _desiredProperties.getHasProperty(p)) && \
        (!skipDefaults || defaultEntityProperties._##P != _##P)) { \
        QScriptValue V = convertScriptValue(engine, _##P); \
        properties.setProperty(#P, V); \
    }

#define COPY_PROPERTY_TO_QSCRIPTVALUE_GETTER_NO_SKIP(P, G) \
    properties.setProperty(#P, G);

#define COPY_PROPERTY_TO_QSCRIPTVALUE_GETTER(p, P, G) \
    if ((_desiredProperties.isEmpty() || _desiredProperties.getHasProperty(p)) && \
        (!skipDefaults || defaultEntityProperties._##P != _##P)) { \
        QScriptValue V = convertScriptValue(engine, G); \
        properties.setProperty(#P, V); \
    }

// same as COPY_PROPERTY_TO_QSCRIPTVALUE_GETTER but uses #X instead of #P in the setProperty() step
#define COPY_PROXY_PROPERTY_TO_QSCRIPTVALUE_GETTER(p, P, X, G) \
    if ((_desiredProperties.isEmpty() || _desiredProperties.getHasProperty(p)) && \
        (!skipDefaults || defaultEntityProperties._##P != _##P)) { \
        QScriptValue V = convertScriptValue(engine, G); \
        properties.setProperty(#X, V); \
    }

#define COPY_PROPERTY_TO_QSCRIPTVALUE_GETTER_ALWAYS(P, G) \
    if (!skipDefaults || defaultEntityProperties._##P != _##P) { \
        QScriptValue V = convertScriptValue(engine, G); \
        properties.setProperty(#P, V); \
    }

typedef glm::vec3 glmVec3;
typedef glm::quat glmQuat;
typedef QVector<glm::vec3> qVectorVec3;
typedef QVector<glm::quat> qVectorQuat;
typedef QVector<bool> qVectorBool;
typedef QVector<float> qVectorFloat;
inline float float_convertFromScriptValue(const QScriptValue& v, bool& isValid) { return v.toVariant().toFloat(&isValid); }
inline quint64 quint64_convertFromScriptValue(const QScriptValue& v, bool& isValid) { return v.toVariant().toULongLong(&isValid); }
inline quint32 quint32_convertFromScriptValue(const QScriptValue& v, bool& isValid) {
    // Use QString::toUInt() so that isValid is set to false if the number is outside the quint32 range.
    return v.toString().toUInt(&isValid);
}
inline quint16 quint16_convertFromScriptValue(const QScriptValue& v, bool& isValid) { return v.toVariant().toInt(&isValid); }
inline uint16_t uint16_t_convertFromScriptValue(const QScriptValue& v, bool& isValid) { return v.toVariant().toInt(&isValid); }
inline int int_convertFromScriptValue(const QScriptValue& v, bool& isValid) { return v.toVariant().toInt(&isValid); }
inline bool bool_convertFromScriptValue(const QScriptValue& v, bool& isValid) { isValid = true; return v.toVariant().toBool(); }
inline uint8_t uint8_t_convertFromScriptValue(const QScriptValue& v, bool& isValid) { isValid = true; return (uint8_t)(0xff & v.toVariant().toInt(&isValid)); }
inline QString QString_convertFromScriptValue(const QScriptValue& v, bool& isValid) { isValid = true; return v.toVariant().toString().trimmed(); }
inline QUuid QUuid_convertFromScriptValue(const QScriptValue& v, bool& isValid) { isValid = true; return v.toVariant().toUuid(); }
inline EntityItemID EntityItemID_convertFromScriptValue(const QScriptValue& v, bool& isValid) { isValid = true; return v.toVariant().toUuid(); }



inline QDateTime QDateTime_convertFromScriptValue(const QScriptValue& v, bool& isValid) {
    isValid = true;
    auto result = QDateTime::fromString(v.toVariant().toString().trimmed(), Qt::ISODate);
    // result.setTimeSpec(Qt::OffsetFromUTC);
    return result;
}



inline QByteArray QByteArray_convertFromScriptValue(const QScriptValue& v, bool& isValid) {
    isValid = true;
    QString b64 = v.toVariant().toString().trimmed();
    return QByteArray::fromBase64(b64.toUtf8());
}

inline glmVec3 glmVec3_convertFromScriptValue(const QScriptValue& v, bool& isValid) { 
    isValid = false; /// assume it can't be converted
    QScriptValue x = v.property("x");
    QScriptValue y = v.property("y");
    QScriptValue z = v.property("z");
    if (x.isValid() && y.isValid() && z.isValid()) {
        glm::vec3 newValue(0);
        newValue.x = x.toVariant().toFloat();
        newValue.y = y.toVariant().toFloat();
        newValue.z = z.toVariant().toFloat();
        isValid = !glm::isnan(newValue.x) &&
                  !glm::isnan(newValue.y) &&
                  !glm::isnan(newValue.z);
        if (isValid) {
            return newValue;
        }
    }
    return glm::vec3(0);
}

inline AACube AACube_convertFromScriptValue(const QScriptValue& v, bool& isValid) {
    isValid = true;
    AACube result;
    aaCubeFromScriptValue(v, result);
    return result;
}

inline qVectorFloat qVectorFloat_convertFromScriptValue(const QScriptValue& v, bool& isValid) {
    isValid = true;
    return qVectorFloatFromScriptValue(v);
}

inline qVectorVec3 qVectorVec3_convertFromScriptValue(const QScriptValue& v, bool& isValid) {
    isValid = true;
    return qVectorVec3FromScriptValue(v);
}

inline qVectorQuat qVectorQuat_convertFromScriptValue(const QScriptValue& v, bool& isValid) {
    isValid = true;
    return qVectorQuatFromScriptValue(v);
}

inline qVectorBool qVectorBool_convertFromScriptValue(const QScriptValue& v, bool& isValid) {
    isValid = true;
    return qVectorBoolFromScriptValue(v);
}

inline glmQuat glmQuat_convertFromScriptValue(const QScriptValue& v, bool& isValid) { 
    isValid = false; /// assume it can't be converted
    QScriptValue x = v.property("x");
    QScriptValue y = v.property("y");
    QScriptValue z = v.property("z");
    QScriptValue w = v.property("w");
    if (x.isValid() && y.isValid() && z.isValid() && w.isValid()) {
        glm::quat newValue;
        newValue.x = x.toVariant().toFloat();
        newValue.y = y.toVariant().toFloat();
        newValue.z = z.toVariant().toFloat();
        newValue.w = w.toVariant().toFloat();
        isValid = !glm::isnan(newValue.x) &&
                  !glm::isnan(newValue.y) &&
                  !glm::isnan(newValue.z) &&
                  !glm::isnan(newValue.w);
        if (isValid) {
            return newValue;
        }
    }
    return glm::quat();
}

inline xColor xColor_convertFromScriptValue(const QScriptValue& v, bool& isValid) { 
    xColor newValue { 255, 255, 255 };
    isValid = false; /// assume it can't be converted
    QScriptValue r = v.property("red");
    QScriptValue g = v.property("green");
    QScriptValue b = v.property("blue");
    if (r.isValid() && g.isValid() && b.isValid()) {
        newValue.red = r.toVariant().toInt();
        newValue.green = g.toVariant().toInt();
        newValue.blue = b.toVariant().toInt();
        isValid = true;
    }
    return newValue;
}


#define COPY_PROPERTY_IF_CHANGED(P) \
{                                   \
    if (other._##P##Changed) {      \
        _##P = other._##P;          \
    }                               \
}



#define COPY_PROPERTY_FROM_QSCRIPTVALUE(P, T, S)                     \
    {                                                                \
        QScriptValue V = object.property(#P);                        \
        if (V.isValid()) {                                           \
            bool isValid = false;                                    \
            T newValue = T##_convertFromScriptValue(V, isValid);     \
            if (isValid && (_defaultSettings || newValue != _##P)) { \
                S(newValue);                                         \
            }                                                        \
        }                                                            \
    }

#define COPY_PROPERTY_FROM_QSCRIPTVALUE_GETTER(P, T, S, G)      \
{                                                               \
    QScriptValue V = object.property(#P);                       \
    if (V.isValid()) {                                          \
        bool isValid = false;                                   \
        T newValue = T##_convertFromScriptValue(V, isValid);    \
        if (isValid && (_defaultSettings || newValue != G())) { \
            S(newValue);                                        \
        }                                                       \
    }                                                           \
}

#define COPY_PROPERTY_FROM_QSCRIPTVALUE_NOCHECK(P, T, S)     \
{                                                            \
    QScriptValue V = object.property(#P);                    \
    if (V.isValid()) {                                       \
        bool isValid = false;                                \
        T newValue = T##_convertFromScriptValue(V, isValid); \
        if (isValid && (_defaultSettings)) {                 \
            S(newValue);                                     \
        }                                                    \
    }                                                        \
}

#define COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(G, P, T, S)                \
    {                                                                    \
        QScriptValue G = object.property(#G);                            \
        if (G.isValid()) {                                               \
            QScriptValue V = G.property(#P);                             \
            if (V.isValid()) {                                           \
                bool isValid = false;                                    \
                T newValue = T##_convertFromScriptValue(V, isValid);     \
                if (isValid && (_defaultSettings || newValue != _##P)) { \
                    S(newValue);                                         \
                }                                                        \
            }                                                            \
        }                                                                \
    }

#define COPY_PROPERTY_FROM_QSCRITPTVALUE_ENUM(P, S)               \
    QScriptValue P = object.property(#P);                         \
    if (P.isValid()) {                                            \
        QString newValue = P.toVariant().toString();              \
        if (_defaultSettings || newValue != get##S##AsString()) { \
            set##S##FromString(newValue);                         \
        }                                                         \
    }

#define DEFINE_PROPERTY_GROUP(N, n, T)           \
    public:                                      \
        const T& get##N() const { return _##n; } \
        T& get##N() { return _##n; }             \
    private:                                     \
        T _##n;                                  \
        static T _static##N; 

#define ADD_PROPERTY_TO_MAP(P, N, n, T) \
        _propertyStringsToEnums[#n] = P;

#define ADD_GROUP_PROPERTY_TO_MAP(P, G, g, N, n) \
        _propertyStringsToEnums[#g "." #n] = P;

#define DEFINE_CORE(N, n, T, V) \
    public: \
        bool n##Changed() const { return _##n##Changed; } \
        void set##N##Changed(bool value) { _##n##Changed = value; } \
    private: \
        T _##n = V; \
        bool _##n##Changed { false };

#define DEFINE_PROPERTY(P, N, n, T, V)        \
    public: \
        T get##N() const { return _##n; } \
        void set##N(T value) { _##n = value; _##n##Changed = true; } \
    DEFINE_CORE(N, n, T, V)

#define DEFINE_PROPERTY_REF(P, N, n, T, V)        \
    public: \
        const T& get##N() const { return _##n; } \
        void set##N(const T& value) { _##n = value; _##n##Changed = true; } \
    DEFINE_CORE(N, n, T, V)

#define DEFINE_PROPERTY_REF_WITH_SETTER(P, N, n, T, V)        \
    public: \
        const T& get##N() const { return _##n; } \
        void set##N(const T& value); \
    DEFINE_CORE(N, n, T, V)

#define DEFINE_PROPERTY_REF_WITH_SETTER_AND_GETTER(P, N, n, T, V)        \
    public: \
        T get##N() const; \
        void set##N(const T& value); \
    DEFINE_CORE(N, n, T, V)

#define DEFINE_PROPERTY_REF_ENUM(P, N, n, T, V) \
    public: \
        const T& get##N() const { return _##n; } \
        void set##N(const T& value) { _##n = value; _##n##Changed = true; } \
        QString get##N##AsString() const; \
        void set##N##FromString(const QString& name); \
    DEFINE_CORE(N, n, T, V)

#define DEBUG_PROPERTY(D, P, N, n, x)                \
    D << "  " << #n << ":" << P.get##N() << x << "[changed:" << P.n##Changed() << "]\n";

#define DEBUG_PROPERTY_IF_CHANGED(D, P, N, n, x)                \
    if (P.n##Changed()) {                                       \
        D << "  " << #n << ":" << P.get##N() << x << "\n";      \
    }

#endif // hifi_EntityItemPropertiesMacros_h
