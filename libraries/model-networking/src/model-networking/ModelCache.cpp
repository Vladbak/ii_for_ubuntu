//
//  ModelCache.cpp
//  interface/src/renderer
//
//  Created by Andrzej Kapolka on 6/21/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ModelCache.h"

#include <cmath>

#include <QNetworkReply>
#include <QThreadPool>

#include <FSTReader.h>
#include <NumericalConstants.h>

#include "TextureCache.h"
#include "ModelNetworkingLogging.h"

#include "model/TextureMap.h"

//#define WANT_DEBUG

ModelCache::ModelCache()
{
    const qint64 GEOMETRY_DEFAULT_UNUSED_MAX_SIZE = DEFAULT_UNUSED_MAX_SIZE;
    setUnusedResourceCacheSize(GEOMETRY_DEFAULT_UNUSED_MAX_SIZE);
}

ModelCache::~ModelCache() {
}

QSharedPointer<Resource> ModelCache::createResource(const QUrl& url, const QSharedPointer<Resource>& fallback,
                                                        bool delayLoad, const void* extra) {
    // NetworkGeometry is no longer a subclass of Resource, but requires this method because, it is pure virtual.
    assert(false);
    return QSharedPointer<Resource>();
}


GeometryReader::GeometryReader(const QUrl& url, const QByteArray& data, const QVariantHash& mapping) :
    _url(url),
    _data(data),
    _mapping(mapping) {
}

void GeometryReader::run() {
    auto originalPriority = QThread::currentThread()->priority();
    if (originalPriority == QThread::InheritPriority) {
        originalPriority = QThread::NormalPriority;
    }
    QThread::currentThread()->setPriority(QThread::LowPriority);
    try {
        if (_data.isEmpty()) {
            throw QString("Reply is NULL ?!");
        }
        QString urlname = _url.path().toLower();
        bool urlValid = true;
        urlValid &= !urlname.isEmpty();
        urlValid &= !_url.path().isEmpty();
        urlValid &= _url.path().toLower().endsWith(".fbx") || _url.path().toLower().endsWith(".obj");

        if (urlValid) {
            // Let's read the binaries from the network
            FBXGeometry* fbxgeo = nullptr;
            if (_url.path().toLower().endsWith(".fbx")) {
                const bool grabLightmaps = true;
                const float lightmapLevel = 1.0f;
                fbxgeo = readFBX(_data, _mapping, _url.path(), grabLightmaps, lightmapLevel);
            } else if (_url.path().toLower().endsWith(".obj")) {
                fbxgeo = OBJReader().readOBJ(_data, _mapping, _url);
            } else {
                QString errorStr("unsupported format");
                emit onError(NetworkGeometry::ModelParseError, errorStr);
            }
            emit onSuccess(fbxgeo);
        } else {
            throw QString("url is invalid");
        }

    } catch (const QString& error) {
        qCDebug(modelnetworking) << "Error reading " << _url << ": " << error;
        emit onError(NetworkGeometry::ModelParseError, error);
    }

    QThread::currentThread()->setPriority(originalPriority);
}

NetworkGeometry::NetworkGeometry(const QUrl& url, bool delayLoad, const QVariantHash& mapping, const QUrl& textureBaseUrl) :
    _url(url),
    _mapping(mapping),
    _textureBaseUrl(textureBaseUrl.isValid() ? textureBaseUrl : url) {

    if (delayLoad) {
        _state = DelayState;
    } else {
        attemptRequestInternal();
    }
}

NetworkGeometry::~NetworkGeometry() {
    if (_resource) {
        _resource->deleteLater();
    }
}

void NetworkGeometry::attemptRequest() {
    if (_state == DelayState) {
        attemptRequestInternal();
    }
}

void NetworkGeometry::attemptRequestInternal() {
    if (_url.path().toLower().endsWith(".fst")) {
        _mappingUrl = _url;
        requestMapping(_url);
    } else {
        _modelUrl = _url;
        requestModel(_url);
    }
}

bool NetworkGeometry::isLoaded() const {
    return _state == SuccessState;
}

