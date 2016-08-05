//
//  Created by Stephen Birarda on 8/12/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_ui_Menu_h
#define hifi_ui_Menu_h

#include <QtCore/QDir>
#include <QtCore/QPointer>
#include <QtCore/QStandardPaths>
#include <QtCore/QHash>
#include <QtGui/QKeySequence>
#include <QtWidgets/QMenuBar>

class Settings;
namespace ui {
    class Menu;
}

enum ItemAccessRoles
{
    RankAndFile = 1 << 0,
    Trainers = 1 << 1,
    THERankAndFile = 1 << 2,
    THETrainers = 1 << 3,
    Admin = 1 << 4,
    All = RankAndFile | Trainers | THERankAndFile | THETrainers | Admin
};

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
    MenuWrapper(ui::Menu& rootMenu, QMenu* menu);
    ui::Menu& _rootMenu;
    QMenu* const _realMenu;
    friend class ui::Menu;
};

namespace ui {

class Menu : public QMenuBar {
    Q_OBJECT
public:
    static const int UNSPECIFIED_POSITION = -1;

    Menu();

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
                                           const QString& grouping = QString(),
                                           ItemAccessRoles accessRoles = ItemAccessRoles::All);

    QAction* addActionToQMenuAndActionHash(MenuWrapper* destinationMenu,
                                           QAction* action,
                                           const QString& actionName = QString(),
                                           const QKeySequence& shortcut = 0,
                                           QAction::MenuRole role = QAction::NoRole,
                                           int menuItemLocation = UNSPECIFIED_POSITION,
                                           const QString& grouping = QString(),
                                           ItemAccessRoles accessRoles = ItemAccessRoles::All);

    QAction* addCheckableActionToQMenuAndActionHash(MenuWrapper* destinationMenu,
                                                    const QString& actionName,
                                                    const QKeySequence& shortcut = 0,
                                                    const bool checked = false,
                                                    const QObject* receiver = NULL,
                                                    const char* member = NULL,
                                                    int menuItemLocation = UNSPECIFIED_POSITION,
                                                    const QString& grouping = QString(),
                                                    ItemAccessRoles accessRoles = ItemAccessRoles::All);

    void removeAction(MenuWrapper* menu, const QString& actionName);

public slots:
    MenuWrapper* addMenu(const QString& menuName, const QString& grouping = QString());
    void removeMenu(const QString& menuName);
    bool menuExists(const QString& menuName);
    void addSeparator(const QString& menuName, const QString& separatorName, const QString& grouping = QString());
    void removeSeparator(const QString& menuName, const QString& separatorName);
    void removeMenuItem(const QString& menuName, const QString& menuitem);
    bool menuItemExists(const QString& menuName, const QString& menuitem);
    void addActionGroup(const QString& groupName, const QStringList& actionList, const QString& selected = QString(), 
        QObject* receiver = nullptr, const char* slot = nullptr);
    void removeActionGroup(const QString& groupName);
    bool isOptionChecked(const QString& menuOption) const;
    void setIsOptionChecked(const QString& menuOption, bool isChecked);

    bool getGroupingIsVisible(const QString& grouping);
    void setGroupingIsVisible(const QString& grouping, bool isVisible); /// NOTE: the "" grouping is always visible
    bool getItemRoleIsVisible(ItemAccessRoles roles);

    void toggleDeveloperMenus();
    void toggleAdvancedMenus();

    static bool isSomeSubmenuShown() { return _isSomeSubmenuShown; }

    void roleChanged(const QString& role)
    {
        if (role == "Admin") {
            _currentRole = Admin;
        } else if (role == "RankAndFile") {
            _currentRole = RankAndFile;
        } else if (role == "Trainers") {
            _currentRole = Trainers;
        } else if (role == "THERankAndFile") {
            _currentRole = THERankAndFile;
        } else if (role == "THETrainers") {
            _currentRole = THETrainers;
        }
        // refresh display of items
        setGroupingIsVisible("", true);
        setGroupingIsVisible("Advanced", getGroupingIsVisible("Advanced"));
        setGroupingIsVisible("Developer", getGroupingIsVisible("Developer"));
    }

protected:
    typedef void(*settingsAction)(Settings&, QAction&);
    static void loadAction(Settings& settings, QAction& action);
    static void saveAction(Settings& settings, QAction& action);
    void scanMenuBar(settingsAction modifySetting);
    void scanMenu(QMenu& menu, settingsAction modifySetting, Settings& settings);

    /// helper method to have separators with labels that are also compatible with OS X
    void addDisabledActionAndSeparator(MenuWrapper* destinationMenu, 
                                       const QString& actionName,
                                       int menuItemLocation = UNSPECIFIED_POSITION, 
                                       const QString& grouping = QString(),
                                       ItemAccessRoles accessRoles = ItemAccessRoles::All);

    QAction* getActionFromName(const QString& menuName, MenuWrapper* menu);
    MenuWrapper* getMenuParent(const QString& menuName, QString& finalMenuPart);

    QAction* getMenuAction(const QString& menuName);
    int findPositionOfMenuItem(MenuWrapper* menu, const QString& searchMenuItem);
    int positionBeforeSeparatorIfNeeded(MenuWrapper* menu, int requestedPosition);

    QHash<QString, QAction*> _actionHash;

    bool isValidGrouping(const QString& grouping) const { return grouping == "Advanced" || grouping == "Developer"; }
    QHash<QString, bool> _groupingVisible;
    QHash<QString, QSet<QAction*>> _groupingActions;
    QHash<QMenu*, MenuWrapper*> _backMap;

    static bool _isSomeSubmenuShown;
    friend class ::MenuWrapper;

    QHash<ItemAccessRoles, QSet<QAction*>> _accessRoleActions;

    ItemAccessRoles _currentRole { ItemAccessRoles::RankAndFile };
};

} // namespace ui

#endif // hifi_Menu_h
