//
//  Overlay.cpp
//  interface/src/ui/overlays
//
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Overlay.h"

#include <NumericalConstants.h>
#include <RegisteredMetaTypes.h>

static const xColor DEFAULT_OVERLAY_COLOR = { 255, 255, 255 };
static const float DEFAULT_ALPHA = 0.7f;

Overlay::Overlay() :
    _renderItemID(render::Item::INVALID_ITEM_ID),
    _isLoaded(true),
    _alpha(DEFAULT_ALPHA),
    _pulse(0.0f),
    _pulseMax(0.0f),
    _pulseMin(0.0f),
    _pulsePeriod(1.0f),
    _pulseDirection(1.0f),
    _lastPulseUpdate(usecTimestampNow()),
    _alphaPulse(0.0f),
    _colorPulse(0.0f),
    _color(DEFAULT_OVERLAY_COLOR),
    _visible(true),
    _anchor(NO_ANCHOR)
{
}

Overlay::Overlay(const Overlay* overlay) :
    _renderItemID(render::Item::INVALID_ITEM_ID),
    _isLoaded(overlay->_isLoaded),
    _alpha(overlay->_alpha),
    _pulse(overlay->_pulse),
    _pulseMax(overlay->_pulseMax),
    _pulseMin(overlay->_pulseMin),
    _pulsePeriod(overlay->_pulsePeriod),
    _pulseDirection(overlay->_pulseDirection),
    _lastPulseUpdate(usecTimestampNow()),
    _alphaPulse(overlay->_alphaPulse),
    _colorPulse(overlay->_colorPulse),
    _color(overlay->_color),
    _visible(overlay->_visible),
    _anchor(overlay->_anchor)
{
}

Overlay::~Overlay() {
}

void Overlay::setProperties(const QVariantMap& properties) {
    bool valid;
    auto color = xColorFromVariant(properties["color"], valid);
    if (valid) {
        _color = color;
    }

    if (properties["alpha"].isValid()) {
        setAlpha(properties["alpha"].toFloat());
    }

    if (properties["pulseMax"].isValid()) {
        setPulseMax(properties["pulseMax"].toFloat());
    }

    if (properties["pulseMin"].isValid()) {
        setPulseMin(properties["pulseMin"].toFloat());
    }

    if (properties["pulsePeriod"].isValid()) {
        setPulsePeriod(properties["pulsePeriod"].toFloat());
    }

    if (properties["alphaPulse"].isValid()) {
        setAlphaPulse(properties["alphaPulse"].toFloat());
    }

    if (properties["colorPulse"].isValid()) {
        setColorPulse(properties["colorPulse"].toFloat());
    }

    if (properties["visible"].isValid()) {
        bool visible = properties["visible"].toBool();
        setVisible(visible);
    }

    if (properties["anchor"].isValid()) {
        QString property = properties["anchor"].toString();
        if (property == "MyAvatar") {
            setAnchor(MY_AVATAR);
        }
    }
}

QVariant Overlay::getProperty(const QString& property) {
    if (property == "color") {
        return xColorToVariant(_color);
    }
    if (property == "alpha") {
        return _alpha;
    }
    if (property == "pulseMax") {
        return _pulseMax;
    }
    if (property == "pulseMin") {
        return _pulseMin;
    }
    if (property == "pulsePeriod") {
        return _pulsePeriod;
    }
    if (property == "alphaPulse") {
        return _alphaPulse;
    }
    if (property == "colorPulse") {
        return _colorPulse;
    }
    if (property == "visible") {
        return _visible;
    }
    if (property == "anchor") {
        return _anchor == MY_AVATAR ? "MyAvatar" : "";
    }

    return QVariant();
}

xColor Overlay::getColor() { 
    if (_colorPulse == 0.0f) {
        return _color; 
    }

    float pulseLevel = updatePulse();
    xColor result = _color;
    if (_colorPulse < 0.0f) {
        result.red *= (1.0f - pulseLevel);
        result.green *= (1.0f - pulseLevel);
        result.blue *= (1.0f - pulseLevel);
    } else {
        result.red *= pulseLevel;
        result.green *= pulseLevel;
        result.blue *= pulseLevel;
    }
    return result;
}

float Overlay::getAlpha() {
    if (_alphaPulse == 0.0f) {
        return _alpha; 
    }
    float pulseLevel = updatePulse();
    return (_alphaPulse >= 0.0f) ? _alpha * pulseLevel : _alpha * (1.0f - pulseLevel);
}


// pulse travels from min to max, then max to min in one period.
float Overlay::updatePulse() {
    if (_pulsePeriod <= 0.0f) {
        return _pulse;
    }
    quint64 now = usecTimestampNow();
    quint64 elapsedUSecs = (now - _lastPulseUpdate);
    float elapsedSeconds =  (float)elapsedUSecs / (float)USECS_PER_SECOND;
    float elapsedPeriods = elapsedSeconds / _pulsePeriod;

    // we can safely remove any "full" periods, since those just rotate us back
    // to our final pulse level
    elapsedPeriods = fmod(elapsedPeriods, 1.0f);
    _lastPulseUpdate = now;

    float pulseDistance =  (_pulseMax - _pulseMin);
    float pulseDistancePerPeriod = pulseDistance * 2.0f;

    float pulseDelta = _pulseDirection * pulseDistancePerPeriod * elapsedPeriods;
    float newPulse = _pulse + pulseDelta;
    float limit = (_pulseDirection > 0.0f) ? _pulseMax : _pulseMin;
    float passedLimit = (_pulseDirection > 0.0f) ? (newPulse >= limit) : (newPulse <= limit);

    if (passedLimit) {
        float pulseDeltaToLimit = newPulse - limit;
        float pulseDeltaFromLimitBack = pulseDelta - pulseDeltaToLimit;
        pulseDelta = -pulseDeltaFromLimitBack;
        _pulseDirection *= -1.0f;
    }
    _pulse += pulseDelta;
    
    return _pulse;
}

bool Overlay::addToScene(Overlay::Pointer overlay, std::shared_ptr<render::Scene> scene, render::PendingChanges& pendingChanges) {
    _renderItemID = scene->allocateID();
    pendingChanges.resetItem(_renderItemID, std::make_shared<Overlay::Payload>(overlay));
    return true;
}

void Overlay::removeFromScene(Overlay::Pointer overlay, std::shared_ptr<render::Scene> scene, render::PendingChanges& pendingChanges) {
    pendingChanges.removeItem(_renderItemID);
    render::Item::clearID(_renderItemID);
}

