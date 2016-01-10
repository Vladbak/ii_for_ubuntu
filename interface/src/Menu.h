//
//  Menu.h
//  interface/src
//
//  Created by Stephen Birarda on 8/12/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_Menu_h
#define hifi_Menu_h

#include <QDir>
#include <QMenuBar>
#include <QHash>
#include <QKeySequence>
#include <QPointer>
#include <QStandardPaths>

#include <MenuItemProperties.h>

#include "DiscoverabilityManager.h"

class Settings;

class MenuWrapper : public QObject {
public:
    QList<QAction*> actions();
    MenuWrapper* addMenu(const QString& menuName);
    void setEnabled(bool enabled = true);
    QAction* addSeparator();
    void addAction(QAction* action);

    QAction* addAction(const QString& menuName);
    void insertAction(QAction* before, QAction* menuName);

    QAction* addAction(const QString& menuName, const QObject* receiver, const char* member, const QKeySequence& shortcut = 0);
    void removeAction(QAction* action);

    QAction* newAction() {
        return new QAction(_realMenu);
    }
private:
    MenuWrapper(QMenu* menu);

    static MenuWrapper* fromMenu(QMenu* menu) {
        return _backMap[menu];
    }

    QMenu* const _realMenu;
    static QHash<QMenu*, MenuWrapper*> _backMap;
    friend class Menu;
};

class Menu : public QMenuBar {
    Q_OBJECT
public:
    Menu();
    static Menu* getInstance();

    void loadSettings();
    void saveSettings();

    MenuWrapper* getMenu(const QString& menuName);
    MenuWrapper* getSubMenuFromName(const QString& menuName, MenuWrapper* menu);

    void triggerOption(const QString& menuOption);
    QAction* getActionForOption(const QString& menuOption);

    QAction* addActionToQMenuAndActionHash(MenuWrapper* destinationMenu,
                                           const QString& actionName,
                                           const QKeySequence& shortcut = 0,
                                           const QObject* receiver = NULL,
                                           const char* member = NULL,
                                           QAction::MenuRole role = QAction::NoRole,
                                           int menuItemLocation = UNSPECIFIED_POSITION,
                                           const QString& grouping = QString());

    QAction* addActionToQMenuAndActionHash(MenuWrapper* destinationMenu,
                                           QAction* action,
                                           const QString& actionName = QString(),
                                           const QKeySequence& shortcut = 0,
                                           QAction::MenuRole role = QAction::NoRole,
                                           int menuItemLocation = UNSPECIFIED_POSITION,
                                           const QString& grouping = QString());

    QAction* addCheckableActionToQMenuAndActionHash(MenuWrapper* destinationMenu,
                                                    const QString& actionName,
                                                    const QKeySequence& shortcut = 0,
                                                    const bool checked = false,
                                                    const QObject* receiver = NULL,
                                                    const char* member = NULL,
                                                    int menuItemLocation = UNSPECIFIED_POSITION,
                                                    const QString& grouping = QString());

    void removeAction(MenuWrapper* menu, const QString& actionName);

public slots:
    MenuWrapper* addMenu(const QString& menuName, const QString& grouping = QString());
    void removeMenu(const QString& menuName);
    bool menuExists(const QString& menuName);
    void addSeparator(const QString& menuName, const QString& separatorName, const QString& grouping = QString());
    void removeSeparator(const QString& menuName, const QString& separatorName);
    void addMenuItem(const MenuItemProperties& properties);
    void removeMenuItem(const QString& menuName, const QString& menuitem);
    bool menuItemExists(const QString& menuName, const QString& menuitem);
    void addActionGroup(const QString& groupName, const QStringList& actionList, const QString& selected = QString());
    void removeActionGroup(const QString& groupName);
    bool isOptionChecked(const QString& menuOption) const;
    void setIsOptionChecked(const QString& menuOption, bool isChecked);

    bool getGroupingIsVisible(const QString& grouping);
    void setGroupingIsVisible(const QString& grouping, bool isVisible); /// NOTE: the "" grouping is always visible

    void toggleDeveloperMenus();
    void toggleAdvancedMenus();

private:
    typedef void(*settingsAction)(Settings&, QAction&);
    static void loadAction(Settings& settings, QAction& action);
    static void saveAction(Settings& settings, QAction& action);
    void scanMenuBar(settingsAction modifySetting);
    void scanMenu(QMenu& menu, settingsAction modifySetting, Settings& settings);

    /// helper method to have separators with labels that are also compatible with OS X
    void addDisabledActionAndSeparator(MenuWrapper* destinationMenu, 
                                       const QString& actionName,
                                       int menuItemLocation = UNSPECIFIED_POSITION, 
                                       const QString& grouping = QString());

    QAction* getActionFromName(const QString& menuName, MenuWrapper* menu);
    MenuWrapper* getMenuParent(const QString& menuName, QString& finalMenuPart);

    QAction* getMenuAction(const QString& menuName);
    int findPositionOfMenuItem(MenuWrapper* menu, const QString& searchMenuItem);
    int positionBeforeSeparatorIfNeeded(MenuWrapper* menu, int requestedPosition);

