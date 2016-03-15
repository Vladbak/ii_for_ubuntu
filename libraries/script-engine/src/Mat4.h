//
//  Mat4.h
//  libraries/script-engine/src
//
//  Created by Anthony Thibault on 3/7/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Scriptable 4x4 Matrix class library.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_Mat4_h
#define hifi_Mat4_h

#include <QObject>
#include <QString>

/// Scriptable Mat4 object.  Used exclusively in the JavaScript API
class Mat4 : public QObject {
    Q_OBJECT

public slots:
    glm::mat4 multiply(const glm::mat4& m1, const glm::mat4& m2) const;
    glm::mat4 createFromRotAndTrans(const glm::quat& rot, const glm::vec3& trans) const;
    glm::mat4 createFromScaleRotAndTrans(const glm::vec3& scale, const glm::quat& rot, const glm::vec3& trans) const;

    glm::vec3 extractTranslation(const glm::mat4& m) const;
    glm::quat extractRotation(const glm::mat4& m) const;
    glm::vec3 extractScale(const glm::mat4& m) const;

    glm::vec3 transformPoint(const glm::mat4& m, const glm::vec3& point) const;
    glm::vec3 transformVector(const glm::mat4& m, const glm::vec3& vector) const;

    glm::mat4 inverse(const glm::mat4& m) const;

    glm::vec3 getFront(const glm::mat4& m) const;
    glm::vec3 getRight(const glm::mat4& m) const;
    glm::vec3 getUp(const glm::mat4& m) const;

    void print(const QString& label, const glm::mat4& m) const;
};

#endif // hifi_Mat4_h
