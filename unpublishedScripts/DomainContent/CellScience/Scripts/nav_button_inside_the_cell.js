//  Copyright 2016 High Fidelity, Inc.
//
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(function() {

    var version = 13;

    var baseURL = "https://hifi-production.s3.amazonaws.com/DomainContent/CellScience/";
    var button;
    var _this;

    function NavButton() {
        _this = this;
    }

    NavButton.prototype = {
        button: null,
        buttonImageURL: baseURL + "GUI/GUI_CellLayout.png?" + version,
        hasButton: false,
        entryPoint: {
            x: 3000,
            y: 13500,
            z: 3000
        },
        target: {
            x: 3276.6,
            y: 13703.3,
            z: 4405.6
        },
        preload: function(entityId) {
            print('CeLL PRELOAD INSIDECELL 1')
            this.entityId = entityId;
            this.addButton();
            Controller.mousePressEvent.connect(this.onClick);
            print('CeLL PRELOAD INSIDECELL 2')
        },
        addButton: function() {
            if (this.hasButton === false) {
                print('CELL ADDBUTTON INSIDECELL 1')
                var windowDimensions = Controller.getViewportDimensions();
                var buttonWidth = 150;
                var buttonHeight = 50;
                var buttonPadding = 10;
                var offset = 1;
                var buttonPositionX = (offset + 1) * (buttonWidth + buttonPadding) + (windowDimensions.x / 2) - (buttonWidth * 3 + buttonPadding * 2.5);
                var buttonPositionY = (windowDimensions.y - buttonHeight) - 150;
                button = Overlays.addOverlay("image", {
                    x: buttonPositionX,
                    y: buttonPositionY,
                    width: buttonWidth,
                    height: buttonHeight,
                    imageURL: this.buttonImageURL,
                    visible: true,
                    alpha: 1.0
                });
                this.hasButton = true;
                print('CELL ADDBUTTON INSIDECELL 2 button id is : ' +button)
            } else {
                print('CELL ADDBUTTON INSIDECELL FAIL - hasButton is' + this.hasButton)
            }
        },
        onClick: function(event) {
            //call to an internal function to get our scope back;
            _this.handleClick(event);
        },
        handleClick: function(event) {
            var clickedOverlay = Overlays.getOverlayAtPoint({
                x: event.x,
                y: event.y
            });

            if (clickedOverlay === button) {
                this.lookAtTarget();
            }
        },
        lookAtTarget: function() {
            var direction = Vec3.normalize(Vec3.subtract(this.target, this.entryPoint));
            var pitch = Quat.angleAxis(Math.asin(-direction.y) * 180.0 / Math.PI, {
                x: 1,
                y: 0,
                z: 0
            });
            var yaw = Quat.angleAxis(Math.atan2(direction.x, direction.z) * 180.0 / Math.PI, {
                x: 0,
                y: 1,
                z: 0
            });

            MyAvatar.goToLocation(this.target, true, yaw);

            MyAvatar.headYaw = 0;
        },
        unload: function() {
            this.hasButton = false;
            Overlays.deleteOverlay(button);
            Controller.mousePressEvent.disconnect(this.onClick);
        }
    }


    return new NavButton();



});