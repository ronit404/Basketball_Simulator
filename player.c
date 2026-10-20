#include "player.h"
#include <math.h>

const struct PlayerColor PLAYER_COLORS[4] = {
    { 0.92f, 0.20f, 0.22f, "PLAYER 1 (RED)" },    // Player 1: Red
    { 0.18f, 0.55f, 0.95f, "PLAYER 2 (BLUE)" },   // Player 2: Blue
    { 0.15f, 0.82f, 0.35f, "PLAYER 3 (GREEN)" },  // Player 3: Green
    { 0.95f, 0.85f, 0.15f, "PLAYER 4 (YELLOW)" }  // Player 4: Yellow
};

void player_init(struct Player* p, int id, float startX, float startZ) {
    p->id = id;
    p->score = 0;
    p->x = startX;
    p->z = startZ;
    p->moveSpeed = 8.5f;

    float toBasketX = HOOP_TARGET_X - p->x;
    float toBasketZ = HOOP_TARGET_Z - p->z;
    p->rotationDeg = -atan2f(toBasketZ, toBasketX) * 180.0f / (float)M_PI;
}

void player_reset(struct Player* p, float startX, float startZ) {
    p->x = startX;
    p->z = startZ;
    float toBasketX = HOOP_TARGET_X - p->x;
    float toBasketZ = HOOP_TARGET_Z - p->z;
    p->rotationDeg = -atan2f(toBasketZ, toBasketX) * 180.0f / (float)M_PI;
}

void player_update_movement(struct Player* p, float deltaTime, const bool keys[256]) {
    float moveStep = p->moveSpeed * deltaTime;

    if (keys['w'] || keys['W']) p->x += moveStep;
    if (keys['s'] || keys['S']) p->x -= moveStep;
    if (keys['a'] || keys['A']) p->z -= moveStep;
    if (keys['d'] || keys['D']) p->z += moveStep;

    // Court boundaries clamp
    float maxCourtX = (COURT_LENGTH_BOUND / 2.0f) - 0.5f;
    float maxCourtZ = (COURT_WIDTH_BOUND / 2.0f) - 0.5f;

    if (p->x < -maxCourtX) p->x = -maxCourtX;
    if (p->x >  maxCourtX) p->x =  maxCourtX;
    if (p->z < -maxCourtZ) p->z = -maxCourtZ;
    if (p->z >  maxCourtZ) p->z =  maxCourtZ;

    // Dynamically calculate orientation facing the target hoop
    float toBasketX = HOOP_TARGET_X - p->x;
    float toBasketZ = HOOP_TARGET_Z - p->z;
    p->rotationDeg = -atan2f(toBasketZ, toBasketX) * 180.0f / (float)M_PI;
}

const struct PlayerColor* player_get_color(const struct Player* p) {
    int idx = p->id % 4;
    if (idx < 0) idx += 4;
    return &PLAYER_COLORS[idx];
}

void player_add_score(struct Player* p, int points) {
    if (p) p->score += points;
}

void player_set_score(struct Player* p, int score) {
    if (p) p->score = score;
}