bool NetworkGeometry::isLoadedWithTextures() const {
    if (!isLoaded()) {
        return false;
    }

    if (!_isLoadedWithTextures) {
        for (auto&& material : _materials) {
            if ((material->diffuseTexture && !material->diffuseTexture->isLoaded()) ||
                (material->normalTexture && !material->normalTexture->isLoaded()) ||
                (material->specularTexture && !material->specularTexture->isLoaded()) ||
                (material->emissiveTexture && !material->emissiveTexture->isLoaded())) {
                return false;
            }
        }
        _isLoadedWithTextures = true;
    }
    return true;
}

void NetworkGeometry::setTextureWithNameToURL(const QString& name, const QUrl& url) {
    if (_meshes.size() > 0) {
        auto textureCache = DependencyManager::get<TextureCache>();
        for (auto&& material : _materials) {
            auto networkMaterial = material->_material;
            auto oldTextureMaps = networkMaterial->getTextureMaps();
            if (material->diffuseTextureName == name) {
                material->diffuseTexture = textureCache->getTexture(url, DEFAULT_TEXTURE);

                auto diffuseMap = model::TextureMapPointer(new model::TextureMap());
                diffuseMap->setTextureSource(material->diffuseTexture->_textureSource);
                diffuseMap->setTextureTransform(
                    oldTextureMaps[model::MaterialKey::DIFFUSE_MAP]->getTextureTransform());

                networkMaterial->setTextureMap(model::MaterialKey::DIFFUSE_MAP, diffuseMap);
            } else if (material->normalTextureName == name) {
                material->normalTexture = textureCache->getTexture(url);

                auto normalMap = model::TextureMapPointer(new model::TextureMap());
                normalMap->setTextureSource(material->normalTexture->_textureSource);

                networkMaterial->setTextureMap(model::MaterialKey::NORMAL_MAP, normalMap);
            } else if (material->specularTextureName == name) {
                material->specularTexture = textureCache->getTexture(url);

                auto glossMap = model::TextureMapPointer(new model::TextureMap());
                glossMap->setTextureSource(material->specularTexture->_textureSource);

                networkMaterial->setTextureMap(model::MaterialKey::GLOSS_MAP, glossMap);
            } else if (material->emissiveTextureName == name) {
                material->emissiveTexture = textureCache->getTexture(url);

                auto lightmapMap = model::TextureMapPointer(new model::TextureMap());
                lightmapMap->setTextureSource(material->emissiveTexture->_textureSource);
                lightmapMap->setTextureTransform(
                    oldTextureMaps[model::MaterialKey::LIGHTMAP_MAP]->getTextureTransform());
                glm::vec2 oldOffsetScale =
                    oldTextureMaps[model::MaterialKey::LIGHTMAP_MAP]->getLightmapOffsetScale();
                lightmapMap->setLightmapOffsetScale(oldOffsetScale.x, oldOffsetScale.y);

                networkMaterial->setTextureMap(model::MaterialKey::LIGHTMAP_MAP, lightmapMap);
            }
        }
    } else {
        qCWarning(modelnetworking) << "Ignoring setTextureWithNameToURL() geometry not ready." << name << url;
    }
    _isLoadedWithTextures = false;
}

QStringList NetworkGeometry::getTextureNames() const {
    QStringList result;
    for (auto&& material : _materials) {
        if (!material->diffuseTextureName.isEmpty() && material->diffuseTexture) {
            QString textureURL = material->diffuseTexture->getURL().toString();
            result << material->diffuseTextureName + ":\"" + textureURL + "\"";
        }

        if (!material->normalTextureName.isEmpty() && material->normalTexture) {
            QString textureURL = material->normalTexture->getURL().toString();
            result << material->normalTextureName + ":\"" + textureURL + "\"";
        }

        if (!material->specularTextureName.isEmpty() && material->specularTexture) {
            QString textureURL = material->specularTexture->getURL().toString();
            result << material->specularTextureName + ":\"" + textureURL + "\"";
        }

        if (!material->emissiveTextureName.isEmpty() && material->emissiveTexture) {
            QString textureURL = material->emissiveTexture->getURL().toString();
            result << material->emissiveTextureName + ":\"" + textureURL + "\"";
        }
    }

    return result;
}

