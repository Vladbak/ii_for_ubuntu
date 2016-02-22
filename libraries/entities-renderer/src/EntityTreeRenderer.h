//
//  EntityTreeRenderer.h
//  interface/src
//
//  Created by Brad Hefta-Gaub on 12/6/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_EntityTreeRenderer_h
#define hifi_EntityTreeRenderer_h

#include <QSet>
#include <QStack>

#include <AbstractAudioInterface.h>
#include <EntityScriptingInterface.h> // for RayToEntityIntersectionResult
#include <EntityTree.h>
#include <MouseEvent.h>
#include <OctreeRenderer.h>
#include <ScriptCache.h>
#include <TextureCache.h>

class AbstractScriptingServicesInterface;
class AbstractViewStateInterface;
class Model;
class ScriptEngine;
class ZoneEntityItem;


// Generic client side Octree renderer class.
class EntityTreeRenderer : public OctreeRenderer, public EntityItemFBXService, public Dependency {
    Q_OBJECT
public:
    EntityTreeRenderer(bool wantScripts, AbstractViewStateInterface* viewState,
                                AbstractScriptingServicesInterface* scriptingServices);
    virtual ~EntityTreeRenderer();

    virtual char getMyNodeType() const { return NodeType::EntityServer; }
    virtual PacketType getMyQueryMessageType() const { return PacketType::EntityQuery; }
    virtual PacketType getExpectedPacketType() const { return PacketType::EntityData; }
    virtual void setTree(OctreePointer newTree);

    void shutdown();
    void update();

    EntityTreePointer getTree() { return std::static_pointer_cast<EntityTree>(_tree); }

    void processEraseMessage(ReceivedMessage& message, const SharedNodePointer& sourceNode);

    virtual void init();

    virtual const FBXGeometry* getGeometryForEntity(EntityItemPointer entityItem);
    virtual const Model* getModelForEntityItem(EntityItemPointer entityItem);
    virtual const FBXGeometry* getCollisionGeometryForEntity(EntityItemPointer entityItem);
    
    /// clears the tree
    virtual void clear();

    /// reloads the entity scripts, calling unload and preload
    void reloadEntityScripts();

    /// if a renderable entity item needs a model, we will allocate it for them
    Q_INVOKABLE Model* allocateModel(const QString& url, const QString& collisionUrl);
    
    /// if a renderable entity item needs to update the URL of a model, we will handle that for the entity
    Q_INVOKABLE Model* updateModel(Model* original, const QString& newUrl, const QString& collisionUrl);

    /// if a renderable entity item is done with a model, it should return it to us
    void releaseModel(Model* model);
    
    void deleteReleasedModels();
    
    // event handles which may generate entity related events
    void mouseReleaseEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    /// connect our signals to anEntityScriptingInterface for firing of events related clicking,
    /// hovering over, and entering entities
    void connectSignalsToSlots(EntityScriptingInterface* entityScriptingInterface);

    // For Scene.shouldRenderEntities
    QList<EntityItemID>& getEntitiesLastInScene() { return _entityIDsLastInScene; }

signals:
    void mousePressOnEntity(const RayToEntityIntersectionResult& intersection, const QMouseEvent* event);
    void mousePressOffEntity(const RayToEntityIntersectionResult& intersection, const QMouseEvent* event);
    void mouseMoveOnEntity(const RayToEntityIntersectionResult& intersection, const QMouseEvent* event);
    void mouseReleaseOnEntity(const RayToEntityIntersectionResult& intersection, const QMouseEvent* event);

    void clickDownOnEntity(const EntityItemID& entityItemID, const MouseEvent& event);
    void holdingClickOnEntity(const EntityItemID& entityItemID, const MouseEvent& event);
    void clickReleaseOnEntity(const EntityItemID& entityItemID, const MouseEvent& event);

