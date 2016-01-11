//
//  EntityTreeRenderer.cpp
//  interface/src
//
//  Created by Brad Hefta-Gaub on 12/6/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <glm/gtx/quaternion.hpp>

#include <QEventLoop>
#include <QScriptSyntaxCheckResult>

#include <ColorUtils.h>
#include <AbstractScriptingServicesInterface.h>
#include <AbstractViewStateInterface.h>
#include <DeferredLightingEffect.h>
#include <Model.h>
#include <NetworkAccessManager.h>
#include <PerfStat.h>
#include <SceneScriptingInterface.h>
#include <ScriptEngine.h>
#include <procedural/ProceduralSkybox.h>
#include <TextureCache.h>

#include "EntityTreeRenderer.h"

#include "RenderableEntityItem.h"

#include "RenderableBoxEntityItem.h"
#include "RenderableLightEntityItem.h"
#include "RenderableModelEntityItem.h"
#include "RenderableParticleEffectEntityItem.h"
#include "RenderableSphereEntityItem.h"
#include "RenderableTextEntityItem.h"
#include "RenderableWebEntityItem.h"
#include "RenderableZoneEntityItem.h"
#include "RenderableLineEntityItem.h"
#include "RenderablePolyVoxEntityItem.h"
#include "RenderablePolyLineEntityItem.h"
#include "EntitiesRendererLogging.h"
#include "AddressManager.h"
#include <Rig.h>