void NetworkGeometry::requestMapping(const QUrl& url) {
    _state = RequestMappingState;
    if (_resource) {
        _resource->deleteLater();
    }
    _resource = new Resource(url, false);
    connect(_resource, &Resource::loaded, this, &NetworkGeometry::mappingRequestDone);
    connect(_resource, &Resource::failed, this, &NetworkGeometry::mappingRequestError);
}

void NetworkGeometry::requestModel(const QUrl& url) {
    _state = RequestModelState;
    if (_resource) {
        _resource->deleteLater();
    }
    _modelUrl = url;
    _resource = new Resource(url, false);
    connect(_resource, &Resource::loaded, this, &NetworkGeometry::modelRequestDone);
    connect(_resource, &Resource::failed, this, &NetworkGeometry::modelRequestError);
}

void NetworkGeometry::mappingRequestDone(const QByteArray& data) {
    assert(_state == RequestMappingState);

    // parse the mapping file
    _mapping = FSTReader::readMapping(data);

    QUrl replyUrl = _mappingUrl;
    QString modelUrlStr = _mapping.value("filename").toString();
    if (modelUrlStr.isNull()) {
        qCDebug(modelnetworking) << "Mapping file " << _url << "has no \"filename\" entry";
        emit onFailure(*this, MissingFilenameInMapping);
    } else {
        // read _textureBase from mapping file, if present
        QString texdir = _mapping.value("texdir").toString();
        if (!texdir.isNull()) {
            if (!texdir.endsWith('/')) {
                texdir += '/';
            }
            _textureBaseUrl = replyUrl.resolved(texdir);
        }

        _modelUrl = replyUrl.resolved(modelUrlStr);
        requestModel(_modelUrl);
    }
}

void NetworkGeometry::mappingRequestError(QNetworkReply::NetworkError error) {
    assert(_state == RequestMappingState);
    _state = ErrorState;
    emit onFailure(*this, MappingRequestError);
}

void NetworkGeometry::modelRequestDone(const QByteArray& data) {
    assert(_state == RequestModelState);

    _state = ParsingModelState;

    // asynchronously parse the model file.
    GeometryReader* geometryReader = new GeometryReader(_modelUrl, data, _mapping);
    connect(geometryReader, SIGNAL(onSuccess(FBXGeometry*)), SLOT(modelParseSuccess(FBXGeometry*)));
    connect(geometryReader, SIGNAL(onError(int, QString)), SLOT(modelParseError(int, QString)));

    QThreadPool::globalInstance()->start(geometryReader);
}

void NetworkGeometry::modelRequestError(QNetworkReply::NetworkError error) {
    assert(_state == RequestModelState);
    _state = ErrorState;
    emit onFailure(*this, ModelRequestError);
}

static NetworkMesh* buildNetworkMesh(const FBXMesh& mesh, const QUrl& textureBaseUrl) {
    NetworkMesh* networkMesh = new NetworkMesh();

    networkMesh->_mesh = mesh._mesh;

    return networkMesh;
}

