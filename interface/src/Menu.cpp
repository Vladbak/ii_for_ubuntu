//
//  Menu.cpp
//  interface/src
//
//  Created by Stephen Birarda on 8/12/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QFileDialog>
#include <QMenuBar>
#include <QShortcut>

#include <AddressManager.h>
#include <AudioClient.h>
#include <DependencyManager.h>
#include <display-plugins/DisplayPlugin.h>
#include <PathUtils.h>
#include <SettingHandle.h>
#include <UserActivityLogger.h>
#include <VrMenu.h>

#include "Application.h"
#include "AccountManager.h"
#include "assets/ATPAssetMigrator.h"
#include "audio/AudioScope.h"
#include "avatar/AvatarManager.h"
#include "devices/DdeFaceTracker.h"
#include "devices/Faceshift.h"
#include "input-plugins/SpacemouseManager.h"
#include "MainWindow.h"
#include "scripting/MenuScriptingInterface.h"
#include "ui/AssetUploadDialogFactory.h"
#include "ui/DialogsManager.h"
#include "ui/StandAloneJSConsole.h"
#include "InterfaceLogging.h"

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#include "SpeechRecognizer.h"
#endif

#include "Menu.h"

static const char* const MENU_PROPERTY_NAME = "com.highfidelity.Menu";

Menu* Menu::getInstance() {
    static Menu* instance = globalInstance<Menu>(MENU_PROPERTY_NAME);
    return instance;
}

