//
//  NodePermissions.h
//  libraries/networking/src/
//
//  Created by Seth Alves on 2016-6-1.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_NodePermissions_h
#define hifi_NodePermissions_h

#include <memory>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QUuid>

#include "GroupRank.h"

class NodePermissions;
using NodePermissionsPointer = std::shared_ptr<NodePermissions>;
using NodePermissionsKey = QPair<QString, QUuid>; // name, rankID
using NodePermissionsKeyList = QList<QPair<QString, QUuid>>;


class NodePermissions {
public:
    NodePermissions() { _id = QUuid::createUuid().toString(); _rankID = QUuid(); }
    NodePermissions(const QString& name) { _id = name.toLower(); _rankID = QUuid(); }
    NodePermissions(const NodePermissionsKey& key) { _id = key.first.toLower(); _rankID = key.second; }
    NodePermissions(QMap<QString, QVariant> perms);

    const QString& getID() const { return _id; } // a user-name or a group-name, not verified
    void setID(const QString& id) { _id = id; }
    void setRankID(QUuid& rankID) { _rankID = rankID; }
    const QUuid& getRankID() const { return _rankID; }
    NodePermissionsKey getKey() const { return NodePermissionsKey(_id, _rankID); }

    // the _id member isn't authenticated/verified and _username is.
    void setVerifiedUserName(QString userName) { _verifiedUserName = userName.toLower(); }
    const QString& getVerifiedUserName() const { return _verifiedUserName; }

    void setGroupID(QUuid groupID) { _groupID = groupID; if (!groupID.isNull()) { _groupIDSet = true; }}
    const QUuid& getGroupID() const { return _groupID; }
    bool isGroup() const { return _groupIDSet; }

    bool isAssignment { false };

    // these 3 names have special meaning.
    static NodePermissionsKey standardNameLocalhost;
    static NodePermissionsKey standardNameLoggedIn;
    static NodePermissionsKey standardNameAnonymous;
    static NodePermissionsKey standardNameFriends;
    static QStringList standardNames;

    enum class Permission {
        none = 0,
        canConnectToDomain = 1,
        canAdjustLocks = 2,
        canRezPermanentEntities = 4,
        canRezTemporaryEntities = 8,
        canWriteToAssetServer = 16,
        canConnectPastMaxCapacity = 32,
        canKick = 64
    };
    Q_DECLARE_FLAGS(Permissions, Permission)
    Permissions permissions;

    QVariant toVariant(QHash<QUuid, GroupRank> groupRanks = QHash<QUuid, GroupRank>());

    void setAll(bool value);

    NodePermissions& operator|=(const NodePermissions& rhs);
    NodePermissions& operator&=(const NodePermissions& rhs);
    NodePermissions operator~();
    friend QDataStream& operator<<(QDataStream& out, const NodePermissions& perms);
    friend QDataStream& operator>>(QDataStream& in, NodePermissions& perms);

    void clear(Permission p) { permissions &= (Permission) (~(uint)p); }
    void set(Permission p) { permissions |= p; }
    bool can(Permission p) const { return permissions.testFlag(p); }

protected:
    QString _id;
    QUuid _rankID { QUuid() }; // 0 unless this is for a group
    QString _verifiedUserName;

    bool _groupIDSet { false };
    QUuid _groupID;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(NodePermissions::Permissions)


// wrap QHash in a class that forces all keys to be lowercase
class NodePermissionsMap {
public:
    NodePermissionsMap() { }
    NodePermissionsPointer& operator[](const NodePermissionsKey& key) {
        NodePermissionsKey dataKey(key.first.toLower(), key.second);
        if (!_data.contains(dataKey)) {
            _data[dataKey] = NodePermissionsPointer(new NodePermissions(key));
        }
        return _data[dataKey];
    }
    NodePermissionsPointer operator[](const NodePermissionsKey& key) const {
        return _data.value(NodePermissionsKey(key.first.toLower(), key.second));
    }
    bool contains(const NodePermissionsKey& key) const {
        return _data.contains(NodePermissionsKey(key.first.toLower(), key.second));
    }
    bool contains(const QString& keyFirst, QUuid keySecond) const {
        return _data.contains(NodePermissionsKey(keyFirst.toLower(), keySecond));
    }
    QList<NodePermissionsKey> keys() const { return _data.keys(); }
    QHash<NodePermissionsKey, NodePermissionsPointer> get() { return _data; }
    void clear() { _data.clear(); }
    void remove(const NodePermissionsKey& key) { _data.remove(key); }

private:
    QHash<NodePermissionsKey, NodePermissionsPointer> _data;
};


const NodePermissions DEFAULT_AGENT_PERMISSIONS;

QDebug operator<<(QDebug debug, const NodePermissions& perms);
QDebug operator<<(QDebug debug, const NodePermissionsPointer& perms);
NodePermissionsPointer& operator&=(NodePermissionsPointer& lhs, const NodePermissionsPointer& rhs);
NodePermissionsPointer& operator&=(NodePermissionsPointer& lhs, NodePermissions::Permission rhs);
NodePermissionsPointer operator~(NodePermissionsPointer& lhs);

#endif // hifi_NodePermissions_h
