#ifndef BASKETBALL_GAME_H
#define BASKETBALL_GAME_H

#include <stdbool.h>
#include "player.h"
#include "ball.h"
#include "renderer.h"

enum GameState {
    GAME_STATE_MENU,      // Main Menu (1-4 Player selection & target points)
    GAME_STATE_PLAYING,   // Active Turn-Based Match / Practice Mode
    GAME_STATE_GAME_OVER  // Victory Screen
};

struct Camera {
    float x;
    float y;
    float z;

    float yaw;
    float pitch;
    float distance;

    float moveSpeed;
    float mouseSensitivity;

    bool keys[256];
    bool specialKeys[256];

    bool mouseDragging;
    int lastMouseX;
    int lastMouseY;
};

void camera_init(struct Camera* cam);
void camera_update(struct Camera* cam, float deltaTime, float targetX, float targetZ);

struct Game {
    enum GameState state;
    bool isPracticeMode;
    int numPlayers;
    int currentPlayer;
    int winnerPlayer;
    int targetScore;

    struct Player players[4];
    struct Ball ball;
    struct Camera camera;

    int windowWidth;
    int windowHeight;

    // Shooting Parameters
    float shootingAngle;
    float shootingPower;

    // Turn & Banner State
    float scoreBannerTimer;
    char scoreBannerText[128];
    float turnEndTimer;
    bool shotFiredThisTurn;

    // Timing
    float globalTime;
    int lastFrameTimeMs;
};

struct Game* game_get_instance(void);

void game_init(struct Game* g, int width, int height);
void game_cleanup(struct Game* g);
void game_reshape(struct Game* g, int width, int height);
void game_update(struct Game* g);
void game_render(struct Game* g);

// Match & Turn Control
void game_start_new_match(struct Game* g, int playersCount);
void game_start_practice_mode(struct Game* g);
void game_pass_turn_to_next_player(struct Game* g);
void game_shoot_ball(struct Game* g);
void game_reset_ball_to_hands(struct Game* g);

// Input Handling
void game_handle_key_down(struct Game* g, unsigned char key, int x, int y);
void game_handle_key_up(struct Game* g, unsigned char key, int x, int y);
void game_handle_special_key_down(struct Game* g, int key, int x, int y);
void game_handle_special_key_up(struct Game* g, int key, int x, int y);
void game_handle_mouse_button(struct Game* g, int button, int state, int x, int y);
void game_handle_mouse_motion(struct Game* g, int x, int y);

#endif // BASKETBALL_GAME_H