Menu::Menu() {
    _currentRole = ItemAccessRoles::RankAndFile;
    MenuWrapper * fileMenu = addMenu("File");
#ifdef Q_OS_MAC
    addActionToQMenuAndActionHash(fileMenu, MenuOption::AboutApp, 0, qApp, SLOT(aboutApp()), QAction::AboutRole);
#endif
    auto dialogsManager = DependencyManager::get<DialogsManager>();
    AccountManager& accountManager = AccountManager::getInstance();

    {
        addActionToQMenuAndActionHash(fileMenu, MenuOption::Login);

        // connect to the appropriate signal of the AccountManager so that we can change the Login/Logout menu item
        connect(&accountManager, &AccountManager::profileChanged,
                dialogsManager.data(), &DialogsManager::toggleLoginDialog);
        connect(&accountManager, &AccountManager::logoutComplete,
                dialogsManager.data(), &DialogsManager::toggleLoginDialog);
    }

    // File Menu > Scripts section -- "Advanced" grouping
    addDisabledActionAndSeparator(fileMenu, "Scripts", UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);
    addActionToQMenuAndActionHash(fileMenu, MenuOption::LoadScript, Qt::CTRL | Qt::Key_O,
                                  qApp, SLOT(loadDialog()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);
    addActionToQMenuAndActionHash(fileMenu, MenuOption::LoadScriptURL,
                                  Qt::CTRL | Qt::SHIFT | Qt::Key_O, qApp, SLOT(loadScriptURLDialog()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);
    addActionToQMenuAndActionHash(fileMenu, MenuOption::StopAllScripts, 0, qApp, SLOT(stopAllScripts()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);
    addActionToQMenuAndActionHash(fileMenu, MenuOption::ReloadAllScripts, Qt::CTRL | Qt::Key_R,
                                  qApp, SLOT(reloadAllScripts()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);
    addActionToQMenuAndActionHash(fileMenu, MenuOption::RunningScripts, Qt::CTRL | Qt::Key_J,
                                  qApp, SLOT(toggleRunningScriptsWidget()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);

    auto addressManager = DependencyManager::get<AddressManager>();

    addDisabledActionAndSeparator(fileMenu, "History", -1, QString(), ItemAccessRoles::Admin);

    QAction* backAction = addActionToQMenuAndActionHash(fileMenu,
                                                        MenuOption::Back,
                                                        0,
                                                        addressManager.data(),
                                                        SLOT(goBack()),
                                                        QAction::NoRole,
                                                        UNSPECIFIED_POSITION,
                                                        QString(),
                                                        ItemAccessRoles::Admin);

    QAction* forwardAction = addActionToQMenuAndActionHash(fileMenu,
                                                           MenuOption::Forward,
                                                           0,
                                                           addressManager.data(),
                                                           SLOT(goForward()),
                                                           QAction::NoRole,
                                                           UNSPECIFIED_POSITION,
                                                           QString(),
                                                           ItemAccessRoles::Admin);

    // connect to the AddressManager signal to enable and disable the back and forward menu items
    connect(addressManager.data(), &AddressManager::goBackPossible, backAction, &QAction::setEnabled);
    connect(addressManager.data(), &AddressManager::goForwardPossible, forwardAction, &QAction::setEnabled);

    // set the two actions to start disabled since the stacks are clear on startup
    backAction->setDisabled(true);
    forwardAction->setDisabled(true);

    addDisabledActionAndSeparator(fileMenu, "Location");
    qApp->getBookmarks()->setupMenus(this, fileMenu);

    addActionToQMenuAndActionHash(fileMenu,
                                  MenuOption::AddressBar,
                                  Qt::CTRL | Qt::Key_L,
                                  dialogsManager.data(),
                                  SLOT(toggleAddressBar()),
                                  QAction::NoRole,
                                  UNSPECIFIED_POSITION,
                                  QString(),
                                  (ItemAccessRoles) (ItemAccessRoles::THERankAndFile | ItemAccessRoles::THETrainers));
    addActionToQMenuAndActionHash(fileMenu, MenuOption::CopyAddress, 0,
                                  addressManager.data(), SLOT(copyAddress()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced");
    addActionToQMenuAndActionHash(fileMenu, MenuOption::CopyPath, 0,
                                  addressManager.data(), SLOT(copyPath()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced");

    addActionToQMenuAndActionHash(fileMenu,
                                  MenuOption::Quit,
                                  Qt::CTRL | Qt::Key_Q,
                                  qApp,
                                  SLOT(quit()),
                                  QAction::QuitRole);


    MenuWrapper* editMenu = addMenu("Edit");

    QUndoStack* undoStack = qApp->getUndoStack();
    QAction* undoAction = undoStack->createUndoAction(editMenu);
    undoAction->setShortcut(Qt::CTRL | Qt::Key_Z);
    addActionToQMenuAndActionHash(editMenu, undoAction, QString(), 0, QAction::NoRole, UNSPECIFIED_POSITION, QString(), ItemAccessRoles::Admin);

    QAction* redoAction = undoStack->createRedoAction(editMenu);
    redoAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_Z);
    addActionToQMenuAndActionHash(editMenu, redoAction, QString(), 0, QAction::NoRole, UNSPECIFIED_POSITION, QString(), ItemAccessRoles::Admin);

    addActionToQMenuAndActionHash(editMenu,
                                  MenuOption::Preferences,
                                  Qt::CTRL | Qt::Key_Comma,
                                  dialogsManager.data(),
                                  SLOT(editPreferences()),
                                  QAction::PreferencesRole);

    addActionToQMenuAndActionHash(editMenu, MenuOption::Attachments, 0,
                                  dialogsManager.data(), SLOT(editAttachments()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);

    MenuWrapper* toolsMenu = addMenu("Tools");
    addActionToQMenuAndActionHash(toolsMenu, MenuOption::ScriptEditor,  Qt::ALT | Qt::Key_S,
                                  dialogsManager.data(), SLOT(showScriptEditor()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    auto speechRecognizer = DependencyManager::get<SpeechRecognizer>();
    QAction* speechRecognizerAction = addCheckableActionToQMenuAndActionHash(toolsMenu, MenuOption::ControlWithSpeech,
                                                                             Qt::CTRL | Qt::SHIFT | Qt::Key_C,
                                                                             speechRecognizer->getEnabled(),
                                                                             speechRecognizer.data(),
                                                                             SLOT(setEnabled(bool)),
                                                                             UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);
    connect(speechRecognizer.data(), SIGNAL(enabledUpdated(bool)), speechRecognizerAction, SLOT(setChecked(bool)));
#endif
    /* //UTII: we don't need this in our viewer
    addActionToQMenuAndActionHash(toolsMenu, MenuOption::Chat,
                                  0, // QML Qt::Key_Backslash,
                                  dialogsManager.data(), SLOT(showIRCLink()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced");
                                 
    addActionToQMenuAndActionHash(toolsMenu, MenuOption::AddRemoveFriends, 0,
                                  qApp, SLOT(showFriendsWindow()));

    MenuWrapper* visibilityMenu = toolsMenu->addMenu("I Am Visible To");
    {
        QActionGroup* visibilityGroup = new QActionGroup(toolsMenu);
        auto discoverabilityManager = DependencyManager::get<DiscoverabilityManager>();

        QAction* visibleToEveryone = addCheckableActionToQMenuAndActionHash(visibilityMenu, MenuOption::VisibleToEveryone,
            0, discoverabilityManager->getDiscoverabilityMode() == Discoverability::All,
            discoverabilityManager.data(), SLOT(setVisibility()));
        visibilityGroup->addAction(visibleToEveryone);

        QAction* visibleToFriends = addCheckableActionToQMenuAndActionHash(visibilityMenu, MenuOption::VisibleToFriends,
            0, discoverabilityManager->getDiscoverabilityMode() == Discoverability::Friends,
            discoverabilityManager.data(), SLOT(setVisibility()));
        visibilityGroup->addAction(visibleToFriends);

        QAction* visibleToNoOne = addCheckableActionToQMenuAndActionHash(visibilityMenu, MenuOption::VisibleToNoOne,
            0, discoverabilityManager->getDiscoverabilityMode() == Discoverability::None,
            discoverabilityManager.data(), SLOT(setVisibility()));
        visibilityGroup->addAction(visibleToNoOne);

        connect(discoverabilityManager.data(), &DiscoverabilityManager::discoverabilityModeChanged,
            discoverabilityManager.data(), &DiscoverabilityManager::visibilityChanged);
    }*/

    addActionToQMenuAndActionHash(toolsMenu,
                                  MenuOption::ToolWindow,
                                  Qt::CTRL | Qt::ALT | Qt::Key_T,
                                  dialogsManager.data(),
                                  SLOT(toggleToolWindow()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);

    addActionToQMenuAndActionHash(toolsMenu,
                                  MenuOption::Console,
                                  Qt::CTRL | Qt::ALT | Qt::Key_J,
                                  DependencyManager::get<StandAloneJSConsole>().data(),
                                  SLOT(toggleConsole()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);

    addActionToQMenuAndActionHash(toolsMenu,
                                  MenuOption::ResetSensors,
                                  0, // QML Qt::Key_Apostrophe,
                                  qApp,
                                  SLOT(resetSensors()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);

    addActionToQMenuAndActionHash(toolsMenu, MenuOption::PackageModel, 0,
                                  qApp, SLOT(packageModel()),
                                  QAction::NoRole, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);

    addMenu(DisplayPlugin::MENU_PATH(), QString(), ItemAccessRoles::Admin);
    {
        MenuWrapper* displayModeMenu = addMenu(MenuOption::OutputMenu, QString(), ItemAccessRoles::Admin);
        QActionGroup* displayModeGroup = new QActionGroup(displayModeMenu);
        displayModeGroup->setExclusive(true);
    }

    MenuWrapper* avatarMenu = addMenu("Avatar");
    QObject* avatar = DependencyManager::get<AvatarManager>()->getMyAvatar();

    MenuWrapper* inputModeMenu = addMenu(MenuOption::InputMenu, "Advanced");
    QActionGroup* inputModeGroup = new QActionGroup(inputModeMenu);
    inputModeGroup->setExclusive(false);
    
    MenuWrapper* avatarSizeMenu = avatarMenu->addMenu("Size");
    addActionToQMenuAndActionHash(avatarSizeMenu,
                                  MenuOption::IncreaseAvatarSize,
                                  0, // QML Qt::Key_Plus,
                                  avatar,
                                  SLOT(increaseSize()), QAction::NoRole, UNSPECIFIED_POSITION, QString(),
                                  ItemAccessRoles::Admin);
    addActionToQMenuAndActionHash(avatarSizeMenu,
                                  MenuOption::DecreaseAvatarSize,
                                  0, // QML Qt::Key_Minus,
                                  avatar,
                                  SLOT(decreaseSize()), QAction::NoRole, UNSPECIFIED_POSITION, QString(),
                                  ItemAccessRoles::Admin);
    addActionToQMenuAndActionHash(avatarSizeMenu,
                                  MenuOption::ResetAvatarSize,
                                  0, // QML Qt::Key_Equal,
                                  avatar,
                                  SLOT(resetSize()), QAction::NoRole, UNSPECIFIED_POSITION, QString(),
                                  ItemAccessRoles::Admin);

    addCheckableActionToQMenuAndActionHash(avatarMenu, MenuOption::NamesAboveHeads, 0, true, 
                                           NULL, NULL, UNSPECIFIED_POSITION, "Advanced", ItemAccessRoles::Admin);

    MenuWrapper* viewMenu = addMenu("View");
    addActionToQMenuAndActionHash(viewMenu, MenuOption::ReloadContent, 0, qApp, SLOT(reloadResourceCaches()), 
                QAction::NoRole, UNSPECIFIED_POSITION, "Advanced");

    MenuWrapper* cameraModeMenu = viewMenu->addMenu("Camera Mode");
    QActionGroup* cameraModeGroup = new QActionGroup(cameraModeMenu);
    cameraModeGroup->setExclusive(true);
    cameraModeGroup->addAction(addCheckableActionToQMenuAndActionHash(cameraModeMenu,
                                                                      MenuOption::FirstPerson, 0, // QML Qt:: Key_P
                                                                      false, qApp, SLOT(cameraMenuChanged())));
    cameraModeGroup->addAction(addCheckableActionToQMenuAndActionHash(cameraModeMenu,
                                                                      MenuOption::ThirdPerson, 0,
                                                                      true, qApp, SLOT(cameraMenuChanged())));
    cameraModeGroup->addAction(addCheckableActionToQMenuAndActionHash(cameraModeMenu,
                                                                      MenuOption::IndependentMode, 0,
                                                                      false, qApp, SLOT(cameraMenuChanged()), UNSPECIFIED_POSITION, QString(), ItemAccessRoles::Admin));
    cameraModeGroup->addAction(addCheckableActionToQMenuAndActionHash(cameraModeMenu,
                                                                      MenuOption::CameraEntityMode, 0,
                                                                      false, qApp, SLOT(cameraMenuChanged()), UNSPECIFIED_POSITION, QString(), ItemAccessRoles::Admin));
    cameraModeGroup->addAction(addCheckableActionToQMenuAndActionHash(cameraModeMenu,
                                                                      MenuOption::FullscreenMirror, 0, // QML Qt::Key_H,
                                                                      false, qApp, SLOT(cameraMenuChanged()), UNSPECIFIED_POSITION, QString(), ItemAccessRoles::Admin));

    addCheckableActionToQMenuAndActionHash(viewMenu, MenuOption::Mirror,
        0, //QML Qt::SHIFT | Qt::Key_H,
        true);
    
    addCheckableActionToQMenuAndActionHash(viewMenu, MenuOption::CenterPlayerInView,
                                           0, false, qApp, SLOT(rotationModeChanged()),
                                           UNSPECIFIED_POSITION, "Advanced");

    addCheckableActionToQMenuAndActionHash(viewMenu, MenuOption::WorldAxes, 0, false, NULL, NULL, UNSPECIFIED_POSITION, "Developer");
    addCheckableActionToQMenuAndActionHash(viewMenu, MenuOption::Stats, 0, false, NULL, NULL, UNSPECIFIED_POSITION, "Developer");
    addActionToQMenuAndActionHash(viewMenu, MenuOption::Log,
                Qt::CTRL | Qt::SHIFT | Qt::Key_L, 
                qApp, SLOT(toggleLogDialog()), QAction::NoRole, UNSPECIFIED_POSITION, "Developer");

    addActionToQMenuAndActionHash(viewMenu, MenuOption::AudioNetworkStats, 0,
                dialogsManager.data(), SLOT(audioStatsDetails()), QAction::NoRole, UNSPECIFIED_POSITION, "Developer");

    addActionToQMenuAndActionHash(viewMenu, MenuOption::BandwidthDetails, 0,
                dialogsManager.data(), SLOT(bandwidthDetails()), QAction::NoRole, UNSPECIFIED_POSITION, "Developer");
    addActionToQMenuAndActionHash(viewMenu, MenuOption::OctreeStats, 0,
                dialogsManager.data(), SLOT(octreeStatsDetails()), QAction::NoRole, UNSPECIFIED_POSITION, "Developer");

    addCheckableActionToQMenuAndActionHash(viewMenu, "Advanced Menus", 0, false, this, SLOT(toggleAdvancedMenus()),
                                           UNSPECIFIED_POSITION, QString(), ItemAccessRoles::Admin);
    addCheckableActionToQMenuAndActionHash(viewMenu, "Developer Menus", 0, false, this, SLOT(toggleDeveloperMenus()),
                                           UNSPECIFIED_POSITION, QString(), ItemAccessRoles::Admin);

    MenuWrapper* developerMenu = addMenu("Developer", "Developer", ItemAccessRoles::Admin);

    MenuWrapper* renderOptionsMenu = developerMenu->addMenu("Render");
    addCheckableActionToQMenuAndActionHash(renderOptionsMenu, MenuOption::Atmosphere,
        0, // QML Qt::SHIFT | Qt::Key_A,
        true);
    addCheckableActionToQMenuAndActionHash(renderOptionsMenu, MenuOption::DebugAmbientOcclusion);
    addCheckableActionToQMenuAndActionHash(renderOptionsMenu, MenuOption::Antialiasing);

    MenuWrapper* ambientLightMenu = renderOptionsMenu->addMenu(MenuOption::RenderAmbientLight);
    QActionGroup* ambientLightGroup = new QActionGroup(ambientLightMenu);
    ambientLightGroup->setExclusive(true);
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLightGlobal, 0, true));
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLight0, 0, false));
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLight1, 0, false));
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLight2, 0, false));
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLight3, 0, false));
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLight4, 0, false));
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLight5, 0, false));
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLight6, 0, false));
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLight7, 0, false));
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLight8, 0, false));
    ambientLightGroup->addAction(addCheckableActionToQMenuAndActionHash(ambientLightMenu, MenuOption::RenderAmbientLight9, 0, false));

    addCheckableActionToQMenuAndActionHash(renderOptionsMenu, MenuOption::ThrottleFPSIfNotFocus, 0, true);

    MenuWrapper* resolutionMenu = renderOptionsMenu->addMenu(MenuOption::RenderResolution);
    QActionGroup* resolutionGroup = new QActionGroup(resolutionMenu);
    resolutionGroup->setExclusive(true);
    resolutionGroup->addAction(addCheckableActionToQMenuAndActionHash(resolutionMenu, MenuOption::RenderResolutionOne, 0, true));
    resolutionGroup->addAction(addCheckableActionToQMenuAndActionHash(resolutionMenu, MenuOption::RenderResolutionTwoThird, 0, false));
    resolutionGroup->addAction(addCheckableActionToQMenuAndActionHash(resolutionMenu, MenuOption::RenderResolutionHalf, 0, false));
    resolutionGroup->addAction(addCheckableActionToQMenuAndActionHash(resolutionMenu, MenuOption::RenderResolutionThird, 0, false));
    resolutionGroup->addAction(addCheckableActionToQMenuAndActionHash(resolutionMenu, MenuOption::RenderResolutionQuarter, 0, false));

    addCheckableActionToQMenuAndActionHash(renderOptionsMenu, MenuOption::Stars,
        0, // QML Qt::Key_Asterisk,
        true);

    addActionToQMenuAndActionHash(renderOptionsMenu, MenuOption::LodTools,
        0, // QML Qt::SHIFT | Qt::Key_L,
        dialogsManager.data(), SLOT(lodTools()));
    
    MenuWrapper* assetDeveloperMenu = developerMenu->addMenu("Assets");
    
    auto& assetDialogFactory = AssetUploadDialogFactory::getInstance();
    assetDialogFactory.setDialogParent(this);
    
    QAction* assetUpload = addActionToQMenuAndActionHash(assetDeveloperMenu,
                                                         MenuOption::UploadAsset,
                                                         0,
                                                         &assetDialogFactory,
                                                         SLOT(showDialog()));
    
    // disable the asset upload action by default - it gets enabled only if asset server becomes present
    assetUpload->setEnabled(false);
    
    auto& atpMigrator = ATPAssetMigrator::getInstance();
    atpMigrator.setDialogParent(this);
    
    addActionToQMenuAndActionHash(assetDeveloperMenu, MenuOption::AssetMigration,
                                                            0, &atpMigrator,
                                                            SLOT(loadEntityServerFile()));
    
    MenuWrapper* avatarDebugMenu = developerMenu->addMenu("Avatar");

    MenuWrapper* faceTrackingMenu = avatarDebugMenu->addMenu("Face Tracking");
    {
        QActionGroup* faceTrackerGroup = new QActionGroup(avatarDebugMenu);

        bool defaultNoFaceTracking = true;
#ifdef HAVE_DDE
        defaultNoFaceTracking = false;
#endif
        QAction* noFaceTracker = addCheckableActionToQMenuAndActionHash(faceTrackingMenu, MenuOption::NoFaceTracking,
            0, defaultNoFaceTracking,
            qApp, SLOT(setActiveFaceTracker()));
        faceTrackerGroup->addAction(noFaceTracker);

#ifdef HAVE_FACESHIFT
        QAction* faceshiftFaceTracker = addCheckableActionToQMenuAndActionHash(faceTrackingMenu, MenuOption::Faceshift,
            0, false,
            qApp, SLOT(setActiveFaceTracker()));
        faceTrackerGroup->addAction(faceshiftFaceTracker);
#endif
#ifdef HAVE_DDE
        QAction* ddeFaceTracker = addCheckableActionToQMenuAndActionHash(faceTrackingMenu, MenuOption::UseCamera,
            0, true,
            qApp, SLOT(setActiveFaceTracker()));
        faceTrackerGroup->addAction(ddeFaceTracker);
#endif
    }
