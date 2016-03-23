//
//  ContentSection.qml
//
//  Created by David Rowe on 16 Feb 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import QtGraphicalEffects 1.0

import "../styles-uit"

Column {
    property string name: "Content Section"
    property bool isFirst: false
    property bool isCollapsible: false  // Set at creation.
    property bool isCollapsed: false

    spacing: 0  // Defer spacing decisions to individual controls.

    anchors {
        left: parent.left
        leftMargin: hifi.dimensions.contentMargin.x
        right: parent.right
        rightMargin: hifi.dimensions.contentMargin.x
    }

    function toggleCollapsed() {
        if (isCollapsible) {
            isCollapsed = !isCollapsed;
            for (var i = 1; i < children.length; i++) {
                children[i].visible = !isCollapsed;
            }
        }
    }

    Item {
        id: sectionName
        anchors.left: parent.left
        anchors.right: parent.right
        height: leadingSpace.height + topBar.height + heading.height + bottomBar.height

        Item {
            id: leadingSpace
            width: 1
            height: isFirst ? hifi.dimensions.contentSpacing.y : hifi.dimensions.controlInterlineHeight
            anchors.top: parent.top
        }

        Item {
            id: topBar
            visible: !isFirst
            height: visible ? 2 : 0
            anchors.top: leadingSpace.bottom

            Rectangle {
                id: shadow
                width: frame.width
                height: 1
                color: hifi.colors.baseGrayShadow
                x: -hifi.dimensions.contentMargin.x
            }

            Rectangle {
                width: frame.width
                height: 1
                color: hifi.colors.baseGrayHighlight
                x: -hifi.dimensions.contentMargin.x
                anchors.top: shadow.bottom
            }
        }

        Item {
            id: heading
            anchors {
                left: parent.left
                right: parent.right
                top: topBar.bottom
            }
            height: (isCollapsible ? 3 : 2) * hifi.dimensions.contentSpacing.y

            RalewayRegular {
                id: title
                anchors {
                    left: parent.left
                    top: parent.top
                    topMargin: hifi.dimensions.contentSpacing.y
                }
                size: hifi.fontSizes.sectionName
                font.capitalization: Font.AllUppercase
                text: name
                color: hifi.colors.lightGrayText
            }

            HiFiGlyphs {
                anchors {
                    verticalCenter: title.verticalCenter
                    right: parent.right
                    rightMargin: -4
                }
                y: -2
                size: hifi.fontSizes.disclosureButton
                text: isCollapsed ? hifi.glyphs.disclosureButtonExpand : hifi.glyphs.disclosureButtonCollapse
                color: hifi.colors.lightGrayText
                visible: isCollapsible
            }

            MouseArea {
                anchors.fill: parent
                onClicked: toggleCollapsed()
            }
        }

        LinearGradient {
            id: bottomBar
            visible: isCollapsible
            width: frame.width
            height: visible ? 4 : 0
            x: -hifi.dimensions.contentMargin.x
            anchors.top: heading.bottom
            start: Qt.point(0, 0)
            end: Qt.point(0, 4)
            gradient: Gradient {
                GradientStop { position: 0.0; color: hifi.colors.darkGray }
                GradientStop { position: 1.0; color: hifi.colors.baseGray }  // Equivalent of darkGray0 over baseGray background.
            }
            cached: true
        }
    }
}
