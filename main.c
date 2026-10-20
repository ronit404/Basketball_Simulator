/**
 * =============================================================================
 * Project: 3D Basketball Shooting Simulator (Pure C Edition)
 * Course: Computer Graphics (OpenGL / FreeGLUT)
 * File: main.c
 *
 * Description:
 *   Pure C translation of the 3D Basketball Shooting Simulator.
 *   - Only contains main() function, window initialization, and core GLUT callbacks.
 *   - Delegates all state, physics, rendering, and gameplay logic to pure C modules:
 *     * game.h / game.c         : State machine, turns, tournament logic & loop
 *     * ball.h / ball.c         : Projectile physics & collision detection
 *     * player.h / player.c     : Kinematics, movement, jersey colors & score
 *     * renderer.h / renderer.c : 3D arena geometry, lighting & 2D HUD overlays
 * =============================================================================
 */

#include <GL/freeglut.h>
#include "game.h"

// GLUT Callback Wrappers delegating to Game Instance
static void displayCallback(void) {
    game_render(game_get_instance());
}

static void reshapeCallback(int width, int height) {
    game_reshape(game_get_instance(), width, height);
}

static void timerCallback(int val) {
    (void)val;
    game_update(game_get_instance());
    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0);
}

static void keyDownCallback(unsigned char key, int x, int y) {
    game_handle_key_down(game_get_instance(), key, x, y);
}

static void keyUpCallback(unsigned char key, int x, int y) {
    game_handle_key_up(game_get_instance(), key, x, y);
}

static void specialKeyDownCallback(int key, int x, int y) {
    game_handle_special_key_down(game_get_instance(), key, x, y);
}

static void specialKeyUpCallback(int key, int x, int y) {
    game_handle_special_key_up(game_get_instance(), key, x, y);
}

static void mouseButtonCallback(int button, int state, int x, int y) {
    game_handle_mouse_button(game_get_instance(), button, state, x, y);
}

static void mouseMotionCallback(int x, int y) {
    game_handle_mouse_motion(game_get_instance(), x, y);
}

static void closeCallback(void) {
    game_cleanup(game_get_instance());
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("3D Basketball Shooting Simulator");

    // Core OpenGL Feature Configuration
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);

    // Initialize Game Engine & Assets
    game_init(game_get_instance(), 1280, 720);

    // Register Callbacks
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutTimerFunc(16, timerCallback, 0);

    glutKeyboardFunc(keyDownCallback);
    glutKeyboardUpFunc(keyUpCallback);
    glutSpecialFunc(specialKeyDownCallback);
    glutSpecialUpFunc(specialKeyUpCallback);

    glutMouseFunc(mouseButtonCallback);
    glutMotionFunc(mouseMotionCallback);

    glutCloseFunc(closeCallback);

    glutMainLoop();
    return 0;
}