#ifdef HAVE_DDE
    faceTrackingMenu->addSeparator();
    QAction* binaryEyelidControl = addCheckableActionToQMenuAndActionHash(faceTrackingMenu, MenuOption::BinaryEyelidControl, 0, true);
    binaryEyelidControl->setVisible(true);  // DDE face tracking is on by default
    QAction* coupleEyelids = addCheckableActionToQMenuAndActionHash(faceTrackingMenu, MenuOption::CoupleEyelids, 0, true);
    coupleEyelids->setVisible(true);  // DDE face tracking is on by default
    QAction* useAudioForMouth = addCheckableActionToQMenuAndActionHash(faceTrackingMenu, MenuOption::UseAudioForMouth, 0, true);
    useAudioForMouth->setVisible(true);  // DDE face tracking is on by default
    QAction* ddeFiltering = addCheckableActionToQMenuAndActionHash(faceTrackingMenu, MenuOption::VelocityFilter, 0, true);
    ddeFiltering->setVisible(true);  // DDE face tracking is on by default
    QAction* ddeCalibrate = addActionToQMenuAndActionHash(faceTrackingMenu, MenuOption::CalibrateCamera, 0,
        DependencyManager::get<DdeFaceTracker>().data(), SLOT(calibrate()));
    ddeCalibrate->setVisible(true);  // DDE face tracking is on by default
