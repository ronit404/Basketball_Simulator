#ifndef BASKETBALL_PLAYER_H
#define BASKETBALL_PLAYER_H

#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Court boundaries for player movement (Meters)
#define COURT_LENGTH_BOUND 28.0f
#define COURT_WIDTH_BOUND  15.0f
#define HOOP_TARGET_X      12.425f
#define HOOP_TARGET_Z      0.0f

// Ball Shooting Pocket Relative Offsets (when in hands)
#define BALL_HAND_OFFSET_DIST 0.46f
#define BALL_HAND_OFFSET_Y    1.88f

// Team Jersey Color Palette
struct PlayerColor {
    float r, g, b;
    const char* name;
};

// 4 Distinct Tournament Team Colors (Red, Blue, Green, Yellow)
extern const struct PlayerColor PLAYER_COLORS[4];

struct Player {
    int id;
    int score;
    float x;
    float z;
    float rotationDeg; // Degrees facing target hoop
    float moveSpeed;   // Meters per second
};

void player_init(struct Player* p, int id, float startX, float startZ);
void player_reset(struct Player* p, float startX, float startZ);
void player_update_movement(struct Player* p, float deltaTime, const bool keys[256]);
const struct PlayerColor* player_get_color(const struct Player* p);
void player_add_score(struct Player* p, int points);
void player_set_score(struct Player* p, int score);

#endif // BASKETBALL_PLAYER_H