EntityTreeRenderer::EntityTreeRenderer(bool wantScripts, AbstractViewStateInterface* viewState,
                                            AbstractScriptingServicesInterface* scriptingServices) :
    OctreeRenderer(),
    _wantScripts(wantScripts),
    _entitiesScriptEngine(NULL),
    _lastMouseEventValid(false),
    _viewState(viewState),
    _scriptingServices(scriptingServices),
    _displayModelBounds(false),
    _dontDoPrecisionPicking(false)
{
    REGISTER_ENTITY_TYPE_WITH_FACTORY(Model, RenderableModelEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(Box, RenderableBoxEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(Sphere, RenderableSphereEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(Light, RenderableLightEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(Text, RenderableTextEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(Web, RenderableWebEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(ParticleEffect, RenderableParticleEffectEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(Zone, RenderableZoneEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(Line, RenderableLineEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(PolyVox, RenderablePolyVoxEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(PolyLine, RenderablePolyLineEntityItem::factory)
    
    _currentHoverOverEntityID = UNKNOWN_ENTITY_ID;
    _currentClickingOnEntityID = UNKNOWN_ENTITY_ID;
}

EntityTreeRenderer::~EntityTreeRenderer() {
    // NOTE: we don't need to delete _entitiesScriptEngine because it is registered with the application and has a
    // signal tied to call it's deleteLater on doneRunning
}

void EntityTreeRenderer::clear() {
    leaveAllEntities();
    _entitiesScriptEngine->unloadAllEntityScripts();

    auto scene = _viewState->getMain3DScene();
    render::PendingChanges pendingChanges;
    foreach(auto entity, _entitiesInScene) {
        entity->removeFromScene(entity, scene, pendingChanges);
    }
    scene->enqueuePendingChanges(pendingChanges);
    _entitiesInScene.clear();

    OctreeRenderer::clear();
}

void EntityTreeRenderer::reloadEntityScripts() {
    _entitiesScriptEngine->unloadAllEntityScripts();
    foreach(auto entity, _entitiesInScene) {
        if (!entity->getScript().isEmpty()) {
            _entitiesScriptEngine->loadEntityScript(entity->getEntityItemID(), entity->getScript(), true);
        }
    }
}

void EntityTreeRenderer::init() {
    OctreeRenderer::init();
    EntityTreePointer entityTree = std::static_pointer_cast<EntityTree>(_tree);
    entityTree->setFBXService(this);

    if (_wantScripts) {
        _entitiesScriptEngine = new ScriptEngine(NO_SCRIPT, "Entities");
        _scriptingServices->registerScriptEngineWithApplicationServices(_entitiesScriptEngine);
        _entitiesScriptEngine->runInThread();
        DependencyManager::get<EntityScriptingInterface>()->setEntitiesScriptEngine(_entitiesScriptEngine);
    }

    forceRecheckEntities(); // setup our state to force checking our inside/outsideness of entities

    connect(entityTree.get(), &EntityTree::deletingEntity, this, &EntityTreeRenderer::deletingEntity, Qt::QueuedConnection);
    connect(entityTree.get(), &EntityTree::addingEntity, this, &EntityTreeRenderer::addingEntity, Qt::QueuedConnection);
    connect(entityTree.get(), &EntityTree::entityScriptChanging,
            this, &EntityTreeRenderer::entitySciptChanging, Qt::QueuedConnection);
}

void EntityTreeRenderer::shutdown() {
    _entitiesScriptEngine->disconnectNonEssentialSignals(); // disconnect all slots/signals from the script engine, except essential
    _shuttingDown = true;
}

void EntityTreeRenderer::setTree(OctreePointer newTree) {
    OctreeRenderer::setTree(newTree);
    std::static_pointer_cast<EntityTree>(_tree)->setFBXService(this);
}

void EntityTreeRenderer::update() {
    if (_tree && !_shuttingDown) {
        EntityTreePointer tree = std::static_pointer_cast<EntityTree>(_tree);
        tree->update();

        // check to see if the avatar has moved and if we need to handle enter/leave entity logic
        checkEnterLeaveEntities();

        // even if we haven't changed positions, if we previously attempted to set the skybox, but
        // have a pending download of the skybox texture, then we should attempt to reapply to 
        // get the correct texture.
        if (_pendingSkyboxTextureDownload) {
            applyZonePropertiesToScene(_bestZone);
        }

        // Even if we're not moving the mouse, if we started clicking on an entity and we have
        // not yet released the hold then this is still considered a holdingClickOnEntity event
        // and we want to simulate this message here as well as in mouse move
        if (_lastMouseEventValid && !_currentClickingOnEntityID.isInvalidID()) {
            emit holdingClickOnEntity(_currentClickingOnEntityID, _lastMouseEvent);
            _entitiesScriptEngine->callEntityScriptMethod(_currentClickingOnEntityID, "holdingClickOnEntity", _lastMouseEvent);
        }

    }
    deleteReleasedModels();
}

void EntityTreeRenderer::checkEnterLeaveEntities() {
    if (_tree && !_shuttingDown) {
        glm::vec3 avatarPosition = _viewState->getAvatarPosition();

        if (avatarPosition != _lastAvatarPosition) {
            float radius = 1.0f; // for now, assume 1 meter radius
            QVector<EntityItemPointer> foundEntities;
            QVector<EntityItemID> entitiesContainingAvatar;
            
            // find the entities near us
            // don't let someone else change our tree while we search
            _tree->withReadLock([&] {
                std::static_pointer_cast<EntityTree>(_tree)->findEntities(avatarPosition, radius, foundEntities);

                // Whenever you're in an intersection between zones, we will always choose the smallest zone.
                _bestZone = NULL; // NOTE: Is this what we want?
                _bestZoneVolume = std::numeric_limits<float>::max();

                // create a list of entities that actually contain the avatar's position
                foreach(EntityItemPointer entity, foundEntities) {
                    if (entity->contains(avatarPosition)) {
                        entitiesContainingAvatar << entity->getEntityItemID();

                        // if this entity is a zone, use this time to determine the bestZone
                        if (entity->getType() == EntityTypes::Zone) {
                            float entityVolumeEstimate = entity->getVolumeEstimate();
                            if (entityVolumeEstimate < _bestZoneVolume) {
                                _bestZoneVolume = entityVolumeEstimate;
                                _bestZone = std::dynamic_pointer_cast<ZoneEntityItem>(entity);
                            } else if (entityVolumeEstimate == _bestZoneVolume) {
                                if (!_bestZone) {
                                    _bestZoneVolume = entityVolumeEstimate;
                                    _bestZone = std::dynamic_pointer_cast<ZoneEntityItem>(entity);
                                } else {
                                    // in the case of the volume being equal, we will use the
                                    // EntityItemID to deterministically pick one entity over the other
                                    if (entity->getEntityItemID() < _bestZone->getEntityItemID()) {
                                        _bestZoneVolume = entityVolumeEstimate;
                                        _bestZone = std::dynamic_pointer_cast<ZoneEntityItem>(entity);
                                    }
                                }
                            }

                        }
                    }
                }

                applyZonePropertiesToScene(_bestZone);

            });
            
            // Note: at this point we don't need to worry about the tree being locked, because we only deal with
            // EntityItemIDs from here. The callEntityScriptMethod() method is robust against attempting to call scripts
            // for entity IDs that no longer exist.

            // for all of our previous containing entities, if they are no longer containing then send them a leave event
            foreach(const EntityItemID& entityID, _currentEntitiesInside) {
                if (!entitiesContainingAvatar.contains(entityID)) {
                    emit leaveEntity(entityID);
                    _entitiesScriptEngine->callEntityScriptMethod(entityID, "leaveEntity");
                }
            }

            // for all of our new containing entities, if they weren't previously containing then send them an enter event
            foreach(const EntityItemID& entityID, entitiesContainingAvatar) {
                if (!_currentEntitiesInside.contains(entityID)) {
                    emit enterEntity(entityID);
                    _entitiesScriptEngine->callEntityScriptMethod(entityID, "enterEntity");
                }
            }
            _currentEntitiesInside = entitiesContainingAvatar;
            _lastAvatarPosition = avatarPosition;
        }
    }
}

void EntityTreeRenderer::leaveAllEntities() {
    if (_tree && !_shuttingDown) {

        // for all of our previous containing entities, if they are no longer containing then send them a leave event
        foreach(const EntityItemID& entityID, _currentEntitiesInside) {
            emit leaveEntity(entityID);
            _entitiesScriptEngine->callEntityScriptMethod(entityID, "leaveEntity");
        }
        _currentEntitiesInside.clear();
        forceRecheckEntities();
    }
}

void EntityTreeRenderer::forceRecheckEntities() {
    // make sure our "last avatar position" is something other than our current position, 
    // so that on our next chance, we'll check for enter/leave entity events.
    _lastAvatarPosition = _viewState->getAvatarPosition() + glm::vec3((float)TREE_SCALE);
}


void EntityTreeRenderer::applyZonePropertiesToScene(std::shared_ptr<ZoneEntityItem> zone) {
    QSharedPointer<SceneScriptingInterface> scene = DependencyManager::get<SceneScriptingInterface>();
    auto sceneStage = scene->getStage();
    auto sceneKeyLight = sceneStage->getKeyLight();
    auto sceneLocation = sceneStage->getLocation();
    auto sceneTime = sceneStage->getTime();

    if (zone) {
        if (!_hasPreviousZone) {
            _previousKeyLightColor = sceneKeyLight->getColor();
            _previousKeyLightIntensity = sceneKeyLight->getIntensity();
            _previousKeyLightAmbientIntensity = sceneKeyLight->getAmbientIntensity();
            _previousKeyLightDirection = sceneKeyLight->getDirection();
            _previousStageSunModelEnabled = sceneStage->isSunModelEnabled();
            _previousStageLongitude = sceneLocation->getLongitude();
            _previousStageLatitude = sceneLocation->getLatitude();
            _previousStageAltitude = sceneLocation->getAltitude();
            _previousStageHour = sceneTime->getHour();
            _previousStageDay = sceneTime->getDay();
            _hasPreviousZone = true;
        }
        sceneKeyLight->setColor(ColorUtils::toVec3(zone->getKeyLightProperties().getColor()));
        sceneKeyLight->setIntensity(zone->getKeyLightProperties().getIntensity());
        sceneKeyLight->setAmbientIntensity(zone->getKeyLightProperties().getAmbientIntensity());
        sceneKeyLight->setDirection(zone->getKeyLightProperties().getDirection());
        sceneStage->setSunModelEnable(zone->getStageProperties().getSunModelEnabled());
        sceneStage->setLocation(zone->getStageProperties().getLongitude(), zone->getStageProperties().getLatitude(),
                                zone->getStageProperties().getAltitude());
        sceneTime->setHour(zone->getStageProperties().calculateHour());
        sceneTime->setDay(zone->getStageProperties().calculateDay());
        
        if (zone->getBackgroundMode() == BACKGROUND_MODE_ATMOSPHERE) {
            EnvironmentData data = zone->getEnvironmentData();
            glm::vec3 keyLightDirection = sceneKeyLight->getDirection();
            glm::vec3 inverseKeyLightDirection = keyLightDirection * -1.0f;
            
            // NOTE: is this right? It seems like the "sun" should be based on the center of the
            //       atmosphere, not where the camera is.
            glm::vec3 keyLightLocation = _viewState->getAvatarPosition()
            + (inverseKeyLightDirection * data.getAtmosphereOuterRadius());
            
            data.setSunLocation(keyLightLocation);
            
            const float KEY_LIGHT_INTENSITY_TO_SUN_BRIGHTNESS_RATIO = 20.0f;
            float sunBrightness = sceneKeyLight->getIntensity() * KEY_LIGHT_INTENSITY_TO_SUN_BRIGHTNESS_RATIO;
            data.setSunBrightness(sunBrightness);
            
            _viewState->overrideEnvironmentData(data);
            scene->getSkyStage()->setBackgroundMode(model::SunSkyStage::SKY_DOME);
            _pendingSkyboxTextureDownload = false;

        } else {
            _viewState->endOverrideEnvironmentData();
            auto stage = scene->getSkyStage();
            if (zone->getBackgroundMode() == BACKGROUND_MODE_SKYBOX) {
                auto skybox = std::dynamic_pointer_cast<ProceduralSkybox>(stage->getSkybox());
                skybox->setColor(zone->getSkyboxProperties().getColorVec3());
                static QString userData;
                if (userData != zone->getUserData()) {
                    userData = zone->getUserData();
                    ProceduralPointer procedural(new Procedural(userData));
                    if (procedural->_enabled) {
                        skybox->setProcedural(procedural);
                    } else {
                        skybox->setProcedural(ProceduralPointer());
                    }
                }
                if (zone->getSkyboxProperties().getURL().isEmpty()) {
                    skybox->setCubemap(gpu::TexturePointer());
                    _pendingSkyboxTextureDownload = false;
                } else {
                    // Update the Texture of the Skybox with the one pointed by this zone
                    auto cubeMap = DependencyManager::get<TextureCache>()->getTexture(zone->getSkyboxProperties().getURL(), CUBE_TEXTURE);

                    if (cubeMap->getGPUTexture()) {
                        skybox->setCubemap(cubeMap->getGPUTexture());
                        _pendingSkyboxTextureDownload = false;
                    } else {
                        _pendingSkyboxTextureDownload = true;
                    }
                }
                stage->setBackgroundMode(model::SunSkyStage::SKY_BOX);
            } else {
                stage->setBackgroundMode(model::SunSkyStage::SKY_DOME); // let the application atmosphere through
                _pendingSkyboxTextureDownload = false;
            }
        }
    } else {
        _pendingSkyboxTextureDownload = false;
        if (_hasPreviousZone) {
            sceneKeyLight->setColor(_previousKeyLightColor);
            sceneKeyLight->setIntensity(_previousKeyLightIntensity);
            sceneKeyLight->setAmbientIntensity(_previousKeyLightAmbientIntensity);
            sceneKeyLight->setDirection(_previousKeyLightDirection);
            sceneStage->setSunModelEnable(_previousStageSunModelEnabled);
            sceneStage->setLocation(_previousStageLongitude, _previousStageLatitude,
                                    _previousStageAltitude);
            sceneTime->setHour(_previousStageHour);
            sceneTime->setDay(_previousStageDay);
            _hasPreviousZone = false;
        }
        _viewState->endOverrideEnvironmentData();
        scene->getSkyStage()->setBackgroundMode(model::SunSkyStage::SKY_DOME);  // let the application atmosphere through
    }
}

const FBXGeometry* EntityTreeRenderer::getGeometryForEntity(EntityItemPointer entityItem) {
    const FBXGeometry* result = NULL;
    
    if (entityItem->getType() == EntityTypes::Model) {
        std::shared_ptr<RenderableModelEntityItem> modelEntityItem =
                                                        std::dynamic_pointer_cast<RenderableModelEntityItem>(entityItem);
        assert(modelEntityItem); // we need this!!!
        Model* model = modelEntityItem->getModel(this);
        if (model) {
            result = &model->getGeometry()->getFBXGeometry();
        }
    }
    return result;
}

const Model* EntityTreeRenderer::getModelForEntityItem(EntityItemPointer entityItem) {
    const Model* result = NULL;
    if (entityItem->getType() == EntityTypes::Model) {
        std::shared_ptr<RenderableModelEntityItem> modelEntityItem =
                                                        std::dynamic_pointer_cast<RenderableModelEntityItem>(entityItem);
        result = modelEntityItem->getModel(this);
    }
    return result;
}

const FBXGeometry* EntityTreeRenderer::getCollisionGeometryForEntity(EntityItemPointer entityItem) {
    const FBXGeometry* result = NULL;
    
    if (entityItem->getType() == EntityTypes::Model) {
        std::shared_ptr<RenderableModelEntityItem> modelEntityItem =
                                                        std::dynamic_pointer_cast<RenderableModelEntityItem>(entityItem);
        if (modelEntityItem->hasCompoundShapeURL()) {
            Model* model = modelEntityItem->getModel(this);
            if (model) {
                const QSharedPointer<NetworkGeometry> collisionNetworkGeometry = model->getCollisionGeometry();
                if (collisionNetworkGeometry && collisionNetworkGeometry->isLoaded()) {
                    result = &collisionNetworkGeometry->getFBXGeometry();
                }
            }
        }
    }
    return result;
}

float EntityTreeRenderer::getSizeScale() const {
    return _viewState->getSizeScale();
}

int EntityTreeRenderer::getBoundaryLevelAdjust() const {
    return _viewState->getBoundaryLevelAdjust();
}


void EntityTreeRenderer::processEraseMessage(ReceivedMessage& message, const SharedNodePointer& sourceNode) {
    std::static_pointer_cast<EntityTree>(_tree)->processEraseMessage(message, sourceNode);
}

Model* EntityTreeRenderer::allocateModel(const QString& url, const QString& collisionUrl) {
    Model* model = NULL;
    // Make sure we only create and delete models on the thread that owns the EntityTreeRenderer
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "allocateModel", Qt::BlockingQueuedConnection,
                Q_RETURN_ARG(Model*, model),
                Q_ARG(const QString&, url));

        return model;
    }
    model = new Model(std::make_shared<Rig>());
    model->init();
    model->setURL(QUrl(url));
    model->setCollisionModelURL(QUrl(collisionUrl));
    return model;
}

Model* EntityTreeRenderer::updateModel(Model* original, const QString& newUrl, const QString& collisionUrl) {
    Model* model = NULL;

    // The caller shouldn't call us if the URL doesn't need to change. But if they
    // do, we just return their original back to them.
    if (!original || (QUrl(newUrl) == original->getURL())) {
        return original;
    }

    // Before we do any creating or deleting, make sure we're on our renderer thread
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "updateModel", Qt::BlockingQueuedConnection,
                Q_RETURN_ARG(Model*, model),
                Q_ARG(Model*, original),
                Q_ARG(const QString&, newUrl));

        return model;
    }

    // at this point we know we need to replace the model, and we know we're on the
    // correct thread, so we can do all our work.
    if (original) {
        delete original; // delete the old model...
    }

    // create the model and correctly initialize it with the new url
    model = new Model(std::make_shared<Rig>());
    model->init();
    model->setURL(QUrl(newUrl));
    model->setCollisionModelURL(QUrl(collisionUrl));
        
    return model;
}

void EntityTreeRenderer::releaseModel(Model* model) {
    // If we're not on the renderer's thread, then remember this model to be deleted later
    if (QThread::currentThread() != thread()) {
        _releasedModels << model;
    } else { // otherwise just delete it right away
        delete model;
    }
}

void EntityTreeRenderer::deleteReleasedModels() {
    if (_releasedModels.size() > 0) {
        foreach(Model* model, _releasedModels) {
            delete model;
        }
        _releasedModels.clear();
    }
}

RayToEntityIntersectionResult EntityTreeRenderer::findRayIntersectionWorker(const PickRay& ray, Octree::lockType lockType,
                                                                                    bool precisionPicking, const QVector<EntityItemID>& entityIdsToInclude,
                                                                                    const QVector<EntityItemID>& entityIdsToDiscard) {
    RayToEntityIntersectionResult result;
    if (_tree) {
        EntityTreePointer entityTree = std::static_pointer_cast<EntityTree>(_tree);

        OctreeElementPointer element;
        EntityItemPointer intersectedEntity = NULL;
        result.intersects = entityTree->findRayIntersection(ray.origin, ray.direction, element, result.distance, 
                                                            result.face, result.surfaceNormal, entityIdsToInclude, entityIdsToDiscard,
                                                            (void**)&intersectedEntity, lockType, &result.accurate,
                                                            precisionPicking);
        if (result.intersects && intersectedEntity) {
            result.entityID = intersectedEntity->getEntityItemID();
            result.properties = intersectedEntity->getProperties();
            result.intersection = ray.origin + (ray.direction * result.distance);
            result.entity = intersectedEntity;
        }
    }
    return result;
}

void EntityTreeRenderer::connectSignalsToSlots(EntityScriptingInterface* entityScriptingInterface) {
    connect(this, &EntityTreeRenderer::mousePressOnEntity, entityScriptingInterface,
        [=](const RayToEntityIntersectionResult& intersection, const QMouseEvent* event, unsigned int deviceId){
        entityScriptingInterface->mousePressOnEntity(intersection.entityID, MouseEvent(*event, deviceId));
    });
    connect(this, &EntityTreeRenderer::mouseMoveOnEntity, entityScriptingInterface,
        [=](const RayToEntityIntersectionResult& intersection, const QMouseEvent* event, unsigned int deviceId) {
        entityScriptingInterface->mouseMoveOnEntity(intersection.entityID, MouseEvent(*event, deviceId));
    });
    connect(this, &EntityTreeRenderer::mouseReleaseOnEntity, entityScriptingInterface,
        [=](const RayToEntityIntersectionResult& intersection, const QMouseEvent* event, unsigned int deviceId) {
        entityScriptingInterface->mouseReleaseOnEntity(intersection.entityID, MouseEvent(*event, deviceId));
    });

    connect(this, &EntityTreeRenderer::clickDownOnEntity, entityScriptingInterface, &EntityScriptingInterface::clickDownOnEntity);
    connect(this, &EntityTreeRenderer::holdingClickOnEntity, entityScriptingInterface, &EntityScriptingInterface::holdingClickOnEntity);
    connect(this, &EntityTreeRenderer::clickReleaseOnEntity, entityScriptingInterface, &EntityScriptingInterface::clickReleaseOnEntity);

    connect(this, &EntityTreeRenderer::hoverEnterEntity, entityScriptingInterface, &EntityScriptingInterface::hoverEnterEntity);
    connect(this, &EntityTreeRenderer::hoverOverEntity, entityScriptingInterface, &EntityScriptingInterface::hoverOverEntity);
    connect(this, &EntityTreeRenderer::hoverLeaveEntity, entityScriptingInterface, &EntityScriptingInterface::hoverLeaveEntity);

    connect(this, &EntityTreeRenderer::enterEntity, entityScriptingInterface, &EntityScriptingInterface::enterEntity);
    connect(this, &EntityTreeRenderer::leaveEntity, entityScriptingInterface, &EntityScriptingInterface::leaveEntity);
    connect(this, &EntityTreeRenderer::collisionWithEntity, entityScriptingInterface, &EntityScriptingInterface::collisionWithEntity);

    connect(DependencyManager::get<SceneScriptingInterface>().data(), &SceneScriptingInterface::shouldRenderEntitiesChanged, this, &EntityTreeRenderer::updateEntityRenderStatus, Qt::QueuedConnection);
}

void EntityTreeRenderer::mousePressEvent(QMouseEvent* event, unsigned int deviceID) {
    // If we don't have a tree, or we're in the process of shutting down, then don't
    // process these events.
    if (!_tree || _shuttingDown) {
        return;
    }
    PerformanceTimer perfTimer("EntityTreeRenderer::mousePressEvent");
    PickRay ray = _viewState->computePickRay(event->x(), event->y());

    bool precisionPicking = !_dontDoPrecisionPicking;
    RayToEntityIntersectionResult rayPickResult = findRayIntersectionWorker(ray, Octree::Lock, precisionPicking);
    if (rayPickResult.intersects) {
        //qCDebug(entitiesrenderer) << "mousePressEvent over entity:" << rayPickResult.entityID;

        QString urlString = rayPickResult.properties.getHref();
        QUrl url = QUrl(urlString, QUrl::StrictMode);
        if (url.isValid() && !url.isEmpty()){
            DependencyManager::get<AddressManager>()->handleLookupString(urlString);

        }

        emit mousePressOnEntity(rayPickResult, event, deviceID);
        _entitiesScriptEngine->callEntityScriptMethod(rayPickResult.entityID, "mousePressOnEntity", MouseEvent(*event, deviceID));
    
        _currentClickingOnEntityID = rayPickResult.entityID;
        emit clickDownOnEntity(_currentClickingOnEntityID, MouseEvent(*event, deviceID));
        _entitiesScriptEngine->callEntityScriptMethod(_currentClickingOnEntityID, "clickDownOnEntity", MouseEvent(*event, deviceID));
    } else {
        emit mousePressOffEntity(rayPickResult, event, deviceID);
    }
    _lastMouseEvent = MouseEvent(*event, deviceID);
    _lastMouseEventValid = true;
}

void EntityTreeRenderer::mouseReleaseEvent(QMouseEvent* event, unsigned int deviceID) {
    // If we don't have a tree, or we're in the process of shutting down, then don't
    // process these events.
    if (!_tree || _shuttingDown) {
        return;
    }
    PerformanceTimer perfTimer("EntityTreeRenderer::mouseReleaseEvent");
    PickRay ray = _viewState->computePickRay(event->x(), event->y());
    bool precisionPicking = !_dontDoPrecisionPicking;
    RayToEntityIntersectionResult rayPickResult = findRayIntersectionWorker(ray, Octree::Lock, precisionPicking);
    if (rayPickResult.intersects) {
        //qCDebug(entitiesrenderer) << "mouseReleaseEvent over entity:" << rayPickResult.entityID;
        emit mouseReleaseOnEntity(rayPickResult, event, deviceID);
        _entitiesScriptEngine->callEntityScriptMethod(rayPickResult.entityID, "mouseReleaseOnEntity", MouseEvent(*event, deviceID));
    }

    // Even if we're no longer intersecting with an entity, if we started clicking on it, and now
    // we're releasing the button, then this is considered a clickOn event
    if (!_currentClickingOnEntityID.isInvalidID()) {
        emit clickReleaseOnEntity(_currentClickingOnEntityID, MouseEvent(*event, deviceID));
        _entitiesScriptEngine->callEntityScriptMethod(rayPickResult.entityID, "clickReleaseOnEntity", MouseEvent(*event, deviceID));
    }

    // makes it the unknown ID, we just released so we can't be clicking on anything
    _currentClickingOnEntityID = UNKNOWN_ENTITY_ID;
    _lastMouseEvent = MouseEvent(*event, deviceID);
    _lastMouseEventValid = true;
}

void EntityTreeRenderer::mouseMoveEvent(QMouseEvent* event, unsigned int deviceID) {
    // If we don't have a tree, or we're in the process of shutting down, then don't
    // process these events.
    if (!_tree || _shuttingDown) {
        return;
    }
    PerformanceTimer perfTimer("EntityTreeRenderer::mouseMoveEvent");

    PickRay ray = _viewState->computePickRay(event->x(), event->y());

    bool precisionPicking = false; // for mouse moves we do not do precision picking
    RayToEntityIntersectionResult rayPickResult = findRayIntersectionWorker(ray, Octree::TryLock, precisionPicking);
    if (rayPickResult.intersects) {

        _entitiesScriptEngine->callEntityScriptMethod(rayPickResult.entityID, "mouseMoveEvent", MouseEvent(*event, deviceID));
        _entitiesScriptEngine->callEntityScriptMethod(rayPickResult.entityID, "mouseMoveOnEntity", MouseEvent(*event, deviceID));
    
        // handle the hover logic...
    
        // if we were previously hovering over an entity, and this new entity is not the same as our previous entity
        // then we need to send the hover leave.
        if (!_currentHoverOverEntityID.isInvalidID() && rayPickResult.entityID != _currentHoverOverEntityID) {
            emit hoverLeaveEntity(_currentHoverOverEntityID, MouseEvent(*event, deviceID));
            _entitiesScriptEngine->callEntityScriptMethod(_currentHoverOverEntityID, "hoverLeaveEntity", MouseEvent(*event, deviceID));
        }

        // If the new hover entity does not match the previous hover entity then we are entering the new one
        // this is true if the _currentHoverOverEntityID is known or unknown
        if (rayPickResult.entityID != _currentHoverOverEntityID) {
            _entitiesScriptEngine->callEntityScriptMethod(rayPickResult.entityID, "hoverEnterEntity", MouseEvent(*event, deviceID));
        }

        // and finally, no matter what, if we're intersecting an entity then we're definitely hovering over it, and
        // we should send our hover over event
        emit hoverOverEntity(rayPickResult.entityID, MouseEvent(*event, deviceID));
        _entitiesScriptEngine->callEntityScriptMethod(rayPickResult.entityID, "hoverOverEntity", MouseEvent(*event, deviceID));

        // remember what we're hovering over
        _currentHoverOverEntityID = rayPickResult.entityID;

    } else {
        // handle the hover logic...
        // if we were previously hovering over an entity, and we're no longer hovering over any entity then we need to
        // send the hover leave for our previous entity
        if (!_currentHoverOverEntityID.isInvalidID()) {
            emit hoverLeaveEntity(_currentHoverOverEntityID, MouseEvent(*event, deviceID));
            _entitiesScriptEngine->callEntityScriptMethod(_currentHoverOverEntityID, "hoverLeaveEntity", MouseEvent(*event, deviceID));
            _currentHoverOverEntityID = UNKNOWN_ENTITY_ID; // makes it the unknown ID
        }
    }

    // Even if we're no longer intersecting with an entity, if we started clicking on an entity and we have
    // not yet released the hold then this is still considered a holdingClickOnEntity event
    if (!_currentClickingOnEntityID.isInvalidID()) {
        emit holdingClickOnEntity(_currentClickingOnEntityID, MouseEvent(*event, deviceID));
        _entitiesScriptEngine->callEntityScriptMethod(_currentClickingOnEntityID, "holdingClickOnEntity", MouseEvent(*event, deviceID));
    }
    _lastMouseEvent = MouseEvent(*event, deviceID);
    _lastMouseEventValid = true;
}

void EntityTreeRenderer::deletingEntity(const EntityItemID& entityID) {
    if (_tree && !_shuttingDown) {
        _entitiesScriptEngine->unloadEntityScript(entityID);
    }

    forceRecheckEntities(); // reset our state to force checking our inside/outsideness of entities

    // here's where we remove the entity payload from the scene
    if (_entitiesInScene.contains(entityID)) {
        auto entity = _entitiesInScene.take(entityID);
        render::PendingChanges pendingChanges;
        auto scene = _viewState->getMain3DScene();
        entity->removeFromScene(entity, scene, pendingChanges);
        scene->enqueuePendingChanges(pendingChanges);
    }
}

void EntityTreeRenderer::addingEntity(const EntityItemID& entityID) {
    forceRecheckEntities(); // reset our state to force checking our inside/outsideness of entities
    checkAndCallPreload(entityID);
    auto entity = std::static_pointer_cast<EntityTree>(_tree)->findEntityByID(entityID);
    if (entity) {
        addEntityToScene(entity);
    }
}

void EntityTreeRenderer::addEntityToScene(EntityItemPointer entity) {
    // here's where we add the entity payload to the scene
    render::PendingChanges pendingChanges;
    auto scene = _viewState->getMain3DScene();
    if (entity->addToScene(entity, scene, pendingChanges)) {
        _entitiesInScene.insert(entity->getEntityItemID(), entity);
    }
    scene->enqueuePendingChanges(pendingChanges);
}


void EntityTreeRenderer::entitySciptChanging(const EntityItemID& entityID, const bool reload) {
    if (_tree && !_shuttingDown) {
        _entitiesScriptEngine->unloadEntityScript(entityID);
        checkAndCallPreload(entityID, reload);
    }
}

void EntityTreeRenderer::checkAndCallPreload(const EntityItemID& entityID, const bool reload) {
    if (_tree && !_shuttingDown) {
        EntityItemPointer entity = getTree()->findEntityByEntityItemID(entityID);
        if (entity && !entity->getScript().isEmpty()) {
            QString scriptUrl = entity->getScript();
            scriptUrl = ResourceManager::normalizeURL(scriptUrl);
            _entitiesScriptEngine->loadEntityScript(entityID, scriptUrl, reload);
        }
    }
}

bool EntityTreeRenderer::isCollisionOwner(const QUuid& myNodeID, EntityTreePointer entityTree,
    const EntityItemID& id, const Collision& collision) {
    EntityItemPointer entity = entityTree->findEntityByEntityItemID(id);
    if (!entity) {
        return false;
    }
    QUuid simulatorID = entity->getSimulatorID();
    if (simulatorID.isNull()) {
        // Can be null if it has never moved since being created or coming out of persistence.
        // However, for there to be a collission, one of the two objects must be moving.
        const EntityItemID& otherID = (id == collision.idA) ? collision.idB : collision.idA;
        EntityItemPointer otherEntity = entityTree->findEntityByEntityItemID(otherID);
        if (!otherEntity) {
            return false;
        }
        simulatorID = otherEntity->getSimulatorID();
    }

    if (simulatorID.isNull() || (simulatorID != myNodeID)) {
        return false;
    }

    return true;
}

void EntityTreeRenderer::playEntityCollisionSound(const QUuid& myNodeID, EntityTreePointer entityTree,
                                                  const EntityItemID& id, const Collision& collision) {

    if (!isCollisionOwner(myNodeID, entityTree, id, collision)) {
        return;
    }

    EntityItemPointer entity = entityTree->findEntityByEntityItemID(id);
    if (!entity) {
        return;
    }

    const QString& collisionSoundURL = entity->getCollisionSoundURL();
    if (collisionSoundURL.isEmpty()) {
        return;
    }
    const float mass = entity->computeMass();
    const float COLLISION_PENETRATION_TO_VELOCITY = 50; // as a subsitute for RELATIVE entity->getVelocity()
    // The collision.penetration is a pretty good indicator of changed velocity AFTER the initial contact,
    // but that first contact depends on exactly where we hit in the physics step.
    // We can get a more consistent initial-contact energy reading by using the changed velocity.
    // Note that velocityChange is not a good indicator for continuing collisions, because it does not distinguish
    // between bounce and sliding along a surface.
    const float linearVelocity = (collision.type == CONTACT_EVENT_TYPE_START) ?
        glm::length(collision.velocityChange) :
        glm::length(collision.penetration) * COLLISION_PENETRATION_TO_VELOCITY;
    const float energy = mass * linearVelocity * linearVelocity / 2.0f;
    const glm::vec3 position = collision.contactPoint;
    const float COLLISION_ENERGY_AT_FULL_VOLUME = (collision.type == CONTACT_EVENT_TYPE_START) ? 150.0f : 5.0f;
    const float COLLISION_MINIMUM_VOLUME = 0.005f;
    const float energyFactorOfFull = fmin(1.0f, energy / COLLISION_ENERGY_AT_FULL_VOLUME);
    if (energyFactorOfFull < COLLISION_MINIMUM_VOLUME) {
        return;
    }
    // Quiet sound aren't really heard at all, so we can compress everything to the range [1-c, 1], if we play it all.
    const float COLLISION_SOUND_COMPRESSION_RANGE = 1.0f; // This section could be removed when the value is 1, but let's see how it goes.
    const float volume = (energyFactorOfFull * COLLISION_SOUND_COMPRESSION_RANGE) + (1.0f - COLLISION_SOUND_COMPRESSION_RANGE);


    // Shift the pitch down by ln(1 + (size / COLLISION_SIZE_FOR_STANDARD_PITCH)) / ln(2)
    const float COLLISION_SIZE_FOR_STANDARD_PITCH = 0.2f;
    bool success;
    auto minAACube = entity->getMinimumAACube(success);
    if (!success) {
        return;
    }
    const float stretchFactor = log(1.0f + (minAACube.getLargestDimension() / COLLISION_SIZE_FOR_STANDARD_PITCH)) / log(2);
    AudioInjector::playSound(collisionSoundURL, volume, stretchFactor, position);
}

void EntityTreeRenderer::entityCollisionWithEntity(const EntityItemID& idA, const EntityItemID& idB,
                                                    const Collision& collision) {
    // If we don't have a tree, or we're in the process of shutting down, then don't
    // process these events.
    if (!_tree || _shuttingDown) {
        return;
    }
    // Don't respond to small continuous contacts.
    const float COLLISION_MINUMUM_PENETRATION = 0.002f;
    if ((collision.type == CONTACT_EVENT_TYPE_CONTINUE) && (glm::length(collision.penetration) < COLLISION_MINUMUM_PENETRATION)) {
        return;
    }

    // See if we should play sounds
    EntityTreePointer entityTree = std::static_pointer_cast<EntityTree>(_tree);
    const QUuid& myNodeID = DependencyManager::get<NodeList>()->getSessionUUID();
    playEntityCollisionSound(myNodeID, entityTree, idA, collision);
    playEntityCollisionSound(myNodeID, entityTree, idB, collision);

    // And now the entity scripts
    if (isCollisionOwner(myNodeID, entityTree, idA, collision)) {
        emit collisionWithEntity(idA, idB, collision);
        _entitiesScriptEngine->callEntityScriptMethod(idA, "collisionWithEntity", idB, collision);
    }

    if (isCollisionOwner(myNodeID, entityTree, idA, collision)) {
        emit collisionWithEntity(idB, idA, collision);
        _entitiesScriptEngine->callEntityScriptMethod(idB, "collisionWithEntity", idA, collision);
    }
}

void EntityTreeRenderer::updateEntityRenderStatus(bool shouldRenderEntities) {
    if (DependencyManager::get<SceneScriptingInterface>()->shouldRenderEntities()) {
        for (auto entityID : _entityIDsLastInScene) {
            addingEntity(entityID);
        }
        _entityIDsLastInScene.clear();
    } else {
        _entityIDsLastInScene = _entitiesInScene.keys();
        for (auto entityID : _entityIDsLastInScene) {
            // FIXME - is this really right? do we want to do the deletingEntity() code or just remove from the scene.
            deletingEntity(entityID);
        }
    }
}
