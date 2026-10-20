#include "game.h"
#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Camera Follow and Orbit Logic with E/Q Height & Zoom
void camera_init(struct Camera* cam) {
    if (!cam) return;
    cam->x = -3.0f;
    cam->y = 4.2f;
    cam->z = 8.5f;

    cam->yaw = -35.0f;
    cam->pitch = -10.0f;
    cam->distance = 9.0f;

    cam->moveSpeed = 15.0f;
    cam->mouseSensitivity = 0.18f;

    memset(cam->keys, 0, sizeof(cam->keys));
    memset(cam->specialKeys, 0, sizeof(cam->specialKeys));

    cam->mouseDragging = false;
    cam->lastMouseX = 0;
    cam->lastMouseY = 0;
}

void camera_update(struct Camera* cam, float deltaTime, float targetX, float targetZ) {
    if (!cam) return;
    float camSpeed = 8.0f * deltaTime;

    // Camera Zoom & Height with E / Q
    if (cam->keys['e'] || cam->keys['E']) {
        cam->distance -= camSpeed;
        cam->pitch += camSpeed * 2.5f;
    }
    if (cam->keys['q'] || cam->keys['Q']) {
        cam->distance += camSpeed;
        cam->pitch -= camSpeed * 2.5f;
    }

    if (cam->distance < 3.0f)  cam->distance = 3.0f;
    if (cam->distance > 26.0f) cam->distance = 26.0f;
    if (cam->pitch > -2.0f)    cam->pitch = -2.0f;
    if (cam->pitch < -70.0f)   cam->pitch = -70.0f;

    float yawRad = cam->yaw * (float)M_PI / 180.0f;
    float pitchRad = cam->pitch * (float)M_PI / 180.0f;

    float camOffsetX = -cam->distance * cosf(pitchRad) * cosf(yawRad);
    float camOffsetY = -cam->distance * sinf(pitchRad);
    float camOffsetZ = -cam->distance * cosf(pitchRad) * sinf(yawRad);

    cam->x = targetX + camOffsetX;
    cam->y = 1.8f + camOffsetY;
    cam->z = targetZ + camOffsetZ;

    if (cam->y < 0.6f) cam->y = 0.6f;
}

static struct Game g_gameInstance;

struct Game* game_get_instance(void) {
    return &g_gameInstance;
}

void game_init(struct Game* g, int width, int height) {
    if (!g) return;

    g->state = GAME_STATE_MENU;
    g->isPracticeMode = false;
    g->numPlayers = 2;
    g->currentPlayer = 0;
    g->winnerPlayer = 0;
    g->targetScore = 11;

    g->windowWidth = width;
    g->windowHeight = height;

    g->shootingAngle = 50.0f;
    g->shootingPower = 63.0f;

    g->scoreBannerTimer = 0.0f;
    g->scoreBannerText[0] = '\0';
    g->turnEndTimer = 0.0f;
    g->shotFiredThisTurn = false;

    g->globalTime = 0.0f;
    g->lastFrameTimeMs = 0;

    for (int i = 0; i < 4; ++i) {
        player_init(&g->players[i], i, 5.5f, 0.0f);
    }

    ball_init(&g->ball);
    camera_init(&g->camera);

    renderer_init();
    game_reset_ball_to_hands(g);

    printf("================================================================\n");
    printf("               3D BASKETBALL SHOOTING SIMULATOR                 \n");
    printf("================================================================\n");
    printf("Game Loop & Features:\n");
    printf("  - Game States: MENU -> PLAYING -> GAME_OVER\n");
    printf("  - [P] Practice Mode (Solo Free Play / Endless Score)\n");
    printf("  - [1-4] Local Turn-Based Competition (Red, Blue, Green, Yellow)\n");
    printf("  - Target Points Preset: 3, 5, 7, 11, 21 (Arrow Keys / T / +/-)\n");
    printf("  - Live Leaderboard, Turn Banners & Dynamic Jersey Colors\n");
    printf("----------------------------------------------------------------\n");
    printf("Menu Controls:\n");
    printf("  - Press [P] for Solo Practice Mode (Free Play)\n");
    printf("  - Press [UP/DOWN] or [+/-] or [T] to set Target Points (3, 5, 7, 11, 21)\n");
    printf("  - Press [1] for 1 Player Match\n");
    printf("  - Press [2] for 2 Players Match\n");
    printf("  - Press [3] for 3 Players Match\n");
    printf("  - Press [4] for 4 Players Match\n");
    printf("================================================================\n");
}

void game_cleanup(struct Game* g) {
    (void)g;
    renderer_cleanup();
}

void game_reshape(struct Game* g, int width, int height) {
    if (!g) return;
    if (height == 0) height = 1;
    g->windowWidth = width;
    g->windowHeight = height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)width / (double)height, 0.1, 500.0);
    glMatrixMode(GL_MODELVIEW);
}