#endif
#if defined(HAVE_FACESHIFT) || defined(HAVE_DDE)
    faceTrackingMenu->addSeparator();
    addCheckableActionToQMenuAndActionHash(faceTrackingMenu, MenuOption::MuteFaceTracking,
        Qt::CTRL | Qt::SHIFT | Qt::Key_F, true);  // DDE face tracking is on by default
    addCheckableActionToQMenuAndActionHash(faceTrackingMenu, MenuOption::AutoMuteAudio, 0, false);
#endif

#ifdef HAVE_IVIEWHMD
    MenuWrapper* eyeTrackingMenu = avatarDebugMenu->addMenu("Eye Tracking");
    addCheckableActionToQMenuAndActionHash(eyeTrackingMenu, MenuOption::SMIEyeTracking, 0, false,
        qApp, SLOT(setActiveEyeTracker()));
    {
        MenuWrapper* calibrateEyeTrackingMenu = eyeTrackingMenu->addMenu("Calibrate");
        addActionToQMenuAndActionHash(calibrateEyeTrackingMenu, MenuOption::OnePointCalibration, 0,
            qApp, SLOT(calibrateEyeTracker1Point()));
        addActionToQMenuAndActionHash(calibrateEyeTrackingMenu, MenuOption::ThreePointCalibration, 0,
            qApp, SLOT(calibrateEyeTracker3Points()));
        addActionToQMenuAndActionHash(calibrateEyeTrackingMenu, MenuOption::FivePointCalibration, 0,
            qApp, SLOT(calibrateEyeTracker5Points()));
    }
    addCheckableActionToQMenuAndActionHash(eyeTrackingMenu, MenuOption::SimulateEyeTracking, 0, false,
        qApp, SLOT(setActiveEyeTracker()));
