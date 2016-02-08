//
//  SwingTwistConstraint.h
//
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SwingTwistConstraint_h
#define hifi_SwingTwistConstraint_h

#include <vector>

#include "RotationConstraint.h"

#include <math.h>

class SwingTwistConstraint : public RotationConstraint {
public:
    // The SwingTwistConstraint starts in the "referenceRotation" and then measures an initial twist
    // about the yAxis followed by a swing about some axis that lies in the XZ plane, such that the twist
    // and swing combine to produce the rotation.  Each partial rotation is constrained within limits
    // then used to construct the new final rotation.

    SwingTwistConstraint();

    /// \param minDots vector of minimum dot products between the twist and swung axes
    /// \brief The values are minimum dot-products between the twist axis and the swung axes
    ///  that correspond to swing axes equally spaced around the XZ plane.  Another way to
    ///  think about it is that the dot-products correspond to correspond to angles (theta)
    ///  about the twist axis ranging from 0 to 2PI-deltaTheta (Note: the cyclic boundary
    ///  conditions are handled internally, so don't duplicate the dot-product at 2PI).
    ///  See the paper by Quang Liu and Edmond C. Prakash mentioned below for a more detailed
    ///  description of how this works.
    void setSwingLimits(std::vector<float> minDots);

    /// \param swungDirections vector of directions that lie on the swing limit boundary
    /// \brief For each swungDirection we compute the corresponding [theta, minDot] pair.
    /// We expect the values of theta to NOT be uniformly spaced around the range [0, TWO_PI]
    /// so we'll use the input set to extrapolate a lookup function of evenly spaced values.
    void setSwingLimits(const std::vector<glm::vec3>& swungDirections);


    /// \param minTwist the minimum angle of rotation about the twist axis
    /// \param maxTwist the maximum angle of rotation about the twist axis
    void setTwistLimits(float minTwist, float maxTwist);

    /// \param rotation[in/out] the current rotation to be modified to fit within constraints
    /// \return true if rotation is changed
    virtual bool apply(glm::quat& rotation) const override;

    void setLowerSpine(bool lowerSpine) { _lowerSpine = lowerSpine; }
    virtual bool isLowerSpine() const override { return _lowerSpine; }

    // SwingLimitFunction is an implementation of the constraint check described in the paper:
    // "The Parameterization of Joint Rotation with the Unit Quaternion" by Quang Liu and Edmond C. Prakash
    class SwingLimitFunction {
    public:
        SwingLimitFunction();

        /// \brief use a uniform conical swing limit
        void setCone(float maxAngle);

        /// \brief use a vector of lookup values for swing limits
        void setMinDots(const std::vector<float>& minDots);

        /// \return minimum dotProduct between reference and swung axes
        float getMinDot(float theta) const;

    protected:
        // the limits are stored in a lookup table with cyclic boundary conditions
        std::vector<float> _minDots;
    };

    /// \return reference to SwingLimitFunction instance for unit-testing
    const SwingLimitFunction& getSwingLimitFunction() const { return _swingLimitFunction; }

    /// \brief exposed for unit testing
    void clearHistory();

protected:
    SwingLimitFunction _swingLimitFunction;
    float _minTwist;
    float _maxTwist;

    // We want to remember the LAST clamped boundary, so we an use it even when the far boundary is closer.
    // This reduces "pops" when the input twist angle goes far beyond and wraps around toward the far boundary.
    mutable int _lastTwistBoundary;
    bool _lowerSpine { false };
};

#endif // hifi_SwingTwistConstraint_h
