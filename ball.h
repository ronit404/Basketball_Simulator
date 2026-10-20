#ifndef BASKETBALL_BALL_H
#define BASKETBALL_BALL_H

#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum BallState {
    BALL_IN_HAND, // Resting in player's hands, moves with player
    BALL_IN_AIR   // Flying in active projectile motion
};

// Physics and Court Constants
#define BALL_RADIUS          0.18f
#define RIM_RADIUS           0.23f
#define RIM_TUBE_RADIUS      0.016f
#define RIM_HEIGHT           3.05f
#define THREE_POINT_RADIUS   6.75f
#define CORNER_3PT_DISTANCE  6.60f

#define BACKBOARD_X          12.80f
#define BACKBOARD_WIDTH      1.80f
#define BACKBOARD_HEIGHT     1.05f
#define BACKBOARD_THICKNESS  0.05f
#define BACKBOARD_Y_MIN      (RIM_HEIGHT - 0.275f)
#define BACKBOARD_Y_MAX      (BACKBOARD_Y_MIN + BACKBOARD_HEIGHT)
#define BACKBOARD_Z_MIN      (-BACKBOARD_WIDTH / 2.0f)
#define BACKBOARD_Z_MAX      ( BACKBOARD_WIDTH / 2.0f)

#define HOOP_CENTER_X        12.425f
#define HOOP_CENTER_Y        RIM_HEIGHT
#define HOOP_CENTER_Z        0.0f

#define GRAVITY_ACCEL        9.81f
#define GROUND_RESTITUTION   0.70f
#define GROUND_FRICTION      0.85f
#define BACKBOARD_ELASTICITY 0.65f
#define RIM_ELASTICITY       0.60f

#define BALL_MAX_TRAIL_POINTS 128

struct BallTrailPoint {
    float x, y, z;
};

struct Ball {
    enum BallState state;
    float x, y, z;
    float prevY;
    float vx, vy, vz;
    float spinAngle;
    float spinSpeed;

    // Shot tracking
    float shotOriginX;
    float shotOriginZ;
    float shotDistance;
    bool shotWas3Pointer;
    bool hasScored;

    // Motion trail ring buffer
    struct BallTrailPoint trail[BALL_MAX_TRAIL_POINTS];
    int trailCount;
};

void ball_init(struct Ball* b);
void ball_reset_to_hands(struct Ball* b, float playerX, float playerZ, float playerRotDeg);
void ball_shoot(struct Ball* b, float originX, float originZ, float angleDeg, float powerPct, float playerRotDeg);
bool ball_update_physics(struct Ball* b, float deltaTime, int* outScoredPoints);
bool ball_is_at_rest_on_floor(const struct Ball* b);
bool ball_is_inside_three_point_zone(float x, float z);
float ball_calculate_distance_to_hoop(float x, float z);

#endif // BASKETBALL_BALL_H
