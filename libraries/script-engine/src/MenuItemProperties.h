//
//  MenuItemProperties.h
//  libraries/script-engine/src
//
//  Created by Brad Hefta-Gaub on 1/28/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_MenuItemProperties_h
#define hifi_MenuItemProperties_h

#include <QtScript/QScriptEngine>

#include <ui/Menu.h>
#include "KeyEvent.h"


class MenuItemProperties {
public:
    MenuItemProperties() {}
    MenuItemProperties(const QString& menuName, const QString& menuItemName,
                       const QString& shortcutKey = QString(""), bool checkable = false, bool checked = false, bool separator = false);
    MenuItemProperties(const QString& menuName, const QString& menuItemName,
                       const KeyEvent& shortcutKeyEvent, bool checkable = false, bool checked = false, bool separator = false);

    QString menuName;
    QString menuItemName;

    // Shortcut key items: in order of priority
    QString shortcutKey;
    KeyEvent shortcutKeyEvent;
    QKeySequence shortcutKeySequence; // this is what we actually use, it's set from one of the above

    // location related items: in order of priority
    int position { ui::Menu::UNSPECIFIED_POSITION };
    QString beforeItem;
    QString afterItem;

    // other properties
    bool isCheckable { false };
    bool isChecked { false };
    bool isSeparator { false };

    QString grouping; /// Either: "", "Advanced", or "Developer"
};
Q_DECLARE_METATYPE(MenuItemProperties)
QScriptValue menuItemPropertiesToScriptValue(QScriptEngine* engine, const MenuItemProperties& props);
void menuItemPropertiesFromScriptValue(const QScriptValue& object, MenuItemProperties& props);
void registerMenuItemProperties(QScriptEngine* engine);



#endif // hifi_MenuItemProperties_h