static NetworkMaterial* buildNetworkMaterial(const FBXMaterial& material, const QUrl& textureBaseUrl) {
    auto textureCache = DependencyManager::get<TextureCache>();
    NetworkMaterial* networkMaterial = new NetworkMaterial();

    networkMaterial->_material = material._material;

    if (!material.diffuseTexture.filename.isEmpty()) {
        networkMaterial->diffuseTexture = textureCache->getTexture(textureBaseUrl.resolved(QUrl(material.diffuseTexture.filename)), DEFAULT_TEXTURE, material.diffuseTexture.content);
        networkMaterial->diffuseTextureName = material.diffuseTexture.name;

        auto diffuseMap = model::TextureMapPointer(new model::TextureMap());
        diffuseMap->setTextureSource(networkMaterial->diffuseTexture->_textureSource);
        diffuseMap->setTextureTransform(material.diffuseTexture.transform);

        material._material->setTextureMap(model::MaterialKey::DIFFUSE_MAP, diffuseMap);
    }
    if (!material.normalTexture.filename.isEmpty()) {
        networkMaterial->normalTexture = textureCache->getTexture(textureBaseUrl.resolved(QUrl(material.normalTexture.filename)), (material.normalTexture.isBumpmap ? BUMP_TEXTURE : NORMAL_TEXTURE), material.normalTexture.content);
        networkMaterial->normalTextureName = material.normalTexture.name;

        auto normalMap = model::TextureMapPointer(new model::TextureMap());
        normalMap->setTextureSource(networkMaterial->normalTexture->_textureSource);

        material._material->setTextureMap(model::MaterialKey::NORMAL_MAP, normalMap);
    }
    if (!material.specularTexture.filename.isEmpty()) {
        networkMaterial->specularTexture = textureCache->getTexture(textureBaseUrl.resolved(QUrl(material.specularTexture.filename)), SPECULAR_TEXTURE, material.specularTexture.content);
        networkMaterial->specularTextureName = material.specularTexture.name;

        auto glossMap = model::TextureMapPointer(new model::TextureMap());
        glossMap->setTextureSource(networkMaterial->specularTexture->_textureSource);

        material._material->setTextureMap(model::MaterialKey::GLOSS_MAP, glossMap);
    }
    if (!material.emissiveTexture.filename.isEmpty()) {
        networkMaterial->emissiveTexture = textureCache->getTexture(textureBaseUrl.resolved(QUrl(material.emissiveTexture.filename)), LIGHTMAP_TEXTURE, material.emissiveTexture.content);
        networkMaterial->emissiveTextureName = material.emissiveTexture.name;


        auto lightmapMap = model::TextureMapPointer(new model::TextureMap());
        lightmapMap->setTextureSource(networkMaterial->emissiveTexture->_textureSource);
        lightmapMap->setTextureTransform(material.emissiveTexture.transform);
        lightmapMap->setLightmapOffsetScale(material.emissiveParams.x, material.emissiveParams.y);

        material._material->setTextureMap(model::MaterialKey::LIGHTMAP_MAP, lightmapMap);
    }

    return networkMaterial;
}


void NetworkGeometry::modelParseSuccess(FBXGeometry* geometry) {
    // assume owner ship of geometry pointer
    _geometry.reset(geometry);



    foreach(const FBXMesh& mesh, _geometry->meshes) {
        _meshes.emplace_back(buildNetworkMesh(mesh, _textureBaseUrl));
    }

    QHash<QString, size_t> fbxMatIDToMatID;
    foreach(const FBXMaterial& material, _geometry->materials) {
        fbxMatIDToMatID[material.materialID] = _materials.size();
        _materials.emplace_back(buildNetworkMaterial(material, _textureBaseUrl));
    }


    int meshID = 0;
    foreach(const FBXMesh& mesh, _geometry->meshes) {
        int partID = 0;
        foreach (const FBXMeshPart& part, mesh.parts) {
            NetworkShape* networkShape = new NetworkShape();
            networkShape->_meshID = meshID;
            networkShape->_partID = partID;
            networkShape->_materialID = (int)fbxMatIDToMatID[part.materialID];
            _shapes.emplace_back(networkShape);
            partID++;
        }
        meshID++;
    }

    _state = SuccessState;
    emit onSuccess(*this, *_geometry.get());

    delete _resource;
    _resource = nullptr;
}

void NetworkGeometry::modelParseError(int error, QString str) {
    _state = ErrorState;
    emit onFailure(*this, (NetworkGeometry::Error)error);

    delete _resource;
    _resource = nullptr;
}

const NetworkMaterial* NetworkGeometry::getShapeMaterial(int shapeID) {
    if ((shapeID >= 0) && (shapeID < (int)_shapes.size())) {
        int materialID = _shapes[shapeID]->_materialID;
        if ((materialID >= 0) && ((unsigned int)materialID < _materials.size())) {
            return _materials[materialID].get();
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

