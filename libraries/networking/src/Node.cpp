//
//  Node.cpp
//  libraries/networking/src
//
//  Created by Stephen Birarda on 2/15/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <cstring>
#include <stdio.h>

#include <UUID.h>

#include "Node.h"
#include "SharedUtil.h"

#include <QtCore/QDataStream>
#include <QtCore/QDebug>

const QString UNKNOWN_NodeType_t_NAME = "Unknown";

int NodePtrMetaTypeId = qRegisterMetaType<Node*>("Node*");
int sharedPtrNodeMetaTypeId = qRegisterMetaType<QSharedPointer<Node>>("QSharedPointer<Node>");
int sharedNodePtrMetaTypeId = qRegisterMetaType<SharedNodePointer>("SharedNodePointer");

namespace NodeType {
    QHash<NodeType_t, QString> TypeNameHash;
}

void NodeType::init() {
    TypeNameHash.insert(NodeType::DomainServer, "Domain Server");
    TypeNameHash.insert(NodeType::EntityServer, "Entity Server");
    TypeNameHash.insert(NodeType::Agent, "Agent");
    TypeNameHash.insert(NodeType::AudioMixer, "Audio Mixer");
    TypeNameHash.insert(NodeType::AvatarMixer, "Avatar Mixer");
    TypeNameHash.insert(NodeType::MessagesMixer, "Messages Mixer");
    TypeNameHash.insert(NodeType::AssetServer, "Asset Server");
    TypeNameHash.insert(NodeType::Unassigned, "Unassigned");
}

const QString& NodeType::getNodeTypeName(NodeType_t nodeType) {
    QHash<NodeType_t, QString>::iterator matchedTypeName = TypeNameHash.find(nodeType);
    return matchedTypeName != TypeNameHash.end() ? matchedTypeName.value() : UNKNOWN_NodeType_t_NAME;
}

Node::Node(const QUuid& uuid, NodeType_t type, const HifiSockAddr& publicSocket,
           const HifiSockAddr& localSocket, bool isAllowedEditor, bool canRez, const QUuid& connectionSecret,
           QObject* parent) :
    NetworkPeer(uuid, publicSocket, localSocket, parent),
    _type(type),
    _connectionSecret(connectionSecret),
    _isAlive(true),
    _pingMs(-1),  // "Uninitialized"
    _clockSkewUsec(0),
    _mutex(),
    _clockSkewMovingPercentile(30, 0.8f),   // moving 80th percentile of 30 samples
    _isAllowedEditor(isAllowedEditor),
    _canRez(canRez)
{
    // Update socket's object name
    setType(_type);
}

void Node::setType(char type) {
    _type = type;
    
    auto typeString = NodeType::getNodeTypeName(type);
    _publicSocket.setObjectName(typeString);
    _localSocket.setObjectName(typeString);
    _symmetricSocket.setObjectName(typeString);
}

void Node::updateClockSkewUsec(qint64 clockSkewSample) {
    _clockSkewMovingPercentile.updatePercentile(clockSkewSample);
    _clockSkewUsec = (quint64)_clockSkewMovingPercentile.getValueAtPercentile();
}


QDataStream& operator<<(QDataStream& out, const Node& node) {
    out << node._type;
    out << node._uuid;
    out << node._publicSocket;
    out << node._localSocket;
    out << node._isAllowedEditor;
    out << node._canRez;

    return out;
}

QDataStream& operator>>(QDataStream& in, Node& node) {
    in >> node._type;
    in >> node._uuid;
    in >> node._publicSocket;
    in >> node._localSocket;
    in >> node._isAllowedEditor;
    in >> node._canRez;

    return in;
}

QDebug operator<<(QDebug debug, const Node& node) {
    debug.nospace() << NodeType::getNodeTypeName(node.getType());
    if (node.getType() == NodeType::Unassigned) {
        debug.nospace() << " (1)";
    } else {
        debug.nospace() << " (" << node.getType() << ")";
    }
    debug << " " << node.getUUID().toString().toLocal8Bit().constData() << " ";
    debug.nospace() << node.getPublicSocket() << "/" << node.getLocalSocket();
    return debug.nospace();
}