#endif

    auto avatarManager = DependencyManager::get<AvatarManager>();
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::AvatarReceiveStats, 0, false,
                                           avatarManager.data(), SLOT(setShouldShowReceiveStats(bool)));

    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::RenderBoundingCollisionShapes);
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::RenderLookAtVectors, 0, false);
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::RenderLookAtTargets, 0, false);
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::RenderFocusIndicator, 0, false);
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::ShowWhosLookingAtMe, 0, false);
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::FixGaze, 0, false);
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::AnimDebugDrawDefaultPose, 0, false,
                                           avatar, SLOT(setEnableDebugDrawDefaultPose(bool)));
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::AnimDebugDrawAnimPose, 0, false,
                                           avatar, SLOT(setEnableDebugDrawAnimPose(bool)));
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::AnimDebugDrawPosition, 0, false,
                                           avatar, SLOT(setEnableDebugDrawPosition(bool)));
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::MeshVisible, 0, true,
                                           avatar, SLOT(setEnableMeshVisible(bool)));
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::DisableEyelidAdjustment, 0, false);
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::TurnWithHead, 0, false);
    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::ComfortMode, 0, true);

    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::KeyboardMotorControl,
        Qt::CTRL | Qt::SHIFT | Qt::Key_K, true, avatar, SLOT(updateMotionBehaviorFromMenu()),
        UNSPECIFIED_POSITION, "Developer");

    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::ScriptedMotorControl, 0, true,
        avatar, SLOT(updateMotionBehaviorFromMenu()),
        UNSPECIFIED_POSITION, "Developer");

    addCheckableActionToQMenuAndActionHash(avatarDebugMenu, MenuOption::EnableCharacterController, 0, true,
        avatar, SLOT(updateMotionBehaviorFromMenu()),
        UNSPECIFIED_POSITION, "Developer");



    MenuWrapper* handOptionsMenu = developerMenu->addMenu("Hands");
    addCheckableActionToQMenuAndActionHash(handOptionsMenu, MenuOption::DisplayHandTargets, 0, false);
    addCheckableActionToQMenuAndActionHash(handOptionsMenu, MenuOption::LowVelocityFilter, 0, true,
                                           qApp, SLOT(setLowVelocityFilter(bool)));

    MenuWrapper* leapOptionsMenu = handOptionsMenu->addMenu("Leap Motion");
    addCheckableActionToQMenuAndActionHash(leapOptionsMenu, MenuOption::LeapMotionOnHMD, 0, false);

    MenuWrapper* networkMenu = developerMenu->addMenu("Network");
    addActionToQMenuAndActionHash(networkMenu, MenuOption::ReloadContent, 0, qApp, SLOT(reloadResourceCaches()));
    addCheckableActionToQMenuAndActionHash(networkMenu, MenuOption::DisableNackPackets, 0, false,
                                           qApp->getEntityEditPacketSender(),
                                           SLOT(toggleNackPackets()));
    addCheckableActionToQMenuAndActionHash(networkMenu,
                                           MenuOption::DisableActivityLogger,
                                           0,
                                           false,
                                           &UserActivityLogger::getInstance(),
                                           SLOT(disable(bool)));
    addActionToQMenuAndActionHash(networkMenu, MenuOption::CachesSize, 0,
                                  dialogsManager.data(), SLOT(cachesSizeDialog()));
    addActionToQMenuAndActionHash(networkMenu, MenuOption::DiskCacheEditor, 0,
                                  dialogsManager.data(), SLOT(toggleDiskCacheEditor()));

    addActionToQMenuAndActionHash(networkMenu, MenuOption::ShowDSConnectTable, 0,
                                  dialogsManager.data(), SLOT(showDomainConnectionDialog()));

    MenuWrapper* timingMenu = developerMenu->addMenu("Timing and Stats");

    MenuWrapper* perfTimerMenu = timingMenu->addMenu("Performance Timer");
    addCheckableActionToQMenuAndActionHash(perfTimerMenu, MenuOption::DisplayDebugTimingDetails, 0, false);
    addCheckableActionToQMenuAndActionHash(perfTimerMenu, MenuOption::OnlyDisplayTopTen, 0, true);
    addCheckableActionToQMenuAndActionHash(perfTimerMenu, MenuOption::ExpandUpdateTiming, 0, false);
    addCheckableActionToQMenuAndActionHash(perfTimerMenu, MenuOption::ExpandMyAvatarTiming, 0, false);
    addCheckableActionToQMenuAndActionHash(perfTimerMenu, MenuOption::ExpandMyAvatarSimulateTiming, 0, false);
    addCheckableActionToQMenuAndActionHash(perfTimerMenu, MenuOption::ExpandOtherAvatarTiming, 0, false);
    addCheckableActionToQMenuAndActionHash(perfTimerMenu, MenuOption::ExpandPaintGLTiming, 0, false);

    addCheckableActionToQMenuAndActionHash(timingMenu, MenuOption::FrameTimer);
    addActionToQMenuAndActionHash(timingMenu, MenuOption::RunTimingTests, 0, qApp, SLOT(runTests()));
    addCheckableActionToQMenuAndActionHash(timingMenu, MenuOption::PipelineWarnings);
    addCheckableActionToQMenuAndActionHash(timingMenu, MenuOption::LogExtraTimings);
    addCheckableActionToQMenuAndActionHash(timingMenu, MenuOption::SuppressShortTimings);
    addCheckableActionToQMenuAndActionHash(timingMenu, MenuOption::ShowRealtimeEntityStats);

    auto audioIO = DependencyManager::get<AudioClient>();
    MenuWrapper* audioDebugMenu = developerMenu->addMenu("Audio");
    addCheckableActionToQMenuAndActionHash(audioDebugMenu, MenuOption::AudioNoiseReduction,
                                           0,
                                           true,
                                           audioIO.data(),
                                           SLOT(toggleAudioNoiseReduction()));

    addCheckableActionToQMenuAndActionHash(audioDebugMenu, MenuOption::EchoServerAudio, 0, false,
                                           audioIO.data(), SLOT(toggleServerEcho()));
    addCheckableActionToQMenuAndActionHash(audioDebugMenu, MenuOption::EchoLocalAudio, 0, false,
                                           audioIO.data(), SLOT(toggleLocalEcho()));
    addCheckableActionToQMenuAndActionHash(audioDebugMenu, MenuOption::MuteAudio,
                                           Qt::CTRL | Qt::Key_M,
                                           false,
                                           audioIO.data(),
                                           SLOT(toggleMute()));
    addActionToQMenuAndActionHash(audioDebugMenu,
                                  MenuOption::MuteEnvironment,
                                  0,
                                  audioIO.data(),
                                  SLOT(sendMuteEnvironmentPacket()));

    auto scope = DependencyManager::get<AudioScope>();

    MenuWrapper* audioScopeMenu = audioDebugMenu->addMenu("Audio Scope");
    addCheckableActionToQMenuAndActionHash(audioScopeMenu, MenuOption::AudioScope,
                                           Qt::CTRL | Qt::Key_P, false,
                                           scope.data(),
                                           SLOT(toggle()));
    addCheckableActionToQMenuAndActionHash(audioScopeMenu, MenuOption::AudioScopePause,
                                           Qt::CTRL | Qt::SHIFT | Qt::Key_P ,
                                           false,
                                           scope.data(),
                                           SLOT(togglePause()));
    addDisabledActionAndSeparator(audioScopeMenu, "Display Frames");
    {
        QAction *fiveFrames = addCheckableActionToQMenuAndActionHash(audioScopeMenu, MenuOption::AudioScopeFiveFrames,
                                               0,
                                               true,
                                               scope.data(),
                                               SLOT(selectAudioScopeFiveFrames()));

        QAction *twentyFrames = addCheckableActionToQMenuAndActionHash(audioScopeMenu, MenuOption::AudioScopeTwentyFrames,
                                               0,
                                               false,
                                               scope.data(),
                                               SLOT(selectAudioScopeTwentyFrames()));

        QAction *fiftyFrames = addCheckableActionToQMenuAndActionHash(audioScopeMenu, MenuOption::AudioScopeFiftyFrames,
                                               0,
                                               false,
                                               scope.data(),
                                               SLOT(selectAudioScopeFiftyFrames()));

        QActionGroup* audioScopeFramesGroup = new QActionGroup(audioScopeMenu);
        audioScopeFramesGroup->addAction(fiveFrames);
        audioScopeFramesGroup->addAction(twentyFrames);
        audioScopeFramesGroup->addAction(fiftyFrames);
    }

    MenuWrapper* physicsOptionsMenu = developerMenu->addMenu("Physics");
    addCheckableActionToQMenuAndActionHash(physicsOptionsMenu, MenuOption::PhysicsShowOwned);
    addCheckableActionToQMenuAndActionHash(physicsOptionsMenu, MenuOption::PhysicsShowHulls);

    addCheckableActionToQMenuAndActionHash(developerMenu, MenuOption::DisplayCrashOptions, 0, true);
    addActionToQMenuAndActionHash(developerMenu, MenuOption::CrashInterface, 0, qApp, SLOT(crashApplication()));

    MenuWrapper* helpMenu = addMenu("Help");
    addActionToQMenuAndActionHash(helpMenu, MenuOption::EditEntitiesHelp, 0, qApp, SLOT(showEditEntitiesHelp()));

#ifndef Q_OS_MAC
    QAction* aboutAction = helpMenu->addAction(MenuOption::AboutApp);
    connect(aboutAction, SIGNAL(triggered()), qApp, SLOT(aboutApp()));
#endif
}

void Menu::toggleAdvancedMenus() {
    setGroupingIsVisible("Advanced", !getGroupingIsVisible("Advanced"));
}

void Menu::toggleDeveloperMenus() {
    setGroupingIsVisible("Developer", !getGroupingIsVisible("Developer"));
}

void Menu::loadSettings() {
    scanMenuBar(&Menu::loadAction);
}

void Menu::saveSettings() {
    scanMenuBar(&Menu::saveAction);
}

