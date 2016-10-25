
//
//  Window.qml
//
//  Created by Bradley Austin Davis on 12 Jan 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0

import "."
import "../styles-uit"
import "../controls-uit" as HiFiControls

// FIXME how do I set the initial position of a window without
// overriding places where the a individual client of the window
// might be setting the position with a Settings{} element?

// FIXME how to I enable dragging without allowing the window to lay outside
// of the desktop?  How do I ensure when the desktop resizes all the windows
// are still at least partially visible?

Window {
    id: window
    HifiConstants { id: hifi }
    children: [ swallower, frame, defocuser, pane, activator ]

    property var footer: Item { }  // Optional static footer at the bottom of the dialog.
    readonly property var footerContentHeight: footer.height > 0 ? (footer.height + 2 * hifi.dimensions.contentSpacing.y + 3) : 0

    property bool keyboardOverride: false  // Set true in derived control if it implements its own keyboard.
    property bool keyboardEnabled: false
    property bool keyboardRaised: false
    property bool punctuationMode: false

    // Scrollable window content.
    // FIXME this should not define any visual content in this type.  The base window
    // type should only consist of logic sized areas, with nothing drawn (although the
    // default value for the frame property does include visual decorations)
    property var pane: Item {
        property bool isScrolling: scrollView.height < scrollView.contentItem.height
        property int contentWidth: scrollView.width - (isScrolling ? 10 : 0)
        property int scrollHeight: scrollView.height

        anchors.fill: parent
        anchors.rightMargin: isScrolling ? 11 : 0

        Rectangle {
            id: contentBackground
            anchors.fill: parent
            anchors.rightMargin: parent.isScrolling ? 11 : 0
            color: hifi.colors.baseGray
            visible: !window.hideBackground && modality != Qt.ApplicationModal
        }


        LinearGradient {
            visible: !window.hideBackground && gradientsSupported && modality != Qt.ApplicationModal
            anchors.top: contentBackground.bottom
            anchors.left: contentBackground.left
            width: contentBackground.width - 1
            height: 4
            start: Qt.point(0, 0)
            end: Qt.point(0, 4)
            gradient: Gradient {
                GradientStop { position: 0.0; color: hifi.colors.darkGray }
                GradientStop { position: 1.0; color: hifi.colors.darkGray0 }
            }
            cached: true
        }

        ScrollView {
            id: scrollView
            contentItem: content
            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
            verticalScrollBarPolicy: Qt.ScrollBarAsNeeded
            anchors.fill: parent
            anchors.rightMargin: parent.isScrolling ? 1 : 0
            anchors.bottomMargin: footerPane.height

            style: ScrollViewStyle {

                padding.right: -7  // Move to right away from content.

                handle: Item {
                    implicitWidth: 8
                    Rectangle {
                        radius: 4
                        color: hifi.colors.white30
                        anchors {
                            fill: parent
                            leftMargin: 2  // Finesse size and position.
                            topMargin: 1
                            bottomMargin: 1
                        }
                    }
                }

                scrollBarBackground: Item {
                    implicitWidth: 10
                    Rectangle {
                        color: hifi.colors.darkGray30
                        radius: 4
                        anchors {
                            fill: parent
                            topMargin: -1  // Finesse size
                            bottomMargin: -2
                        }
                    }
                }

                incrementControl: Item {
                    visible: false
                }

                decrementControl: Item {
                    visible: false
                }
            }
        }

        function scrollBy(delta) {
            scrollView.flickableItem.contentY += delta;
        }

        Rectangle {
            // Optional non-scrolling footer.
            id: footerPane

            property alias keyboardOverride: window.keyboardOverride
            property alias keyboardRaised: window.keyboardRaised
            property alias punctuationMode: window.punctuationMode

            anchors {
                left: parent.left
                bottom: parent.bottom
            }
            width: parent.contentWidth
            height: footerContentHeight + (keyboard.enabled && keyboard.raised ? keyboard.height : 0)
            color: hifi.colors.baseGray
            visible: footer.height > 0 || keyboard.enabled && keyboard.raised

            Item {
                // Horizontal rule.
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }

                visible: footer.height > 0

                Rectangle {
                    width: parent.width
                    height: 1
                    y: 1  // Stop displaying content just above horizontal rule/=.
                    color: hifi.colors.baseGrayShadow
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    y: 2
                    color: hifi.colors.baseGrayHighlight
                }
            }

            Item {
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    topMargin: hifi.dimensions.contentSpacing.y + 3
                }
                children: [ footer ]
            }

            HiFiControls.Keyboard {
                id: keyboard
                enabled: !keyboardOverride
                raised: keyboardEnabled && keyboardRaised
                numeric: punctuationMode
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
            }
        }
    }

    onKeyboardRaisedChanged: {
        if (!keyboardOverride && keyboardEnabled && keyboardRaised) {
            var delta = activator.mouseY
                    - (activator.height + activator.y - keyboard.raisedHeight - footerContentHeight - hifi.dimensions.controlLineHeight);

            if (delta > 0) {
                pane.scrollBy(delta);
            } else {
                // HACK: Work around for case where are 100% scrolled; stops window from erroneously scrolling to 100% when show keyboard.
                pane.scrollBy(-1);
                pane.scrollBy(1);
            }
        }
    }

    Component.onCompleted: {
        if (typeof HMD !== "undefined") {
            keyboardEnabled = HMD.active;
        }
    }
}
