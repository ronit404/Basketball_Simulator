#ifndef BASKETBALL_RENDERER_H
#define BASKETBALL_RENDERER_H

#include <GL/freeglut.h>
#include <stdbool.h>
#include "player.h"
#include "ball.h"

struct MaterialProperties {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat shininess;
};

// Audience Member Structure
struct AudienceMember {
    float x, y, z;
    float scale;
    int textureIdx;
    float cheeringFreq;
    float cheeringPhase;
};

#define MAX_AUDIENCE_MEMBERS 600

void renderer_init(void);
void renderer_cleanup(void);

// Lighting and Materials
void renderer_setup_lighting(void);
void renderer_apply_material(const struct MaterialProperties* mat);

// 3D Scene Geometry
void renderer_draw_court(void);
void renderer_draw_hoop(float baselineX, float facingDirection);
void renderer_draw_basketball(float radius);
void renderer_draw_player(const struct Player* player, bool ballInHands);
void renderer_draw_bleachers(void);
void renderer_draw_audience(float camX, float camZ, float globalTime);
void renderer_draw_ball_trail_and_shadow(const struct Ball* ball);

// 2D Orthographic HUD & Menu Rendering
void renderer_render_bitmap_text(float x, float y, void* font, const char* text);
void renderer_render_centered_text(float centerX, float y, void* font, const char* text);
float renderer_get_text_width(void* font, const char* text);

void renderer_render_main_menu(int windowWidth, int windowHeight, int targetScore);
void renderer_render_game_over_screen(int windowWidth, int windowHeight, const struct Player* winner, int targetScore);
void renderer_render_hud(
    int windowWidth, int windowHeight,
    const struct Player* players, int numPlayers, int currentPlayerIdx,
    int targetScore, float shootingAngle, float shootingPower,
    float scoreBannerTimer, const char* scoreBannerText,
    float playerX, float playerZ,
    bool isPracticeMode
);

// Geometric Primitives
void renderer_draw_solid_box(float widthX, float heightY, float depthZ);
void renderer_draw_cylinder_x(float radiusBase, float radiusTop, float length, int slices);
void renderer_draw_cylinder_y(float radiusBase, float radiusTop, float height, int slices);
void renderer_draw_court_line(float x1, float z1, float x2, float z2, float y);
void renderer_draw_court_circle(float cx, float cz, float radius, int segments, float y);
void renderer_draw_court_arc(float cx, float cz, float radius, float startAngle, float endAngle, int segments, float y);

#endif // BASKETBALL_RENDERER_H