void Menu::loadAction(Settings& settings, QAction& action) {
    if (action.isChecked() != settings.value(action.text(), action.isChecked()).toBool()) {
        action.trigger();
    }
}

void Menu::saveAction(Settings& settings, QAction& action) {
    settings.setValue(action.text(), action.isChecked());
}

void Menu::scanMenuBar(settingsAction modifySetting) {
    Settings settings;
    foreach (QMenu* menu, findChildren<QMenu*>()) {
        scanMenu(*menu, modifySetting, settings);
    }
}

void Menu::scanMenu(QMenu& menu, settingsAction modifySetting, Settings& settings) {
    settings.beginGroup(menu.title());
    foreach (QAction* action, menu.actions()) {
        if (action->menu()) {
            scanMenu(*action->menu(), modifySetting, settings);
        } else if (action->isCheckable()) {
            modifySetting(settings, *action);
        }
    }
    settings.endGroup();
}

void Menu::addDisabledActionAndSeparator(MenuWrapper* destinationMenu, const QString& actionName, 
        int menuItemLocation, const QString& grouping, ItemAccessRoles accessRoles) {
    QAction* actionBefore = NULL;
    QAction* separator;
    QAction* separatorText;

    if (menuItemLocation >= 0 && destinationMenu->actions().size() > menuItemLocation) {
        actionBefore = destinationMenu->actions()[menuItemLocation];
    }
    if (actionBefore) {
        separator = new QAction("",destinationMenu);
        destinationMenu->insertAction(actionBefore, separator);
        separator->setSeparator(true);

        separatorText = new QAction(actionName,destinationMenu);
        separatorText->setEnabled(false);
        destinationMenu->insertAction(actionBefore, separatorText);

    } else {
        separator = destinationMenu->addSeparator();
        separatorText = destinationMenu->addAction(actionName);
        separatorText->setEnabled(false);
    }

    if ((accessRoles & RankAndFile) == RankAndFile) {
        _accessRoleActions[RankAndFile] << separator;
        _accessRoleActions[RankAndFile] << separatorText;
    }
    if ((accessRoles & Trainers) == Trainers) {
        _accessRoleActions[Trainers] << separator;
        _accessRoleActions[Trainers] << separatorText;
    }
    if ((accessRoles & THERankAndFile) == THERankAndFile) {
        _accessRoleActions[THERankAndFile] << separator;
        _accessRoleActions[THERankAndFile] << separatorText;
    }
    if ((accessRoles & THETrainers) == THETrainers) {
        _accessRoleActions[THETrainers] << separator;
        _accessRoleActions[THETrainers] << separatorText;
    }

    if (isValidGrouping(grouping)) {
        _groupingActions[grouping] << separator;
        _groupingActions[grouping] << separatorText;
    }
    bool isVisible = getGroupingIsVisible(grouping) && getItemRoleIsVisible(accessRoles);
    separator->setVisible(isVisible);
    separatorText->setVisible(isVisible);
}

QAction* Menu::addActionToQMenuAndActionHash(MenuWrapper* destinationMenu,
                                             const QString& actionName,
                                             const QKeySequence& shortcut,
                                             const QObject* receiver,
                                             const char* member,
                                             QAction::MenuRole role,
                                             int menuItemLocation, 
                                             const QString& grouping,
                                             ItemAccessRoles accessRoles) {
    QAction* action = NULL;
    QAction* actionBefore = NULL;

    if (menuItemLocation >= 0 && destinationMenu->actions().size() > menuItemLocation) {
        actionBefore = destinationMenu->actions()[menuItemLocation];
    }

    if (!actionBefore) {
        if (receiver && member) {
            action = destinationMenu->addAction(actionName, receiver, member, shortcut);
        } else {
            action = destinationMenu->addAction(actionName);
            action->setShortcut(shortcut);
        }
    } else {
        action = new QAction(actionName, destinationMenu);
        action->setShortcut(shortcut);
        destinationMenu->insertAction(actionBefore, action);

        if (receiver && member) {
            connect(action, SIGNAL(triggered()), receiver, member);
        }
    }
    action->setMenuRole(role);

    _actionHash.insert(actionName, action);

    if ((accessRoles & RankAndFile) == RankAndFile) {
        _accessRoleActions[RankAndFile] << action;
    }
    if ((accessRoles & Trainers) == Trainers) {
        _accessRoleActions[Trainers] << action;
    }
    if ((accessRoles & THERankAndFile) == THERankAndFile) {
        _accessRoleActions[THERankAndFile] << action;
    }
    if ((accessRoles & THETrainers) == THETrainers) {
        _accessRoleActions[THETrainers] << action;
    }

    if (isValidGrouping(grouping)) {
        _groupingActions[grouping] << action;  
    }
    action->setVisible(getGroupingIsVisible(grouping) && getItemRoleIsVisible(accessRoles));

    return action;
}

QAction* Menu::addActionToQMenuAndActionHash(MenuWrapper* destinationMenu,
                                             QAction* action,
                                             const QString& actionName,
                                             const QKeySequence& shortcut,
                                             QAction::MenuRole role,
                                             int menuItemLocation, 
                                             const QString& grouping,
                                             ItemAccessRoles accessRoles) {
    QAction* actionBefore = NULL;

    if (menuItemLocation >= 0 && destinationMenu->actions().size() > menuItemLocation) {
        actionBefore = destinationMenu->actions()[menuItemLocation];
    }

    if (!actionName.isEmpty()) {
        action->setText(actionName);
    }

    if (shortcut != 0) {
        action->setShortcut(shortcut);
    }

    if (role != QAction::NoRole) {
        action->setMenuRole(role);
    }

    if (!actionBefore) {
        destinationMenu->addAction(action);
    } else {
        destinationMenu->insertAction(actionBefore, action);
    }

    _actionHash.insert(action->text(), action);

    if ((accessRoles & RankAndFile) == RankAndFile) {
        _accessRoleActions[RankAndFile] << action;
    }
    if ((accessRoles & Trainers) == Trainers) {
        _accessRoleActions[Trainers] << action;
    }
    if ((accessRoles & THERankAndFile) == THERankAndFile) {
        _accessRoleActions[THERankAndFile] << action;
    }
    if ((accessRoles & THETrainers) == THETrainers) {
        _accessRoleActions[THETrainers] << action;
    }

    if (isValidGrouping(grouping)) {
        _groupingActions[grouping] << action;
    }

    action->setVisible(getGroupingIsVisible(grouping) && getItemRoleIsVisible(accessRoles));

    return action;
}

