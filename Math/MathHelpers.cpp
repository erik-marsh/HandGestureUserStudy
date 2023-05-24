#include "MathHelpers.hpp"

namespace Math
{

using Vec3 = Vector3Common;

bool IsVectorInCone(Vec3 coneAxis, float coneAngle, Vec3 vector)
{
    // We assume all inputs are unit vectors (length 1)
    // To accomodate this, we assume that the cone has a height of 2
    // We could probably get away with a height of 1 but I'm too paranoid for that
    constexpr float coneHeight = 2;

    // This function implements an algorithm that detects if a point is inside a cone.
    // All of our vectors start at the origin, so we can assume that they are points.
    // The algorithm itself is trivial (read: I don't want to explain it right now).
    // However, we must do one step of preprocessing to find the radius of the cone's base given an
    // angle. That's what this step is:
    float coneBaseRadius = coneHeight * std::tan(coneAngle);

    // Find the projection of the input vector against the cone's axis
    float targetDist = Vec3::DotProduct(vector, coneAxis);

    // Find the radius of the cone at the target distance
    float targetConeRadius = (coneBaseRadius * targetDist) / coneHeight;
    float targetConeRadiusSquared = targetConeRadius * targetConeRadius;

    // Find the orthogonal component of the input vector to the targetDist vector
    Vec3 orthVector = Vec3::Subtract(vector, Vec3::ScalarMultiply(coneAxis, targetDist));
    float orthMagnitudeSquared = orthVector.MagnitudeSquared();  // squared for efficiency

    // Check if the orthoganal component falls within the target radius
    return orthMagnitudeSquared < targetConeRadiusSquared;
}

}