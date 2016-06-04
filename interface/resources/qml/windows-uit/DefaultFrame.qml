//
//  DefaultFrame.qml
//
//  Created by Bradley Austin Davis on 12 Jan 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import QtGraphicalEffects 1.0

import "."
import "../styles-uit"

Frame {
    HifiConstants { id: hifi }

    Rectangle {
        // Dialog frame
        id: frameContent

        readonly property int iconSize: hifi.dimensions.frameIconSize
        readonly property int frameMargin: 9
        readonly property int frameMarginLeft: frameMargin
        readonly property int frameMarginRight: frameMargin
        readonly property int frameMarginTop: 2 * frameMargin + iconSize
        readonly property int frameMarginBottom: iconSize + 11

        anchors {
            topMargin: -frameMarginTop
            leftMargin: -frameMarginLeft
            rightMargin: -frameMarginRight
            bottomMargin: -frameMarginBottom
        }
        anchors.fill: parent
        color: hifi.colors.baseGrayHighlight40
        border {
            width: hifi.dimensions.borderWidth
            color: hifi.colors.faintGray50
        }
        radius: hifi.dimensions.borderRadius

        // Enable dragging of the window
        MouseArea {
            anchors.fill: parent
            drag.target: window
        }

        Row {
            id: controlsRow
            anchors {
                right: parent.right;
                top: parent.top;
                topMargin: frameContent.frameMargin + 1  // Move down a little to visually align with the title
                rightMargin: frameContent.frameMarginRight;
            }
            spacing: frameContent.iconSize / 4

            HiFiGlyphs {
                // "Pin" button
                visible: false
                text: (frame.pinned && !pinClickArea.containsMouse) || (!frame.pinned && pinClickArea.containsMouse) ? hifi.glyphs.pinInverted : hifi.glyphs.pin
                color: pinClickArea.containsMouse && !pinClickArea.pressed ? hifi.colors.redHighlight : hifi.colors.white
                size: frameContent.iconSize
                MouseArea {
                    id: pinClickArea
                    anchors.fill: parent
                    hoverEnabled: true
                    propagateComposedEvents: true
                    onClicked: { frame.pin(); mouse.accepted = false; }
                }
            }

            HiFiGlyphs {
                // "Close" button
                visible: window ? window.closable : false
                text: closeClickArea.containsPress ? hifi.glyphs.closeInverted : hifi.glyphs.close
                color: closeClickArea.containsMouse ? hifi.colors.redHighlight : hifi.colors.white
                size: frameContent.iconSize
                MouseArea {
                    id: closeClickArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: window.visible = false;
                }
            }
        }

        RalewayRegular {
            // Title
            id: titleText
            anchors {
                left: parent.left
                leftMargin: frameContent.frameMarginLeft + hifi.dimensions.contentMargin.x
                right: controlsRow.left
                rightMargin: frameContent.iconSize
                top: parent.top
                topMargin: frameContent.frameMargin
            }
            text: window ? window.title : ""
            color: hifi.colors.white
            size: hifi.fontSizes.overlayTitle
        }

        DropShadow {
            source: titleText
            anchors.fill: titleText
            horizontalOffset: 2
            verticalOffset: 2
            samples: 2
            color: hifi.colors.baseGrayShadow60
            visible: (window && window.focus)
            cached: true
        }
    }
}