QAction* Menu::addCheckableActionToQMenuAndActionHash(MenuWrapper* destinationMenu,
                                                      const QString& actionName,
                                                      const QKeySequence& shortcut,
                                                      const bool checked,
                                                      const QObject* receiver,
                                                      const char* member,
                                                      int menuItemLocation, 
                                                      const QString& grouping,
                                                      ItemAccessRoles accessRoles) {

    QAction* action = addActionToQMenuAndActionHash(destinationMenu, actionName, shortcut, receiver, member,
                                                        QAction::NoRole, menuItemLocation);
    action->setCheckable(true);
    action->setChecked(checked);

    if ((accessRoles & RankAndFile) == RankAndFile) {
        _accessRoleActions[RankAndFile] << action;
    }
    if ((accessRoles & Trainers) == Trainers) {
        _accessRoleActions[Trainers] << action;
    }
    if ((accessRoles & THERankAndFile) == THERankAndFile) {
        _accessRoleActions[THERankAndFile] << action;
    }
    if ((accessRoles & THETrainers) == THETrainers) {
        _accessRoleActions[THETrainers] << action;
    }

    if (isValidGrouping(grouping)) {
        _groupingActions[grouping] << action;
    }

    action->setVisible(getGroupingIsVisible(grouping) && getItemRoleIsVisible(accessRoles));

    return action;
}

void Menu::removeAction(MenuWrapper* menu, const QString& actionName) {
    auto action = _actionHash.value(actionName);
    menu->removeAction(action);
    _actionHash.remove(actionName);
    for (auto& grouping : _groupingActions) {
        grouping.remove(action);
    }
}

void Menu::setIsOptionChecked(const QString& menuOption, bool isChecked) {
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(Menu::getInstance(), "setIsOptionChecked", Qt::BlockingQueuedConnection,
                    Q_ARG(const QString&, menuOption),
                    Q_ARG(bool, isChecked));
        return;
    }
    QAction* menu = _actionHash.value(menuOption);
    if (menu) {
        menu->setChecked(isChecked);
    }
}

bool Menu::isOptionChecked(const QString& menuOption) const {
    const QAction* menu = _actionHash.value(menuOption);
    if (menu) {
        return menu->isChecked();
    }
    return false;
}

void Menu::triggerOption(const QString& menuOption) {
    QAction* action = _actionHash.value(menuOption);
    if (action) {
        action->trigger();
    } else {
        qCDebug(interfaceapp) << "NULL Action for menuOption '" << menuOption << "'";
    }
}

QAction* Menu::getActionForOption(const QString& menuOption) {
    return _actionHash.value(menuOption);
}

QAction* Menu::getActionFromName(const QString& menuName, MenuWrapper* menu) {
    QList<QAction*> menuActions;
    if (menu) {
        menuActions = menu->actions();
    } else {
        menuActions = actions();
    }

    foreach (QAction* menuAction, menuActions) {
        QString actionText = menuAction->text();
        if (menuName == menuAction->text()) {
            return menuAction;
        }
    }
    return NULL;
}

MenuWrapper* Menu::getSubMenuFromName(const QString& menuName, MenuWrapper* menu) {
    QAction* action = getActionFromName(menuName, menu);
    if (action) {
        return MenuWrapper::fromMenu(action->menu());
    }
    return NULL;
}

MenuWrapper* Menu::getMenuParent(const QString& menuName, QString& finalMenuPart) {
    QStringList menuTree = menuName.split(">");
    MenuWrapper* parent = NULL;
    MenuWrapper* menu = NULL;
    foreach (QString menuTreePart, menuTree) {
        parent = menu;
        finalMenuPart = menuTreePart.trimmed();
        menu = getSubMenuFromName(finalMenuPart, parent);
        if (!menu) {
            break;
        }
    }
    return parent;
}

MenuWrapper* Menu::getMenu(const QString& menuName) {
    QStringList menuTree = menuName.split(">");
    MenuWrapper* parent = NULL;
    MenuWrapper* menu = NULL;
    int item = 0;
    foreach (QString menuTreePart, menuTree) {
        menu = getSubMenuFromName(menuTreePart.trimmed(), parent);
        if (!menu) {
            break;
        }
        parent = menu;
        item++;
    }
    return menu;
}

QAction* Menu::getMenuAction(const QString& menuName) {
    QStringList menuTree = menuName.split(">");
    MenuWrapper* parent = NULL;
    QAction* action = NULL;
    foreach (QString menuTreePart, menuTree) {
        action = getActionFromName(menuTreePart.trimmed(), parent);
        if (!action) {
            break;
        }
        parent = MenuWrapper::fromMenu(action->menu());
    }
    return action;
}

int Menu::findPositionOfMenuItem(MenuWrapper* menu, const QString& searchMenuItem) {
    int position = 0;
    foreach(QAction* action, menu->actions()) {
        if (action->text() == searchMenuItem) {
            return position;
        }
        position++;
    }
    return UNSPECIFIED_POSITION; // not found
}

int Menu::positionBeforeSeparatorIfNeeded(MenuWrapper* menu, int requestedPosition) {
    QList<QAction*> menuActions = menu->actions();
    if (requestedPosition > 1 && requestedPosition < menuActions.size()) {
        QAction* beforeRequested = menuActions[requestedPosition - 1];
        if (beforeRequested->isSeparator()) {
            requestedPosition--;
        }
    }
    return requestedPosition;
}


MenuWrapper* Menu::addMenu(const QString& menuName, const QString& grouping, ItemAccessRoles accessRoles) {
    QStringList menuTree = menuName.split(">");
    MenuWrapper* addTo = NULL;
    MenuWrapper* menu = NULL;
    foreach (QString menuTreePart, menuTree) {
        menu = getSubMenuFromName(menuTreePart.trimmed(), addTo);
        if (!menu) {
            if (!addTo) {
                menu = new MenuWrapper(QMenuBar::addMenu(menuTreePart.trimmed()));
            } else {
                menu = addTo->addMenu(menuTreePart.trimmed());
            }
        }
        addTo = menu;
    }

    auto action = getMenuAction(menuName);
    if (action) {
        if ((accessRoles & RankAndFile) == RankAndFile) {
            _accessRoleActions[RankAndFile] << action;
        }
        if ((accessRoles & Trainers) == Trainers) {
            _accessRoleActions[Trainers] << action;
        }
        if ((accessRoles & THERankAndFile) == THERankAndFile) {
            _accessRoleActions[THERankAndFile] << action;
        }
        if ((accessRoles & THETrainers) == THETrainers) {
            _accessRoleActions[THETrainers] << action;
        }
        if (isValidGrouping(grouping)) {
            _groupingActions[grouping] << action;
        }
        action->setVisible(getGroupingIsVisible(grouping) && getItemRoleIsVisible(accessRoles));
    }

    QMenuBar::repaint();
    return menu;
}

void Menu::removeMenu(const QString& menuName) {
    QAction* action = getMenuAction(menuName);

    // only proceed if the menu actually exists
    if (action) {
        QString finalMenuPart;
        MenuWrapper* parent = getMenuParent(menuName, finalMenuPart);
        if (parent) {
            parent->removeAction(action);
        } else {
            QMenuBar::removeAction(action);
        }

        QMenuBar::repaint();
    }
}

bool Menu::menuExists(const QString& menuName) {
    QAction* action = getMenuAction(menuName);

    // only proceed if the menu actually exists
    if (action) {
        return true;
    }
    return false;
}

void Menu::addSeparator(const QString& menuName, const QString& separatorName, const QString& grouping) {
    MenuWrapper* menuObj = getMenu(menuName);
    if (menuObj) {
        addDisabledActionAndSeparator(menuObj, separatorName);
    }
}