void game_start_new_match(struct Game* g, int playersCount) {
    if (!g) return;
    g->isPracticeMode = false;

    if (playersCount < 1) playersCount = 1;
    if (playersCount > 4) playersCount = 4;
    g->numPlayers = playersCount;

    for (int i = 0; i < g->numPlayers; ++i) {
        player_set_score(&g->players[i], 0);
        player_reset(&g->players[i], 5.5f, 0.0f);
    }

    g->currentPlayer = 0;
    g->winnerPlayer = 0;
    g->state = GAME_STATE_PLAYING;
    g->shotFiredThisTurn = false;
    g->scoreBannerTimer = 0.0f;
    g->turnEndTimer = 0.0f;

    game_reset_ball_to_hands(g);

    printf("\n================================================================\n");
    printf(">>> NEW MATCH STARTED: %d PLAYERS | FIRST TO %d PTS WINS!\n", g->numPlayers, g->targetScore);
    printf("    Turn 1: %s\n", player_get_color(&g->players[g->currentPlayer])->name);
    printf("================================================================\n");
}

void game_start_practice_mode(struct Game* g) {
    if (!g) return;
    g->isPracticeMode = true;
    g->numPlayers = 1;
    g->currentPlayer = 0;
    g->winnerPlayer = 0;

    player_set_score(&g->players[0], 0);
    player_reset(&g->players[0], 5.5f, 0.0f);

    g->state = GAME_STATE_PLAYING;
    g->shotFiredThisTurn = false;
    g->scoreBannerTimer = 0.0f;
    g->turnEndTimer = 0.0f;

    game_reset_ball_to_hands(g);

    printf("\n================================================================\n");
    printf(">>> PRACTICE MODE STARTED: SOLO FREE PLAY / UNLIMITED SHOTS\n");
    printf("    Score counts endlessly! Press [M] or [ESC] to return to Menu.\n");
    printf("================================================================\n");
}

void game_pass_turn_to_next_player(struct Game* g) {
    if (!g || g->state != GAME_STATE_PLAYING) return;

    if (g->isPracticeMode) {
        g->shotFiredThisTurn = false;
        g->turnEndTimer = 0.0f;
        game_reset_ball_to_hands(g);
        return;
    }

    g->currentPlayer = (g->currentPlayer + 1) % g->numPlayers;
    g->shotFiredThisTurn = false;
    g->turnEndTimer = 0.0f;

    player_reset(&g->players[g->currentPlayer], 5.5f, 0.0f);
    game_reset_ball_to_hands(g);

    printf("\n>>> TURN PASSED -> It is now %s's Turn!\n", player_get_color(&g->players[g->currentPlayer])->name);
}

void game_reset_ball_to_hands(struct Game* g) {
    if (!g) return;
    struct Player* curP = &g->players[g->currentPlayer];
    ball_reset_to_hands(&g->ball, curP->x, curP->z, curP->rotationDeg);
}

void game_shoot_ball(struct Game* g) {
    if (!g || g->state != GAME_STATE_PLAYING || g->ball.state != BALL_IN_HAND) return;

    struct Player* curP = &g->players[g->currentPlayer];
    ball_shoot(&g->ball, curP->x, curP->z, g->shootingAngle, g->shootingPower, curP->rotationDeg);
    g->shotFiredThisTurn = true;
    g->turnEndTimer = 0.0f;

    printf("\n======================================================\n");
    printf(">>> [%s SHOT] Type: %s | Dist: %.2fm\n",
           player_get_color(curP)->name,
           g->ball.shotWas3Pointer ? "3-POINTER" : "2-POINTER",
           g->ball.shotDistance);
    printf("    Angle: %.1f deg | Power: %.1f %%\n", g->shootingAngle, g->shootingPower);
    printf("======================================================\n");
}

