#include "ball.h"
#include "player.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

void ball_init(struct Ball* b) {
    b->state = BALL_IN_HAND;
    b->x = 5.5f + BALL_HAND_OFFSET_DIST;
    b->y = BALL_HAND_OFFSET_Y;
    b->z = 0.0f;
    b->prevY = BALL_HAND_OFFSET_Y;
    b->vx = 0.0f;
    b->vy = 0.0f;
    b->vz = 0.0f;
    b->spinAngle = 0.0f;
    b->spinSpeed = 360.0f;
    b->shotOriginX = 5.5f;
    b->shotOriginZ = 0.0f;
    b->shotDistance = 0.0f;
    b->shotWas3Pointer = false;
    b->hasScored = false;
    b->trailCount = 0;
}

void ball_reset_to_hands(struct Ball* b, float playerX, float playerZ, float playerRotDeg) {
    b->state = BALL_IN_HAND;
    float rotRad = playerRotDeg * (float)M_PI / 180.0f;
    b->x = playerX + cosf(rotRad) * BALL_HAND_OFFSET_DIST;
    b->y = BALL_HAND_OFFSET_Y;
    b->z = playerZ - sinf(rotRad) * BALL_HAND_OFFSET_DIST;
    b->prevY = b->y;

    b->vx = 0.0f;
    b->vy = 0.0f;
    b->vz = 0.0f;
    b->trailCount = 0;
    b->hasScored = false;
}

void ball_shoot(struct Ball* b, float originX, float originZ, float angleDeg, float powerPct, float playerRotDeg) {
    b->state = BALL_IN_AIR;
    b->shotOriginX = originX;
    b->shotOriginZ = originZ;
    b->shotDistance = ball_calculate_distance_to_hoop(b->shotOriginX, b->shotOriginZ);
    b->shotWas3Pointer = ball_is_inside_three_point_zone(b->shotOriginX, b->shotOriginZ);

    float rotRad = playerRotDeg * (float)M_PI / 180.0f;
    b->x = originX + cosf(rotRad) * BALL_HAND_OFFSET_DIST;
    b->y = BALL_HAND_OFFSET_Y;
    b->z = originZ - sinf(rotRad) * BALL_HAND_OFFSET_DIST;
    b->prevY = b->y;
    b->trailCount = 0;
    b->hasScored = false;

    // Unit direction vector pointing from launch pocket towards target hoop center
    float targetDirX = HOOP_CENTER_X - b->x;
    float targetDirZ = HOOP_CENTER_Z - b->z;
    float distXZ = sqrtf(targetDirX * targetDirX + targetDirZ * targetDirZ);
    if (distXZ > 0.0001f) {
        targetDirX /= distXZ;
        targetDirZ /= distXZ;
    }

    float initialSpeed = 3.0f + (powerPct / 100.0f) * 11.5f;
    float angleRad = angleDeg * (float)M_PI / 180.0f;

    float speedXZ = initialSpeed * cosf(angleRad);
    b->vy = initialSpeed * sinf(angleRad);
    b->vx = speedXZ * targetDirX;
    b->vz = speedXZ * targetDirZ;
}