void Menu::removeSeparator(const QString& menuName, const QString& separatorName) {
    MenuWrapper* menu = getMenu(menuName);
    bool separatorRemoved = false;
    if (menu) {
        int textAt = findPositionOfMenuItem(menu, separatorName);
        QList<QAction*> menuActions = menu->actions();
        QAction* separatorText = menuActions[textAt];
        if (textAt > 0 && textAt < menuActions.size()) {
            QAction* separatorLine = menuActions[textAt - 1];
            if (separatorLine) {
                if (separatorLine->isSeparator()) {
                    menu->removeAction(separatorText);
                    menu->removeAction(separatorLine);
                    separatorRemoved = true;
                }
            }
        }
    }
    if (separatorRemoved) {
        QMenuBar::repaint();
    }
}

void Menu::addMenuItem(const MenuItemProperties& properties) {
    MenuWrapper* menuObj = getMenu(properties.menuName);
    if (menuObj) {
        QShortcut* shortcut = NULL;
        if (!properties.shortcutKeySequence.isEmpty()) {
            shortcut = new QShortcut(properties.shortcutKeySequence, this);
            shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        }

        // check for positioning requests
        int requestedPosition = properties.position;
        if (requestedPosition == UNSPECIFIED_POSITION && !properties.beforeItem.isEmpty()) {
            requestedPosition = findPositionOfMenuItem(menuObj, properties.beforeItem);
            // double check that the requested location wasn't a separator label
            requestedPosition = positionBeforeSeparatorIfNeeded(menuObj, requestedPosition);
        }
        if (requestedPosition == UNSPECIFIED_POSITION && !properties.afterItem.isEmpty()) {
            int afterPosition = findPositionOfMenuItem(menuObj, properties.afterItem);
            if (afterPosition != UNSPECIFIED_POSITION) {
                requestedPosition = afterPosition + 1;
            }
        }

        QAction* menuItemAction = NULL;
        if (properties.isSeparator) {
            addDisabledActionAndSeparator(menuObj, properties.menuItemName, requestedPosition, properties.grouping);
        } else if (properties.isCheckable) {
            menuItemAction = addCheckableActionToQMenuAndActionHash(menuObj, properties.menuItemName,
                                                                    properties.shortcutKeySequence, properties.isChecked,
                                                                    MenuScriptingInterface::getInstance(), SLOT(menuItemTriggered()), 
                                                                    requestedPosition, properties.grouping);
        } else {
            menuItemAction = addActionToQMenuAndActionHash(menuObj, properties.menuItemName, properties.shortcutKeySequence,
                                                           MenuScriptingInterface::getInstance(), SLOT(menuItemTriggered()),
                                                           QAction::NoRole, requestedPosition, properties.grouping);
        }
        if (shortcut && menuItemAction) {
            connect(shortcut, SIGNAL(activated()), menuItemAction, SLOT(trigger()));
        }
        QMenuBar::repaint();
    }
}

void Menu::removeMenuItem(const QString& menu, const QString& menuitem) {
    MenuWrapper* menuObj = getMenu(menu);
    if (menuObj) {
        removeAction(menuObj, menuitem);
        QMenuBar::repaint();
    }
}

bool Menu::menuItemExists(const QString& menu, const QString& menuitem) {
    QAction* menuItemAction = _actionHash.value(menuitem);
    if (menuItemAction) {
        return (getMenu(menu) != NULL);
    }
    return false;
}

bool Menu::getGroupingIsVisible(const QString& grouping) {
    if (grouping.isEmpty() || grouping.isNull()) {
        return true;
    }
    if (_groupingVisible.contains(grouping)) {
        return _groupingVisible[grouping];
    }
    return false;
}

bool Menu::getItemRoleIsVisible(ItemAccessRoles roles) {
    if (_currentRole == Admin) {
        return true;
    }
    if ((roles & _currentRole) == _currentRole) {
        return true;
    }
    return false;
}

void Menu::setGroupingIsVisible(const QString& grouping, bool isVisible) {
    // NOTE: Default grouping always visible
    if (grouping.isEmpty() || grouping.isNull()) {
        return;
    }
    _groupingVisible[grouping] = isVisible;

    for (auto action: _groupingActions[grouping]) {
        action->setVisible(isVisible && (_currentRole == ItemAccessRoles::Admin || _accessRoleActions[_currentRole].contains(action)));
    }

    QMenuBar::repaint();
}

void Menu::addActionGroup(const QString& groupName, const QStringList& actionList, const QString& selected) {
    auto menu = addMenu(groupName);
    
    QActionGroup* actionGroup = new QActionGroup(menu);
    actionGroup->setExclusive(true);
    
    auto menuScriptingInterface = MenuScriptingInterface::getInstance();
    for (auto action : actionList) {
        auto item = addCheckableActionToQMenuAndActionHash(menu, action, 0, action == selected,
                                                           menuScriptingInterface,
                                                           SLOT(menuItemTriggered()));
        actionGroup->addAction(item);
    }
    
    QMenuBar::repaint();
}

void Menu::removeActionGroup(const QString& groupName) {
    removeMenu(groupName);
}

MenuWrapper::MenuWrapper(QMenu* menu) : _realMenu(menu) {
    VrMenu::executeOrQueue([=](VrMenu* vrMenu) {
        vrMenu->addMenu(menu);
    });
    _backMap[menu] = this;
}

QList<QAction*> MenuWrapper::actions() {
    return _realMenu->actions();
}

MenuWrapper* MenuWrapper::addMenu(const QString& menuName) {
    return new MenuWrapper(_realMenu->addMenu(menuName));
}

void MenuWrapper::setEnabled(bool enabled) {
    _realMenu->setEnabled(enabled);
}

QAction* MenuWrapper::addSeparator() {
    return _realMenu->addSeparator();
}

void MenuWrapper::addAction(QAction* action) {
    _realMenu->addAction(action);
    VrMenu::executeOrQueue([=](VrMenu* vrMenu) {
        vrMenu->addAction(_realMenu, action);
    });
}

QAction* MenuWrapper::addAction(const QString& menuName) {
    QAction* action = _realMenu->addAction(menuName);
    VrMenu::executeOrQueue([=](VrMenu* vrMenu) {
        vrMenu->addAction(_realMenu, action);
    });
    return action;
}

QAction* MenuWrapper::addAction(const QString& menuName, const QObject* receiver, const char* member, const QKeySequence& shortcut) {
    QAction* action = _realMenu->addAction(menuName, receiver, member, shortcut);
    VrMenu::executeOrQueue([=](VrMenu* vrMenu) {
        vrMenu->addAction(_realMenu, action);
    });
    return action;
}

void MenuWrapper::removeAction(QAction* action) {
    _realMenu->removeAction(action);
    VrMenu::executeOrQueue([=](VrMenu* vrMenu) {
        vrMenu->removeAction(action);
    });
}

void MenuWrapper::insertAction(QAction* before, QAction* action) {
    _realMenu->insertAction(before, action);
    VrMenu::executeOrQueue([=](VrMenu* vrMenu) {
        vrMenu->insertAction(before, action);
    });
}

QHash<QMenu*, MenuWrapper*> MenuWrapper::_backMap;
