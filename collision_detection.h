//
// collision_detection.h
// FBRunner3 - Optimized collision detection routines
//
// Provides efficient collision detection functions for game development:
// - Circle vs Circle collision
// - Circle vs Rectangle collision (with special bottom-edge detection for platforms)
// - Rectangle vs Rectangle collision (AABB)
// - Point containment tests
//
// All functions use optimized algorithms (avoiding sqrt when possible)
//

#ifndef COLLISION_DETECTION_H
#define COLLISION_DETECTION_H

namespace SuperTerminal {
namespace Collision {

// =============================================================================
// Circle-Circle Collision Detection
// =============================================================================

// Returns true if two circles intersect
// Parameters:
//   x1, y1, r1: Center and radius of first circle
//   x2, y2, r2: Center and radius of second circle
bool circleCircleCollision(float x1, float y1, float r1,
                           float x2, float y2, float r2);

// =============================================================================
// Circle-Rectangle Collision Detection
// =============================================================================

// Returns true if a circle intersects with a rectangle
// Parameters:
//   cx, cy, radius: Circle center and radius
//   rx, ry, rw, rh: Rectangle position (top-left corner) and dimensions
bool circleRectCollision(float cx, float cy, float radius,
                         float rx, float ry, float rw, float rh);

// Special version that checks if bottom of circle hits top of rectangle
// Useful for platform/paddle collision where you want specific edge detection
// Returns true if:
//   1. Bottom of circle (cy + radius) has reached or passed top of rectangle (ry)
//   2. Circle center is horizontally within rectangle bounds
// This is the correct way to detect ball-paddle collisions!
bool circleRectCollisionBottom(float cx, float cy, float radius,
                               float rx, float ry, float rw, float rh);

// =============================================================================
// Rectangle-Rectangle Collision Detection (AABB)
// =============================================================================

// Returns true if two rectangles intersect (Axis-Aligned Bounding Box collision)
// Parameters:
//   x1, y1, w1, h1: First rectangle position (top-left corner) and dimensions
//   x2, y2, w2, h2: Second rectangle position (top-left corner) and dimensions
bool rectRectCollision(float x1, float y1, float w1, float h1,
                       float x2, float y2, float w2, float h2);

// =============================================================================
// Point Containment Tests
// =============================================================================

// Returns true if a point is inside a circle
bool pointInCircle(float px, float py, float cx, float cy, float radius);

// Returns true if a point is inside a rectangle
bool pointInRect(float px, float py, float rx, float ry, float rw, float rh);

// =============================================================================
// Advanced Collision Detection with Physics Info
// =============================================================================

// Collision info structure for physics response
struct CollisionInfo {
    bool colliding;
    float penetrationDepth;
    float normalX;
    float normalY;
};

// Returns detailed collision info for circle-rectangle collision
// Includes penetration depth and collision normal for physics response
CollisionInfo circleRectCollisionInfo(float cx, float cy, float radius,
                                      float rx, float ry, float rw, float rh);

// Get the penetration depth for circle-circle collision
float circleCirclePenetration(float x1, float y1, float r1,
                              float x2, float y2, float r2);

// Get the overlap amounts for rectangle-rectangle collision
void rectRectOverlap(float x1, float y1, float w1, float h1,
                     float x2, float y2, float w2, float h2,
                     float* overlapX, float* overlapY);

} // namespace Collision
} // namespace SuperTerminal

#endif // COLLISION_DETECTION_H