void game_update(struct Game* g) {
    if (!g) return;

    int currentTimeMs = glutGet(GLUT_ELAPSED_TIME);
    if (g->lastFrameTimeMs == 0) g->lastFrameTimeMs = currentTimeMs;
    float deltaTime = (float)(currentTimeMs - g->lastFrameTimeMs) / 1000.0f;
    g->lastFrameTimeMs = currentTimeMs;

    if (deltaTime > 0.1f) deltaTime = 0.1f;
    g->globalTime += deltaTime;

    if (g->state == GAME_STATE_PLAYING) {
        struct Player* curP = &g->players[g->currentPlayer];

        // 1. Move active player & update camera
        player_update_movement(curP, deltaTime, g->camera.keys);

        if (g->ball.state == BALL_IN_HAND) {
            game_reset_ball_to_hands(g);
        }

        camera_update(&g->camera, deltaTime, curP->x, curP->z);

        // 2. Update Ball physics & collision detection
        int scoredPoints = 0;
        ball_update_physics(&g->ball, deltaTime, &scoredPoints);

        if (scoredPoints > 0) {
            g->scoreBannerTimer = 3.5f;
            player_add_score(curP, scoredPoints);

            snprintf(g->scoreBannerText, sizeof(g->scoreBannerText), "%s: %s",
                     player_get_color(curP)->name,
                     scoredPoints == 3 ? "THREE POINTER! +3 PTS!" : "TWO POINTER! +2 PTS!");

            printf("\n******************************************************\n");
            printf("🎉 BASKET MADE BY %s! (+%d PTS)\n", player_get_color(curP)->name, scoredPoints);
            if (g->isPracticeMode) {
                printf("   Practice Score: %d PTS (Endless Free Play)\n", curP->score);
            } else {
                printf("   Current Score: %d / %d PTS\n", curP->score, g->targetScore);
            }
            printf("******************************************************\n");

            // Check Victory Condition (Ignored in Practice Mode)
            if (!g->isPracticeMode && curP->score >= g->targetScore) {
                g->winnerPlayer = g->currentPlayer;
                g->state = GAME_STATE_GAME_OVER;
                printf("\n🏆🏆🏆 VICTORY! %s HAS REACHED %d POINTS AND WINS THE GAME! 🏆🏆🏆\n",
                       player_get_color(curP)->name, g->targetScore);
            }
        }

        // 3. Score banner timer
        if (g->scoreBannerTimer > 0.0f) {
            g->scoreBannerTimer -= deltaTime;
            if (g->scoreBannerTimer < 0.0f) g->scoreBannerTimer = 0.0f;
        }

        // 4. Auto turn advance after ball comes to rest
        if (ball_is_at_rest_on_floor(&g->ball)) {
            g->turnEndTimer += deltaTime;
            if (g->turnEndTimer >= (g->isPracticeMode ? 1.2f : 2.0f)) {
                game_pass_turn_to_next_player(g);
            }
        }
    }
}

void game_render(struct Game* g) {
    if (!g) return;

    glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    struct Player* curP = &g->players[g->currentPlayer];
    gluLookAt(
        g->camera.x, g->camera.y, g->camera.z,
        curP->x, 1.6f, curP->z,
        0.0f, 1.0f, 0.0f
    );

    renderer_setup_lighting();

    // 1. Arena Geometry
    renderer_draw_court();
    renderer_draw_bleachers();
    renderer_draw_audience(g->camera.x, g->camera.z, g->globalTime);

    // 2. Basketball Hoops
    renderer_draw_hoop(14.0f, -1.0f);
    renderer_draw_hoop(-14.0f, 1.0f);

    // 3. Active 3D Player Model
    renderer_draw_player(curP, g->ball.state == BALL_IN_HAND);

    // 4. Airborne Ball, Trail & Shadow
    renderer_draw_ball_trail_and_shadow(&g->ball);

    // 5. 2D Orthographic HUD / Menu Overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, g->windowWidth, 0, g->windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    if (g->state == GAME_STATE_MENU) {
        renderer_render_main_menu(g->windowWidth, g->windowHeight, g->targetScore);
    } else if (g->state == GAME_STATE_GAME_OVER) {
        renderer_render_game_over_screen(g->windowWidth, g->windowHeight, &g->players[g->winnerPlayer], g->targetScore);
    } else {
        renderer_render_hud(
            g->windowWidth, g->windowHeight,
            g->players, g->numPlayers, g->currentPlayer,
            g->targetScore, g->shootingAngle, g->shootingPower,
            g->scoreBannerTimer, g->scoreBannerText,
            curP->x, curP->z,
            g->isPracticeMode
        );
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glutSwapBuffers();
}

static const int PRESET_POINTS[] = { 3, 5, 7, 11, 21 };
static const int NUM_PRESETS = 5;

static int getNextPresetPoints(int current) {
    for (int i = 0; i < NUM_PRESETS; ++i) {
        if (PRESET_POINTS[i] > current) {
            return PRESET_POINTS[i];
        }
    }
    return PRESET_POINTS[0]; // Wrap around to 3
}

static int getPrevPresetPoints(int current) {
    for (int i = NUM_PRESETS - 1; i >= 0; --i) {
        if (PRESET_POINTS[i] < current) {
            return PRESET_POINTS[i];
        }
    }
    return PRESET_POINTS[NUM_PRESETS - 1]; // Wrap around to 21
}