    void hoverEnterEntity(const EntityItemID& entityItemID, const MouseEvent& event);
    void hoverOverEntity(const EntityItemID& entityItemID, const MouseEvent& event);
    void hoverLeaveEntity(const EntityItemID& entityItemID, const MouseEvent& event);

    void enterEntity(const EntityItemID& entityItemID);
    void leaveEntity(const EntityItemID& entityItemID);
    void collisionWithEntity(const EntityItemID& idA, const EntityItemID& idB, const Collision& collision);

public slots:
    void addingEntity(const EntityItemID& entityID);
    void deletingEntity(const EntityItemID& entityID);
    void entitySciptChanging(const EntityItemID& entityID, const bool reload);
    void entityCollisionWithEntity(const EntityItemID& idA, const EntityItemID& idB, const Collision& collision);
    void updateEntityRenderStatus(bool shouldRenderEntities);

    // optional slots that can be wired to menu items
    void setDisplayModelBounds(bool value) { _displayModelBounds = value; }
    void setDontDoPrecisionPicking(bool value) { _dontDoPrecisionPicking = value; }

protected:
    virtual OctreePointer createTree() {
        EntityTreePointer newTree = EntityTreePointer(new EntityTree(true));
        newTree->createRootElement();
        return newTree;
    }

private:
    void addEntityToScene(EntityItemPointer entity);

    void applyZonePropertiesToScene(std::shared_ptr<ZoneEntityItem> zone);
    void checkAndCallPreload(const EntityItemID& entityID, const bool reload = false);

    QList<Model*> _releasedModels;
    RayToEntityIntersectionResult findRayIntersectionWorker(const PickRay& ray, Octree::lockType lockType,
                                                                bool precisionPicking, const QVector<EntityItemID>& entityIdsToInclude = QVector<EntityItemID>(),
                                                                const QVector<EntityItemID>& entityIdsToDiscard = QVector<EntityItemID>());

    EntityItemID _currentHoverOverEntityID;
    EntityItemID _currentClickingOnEntityID;

    QScriptValueList createEntityArgs(const EntityItemID& entityID);
    void checkEnterLeaveEntities();
    void leaveAllEntities();
    void forceRecheckEntities();

    glm::vec3 _lastAvatarPosition;
    QVector<EntityItemID> _currentEntitiesInside;

    bool _pendingSkyboxTexture { false };
    NetworkTexturePointer _skyboxTexture;

    bool _wantScripts;
    ScriptEngine* _entitiesScriptEngine;

    bool isCollisionOwner(const QUuid& myNodeID, EntityTreePointer entityTree,
                          const EntityItemID& id, const Collision& collision);

    void playEntityCollisionSound(const QUuid& myNodeID, EntityTreePointer entityTree,
                                  const EntityItemID& id, const Collision& collision);

    bool _lastMouseEventValid;
    MouseEvent _lastMouseEvent;
    AbstractViewStateInterface* _viewState;
    AbstractScriptingServicesInterface* _scriptingServices;
    bool _displayModelBounds;
    bool _dontDoPrecisionPicking;
    
    bool _shuttingDown { false };

    QMultiMap<QUrl, EntityItemID> _waitingOnPreload;

    bool _hasPreviousZone { false };
    std::shared_ptr<ZoneEntityItem> _bestZone;
    float _bestZoneVolume;

    glm::vec3 _previousKeyLightColor;
    float _previousKeyLightIntensity;
    float _previousKeyLightAmbientIntensity;
    glm::vec3 _previousKeyLightDirection;
    bool _previousStageSunModelEnabled;
    float _previousStageLongitude;
    float _previousStageLatitude;
    float _previousStageAltitude;
    float _previousStageHour;
    int _previousStageDay;
    
    QHash<EntityItemID, EntityItemPointer> _entitiesInScene;
    // For Scene.shouldRenderEntities
    QList<EntityItemID> _entityIDsLastInScene;
};


#endif // hifi_EntityTreeRenderer_h