    QHash<QString, QAction*> _actionHash;

    bool isValidGrouping(const QString& grouping) const { return grouping == "Advanced" || grouping == "Developer"; }
    QHash<QString, bool> _groupingVisible;
    QHash<QString, QSet<QAction*>> _groupingActions;
};

namespace MenuOption {
    const QString AboutApp = "About Interface";
    const QString AddRemoveFriends = "Add/Remove Friends...";
    const QString AddressBar = "Show Address Bar";
    const QString Animations = "Animations...";
    const QString AnimDebugDrawAnimPose = "Debug Draw Animation";
    const QString AnimDebugDrawDefaultPose = "Debug Draw Default Pose";
    const QString AnimDebugDrawPosition= "Debug Draw Position";
    const QString Antialiasing = "Antialiasing";
    const QString AssetMigration = "ATP Asset Migration";
    const QString Atmosphere = "Atmosphere";
    const QString Attachments = "Attachments...";
    const QString AudioNoiseReduction = "Audio Noise Reduction";
    const QString AudioScope = "Show Scope";
    const QString AudioScopeFiftyFrames = "Fifty";
    const QString AudioScopeFiveFrames = "Five";
    const QString AudioScopeFrames = "Display Frames";
    const QString AudioScopePause = "Pause Scope";
    const QString AudioScopeTwentyFrames = "Twenty";
    const QString AudioNetworkStats = "Audio Network Stats";
    const QString AudioStatsShowInjectedStreams = "Audio Stats Show Injected Streams";
    const QString AutoMuteAudio = "Auto Mute Microphone";
    const QString AvatarReceiveStats = "Show Receive Stats";
    const QString Back = "Back";
    const QString BandwidthDetails = "Bandwidth Details";
    const QString BinaryEyelidControl = "Binary Eyelid Control";
    const QString BookmarkLocation = "Bookmark Location";
    const QString Bookmarks = "Bookmarks";
    const QString CachesSize = "RAM Caches Size";
    const QString CalibrateCamera = "Calibrate Camera";
    const QString CameraEntityMode = "Entity Mode";
    const QString CenterPlayerInView = "Center Player In View";
    const QString Chat = "Chat...";
    const QString Collisions = "Collisions";
    const QString ComfortMode = "Comfort Mode";
    const QString Connexion = "Activate 3D Connexion Devices";
    const QString Console = "Console...";
    const QString ControlWithSpeech = "Control With Speech";
    const QString CopyAddress = "Copy Address to Clipboard";
    const QString CopyPath = "Copy Path to Clipboard";
    const QString CoupleEyelids = "Couple Eyelids";
    const QString CrashInterface = "Crash Interface";
    const QString DebugAmbientOcclusion = "Debug Ambient Occlusion";
    const QString DecreaseAvatarSize = "Decrease Avatar Size";
    const QString DeleteBookmark = "Delete Bookmark...";
    const QString DisableActivityLogger = "Disable Activity Logger";
    const QString DisableEyelidAdjustment = "Disable Eyelid Adjustment";
    const QString DisableLightEntities = "Disable Light Entities";
    const QString DisableNackPackets = "Disable Entity NACK Packets";
    const QString DiskCacheEditor = "Disk Cache Editor";
    const QString DisplayCrashOptions = "Display Crash Options";
    const QString DisplayHandTargets = "Show Hand Targets";
    const QString DisplayModelBounds = "Display Model Bounds";
    const QString DisplayModelTriangles = "Display Model Triangles";
    const QString DisplayModelElementChildProxies = "Display Model Element Children";
    const QString DisplayModelElementProxy = "Display Model Element Bounds";
    const QString DisplayDebugTimingDetails = "Display Timing Details";
    const QString DontDoPrecisionPicking = "Don't Do Precision Picking";
    const QString DontRenderEntitiesAsScene = "Don't Render Entities as Scene";
    const QString EchoLocalAudio = "Echo Local Audio";
    const QString EchoServerAudio = "Echo Server Audio";
    const QString Enable3DTVMode = "Enable 3DTV Mode";
    const QString EnableCharacterController = "Enable avatar collisions";
    const QString ExpandMyAvatarSimulateTiming = "Expand /myAvatar/simulation";
    const QString ExpandMyAvatarTiming = "Expand /myAvatar";
    const QString ExpandOtherAvatarTiming = "Expand /otherAvatar";
    const QString ExpandPaintGLTiming = "Expand /paintGL";
    const QString ExpandUpdateTiming = "Expand /update";
    const QString Faceshift = "Faceshift";
    const QString FirstPerson = "First Person";
    const QString FivePointCalibration = "5 Point Calibration";
    const QString FixGaze = "Fix Gaze (no saccade)";
    const QString Forward = "Forward";
    const QString FrameTimer = "Show Timer";
    const QString FullscreenMirror = "Mirror";
    const QString GlowWhenSpeaking = "Glow When Speaking";
    const QString Help = "Help...";
    const QString IncreaseAvatarSize = "Increase Avatar Size";
    const QString IndependentMode = "Independent Mode";
    const QString InputMenu = "Avatar>Input Devices";
    const QString KeyboardMotorControl = "Enable Keyboard Motor Control";
    const QString LeapMotionOnHMD = "Leap Motion on HMD";
    const QString LoadScript = "Open and Run Script File...";
    const QString LoadScriptURL = "Open and Run Script from URL...";
    const QString LodTools = "LOD Tools";
    const QString Login = "Login";
    const QString Log = "Log";
    const QString LogExtraTimings = "Log Extra Timing Details";
    const QString LowVelocityFilter = "Low Velocity Filter";
    const QString MeshVisible = "Draw Mesh";
    const QString MiniMirror = "Mini Mirror";
    const QString MuteAudio = "Mute Microphone";
    const QString MuteEnvironment = "Mute Environment";
    const QString MuteFaceTracking = "Mute Face Tracking";
    const QString NamesAboveHeads = "Names Above Heads";
    const QString NoFaceTracking = "None";
    const QString OctreeStats = "Entity Statistics";
    const QString OnePointCalibration = "1 Point Calibration";
    const QString OnlyDisplayTopTen = "Only Display Top Ten";
    const QString OutputMenu = "Display>Mode";
    const QString PackageModel = "Package Model...";
    const QString Pair = "Pair";
    const QString PhysicsShowOwned = "Highlight Simulation Ownership";
    const QString PhysicsShowHulls = "Draw Collision Hulls";
    const QString PipelineWarnings = "Log Render Pipeline Warnings";
    const QString Preferences = "General...";
    const QString Quit =  "Quit";
    const QString ReloadAllScripts = "Reload All Scripts";
    const QString ReloadContent = "Reload Content (Clears all caches)";
    const QString RenderBoundingCollisionShapes = "Show Bounding Collision Shapes";
    const QString RenderFocusIndicator = "Show Eye Focus";
    const QString RenderLookAtTargets = "Show Look-at Targets";
    const QString RenderLookAtVectors = "Show Look-at Vectors";
    const QString RenderResolution = "Scale Resolution";
    const QString RenderResolutionOne = "1";
    const QString RenderResolutionTwoThird = "2/3";
    const QString RenderResolutionHalf = "1/2";
    const QString RenderResolutionThird = "1/3";
    const QString RenderResolutionQuarter = "1/4";
    const QString RenderAmbientLight = "Ambient Light";
    const QString RenderAmbientLightGlobal = "Global";
    const QString RenderAmbientLight0 = "OLD_TOWN_SQUARE";
    const QString RenderAmbientLight1 = "GRACE_CATHEDRAL";
    const QString RenderAmbientLight2 = "EUCALYPTUS_GROVE";
    const QString RenderAmbientLight3 = "ST_PETERS_BASILICA";
    const QString RenderAmbientLight4 = "UFFIZI_GALLERY";
    const QString RenderAmbientLight5 = "GALILEOS_TOMB";
    const QString RenderAmbientLight6 = "VINE_STREET_KITCHEN";
    const QString RenderAmbientLight7 = "BREEZEWAY";
    const QString RenderAmbientLight8 = "CAMPUS_SUNSET";
    const QString RenderAmbientLight9 = "FUNSTON_BEACH_SUNSET";
    const QString ResetAvatarSize = "Reset Avatar Size";
    const QString ResetSensors = "Reset Sensors";
    const QString RunningScripts = "Running Scripts...";
    const QString RunTimingTests = "Run Timing Tests";
    const QString ScriptEditor = "Script Editor...";
    const QString ScriptedMotorControl = "Enable Scripted Motor Control";
    const QString ShowDSConnectTable = "Show Domain Connection Timing";
    const QString ShowBordersEntityNodes = "Show Entity Nodes";
    const QString ShowRealtimeEntityStats = "Show Realtime Entity Stats";
    const QString ShowWhosLookingAtMe = "Show Who's Looking at Me";
    const QString StandingHMDSensorMode = "Standing HMD Sensor Mode";
    const QString SimulateEyeTracking = "Simulate";
    const QString SMIEyeTracking = "SMI Eye Tracking";
    const QString Stars = "Stars";
    const QString Stats = "Stats";
    const QString StopAllScripts = "Stop All Scripts";
    const QString SuppressShortTimings = "Suppress Timings Less than 10ms";
    const QString ThirdPerson = "Third Person";
    const QString ThreePointCalibration = "3 Point Calibration";
    const QString ThrottleFPSIfNotFocus = "Throttle FPS If Not Focus"; // FIXME - this value duplicated in Basic2DWindowOpenGLDisplayPlugin.cpp
    const QString ToolWindow = "Tool Window";
    const QString TransmitterDrive = "Transmitter Drive";
    const QString TurnWithHead = "Turn using Head";
    const QString UploadAsset = "Upload File to Asset Server";
    const QString UseAudioForMouth = "Use Audio for Mouth";
    const QString UseCamera = "Use Camera";
    const QString VelocityFilter = "Velocity Filter";
    const QString VisibleToEveryone = "Everyone";
    const QString VisibleToFriends = "Friends";
    const QString VisibleToNoOne = "No one";
    const QString WorldAxes = "World Axes";
}

#endif // hifi_Menu_h
