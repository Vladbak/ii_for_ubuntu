//
//  entityPropertyDialogBox.js
//  examples
//
//  Created by Brad hefta-Gaub on 10/1/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  This script implements a class useful for building tools for editing entities.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

var DEGREES_TO_RADIANS = Math.PI / 180.0;
var RADIANS_TO_DEGREES = 180.0 / Math.PI;

EntityPropertyDialogBox = (function () {
    var that = {};

    var propertiesForEditedEntity;
    var editEntityFormArray;
    var decimals = 3;
    var dimensionX;
    var dimensionY;
    var dimensionZ;
    var rescalePercentage;
    var editModelID = -1;
    var previousAnimationIsPlaying;
    var previousAnimationCurrentFrame;

    that.cleanup = function () {
    };

    that.openDialog = function (entityID) {
        print("  Edit Properties.... about to edit properties...");

        editModelID = entityID;
        propertiesForEditedEntity = Entities.getEntityProperties(editModelID);
        var properties = propertiesForEditedEntity;

        var array = new Array();
        var index = 0;

        array.push({ label: "Entity Type:" + properties.type, type: "header" });
        index++;
        array.push({ label: "ID:", value: properties.id });
        index++;
        array.push({ label: "Locked:", type: "checkbox", value: properties.locked });
        index++;

        if (properties.type == "Model") {
            array.push({ label: "Model URL:", value: properties.modelURL });
            index++;
            array.push({ label: "Shape Type:", value: properties.shapeType });
            index++;
            array.push({ label: "Compound Shape URL:", value: properties.compoundShapeURL });
            index++;
            array.push({ label: "Animation URL:", value: properties.animation.url });
            index++;
            array.push({ label: "Animation is playing:", type: "checkbox", value: properties.animation.running });
            previousAnimationIsPlaying = properties.animation.running;
            index++;
            array.push({ label: "Animation FPS:", value: properties.animation.fps });
            index++;
            array.push({ label: "Animation Frame:", value: properties.animation.currentFrame });
            previousAnimationCurrentFrame = properties.animation.currentFrame;
            index++;
            array.push({ label: "Textures:", value: properties.textures });
            index++;
            array.push({ label: "Original Textures:\n" + properties.originalTextures, type: "header" });
            index++;
        }

        if (properties.type == "Text") {
            array.push({ label: "Text:", value: properties.text });
            index++;
            array.push({ label: "Line Height:", value: properties.lineHeight });
            index++;
            array.push({ label: "Text Color:", type: "header" });
            index++;
            array.push({ label: "Red:", value: properties.textColor.red });
            index++;
            array.push({ label: "Green:", value: properties.textColor.green });
            index++;
            array.push({ label: "Blue:", value: properties.textColor.blue });
            index++;
            array.push({ label: "Background Color:", type: "header" });
            index++;
            array.push({ label: "Red:", value: properties.backgroundColor.red });
            index++;
            array.push({ label: "Green:", value: properties.backgroundColor.green });
            index++;
            array.push({ label: "Blue:", value: properties.backgroundColor.blue });
            index++;
        }

        if (properties.type == "PolyVox") {
            array.push({ label: "Voxel Space Size:", type: "header" });
            index++;

            array.push({ label: "X:", value: properties.voxelVolumeSize.x.toFixed(decimals) });
            index++;
            array.push({ label: "Y:", value: properties.voxelVolumeSize.y.toFixed(decimals) });
            index++;
            array.push({ label: "Z:", value: properties.voxelVolumeSize.z.toFixed(decimals) });
            index++;

            array.push({ label: "Surface Extractor", value: properties.voxelSurfaceStyle });
            index++;

            array.push({ label: "X-axis Texture URL:", value: properties.xTextureURL });
            index++;
            array.push({ label: "Y-axis Texture URL:", value: properties.yTextureURL });
            index++;
            array.push({ label: "Z-axis Texture URL:", value: properties.zTextureURL });
            index++;
        }

        array.push({ label: "Position:", type: "header" });
        index++;
        array.push({ label: "X:", value: properties.position.x.toFixed(decimals) });
        index++;
        array.push({ label: "Y:", value: properties.position.y.toFixed(decimals) });
        index++;
        array.push({ label: "Z:", value: properties.position.z.toFixed(decimals) });
        index++;

        array.push({ label: "Registration X:", value: properties.registrationPoint.x.toFixed(decimals) });
        index++;
        array.push({ label: "Registration Y:", value: properties.registrationPoint.y.toFixed(decimals) });
        index++;
        array.push({ label: "Registration Z:", value: properties.registrationPoint.z.toFixed(decimals) });
        index++;

        array.push({ label: "Rotation:", type: "header" });
        index++;
        var angles = Quat.safeEulerAngles(properties.rotation);
        array.push({ label: "Pitch:", value: angles.x.toFixed(decimals) });
        index++;
        array.push({ label: "Yaw:", value: angles.y.toFixed(decimals) });
        index++;
        array.push({ label: "Roll:", value: angles.z.toFixed(decimals) });
        index++;

        array.push({ label: "Dimensions:", type: "header" });
        index++;
        array.push({ label: "Width:", value: properties.dimensions.x.toFixed(decimals) });
        dimensionX = index;
        index++;
        array.push({ label: "Height:", value: properties.dimensions.y.toFixed(decimals) });
        dimensionY = index;
        index++;
        array.push({ label: "Depth:", value: properties.dimensions.z.toFixed(decimals) });
        dimensionZ = index;
        index++;
        array.push({ label: "", type: "inlineButton", buttonLabel: "Reset to Natural Dimensions", name: "resetDimensions" });
        index++;
        array.push({ label: "Rescale Percentage:", value: 100 });
        rescalePercentage = index;
        index++;
        array.push({ label: "", type: "inlineButton", buttonLabel: "Rescale", name: "rescaleDimensions" });
        index++;

        array.push({ label: "Velocity:", type: "header" });
        index++;
        array.push({ label: "Linear X:", value: properties.velocity.x.toFixed(decimals) });
        index++;
        array.push({ label: "Linear Y:", value: properties.velocity.y.toFixed(decimals) });
        index++;
        array.push({ label: "Linear Z:", value: properties.velocity.z.toFixed(decimals) });
        index++;
        array.push({ label: "Linear Damping:", value: properties.damping.toFixed(decimals) });
        index++;
        // NOTE: angular velocity is in radians/sec but we display degrees/sec for users
        array.push({ label: "Angular Pitch:", value: (properties.angularVelocity.x * RADIANS_TO_DEGREES).toFixed(decimals) });
        index++;
        array.push({ label: "Angular Yaw:", value: (properties.angularVelocity.y * RADIANS_TO_DEGREES).toFixed(decimals) });
        index++;
        array.push({ label: "Angular Roll:", value: (properties.angularVelocity.z * RADIANS_TO_DEGREES).toFixed(decimals) });
        index++;
        array.push({ label: "Angular Damping:", value: properties.angularDamping.toFixed(decimals) });
        index++;

        array.push({ label: "Gravity X:", value: properties.gravity.x.toFixed(decimals) });
        index++;
        array.push({ label: "Gravity Y:", value: properties.gravity.y.toFixed(decimals) });
        index++;
        array.push({ label: "Gravity Z:", value: properties.gravity.z.toFixed(decimals) });
        index++;

        array.push({ label: "Acceleration X:", value: properties.acceleration.x.toFixed(decimals) });
        index++;
        array.push({ label: "Acceleration Y:", value: properties.acceleration.y.toFixed(decimals) });
        index++;
        array.push({ label: "Acceleration Z:", value: properties.acceleration.z.toFixed(decimals) });
        index++;

        array.push({ label: "Collisions:", type: "header" });
        index++;
        array.push({ label: "Density:", value: properties.density.toFixed(decimals) });
        index++;
        array.push({ label: "Ignore for Collisions:", type: "checkbox", value: properties.ignoreForCollisions });
        index++;
        array.push({ label: "Dynamic:", type: "checkbox", value: properties.dynamic });
        index++;
        array.push({ label: "Collision Sound URL:", value: properties.collisionSoundURL });
        index++;

        array.push({ label: "Lifetime:", value: properties.lifetime.toFixed(decimals) });
        index++;

        array.push({ label: "Visible:", type: "checkbox", value: properties.visible });
        index++;

        array.push({ label: "Script:", value: properties.script });
        index++;

        if (properties.type == "Box" || properties.type == "Sphere") {
            array.push({ label: "Color:", type: "header" });
            index++;
            array.push({ label: "Red:", value: properties.color.red });
            index++;
            array.push({ label: "Green:", value: properties.color.green });
            index++;
            array.push({ label: "Blue:", value: properties.color.blue });
            index++;
        }

        if (properties.type == "Light") {
            array.push({ label: "Light Properties:", type: "header" });
            index++;
            array.push({ label: "Is Spot Light:", value: properties.isSpotlight });
            index++;
            array.push({ label: "Diffuse Red:", value: properties.diffuseColor.red });
            index++;
            array.push({ label: "Diffuse Green:", value: properties.diffuseColor.green });
            index++;
            array.push({ label: "Diffuse Blue:", value: properties.diffuseColor.blue });
            index++;
            array.push({ label: "Ambient Red:", value: properties.ambientColor.red });
            index++;
            array.push({ label: "Ambient Green:", value: properties.ambientColor.green });
            index++;
            array.push({ label: "Ambient Blue:", value: properties.ambientColor.blue });
            index++;
            array.push({ label: "Specular Red:", value: properties.specularColor.red });
            index++;
            array.push({ label: "Specular Green:", value: properties.specularColor.green });
            index++;
            array.push({ label: "Specular Blue:", value: properties.specularColor.blue });
            index++;
            array.push({ label: "Constant Attenuation:", value: properties.constantAttenuation });
            index++;
            array.push({ label: "Linear Attenuation:", value: properties.linearAttenuation });
            index++;
            array.push({ label: "Quadratic Attenuation:", value: properties.quadraticAttenuation });
            index++;
            array.push({ label: "Exponent:", value: properties.exponent });
            index++;
            array.push({ label: "Cutoff (in degrees):", value: properties.cutoff });
            index++;
        }

        array.push({ label: "", type: "inlineButton", buttonLabel: "Preview Camera", name: "previewCamera" });
        index++;

        array.push({ button: "Cancel" });
        index++;

        editEntityFormArray = array;
        Window.nonBlockingForm("Edit Properties", array);
    };

    Window.inlineButtonClicked.connect(function (name) {
        if (name == "previewCamera") {
            Camera.mode = "entity";
            Camera.cameraEntity = propertiesForEditedEntity.id;
        }

        if (name == "resetDimensions") {
            Window.reloadNonBlockingForm([
                { value: propertiesForEditedEntity.naturalDimensions.x.toFixed(decimals), oldIndex: dimensionX },
                { value: propertiesForEditedEntity.naturalDimensions.y.toFixed(decimals), oldIndex: dimensionY },
                { value: propertiesForEditedEntity.naturalDimensions.z.toFixed(decimals), oldIndex: dimensionZ }
            ]);
        }

        if (name == "rescaleDimensions") {
            var peekValues = editEntityFormArray;
            Window.peekNonBlockingFormResult(peekValues);
            var peekX = peekValues[dimensionX].value;
            var peekY = peekValues[dimensionY].value;
            var peekZ = peekValues[dimensionZ].value;
            var peekRescale = peekValues[rescalePercentage].value;
            var rescaledX = peekX * peekRescale / 100.0;
            var rescaledY = peekY * peekRescale / 100.0;
            var rescaledZ = peekZ * peekRescale / 100.0;

            Window.reloadNonBlockingForm([
                { value: rescaledX.toFixed(decimals), oldIndex: dimensionX },
                { value: rescaledY.toFixed(decimals), oldIndex: dimensionY },
                { value: rescaledZ.toFixed(decimals), oldIndex: dimensionZ },
                { value: 100, oldIndex: rescalePercentage }
            ]);
        }

    });
    Window.nonBlockingFormClosed.connect(function() {
        array = editEntityFormArray;
        if (Window.getNonBlockingFormResult(array)) {
            var properties = propertiesForEditedEntity;
            var index = 0;
            index++; // skip type header
            index++; // skip id item
            properties.locked = array[index++].value;
            if (properties.type == "Model") {
                properties.modelURL = array[index++].value;
                properties.shapeType = array[index++].value;
                properties.compoundShapeURL = array[index++].value;
                properties.animation.url = array[index++].value;

                var newAnimationIsPlaying = array[index++].value;
                if (previousAnimationIsPlaying != newAnimationIsPlaying) {
                    properties.animation.running = newAnimationIsPlaying;
                } else {
                    delete properties.animation.running;
                }

                properties.animation.fps = array[index++].value;
                
                var newAnimationCurrentFrame = array[index++].value;
                if (previousAnimationCurrentFrame != newAnimationCurrentFrame) {
                    properties.animation.currentFrame = newAnimationCurrentFrame;
                } else {
                    delete properties.animation.currentFrame;
                }
                
                properties.textures = array[index++].value;
                index++; // skip textureNames label
            }

            if (properties.type == "Text") {
                properties.text = array[index++].value;
                properties.lineHeight = array[index++].value;

                index++; // skip header
                properties.textColor.red = array[index++].value;
                properties.textColor.green = array[index++].value;
                properties.textColor.blue = array[index++].value;

                index++; // skip header
                properties.backgroundColor.red = array[index++].value;
                properties.backgroundColor.green = array[index++].value;
                properties.backgroundColor.blue = array[index++].value;
            }

            if (properties.type == "PolyVox") {
                properties.shapeType = array[index++].value;

                index++; // skip header
                properties.voxelVolumeSize.x = array[index++].value;
                properties.voxelVolumeSize.y = array[index++].value;
                properties.voxelVolumeSize.z = array[index++].value;
                properties.voxelSurfaceStyle = array[index++].value;
                properties.xTextureURL = array[index++].value;
                properties.yTextureURL = array[index++].value;
                properties.zTextureURL = array[index++].value;
            }

            index++; // skip header
            properties.position.x = array[index++].value;
            properties.position.y = array[index++].value;
            properties.position.z = array[index++].value;
            properties.registrationPoint.x = array[index++].value;
            properties.registrationPoint.y = array[index++].value;
            properties.registrationPoint.z = array[index++].value;

            index++; // skip header
            var angles = Quat.safeEulerAngles(properties.rotation);
            angles.x = array[index++].value;
            angles.y = array[index++].value;
            angles.z = array[index++].value;
            properties.rotation = Quat.fromVec3Degrees(angles);

            index++; // skip header
            properties.dimensions.x = array[index++].value;
            properties.dimensions.y = array[index++].value;
            properties.dimensions.z = array[index++].value;
            index++; // skip reset button
            index++; // skip rescale percentage
            index++; // skip rescale button

            index++; // skip header
            properties.velocity.x = array[index++].value;
            properties.velocity.y = array[index++].value;
            properties.velocity.z = array[index++].value;
            properties.damping = array[index++].value;

            // NOTE: angular velocity is in radians/sec but we display degrees/sec for users
            properties.angularVelocity.x = array[index++].value * DEGREES_TO_RADIANS;
            properties.angularVelocity.y = array[index++].value * DEGREES_TO_RADIANS;
            properties.angularVelocity.z = array[index++].value * DEGREES_TO_RADIANS;
            properties.angularDamping = array[index++].value;

            properties.gravity.x = array[index++].value;
            properties.gravity.y = array[index++].value;
            properties.gravity.z = array[index++].value;

            properties.acceleration.x = array[index++].value;
            properties.acceleration.y = array[index++].value;
            properties.acceleration.z = array[index++].value;

            index++; // skip header
            properties.density = array[index++].value;
            properties.ignoreForCollisions = array[index++].value;
            properties.dynamic = array[index++].value;

            properties.lifetime = array[index++].value;
            properties.visible = array[index++].value;
            properties.script = array[index++].value;

            if (properties.type == "Box" || properties.type == "Sphere") {
                index++; // skip header
                properties.color.red = array[index++].value;
                properties.color.green = array[index++].value;
                properties.color.blue = array[index++].value;
            }
            if (properties.type == "Light") {
                index++; // skip header
                properties.isSpotlight = array[index++].value;
                properties.diffuseColor.red = array[index++].value;
                properties.diffuseColor.green = array[index++].value;
                properties.diffuseColor.blue = array[index++].value;
                properties.ambientColor.red = array[index++].value;
                properties.ambientColor.green = array[index++].value;
                properties.ambientColor.blue = array[index++].value;
                properties.specularColor.red = array[index++].value;
                properties.specularColor.green = array[index++].value;
                properties.specularColor.blue = array[index++].value;
                properties.constantAttenuation = array[index++].value;
                properties.linearAttenuation = array[index++].value;
                properties.quadraticAttenuation = array[index++].value;
                properties.exponent = array[index++].value;
                properties.cutoff = array[index++].value;
            }

            Entities.editEntity(editModelID, properties);
            if (typeof(selectionDisplay) != "undefined") {
                selectionDisplay.select(editModelID, false);
            }
        }
        modelSelected = false;
    });

    return that;

}());