static void ball_handle_collisions(struct Ball* b, int* outScoredPoints) {
    *outScoredPoints = 0;

    // 1. Ground Collision
    if (b->y - BALL_RADIUS <= 0.0f) {
        b->y = BALL_RADIUS;
        b->vy = -b->vy * GROUND_RESTITUTION;
        b->vx *= GROUND_FRICTION;
        b->vz *= GROUND_FRICTION;

        if (fabsf(b->vy) < 0.25f) {
            b->vy = 0.0f;
        }
    }

    // 2. Glass Backboard Collision
    float backboardFrontX = BACKBOARD_X - (BACKBOARD_THICKNESS / 2.0f);
    if (b->y >= BACKBOARD_Y_MIN && b->y <= BACKBOARD_Y_MAX &&
        b->z >= BACKBOARD_Z_MIN && b->z <= BACKBOARD_Z_MAX) {
        
        if (b->x + BALL_RADIUS >= backboardFrontX &&
            b->x - BALL_RADIUS < BACKBOARD_X + (BACKBOARD_THICKNESS / 2.0f) &&
            b->vx > 0.0f) {
            
            b->x = backboardFrontX - BALL_RADIUS;
            b->vx = -b->vx * BACKBOARD_ELASTICITY;
            b->vy *= 0.85f;
            printf(">>> [COLLISION] Backboard Rebound!\n");
        }
    }

    // 3. Rim Torus Collision
    float dxRim = b->x - HOOP_CENTER_X;
    float dzRim = b->z - HOOP_CENTER_Z;
    float distXZFromCenter = sqrtf(dxRim * dxRim + dzRim * dzRim);

    if (distXZFromCenter > 0.001f) {
        float rimPointX = HOOP_CENTER_X + (dxRim / distXZFromCenter) * RIM_RADIUS;
        float rimPointY = HOOP_CENTER_Y;
        float rimPointZ = HOOP_CENTER_Z + (dzRim / distXZFromCenter) * RIM_RADIUS;

        float distVecX = b->x - rimPointX;
        float distVecY = b->y - rimPointY;
        float distVecZ = b->z - rimPointZ;
        float distToRim = sqrtf(distVecX * distVecX + distVecY * distVecY + distVecZ * distVecZ);

        float collisionThreshold = BALL_RADIUS + RIM_TUBE_RADIUS;
        if (distToRim < collisionThreshold && distToRim > 0.0001f) {
            float nx = distVecX / distToRim;
            float ny = distVecY / distToRim;
            float nz = distVecZ / distToRim;

            float dotVel = b->vx * nx + b->vy * ny + b->vz * nz;

            if (dotVel < 0.0f) {
                b->vx -= (1.0f + RIM_ELASTICITY) * dotVel * nx;
                b->vy -= (1.0f + RIM_ELASTICITY) * dotVel * ny;
                b->vz -= (1.0f + RIM_ELASTICITY) * dotVel * nz;

                b->x = rimPointX + nx * (collisionThreshold + 0.005f);
                b->y = rimPointY + ny * (collisionThreshold + 0.005f);
                b->z = rimPointZ + nz * (collisionThreshold + 0.005f);

                printf(">>> [COLLISION] Rim Clank!\n");
            }
        }
    }

    // 4. Basket Scoring Check
    if (!b->hasScored && b->vy < -0.2f) {
        if (distXZFromCenter <= (RIM_RADIUS - 0.02f)) {
            if (b->prevY >= HOOP_CENTER_Y && b->y <= HOOP_CENTER_Y + 0.05f) {
                b->hasScored = true;
                *outScoredPoints = b->shotWas3Pointer ? 3 : 2;

                // Net friction
                b->vx *= 0.40f;
                b->vz *= 0.40f;
                b->vy *= 0.70f;
            }
        }
    }

    b->prevY = b->y;
}

bool ball_update_physics(struct Ball* b, float deltaTime, int* outScoredPoints) {
    *outScoredPoints = 0;
    if (b->state != BALL_IN_AIR) return false;

    // Apply gravity
    b->vy -= GRAVITY_ACCEL * deltaTime;

    // Numerical integration
    b->x += b->vx * deltaTime;
    b->y += b->vy * deltaTime;
    b->z += b->vz * deltaTime;

    // Spin animation
    b->spinAngle += b->spinSpeed * deltaTime;
    if (b->spinAngle > 360.0f) b->spinAngle -= 360.0f;

    // Trail recording
    if (b->trailCount == 0 || fabsf(b->x - b->trail[b->trailCount - 1].x) > 0.04f) {
        if (b->trailCount < BALL_MAX_TRAIL_POINTS) {
            b->trail[b->trailCount].x = b->x;
            b->trail[b->trailCount].y = b->y;
            b->trail[b->trailCount].z = b->z;
            b->trailCount++;
        } else {
            memmove(&b->trail[0], &b->trail[1], sizeof(struct BallTrailPoint) * (BALL_MAX_TRAIL_POINTS - 1));
            b->trail[BALL_MAX_TRAIL_POINTS - 1].x = b->x;
            b->trail[BALL_MAX_TRAIL_POINTS - 1].y = b->y;
            b->trail[BALL_MAX_TRAIL_POINTS - 1].z = b->z;
        }
    }

    ball_handle_collisions(b, outScoredPoints);
    return (*outScoredPoints > 0);
}

bool ball_is_at_rest_on_floor(const struct Ball* b) {
    return (b->state == BALL_IN_AIR && fabsf(b->vy) < 0.05f && b->y <= BALL_RADIUS + 0.015f);
}

bool ball_is_inside_three_point_zone(float px, float pz) {
    float dx = px - HOOP_CENTER_X;
    float dz = pz - HOOP_CENTER_Z;
    float dist = sqrtf(dx * dx + dz * dz);

    if (dist > THREE_POINT_RADIUS) return true;
    if (fabsf(pz) > CORNER_3PT_DISTANCE) return true;
    return false;
}

float ball_calculate_distance_to_hoop(float px, float pz) {
    float dx = px - HOOP_CENTER_X;
    float dz = pz - HOOP_CENTER_Z;
    return sqrtf(dx * dx + dz * dz);
}