void game_handle_key_down(struct Game* g, unsigned char key, int x, int y) {
    (void)x; (void)y;
    if (!g) return;

    if (key == 27) { // ESC key
        if (g->state == GAME_STATE_PLAYING) {
            g->state = GAME_STATE_MENU;
        } else {
            glutLeaveMainLoop();
        }
        glutPostRedisplay();
        return;
    }

    // MENU STATE SELECTION & TARGET SCORE ADJUSTMENT (3, 5, 7, 11, 21 ONLY)
    if (g->state == GAME_STATE_MENU) {
        if (key == 'p' || key == 'P') {
            game_start_practice_mode(g);
        } else if (key >= '1' && key <= '4') {
            int selected = key - '0';
            game_start_new_match(g, selected);
        } else if (key == 't' || key == 'T' || key == '+' || key == '=') {
            g->targetScore = getNextPresetPoints(g->targetScore);
        } else if (key == '-' || key == '_') {
            g->targetScore = getPrevPresetPoints(g->targetScore);
        }
        glutPostRedisplay();
        return;
    }

    // GAME OVER STATE REPLAY
    if (g->state == GAME_STATE_GAME_OVER) {
        if (key == 'r' || key == 'R' || key == ' ' || key == 'm' || key == 'M') {
            g->state = GAME_STATE_MENU;
        }
        glutPostRedisplay();
        return;
    }

    // PLAYING STATE CONTROLS
    if (g->state == GAME_STATE_PLAYING) {
        if (key == 'm' || key == 'M') {
            g->state = GAME_STATE_MENU;
            glutPostRedisplay();
            return;
        }
        if (key == ' ') {
            game_shoot_ball(g);
        } else if (key == 'r' || key == 'R' || key == 'n' || key == 'N') {
            game_pass_turn_to_next_player(g);
        }

        g->camera.keys[key] = true;
    }

    glutPostRedisplay();
}

void game_handle_key_up(struct Game* g, unsigned char key, int x, int y) {
    (void)x; (void)y;
    if (!g) return;
    g->camera.keys[key] = false;
}

void game_handle_special_key_down(struct Game* g, int key, int x, int y) {
    (void)x; (void)y;
    if (!g) return;

    if (g->state == GAME_STATE_MENU) {
        if (key == GLUT_KEY_UP || key == GLUT_KEY_RIGHT) {
            g->targetScore = getNextPresetPoints(g->targetScore);
        } else if (key == GLUT_KEY_DOWN || key == GLUT_KEY_LEFT) {
            g->targetScore = getPrevPresetPoints(g->targetScore);
        }
        glutPostRedisplay();
        return;
    }

    if (g->state != GAME_STATE_PLAYING) return;

    const float angleStep = 1.0f;
    const float powerStep = 2.0f;

    switch (key) {
        case GLUT_KEY_UP:
            g->shootingAngle += angleStep;
            if (g->shootingAngle > 85.0f) g->shootingAngle = 85.0f;
            break;

        case GLUT_KEY_DOWN:
            g->shootingAngle -= angleStep;
            if (g->shootingAngle < 15.0f) g->shootingAngle = 15.0f;
            break;

        case GLUT_KEY_RIGHT:
            g->shootingPower += powerStep;
            if (g->shootingPower > 100.0f) g->shootingPower = 100.0f;
            break;

        case GLUT_KEY_LEFT:
            g->shootingPower -= powerStep;
            if (g->shootingPower < 10.0f) g->shootingPower = 10.0f;
            break;

        default:
            if (key >= 0 && key < 256) g->camera.specialKeys[key] = true;
            break;
    }

    glutPostRedisplay();
}

void game_handle_special_key_up(struct Game* g, int key, int x, int y) {
    (void)x; (void)y;
    if (!g) return;
    if (key >= 0 && key < 256) g->camera.specialKeys[key] = false;
}

void game_handle_mouse_button(struct Game* g, int button, int state, int x, int y) {
    if (!g) return;
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            g->camera.mouseDragging = true;
            g->camera.lastMouseX = x;
            g->camera.lastMouseY = y;
        } else if (state == GLUT_UP) {
            g->camera.mouseDragging = false;
        }
    }
}

void game_handle_mouse_motion(struct Game* g, int x, int y) {
    if (!g) return;
    if (g->camera.mouseDragging) {
        int deltaX = x - g->camera.lastMouseX;
        int deltaY = y - g->camera.lastMouseY;

        g->camera.yaw   += (float)deltaX * g->camera.mouseSensitivity;
        g->camera.pitch += (float)deltaY * g->camera.mouseSensitivity;

        if (g->camera.pitch > -2.0f)  g->camera.pitch = -2.0f;
        if (g->camera.pitch < -60.0f) g->camera.pitch = -60.0f;

        g->camera.lastMouseX = x;
        g->camera.lastMouseY = y;

        glutPostRedisplay();
    }
}
