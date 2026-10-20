#include "renderer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Static Globals
static GLUquadricObj* g_quadric = NULL;
static GLuint g_crowdTextureIds[6] = { 0 };
static struct AudienceMember g_audienceList[MAX_AUDIENCE_MEMBERS];
static int g_audienceCount = 0;
static bool g_isInitialized = false;

// Predefined Materials
static const struct MaterialProperties MAT_HARDWOOD_MAPLE = {
    { 0.32f, 0.22f, 0.12f, 1.0f },
    { 0.88f, 0.65f, 0.38f, 1.0f }, // Warm golden maple wood tone
    { 0.25f, 0.25f, 0.25f, 1.0f },
    22.0f
};

static const struct MaterialProperties MAT_PAINTED_KEY = {
    { 0.08f, 0.14f, 0.26f, 1.0f },
    { 0.15f, 0.32f, 0.62f, 1.0f }, // Classic NBA stadium navy paint
    { 0.20f, 0.20f, 0.25f, 1.0f },
    16.0f
};

static const struct MaterialProperties MAT_COURT_APRON = {
    { 0.05f, 0.07f, 0.12f, 1.0f },
    { 0.10f, 0.14f, 0.22f, 1.0f }, // Deep arena border
    { 0.12f, 0.12f, 0.12f, 1.0f },
    10.0f
};

static const struct MaterialProperties MAT_ORANGE_RIM = {
    { 0.30f, 0.08f, 0.02f, 1.0f },
    { 0.95f, 0.32f, 0.05f, 1.0f },
    { 0.40f, 0.40f, 0.40f, 1.0f },
    32.0f
};

static const struct MaterialProperties MAT_STEEL_POLE = {
    { 0.12f, 0.12f, 0.14f, 1.0f },
    { 0.25f, 0.25f, 0.28f, 1.0f },
    { 0.60f, 0.60f, 0.65f, 1.0f },
    64.0f
};

static const struct MaterialProperties MAT_GLASS_BACKBOARD = {
    { 0.18f, 0.22f, 0.25f, 1.0f },
    { 0.82f, 0.88f, 0.94f, 0.75f },
    { 0.90f, 0.90f, 0.95f, 1.0f },
    96.0f
};

static const struct MaterialProperties MAT_BASKETBALL_RUBBER = {
    { 0.32f, 0.12f, 0.04f, 1.0f },
    { 0.90f, 0.42f, 0.12f, 1.0f },
    { 0.25f, 0.20f, 0.15f, 1.0f },
    24.0f
};

static const struct MaterialProperties MAT_PLAYER_SKIN = {
    { 0.25f, 0.18f, 0.14f, 1.0f },
    { 0.82f, 0.60f, 0.48f, 1.0f },
    { 0.10f, 0.10f, 0.10f, 1.0f },
    10.0f
};

static const struct MaterialProperties MAT_BLEACHERS = {
    { 0.10f, 0.12f, 0.16f, 1.0f },
    { 0.18f, 0.22f, 0.28f, 1.0f },
    { 0.12f, 0.12f, 0.15f, 1.0f },
    14.0f
};

static void generate_crowd_textures(void) {
    const int TEX_SIZE = 64;
    glGenTextures(6, g_crowdTextureIds);

    struct JerseyStyle {
        unsigned char r, g, b;
    };
    struct JerseyStyle styles[6] = {
        { 225, 45, 45 },   // Red
        { 40, 110, 230 },  // Blue
        { 240, 200, 30 },  // Yellow
        { 35, 190, 80 },   // Green
        { 160, 50, 210 },  // Purple
        { 235, 235, 240 }  // White
    };

    unsigned char texData[64 * 64 * 4];

    for (int t = 0; t < 6; ++t) {
        for (int y = 0; y < TEX_SIZE; ++y) {
            for (int x = 0; x < TEX_SIZE; ++x) {
                int idx = (y * TEX_SIZE + x) * 4;
                float nx = (float)x / (float)(TEX_SIZE - 1);
                float ny = (float)y / (float)(TEX_SIZE - 1);

                texData[idx + 0] = 0;
                texData[idx + 1] = 0;
                texData[idx + 2] = 0;
                texData[idx + 3] = 0;

                // Head Region (ny: 0.70 to 0.95)
                float dxHead = nx - 0.5f;
                float dyHead = ny - 0.82f;
                if (dxHead * dxHead + dyHead * dyHead * 1.5f < 0.022f) {
                    texData[idx + 0] = 220;
                    texData[idx + 1] = 170;
                    texData[idx + 2] = 140;
                    texData[idx + 3] = 255;
                }

                // Torso Region (ny: 0.15 to 0.70)
                if (ny >= 0.15f && ny < 0.70f && nx >= 0.22f && nx <= 0.78f) {
                    texData[idx + 0] = styles[t].r;
                    texData[idx + 1] = styles[t].g;
                    texData[idx + 2] = styles[t].b;
                    texData[idx + 3] = 255;
                }
            }
        }

        glBindTexture(GL_TEXTURE_2D, g_crowdTextureIds[t]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    }
}

static void init_audience(void) {
    g_audienceCount = 0;

    int numTiers = 7;
    float tierDepth  = 1.4f;
    float tierHeight = 0.65f;
    float courtHalfW = (COURT_WIDTH_BOUND / 2.0f) + 3.2f;
    float courtHalfL = (COURT_LENGTH_BOUND / 2.0f) + 3.2f;

    // North & South Audience
    for (int t = 0; t < numTiers && g_audienceCount < MAX_AUDIENCE_MEMBERS - 2; ++t) {
        float y = (float)t * tierHeight + tierHeight + 0.1f;
        float zOffset = (float)t * tierDepth + (tierDepth / 2.0f);

        for (float x = -courtHalfL + 1.0f; x <= courtHalfL - 1.0f && g_audienceCount < MAX_AUDIENCE_MEMBERS - 2; x += 1.35f) {
            // North
            struct AudienceMember* an = &g_audienceList[g_audienceCount++];
            an->x = x + (((rand() % 100) - 50) / 300.0f);
            an->y = y;
            an->z = courtHalfW + zOffset;
            an->scale = 1.15f + ((rand() % 20) / 100.0f);
            an->textureIdx = rand() % 6;
            an->cheeringFreq = 1.5f + ((rand() % 100) / 50.0f);
            an->cheeringPhase = ((rand() % 100) / 100.0f) * 6.28f;

            // South
            struct AudienceMember* as = &g_audienceList[g_audienceCount++];
            as->x = x + (((rand() % 100) - 50) / 300.0f);
            as->y = y;
            as->z = -(courtHalfW + zOffset);
            as->scale = 1.15f + ((rand() % 20) / 100.0f);
            as->textureIdx = rand() % 6;
            as->cheeringFreq = 1.5f + ((rand() % 100) / 50.0f);
            as->cheeringPhase = ((rand() % 100) / 100.0f) * 6.28f;
        }
    }

    // East & West Audience
    for (int t = 0; t < numTiers && g_audienceCount < MAX_AUDIENCE_MEMBERS - 2; ++t) {
        float y = (float)t * tierHeight + tierHeight + 0.1f;
        float xOffset = (float)t * tierDepth + (tierDepth / 2.0f);

        for (float z = -courtHalfW + 1.0f; z <= courtHalfW - 1.0f && g_audienceCount < MAX_AUDIENCE_MEMBERS - 2; z += 1.35f) {
            // East
            struct AudienceMember* ae = &g_audienceList[g_audienceCount++];
            ae->x = courtHalfL + xOffset;
            ae->y = y;
            ae->z = z + (((rand() % 100) - 50) / 300.0f);
            ae->scale = 1.15f + ((rand() % 20) / 100.0f);
            ae->textureIdx = rand() % 6;
            ae->cheeringFreq = 1.5f + ((rand() % 100) / 50.0f);
            ae->cheeringPhase = ((rand() % 100) / 100.0f) * 6.28f;

            // West
            struct AudienceMember* aw = &g_audienceList[g_audienceCount++];
            aw->x = -(courtHalfL + xOffset);
            aw->y = y;
            aw->z = z + (((rand() % 100) - 50) / 300.0f);
            aw->scale = 1.15f + ((rand() % 20) / 100.0f);
            aw->textureIdx = rand() % 6;
            aw->cheeringFreq = 1.5f + ((rand() % 100) / 50.0f);
            aw->cheeringPhase = ((rand() % 100) / 100.0f) * 6.28f;
        }
    }
}

void renderer_init(void) {
    if (g_isInitialized) return;

    g_quadric = gluNewQuadric();
    gluQuadricNormals(g_quadric, GLU_SMOOTH);

    generate_crowd_textures();
    init_audience();

    g_isInitialized = true;
}

void renderer_cleanup(void) {
    if (g_quadric) {
        gluDeleteQuadric(g_quadric);
        g_quadric = NULL;
    }
    glDeleteTextures(6, g_crowdTextureIds);
    g_audienceCount = 0;
    g_isInitialized = false;
}

void renderer_setup_lighting(void) {
    GLfloat light0_pos[]      = { 0.0f, 18.0f, 0.0f, 1.0f };
    GLfloat light0_ambient[]  = { 0.24f, 0.24f, 0.26f, 1.0f };
    GLfloat light0_diffuse[]  = { 0.95f, 0.93f, 0.90f, 1.0f };
    GLfloat light0_specular[] = { 0.85f, 0.85f, 0.85f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    GLfloat light1_pos[]      = { 12.0f, 14.0f, 0.0f, 1.0f };
    GLfloat light1_ambient[]  = { 0.05f, 0.05f, 0.05f, 1.0f };
    GLfloat light1_diffuse[]  = { 0.45f, 0.42f, 0.38f, 1.0f };
    GLfloat light1_specular[] = { 0.35f, 0.35f, 0.35f, 1.0f };

    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
}

void renderer_apply_material(const struct MaterialProperties* mat) {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat->ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat->diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat->specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->shininess);
}

void renderer_draw_solid_box(float widthX, float heightY, float depthZ) {
    float hx = widthX / 2.0f;
    float hy = heightY / 2.0f;
    float hz = depthZ / 2.0f;

    glBegin(GL_QUADS);
    // Front (+X)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(hx, -hy, -hz);
    glVertex3f(hx,  hy, -hz);
    glVertex3f(hx,  hy,  hz);
    glVertex3f(hx, -hy,  hz);

    // Back (-X)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-hx, -hy,  hz);
    glVertex3f(-hx,  hy,  hz);
    glVertex3f(-hx,  hy, -hz);
    glVertex3f(-hx, -hy, -hz);

    // Top (+Y)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-hx, hy, -hz);
    glVertex3f(-hx, hy,  hz);
    glVertex3f( hx, hy,  hz);
    glVertex3f( hx, hy, -hz);

    // Bottom (-Y)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-hx, -hy, -hz);
    glVertex3f( hx, -hy, -hz);
    glVertex3f( hx, -hy,  hz);
    glVertex3f(-hx, -hy,  hz);

    // Right (+Z)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-hx, -hy, hz);
    glVertex3f( hx, -hy, hz);
    glVertex3f( hx,  hy, hz);
    glVertex3f(-hx,  hy, hz);

    // Left (-Z)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-hx, -hy, -hz);
    glVertex3f(-hx,  hy, -hz);
    glVertex3f( hx,  hy, -hz);
    glVertex3f( hx, -hy, -hz);
    glEnd();
}

void renderer_draw_cylinder_y(float radiusBase, float radiusTop, float height, int slices) {
    if (!g_quadric) g_quadric = gluNewQuadric();
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(g_quadric, radiusBase, radiusTop, height, slices, 1);
    glPopMatrix();
}

void renderer_draw_cylinder_x(float radiusBase, float radiusTop, float length, int slices) {
    if (!g_quadric) g_quadric = gluNewQuadric();
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    gluCylinder(g_quadric, radiusBase, radiusTop, length, slices, 1);
    glPopMatrix();
}

void renderer_draw_court_line(float x1, float z1, float x2, float z2, float y) {
    glBegin(GL_LINES);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(x1, y, z1);
    glVertex3f(x2, y, z2);
    glEnd();
}

void renderer_draw_court_circle(float cx, float cz, float radius, int segments, float y) {
    glBegin(GL_LINE_LOOP);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < segments; ++i) {
        float angle = (float)i * 2.0f * (float)M_PI / (float)segments;
        glVertex3f(cx + cosf(angle) * radius, y, cz + sinf(angle) * radius);
    }
    glEnd();
}

void renderer_draw_court_arc(float cx, float cz, float radius, float startAngle, float endAngle, int segments, float y) {
    glBegin(GL_LINE_STRIP);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        float angle = startAngle + t * (endAngle - startAngle);
        glVertex3f(cx + cosf(angle) * radius, y, cz + sinf(angle) * radius);
    }
    glEnd();
}

void renderer_draw_court(void) {
    float halfL = COURT_LENGTH_BOUND / 2.0f; // 14.0m
    float halfW = COURT_WIDTH_BOUND / 2.0f;  // 7.5m
    float apron = 3.0f;

    // 1. Dark Perimeter Apron
    renderer_apply_material(&MAT_COURT_APRON);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-halfL - apron, 0.0f, -halfW - apron);
    glVertex3f( halfL + apron, 0.0f, -halfW - apron);
    glVertex3f( halfL + apron, 0.0f,  halfW + apron);
    glVertex3f(-halfL - apron, 0.0f,  halfW + apron);
    glEnd();

    // 2. Main Hardwood Maple Playing Court
    renderer_apply_material(&MAT_HARDWOOD_MAPLE);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-halfL, 0.001f, -halfW);
    glVertex3f( halfL, 0.001f, -halfW);
    glVertex3f( halfL, 0.001f,  halfW);
    glVertex3f(-halfL, 0.001f,  halfW);
    glEnd();

    // Subtle longitudinal floorboard seams
    glDisable(GL_LIGHTING);
    glColor4f(0.72f, 0.50f, 0.28f, 0.40f);
    glLineWidth(1.0f);
    for (float z = -halfW + 0.8f; z < halfW; z += 0.8f) {
        glBegin(GL_LINES);
        glVertex3f(-halfL, 0.002f, z);
        glVertex3f( halfL, 0.002f, z);
        glEnd();
    }
    glEnable(GL_LIGHTING);

    // 3. Painted Free-Throw Lanes / Keys
    float keyLength = 5.8f;
    float keyHalfWidth = 2.45f;

    renderer_apply_material(&MAT_PAINTED_KEY);
    int sides[2] = { -1, 1 };
    for (int s = 0; s < 2; ++s) {
        int side = sides[s];
        float baselineX = (float)side * halfL;
        float ftLineX   = baselineX - (float)side * keyLength;
        float minX = baselineX < ftLineX ? baselineX : ftLineX;
        float maxX = baselineX > ftLineX ? baselineX : ftLineX;

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(minX, 0.003f, -keyHalfWidth);
        glVertex3f(maxX, 0.003f, -keyHalfWidth);
        glVertex3f(maxX, 0.003f,  keyHalfWidth);
        glVertex3f(minX, 0.003f,  keyHalfWidth);
        glEnd();
    }

    // 4. Painted Center Jump Circle Fill
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.003f, 0.0f);
    for (int i = 0; i <= 36; ++i) {
        float angle = (float)i * 2.0f * (float)M_PI / 36.0f;
        glVertex3f(cosf(angle) * 1.8f, 0.003f, sinf(angle) * 1.8f);
    }
    glEnd();

    // 5. Official FIBA Court Lines & Markings
    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Outer Boundary Lines
    glBegin(GL_LINE_LOOP);
    glVertex3f(-halfL, 0.005f, -halfW);
    glVertex3f( halfL, 0.005f, -halfW);
    glVertex3f( halfL, 0.005f,  halfW);
    glVertex3f(-halfL, 0.005f,  halfW);
    glEnd();

    // Half-Court Division Line & Center Circles
    renderer_draw_court_line(0.0f, -halfW, 0.0f, halfW, 0.005f);
    renderer_draw_court_circle(0.0f, 0.0f, 1.8f, 48, 0.005f);
    renderer_draw_court_circle(0.0f, 0.0f, 0.6f, 32, 0.005f);

    for (int s = 0; s < 2; ++s) {
        int side = sides[s];
        float baselineX = (float)side * halfL;
        float dir = (float)(-side);
        float freeThrowLineX = baselineX + dir * keyLength;

        renderer_draw_court_line(baselineX, -keyHalfWidth, freeThrowLineX, -keyHalfWidth, 0.005f);
        renderer_draw_court_line(baselineX,  keyHalfWidth, freeThrowLineX,  keyHalfWidth, 0.005f);
        renderer_draw_court_line(freeThrowLineX, -keyHalfWidth, freeThrowLineX, keyHalfWidth, 0.005f);

        float arcStartAngle = (side == -1) ? -(float)M_PI / 2.0f : (float)M_PI / 2.0f;
        float arcEndAngle   = (side == -1) ?  (float)M_PI / 2.0f : 3.0f * (float)M_PI / 2.0f;
        renderer_draw_court_arc(freeThrowLineX, 0.0f, 1.8f, arcStartAngle, arcEndAngle, 32, 0.005f);

        for (int d = 0; d < 8; ++d) {
            float t0 = (float)d / 8.0f;
            float t1 = (float)(d + 0.5f) / 8.0f;
            float a0 = (side == -1) ? (arcEndAngle + t0 * (float)M_PI) : (arcStartAngle + t0 * (float)M_PI);
            float a1 = (side == -1) ? (arcEndAngle + t1 * (float)M_PI) : (arcStartAngle + t1 * (float)M_PI);
            renderer_draw_court_arc(freeThrowLineX, 0.0f, 1.8f, a0, a1, 4, 0.005f);
        }

        float hashPositions[] = { 1.75f, 2.60f, 3.45f, 4.30f };
        for (int h = 0; h < 4; ++h) {
            float hx = baselineX + dir * hashPositions[h];
            renderer_draw_court_line(hx, -keyHalfWidth, hx, -keyHalfWidth - 0.20f, 0.005f);
            renderer_draw_court_line(hx,  keyHalfWidth, hx,  keyHalfWidth + 0.20f, 0.005f);
        }

        float basketX = baselineX + dir * 1.575f;
        float threePointRadius = THREE_POINT_RADIUS;
        float cornerDistanceZ  = CORNER_3PT_DISTANCE;
        float arcDepth = sqrtf(threePointRadius * threePointRadius - cornerDistanceZ * cornerDistanceZ);
        float straightLengthX = (baselineX + dir * 1.575f + dir * arcDepth);

        renderer_draw_court_line(baselineX, -cornerDistanceZ, straightLengthX, -cornerDistanceZ, 0.005f);
        renderer_draw_court_line(baselineX,  cornerDistanceZ, straightLengthX,  cornerDistanceZ, 0.005f);

        float angleExtent = asinf(cornerDistanceZ / threePointRadius);
        if (side == -1) {
            renderer_draw_court_arc(basketX, 0.0f, threePointRadius, -angleExtent, angleExtent, 36, 0.005f);
        } else {
            renderer_draw_court_arc(basketX, 0.0f, threePointRadius, (float)M_PI - angleExtent, (float)M_PI + angleExtent, 36, 0.005f);
        }

        if (side == -1) {
            renderer_draw_court_arc(basketX, 0.0f, 1.25f, -(float)M_PI / 2.0f, (float)M_PI / 2.0f, 24, 0.005f);
        } else {
            renderer_draw_court_arc(basketX, 0.0f, 1.25f, (float)M_PI / 2.0f, 3.0f * (float)M_PI / 2.0f, 24, 0.005f);
        }
    }

    glEnable(GL_LIGHTING);
}

void renderer_draw_hoop(float baselineX, float facingDirection) {
    float poleX      = baselineX - facingDirection * 1.0f;
    float backboardX = baselineX + facingDirection * 1.20f;
    float rimCenterX = backboardX + facingDirection * 0.38f;

    glPushMatrix();

    // 1. Post & Padding
    glPushMatrix();
    glTranslatef(poleX, 0.0f, 0.0f);
    
    renderer_apply_material(&MAT_STEEL_POLE);
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, 0.0f);
    renderer_draw_solid_box(1.4f, 0.8f, 1.2f);
    glPopMatrix();

    renderer_apply_material(&MAT_COURT_APRON);
    glPushMatrix();
    glTranslatef(facingDirection * 0.4f, 0.6f, 0.0f);
    renderer_draw_solid_box(0.6f, 1.2f, 1.1f);
    glPopMatrix();

    renderer_apply_material(&MAT_STEEL_POLE);
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.0f);
    renderer_draw_cylinder_y(0.12f, 0.12f, 3.2f, 24);
    glPopMatrix();

    glPopMatrix();

    // 2. Extension Arm
    glPushMatrix();
    glTranslatef(poleX, 3.9f, 0.0f);
    float armDist = fabsf(backboardX - poleX);
    float armAngle = (facingDirection < 0.0f) ? 192.0f : -12.0f;
    glRotatef(armAngle, 0.0f, 0.0f, 1.0f);
    renderer_apply_material(&MAT_STEEL_POLE);
    renderer_draw_cylinder_x(0.08f, 0.08f, armDist * 1.05f, 16);
    glPopMatrix();

    // 3. Backboard & Target Square
    glPushMatrix();
    glTranslatef(backboardX, RIM_HEIGHT + 0.25f, 0.0f);

    renderer_apply_material(&MAT_GLASS_BACKBOARD);
    renderer_draw_solid_box(BACKBOARD_THICKNESS, BACKBOARD_HEIGHT, BACKBOARD_WIDTH);

    glDisable(GL_LIGHTING);
    glLineWidth(3.0f);
    glColor3f(1.0f, 0.15f, 0.15f);

    float targetX = (facingDirection < 0.0f) ? -0.026f : 0.026f;
    glBegin(GL_LINE_LOOP);
    glVertex3f(targetX, -0.25f, -0.295f);
    glVertex3f(targetX, -0.25f,  0.295f);
    glVertex3f(targetX,  0.20f,  0.295f);
    glVertex3f(targetX,  0.20f, -0.295f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(targetX, -0.52f, -0.89f);
    glVertex3f(targetX, -0.52f,  0.89f);
    glVertex3f(targetX,  0.52f,  0.89f);
    glVertex3f(targetX,  0.52f, -0.89f);
    glEnd();

    glEnable(GL_LIGHTING);
    glPopMatrix();

    // 4. Rim Torus
    glPushMatrix();
    glTranslatef(rimCenterX, RIM_HEIGHT, 0.0f);

    renderer_apply_material(&MAT_ORANGE_RIM);
    glPushMatrix();
    glTranslatef(-facingDirection * 0.10f, -0.02f, 0.0f);
    renderer_draw_solid_box(0.20f, 0.06f, 0.08f);
    glPopMatrix();

    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidTorus(RIM_TUBE_RADIUS, RIM_RADIUS, 16, 32);
    glPopMatrix();

    // 5. White Nylon Net
    glDisable(GL_LIGHTING);
    glColor3f(0.95f, 0.95f, 0.95f);
    glLineWidth(1.5f);

    int netRings = 5;
    int netStrands = 12;
    float netDepth = 0.40f;

    for (int r = 1; r <= netRings; ++r) {
        float frac = (float)r / (float)netRings;
        float y = -frac * netDepth;
        float curRadius = RIM_RADIUS * (1.0f - frac * 0.35f);

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < netStrands; ++i) {
            float angle = (float)i * 2.0f * (float)M_PI / (float)netStrands;
            glVertex3f(cosf(angle) * curRadius, y, sinf(angle) * curRadius);
        }
        glEnd();
    }

    for (int i = 0; i < netStrands; ++i) {
        float angle0 = (float)i * 2.0f * (float)M_PI / (float)netStrands;
        float angle1 = (float)(i + 1) * 2.0f * (float)M_PI / (float)netStrands;

        glBegin(GL_LINES);
        glVertex3f(cosf(angle0) * RIM_RADIUS, 0.0f, sinf(angle0) * RIM_RADIUS);
        glVertex3f(cosf(angle1) * (RIM_RADIUS * 0.65f), -netDepth, sinf(angle1) * (RIM_RADIUS * 0.65f));

        glVertex3f(cosf(angle1) * RIM_RADIUS, 0.0f, sinf(angle1) * RIM_RADIUS);
        glVertex3f(cosf(angle0) * (RIM_RADIUS * 0.65f), -netDepth, sinf(angle0) * (RIM_RADIUS * 0.65f));
        glEnd();
    }

    glEnable(GL_LIGHTING);
    glPopMatrix();

    glPopMatrix();
}

void renderer_draw_basketball(float radius) {
    glPushMatrix();

    renderer_apply_material(&MAT_BASKETBALL_RUBBER);
    glutSolidSphere(radius, 32, 32);

    glDisable(GL_LIGHTING);
    glColor3f(0.1f, 0.1f, 0.1f);
    glLineWidth(2.0f);

    float seamR = radius * 1.008f;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 36; ++i) {
        float a = (float)i * 2.0f * (float)M_PI / 36.0f;
        glVertex3f(cosf(a) * seamR, 0.0f, sinf(a) * seamR);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 36; ++i) {
        float a = (float)i * 2.0f * (float)M_PI / 36.0f;
        glVertex3f(cosf(a) * seamR, sinf(a) * seamR, 0.0f);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 36; ++i) {
        float a = (float)i * 2.0f * (float)M_PI / 36.0f;
        glVertex3f(0.0f, sinf(a) * seamR, cosf(a) * seamR);
    }
    glEnd();

    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void renderer_draw_player(const struct Player* player, bool ballInHands) {
    glPushMatrix();

    glTranslatef(player->x, 0.0f, player->z);
    glRotatef(player->rotationDeg, 0.0f, 1.0f, 0.0f);

    const struct PlayerColor* pCol = player_get_color(player);
    struct MaterialProperties matJersey = {
        { pCol->r * 0.35f, pCol->g * 0.35f, pCol->b * 0.35f, 1.0f },
        { pCol->r, pCol->g, pCol->b, 1.0f },
        { 0.15f, 0.15f, 0.15f, 1.0f },
        12.0f
    };

    // 1. Shoes
    renderer_apply_material(&MAT_STEEL_POLE);
    float legOffsets[2] = { -0.22f, 0.22f };
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        glTranslatef(0.06f, 0.08f, legOffsets[i]);
        renderer_draw_solid_box(0.32f, 0.16f, 0.18f);
        glPopMatrix();
    }

    // 2. Legs & Socks
    for (int i = 0; i < 2; ++i) {
        renderer_apply_material(&MAT_PLAYER_SKIN);
        glPushMatrix();
        glTranslatef(0.0f, 0.16f, legOffsets[i]);
        renderer_draw_cylinder_y(0.07f, 0.08f, 0.72f, 16);
        glPopMatrix();

        renderer_apply_material(&MAT_GLASS_BACKBOARD);
        glPushMatrix();
        glTranslatef(0.0f, 0.16f, legOffsets[i]);
        renderer_draw_cylinder_y(0.075f, 0.075f, 0.25f, 16);
        glPopMatrix();
    }

    // 3. Team Shorts
    renderer_apply_material(&matJersey);
    glPushMatrix();
    glTranslatef(0.0f, 0.80f, 0.0f);
    renderer_draw_solid_box(0.38f, 0.35f, 0.60f);
    glPopMatrix();

    // 4. Jersey Torso
    glPushMatrix();
    glTranslatef(0.0f, 1.05f, 0.0f);
    renderer_draw_cylinder_y(0.24f, 0.28f, 0.68f, 20);
    glPopMatrix();

    renderer_apply_material(&MAT_GLASS_BACKBOARD);
    glPushMatrix();
    glTranslatef(0.22f, 1.45f, 0.0f);
    renderer_draw_solid_box(0.02f, 0.22f, 0.18f);
    glPopMatrix();

    // 5. Head & Hair
    renderer_apply_material(&MAT_PLAYER_SKIN);
    glPushMatrix();
    glTranslatef(0.0f, 1.70f, 0.0f);
    renderer_draw_cylinder_y(0.09f, 0.09f, 0.12f, 16);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 1.94f, 0.0f);
    glutSolidSphere(0.16f, 24, 24);
    glPopMatrix();

    renderer_apply_material(&MAT_COURT_APRON);
    glPushMatrix();
    glTranslatef(-0.02f, 2.02f, 0.0f);
    glScalef(1.02f, 0.65f, 1.02f);
    glutSolidSphere(0.155f, 20, 20);
    glPopMatrix();

    // 6. Arms & Hands in Shooting Pocket
    float armShoulderY = 1.62f;

    renderer_apply_material(&MAT_PLAYER_SKIN);
    glPushMatrix();
    glTranslatef(0.08f, armShoulderY, 0.22f);
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(25.0f, 0.0f, 1.0f, 0.0f);
    renderer_draw_cylinder_x(0.06f, 0.05f, 0.45f, 16);
    glTranslatef(0.45f, 0.0f, 0.0f);
    glutSolidSphere(0.06f, 12, 12);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.08f, armShoulderY, -0.22f);
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(-25.0f, 0.0f, 1.0f, 0.0f);
    renderer_draw_cylinder_x(0.06f, 0.05f, 0.45f, 16);
    glTranslatef(0.45f, 0.0f, 0.0f);
    glutSolidSphere(0.06f, 12, 12);
    glPopMatrix();

    // 7. Ball resting in hands
    if (ballInHands) {
        glPushMatrix();
        glTranslatef(BALL_HAND_OFFSET_DIST, BALL_HAND_OFFSET_Y, 0.0f);
        renderer_draw_basketball(BALL_RADIUS);
        glPopMatrix();
    }

    glPopMatrix();
}

void renderer_draw_bleachers(void) {
    renderer_apply_material(&MAT_BLEACHERS);

    int numTiers = 7;
    float tierDepth  = 1.4f;
    float tierHeight = 0.65f;

    // North & South Bleachers
    float courtHalfW = (COURT_WIDTH_BOUND / 2.0f) + 3.2f;
    float courtLen = COURT_LENGTH_BOUND + 10.0f;

    for (int t = 0; t < numTiers; ++t) {
        float y = (float)t * tierHeight + (tierHeight / 2.0f);
        float zOffset = (float)t * tierDepth;

        // North Side (+Z)
        glPushMatrix();
        glTranslatef(0.0f, y, courtHalfW + zOffset + (tierDepth / 2.0f));
        renderer_draw_solid_box(courtLen, tierHeight, tierDepth);
        glPopMatrix();

        // South Side (-Z)
        glPushMatrix();
        glTranslatef(0.0f, y, -(courtHalfW + zOffset + (tierDepth / 2.0f)));
        renderer_draw_solid_box(courtLen, tierHeight, tierDepth);
        glPopMatrix();
    }

    // East & West Bleachers
    float courtHalfL = (COURT_LENGTH_BOUND / 2.0f) + 3.2f;
    float bleacherWidthZ = (courtHalfW + (float)numTiers * tierDepth) * 2.0f;

    for (int t = 0; t < numTiers; ++t) {
        float y = (float)t * tierHeight + (tierHeight / 2.0f);
        float xOffset = (float)t * tierDepth;

        // East Side (+X)
        glPushMatrix();
        glTranslatef(courtHalfL + xOffset + (tierDepth / 2.0f), y, 0.0f);
        renderer_draw_solid_box(tierDepth, tierHeight, bleacherWidthZ);
        glPopMatrix();

        // West Side (-X)
        glPushMatrix();
        glTranslatef(-(courtHalfL + xOffset + (tierDepth / 2.0f)), y, 0.0f);
        renderer_draw_solid_box(tierDepth, tierHeight, bleacherWidthZ);
        glPopMatrix();
    }
}

static void draw_audience_quad(float x, float y, float z, float width, float height, float camX, float camZ, GLuint textureId) {
    float dx = camX - x;
    float dz = camZ - z;
    float angleY = atan2f(dx, dz) * 180.0f / (float)M_PI;

    glPushMatrix();
    glTranslatef(x, y + height / 2.0f, z);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    glBindTexture(GL_TEXTURE_2D, textureId);

    float hw = width / 2.0f;
    float hh = height / 2.0f;

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, -hh, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( hw, -hh, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( hw,  hh, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw,  hh, 0.0f);
    glEnd();

    glPopMatrix();
}

void renderer_draw_audience(float camX, float camZ, float globalTime) {
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    for (int i = 0; i < g_audienceCount; ++i) {
        const struct AudienceMember* a = &g_audienceList[i];
        float cheerOffset = sinf(globalTime * a->cheeringFreq + a->cheeringPhase) * 0.06f;
        if (cheerOffset < 0.0f) cheerOffset = 0.0f;

        draw_audience_quad(
            a->x, a->y + cheerOffset, a->z,
            0.9f * a->scale, 1.25f * a->scale,
            camX, camZ,
            g_crowdTextureIds[a->textureIdx]
        );
    }

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

void renderer_draw_ball_trail_and_shadow(const struct Ball* ball) {
    if (ball->state != BALL_IN_AIR) return;

    // Glowing Trail
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glColor4f(1.0f, 0.65f, 0.1f, 0.6f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < ball->trailCount; ++i) {
        glVertex3f(ball->trail[i].x, ball->trail[i].y, ball->trail[i].z);
    }
    glVertex3f(ball->x, ball->y, ball->z);
    glEnd();

    // Ground Shadow
    glColor4f(0.12f, 0.08f, 0.06f, 0.50f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(ball->x, 0.012f, ball->z);
    for (int i = 0; i <= 24; ++i) {
        float a = (float)i * 2.0f * (float)M_PI / 24.0f;
        glVertex3f(ball->x + cosf(a) * (BALL_RADIUS * 1.2f), 0.012f, ball->z + sinf(a) * (BALL_RADIUS * 1.2f));
    }
    glEnd();
    glEnable(GL_LIGHTING);

    // Ball Sphere with Spin
    glPushMatrix();
    glTranslatef(ball->x, ball->y, ball->z);
    glRotatef(ball->spinAngle, 0.0f, 0.0f, 1.0f);
    renderer_draw_basketball(BALL_RADIUS);
    glPopMatrix();
}

void renderer_render_bitmap_text(float x, float y, void* font, const char* text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

float renderer_get_text_width(void* font, const char* text) {
    return (float)glutBitmapLength(font, (const unsigned char*)text);
}

void renderer_render_centered_text(float centerX, float y, void* font, const char* text) {
    float textW = renderer_get_text_width(font, text);
    renderer_render_bitmap_text(centerX - (textW / 2.0f), y, font, text);
}

void renderer_render_main_menu(int windowWidth, int windowHeight, int targetScore) {
    glEnable(GL_BLEND);
    glColor4f(0.03f, 0.04f, 0.07f, 0.90f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f((float)windowWidth, 0);
    glVertex2f((float)windowWidth, (float)windowHeight);
    glVertex2f(0, (float)windowHeight);
    glEnd();

    float cardW = 680.0f;
    float cardH = 490.0f;
    float cardX = ((float)windowWidth - cardW) / 2.0f;
    float cardY = ((float)windowHeight - cardH) / 2.0f;
    float cardCenterX = cardX + cardW / 2.0f;

    // Outer Glow / Shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(cardX - 8, cardY - 8);
    glVertex2f(cardX + cardW + 8, cardY - 8);
    glVertex2f(cardX + cardW + 8, cardY + cardH + 8);
    glVertex2f(cardX - 8, cardY + cardH + 8);
    glEnd();

    // Main Card Background
    glColor4f(0.07f, 0.09f, 0.14f, 0.96f);
    glBegin(GL_QUADS);
    glVertex2f(cardX, cardY);
    glVertex2f(cardX + cardW, cardY);
    glVertex2f(cardX + cardW, cardY + cardH);
    glVertex2f(cardX, cardY + cardH);
    glEnd();

    // Arcade Header Border
    glColor3f(1.0f, 0.65f, 0.15f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cardX, cardY);
    glVertex2f(cardX + cardW, cardY);
    glVertex2f(cardX + cardW, cardY + cardH);
    glVertex2f(cardX, cardY + cardH);
    glEnd();

    // Arcade Title & Subtitle (Centered)
    glColor3f(1.0f, 0.85f, 0.20f);
    renderer_render_centered_text(cardCenterX, cardY + cardH - 45, GLUT_BITMAP_TIMES_ROMAN_24, "3D BASKETBALL ARCADE");

    glColor3f(0.35f, 0.75f, 1.0f);
    renderer_render_centered_text(cardCenterX, cardY + cardH - 75, GLUT_BITMAP_HELVETICA_18, "MULTIPLAYER TOURNAMENT & PRACTICE");

    // Target Score Configuration Box (Presets: 3, 5, 7, 11, 21)
    float targetBoxY = cardY + cardH - 145;
    glColor4f(0.12f, 0.16f, 0.25f, 0.90f);
    glBegin(GL_QUADS);
    glVertex2f(cardX + 40, targetBoxY);
    glVertex2f(cardX + cardW - 40, targetBoxY);
    glVertex2f(cardX + cardW - 40, targetBoxY + 55);
    glVertex2f(cardX + 40, targetBoxY + 55);
    glEnd();

    glColor3f(0.30f, 0.60f, 0.95f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cardX + 40, targetBoxY);
    glVertex2f(cardX + cardW - 40, targetBoxY);
    glVertex2f(cardX + cardW - 40, targetBoxY + 55);
    glVertex2f(cardX + 40, targetBoxY + 55);
    glEnd();

    char targetStr[64];
    snprintf(targetStr, sizeof(targetStr), "MATCH TARGET POINTS:   [ <  %d POINTS  > ]", targetScore);
    glColor3f(1.0f, 0.90f, 0.25f);
    renderer_render_centered_text(cardCenterX, targetBoxY + 32, GLUT_BITMAP_HELVETICA_18, targetStr);

    glColor3f(0.75f, 0.85f, 0.95f);
    renderer_render_centered_text(cardCenterX, targetBoxY + 12, GLUT_BITMAP_HELVETICA_12, "Options: 3, 5, 7, 11, 21 PTS (Use UP/DOWN or T to change)");

    // Mode Options
    float optLeftX = cardX + 85.0f;
    float optY = cardY + cardH - 180;
    glColor3f(1.0f, 1.0f, 1.0f);
    renderer_render_bitmap_text(optLeftX, optY, GLUT_BITMAP_HELVETICA_18, "SELECT GAME MODE (Press Key):");

    optY -= 32;
    glColor3f(0.25f, 0.90f, 0.95f);
    renderer_render_bitmap_text(optLeftX, optY, GLUT_BITMAP_HELVETICA_18, "[P] - Practice Mode (Solo Free Play / Endless Score)");

    const char* options[] = {
        "[1] - 1 Player Match  (Red Team - Solo Challenge)",
        "[2] - 2 Players Match (Red vs Blue - 1v1 Match)",
        "[3] - 3 Players Match (Red vs Blue vs Green - Triple Threat)",
        "[4] - 4 Players Match (Red vs Blue vs Green vs Yellow - Battle)"
    };

    optY -= 30;
    for (int i = 0; i < 4; ++i) {
        const struct PlayerColor* col = &PLAYER_COLORS[i];
        glColor3f(col->r, col->g, col->b);
        renderer_render_bitmap_text(optLeftX, optY, GLUT_BITMAP_HELVETICA_18, options[i]);
        optY -= 28;
    }

    glColor3f(0.70f, 0.75f, 0.80f);
    renderer_render_centered_text(cardCenterX, cardY + 22, GLUT_BITMAP_HELVETICA_12, "Press [P] for Practice Mode, or [1], [2], [3], [4] for Match");
}

void renderer_render_game_over_screen(int windowWidth, int windowHeight, const struct Player* winner, int targetScore) {
    glEnable(GL_BLEND);
    glColor4f(0.02f, 0.03f, 0.06f, 0.88f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f((float)windowWidth, 0);
    glVertex2f((float)windowWidth, (float)windowHeight);
    glVertex2f(0, (float)windowHeight);
    glEnd();

    float cardW = 600.0f;
    float cardH = 340.0f;
    float cardX = ((float)windowWidth - cardW) / 2.0f;
    float cardY = ((float)windowHeight - cardH) / 2.0f;
    float cardCenterX = cardX + cardW / 2.0f;

    const struct PlayerColor* winCol = player_get_color(winner);

    glColor4f(0.08f, 0.10f, 0.15f, 0.96f);
    glBegin(GL_QUADS);
    glVertex2f(cardX, cardY);
    glVertex2f(cardX + cardW, cardY);
    glVertex2f(cardX + cardW, cardY + cardH);
    glVertex2f(cardX, cardY + cardH);
    glEnd();

    glColor3f(winCol->r, winCol->g, winCol->b);
    glLineWidth(4.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cardX, cardY);
    glVertex2f(cardX + cardW, cardY);
    glVertex2f(cardX + cardW, cardY + cardH);
    glVertex2f(cardX, cardY + cardH);
    glEnd();

    // Victory Title (Centered)
    glColor3f(1.0f, 0.85f, 0.20f);
    renderer_render_centered_text(cardCenterX, cardY + cardH - 55, GLUT_BITMAP_TIMES_ROMAN_24, "VICTORY!");

    char winStr[128];
    snprintf(winStr, sizeof(winStr), "★ %s WINS THE MATCH! ★", winCol->name);
    glColor3f(winCol->r, winCol->g, winCol->b);
    renderer_render_centered_text(cardCenterX, cardY + cardH - 105, GLUT_BITMAP_HELVETICA_18, winStr);

    char scoreStr[128];
    snprintf(scoreStr, sizeof(scoreStr), "Final Score: %d Points (Target: %d PTS)", winner->score, targetScore);
    glColor3f(1.0f, 1.0f, 1.0f);
    renderer_render_centered_text(cardCenterX, cardY + cardH - 145, GLUT_BITMAP_HELVETICA_18, scoreStr);

    glColor3f(0.40f, 0.80f, 1.0f);
    renderer_render_centered_text(cardCenterX, cardY + 70, GLUT_BITMAP_HELVETICA_18, "Press [R], [M] or [SPACE] to Return to Main Menu");
    glColor3f(0.80f, 0.80f, 0.80f);
    renderer_render_centered_text(cardCenterX, cardY + 35, GLUT_BITMAP_HELVETICA_12, "Press [ESC] to Exit");
}

void renderer_render_hud(
    int windowWidth, int windowHeight,
    const struct Player* players, int numPlayers, int currentPlayerIdx,
    int targetScore, float shootingAngle, float shootingPower,
    float scoreBannerTimer, const char* scoreBannerText,
    float playerX, float playerZ,
    bool isPracticeMode
) {
    float fW = (float)windowWidth;
    float fH = (float)windowHeight;

    // =========================================================================
    // 1. TOP-LEFT MODE TAG BADGE (Sleek Compact Status)
    // =========================================================================
    float tagW = isPracticeMode ? 175.0f : 160.0f;
    float tagH = 36.0f;
    float tagX = 18.0f;
    float tagY = fH - tagH - 12.0f;
    float tagCenterX = tagX + tagW / 2.0f;

    glEnable(GL_BLEND);
    glColor4f(0.06f, 0.08f, 0.13f, 0.90f);
    glBegin(GL_QUADS);
    glVertex2f(tagX, tagY);
    glVertex2f(tagX + tagW, tagY);
    glVertex2f(tagX + tagW, tagY + tagH);
    glVertex2f(tagX, tagY + tagH);
    glEnd();

    if (isPracticeMode) {
        glColor3f(0.25f, 0.90f, 0.95f);
    } else {
        glColor3f(0.35f, 0.65f, 1.0f);
    }
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(tagX, tagY);
    glVertex2f(tagX + tagW, tagY);
    glVertex2f(tagX + tagW, tagY + tagH);
    glVertex2f(tagX, tagY + tagH);
    glEnd();

    if (isPracticeMode) {
        glColor3f(0.25f, 0.90f, 0.95f);
        renderer_render_centered_text(tagCenterX, tagY + 12.0f, GLUT_BITMAP_HELVETICA_12, "★ FREE PLAY MODE");
    } else {
        glColor3f(1.0f, 0.85f, 0.25f);
        char tagStr[64];
        snprintf(tagStr, sizeof(tagStr), "★ %d-PLAYER MATCH", numPlayers);
        renderer_render_centered_text(tagCenterX, tagY + 12.0f, GLUT_BITMAP_HELVETICA_12, tagStr);
    }

    // =========================================================================
    // 2. TOP-LEFT CONTROLS BAR (Row 2 - Zero Overlap with Center Banner)
    // =========================================================================
    float ctrlW = 390.0f;
    float ctrlH = 28.0f;
    float ctrlX = 18.0f;
    float ctrlY = tagY - ctrlH - 8.0f;
    float ctrlCenterX = ctrlX + ctrlW / 2.0f;

    glColor4f(0.05f, 0.07f, 0.11f, 0.88f);
    glBegin(GL_QUADS);
    glVertex2f(ctrlX, ctrlY);
    glVertex2f(ctrlX + ctrlW, ctrlY);
    glVertex2f(ctrlX + ctrlW, ctrlY + ctrlH);
    glVertex2f(ctrlX, ctrlY + ctrlH);
    glEnd();

    glColor3f(0.22f, 0.30f, 0.42f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(ctrlX, ctrlY);
    glVertex2f(ctrlX + ctrlW, ctrlY);
    glVertex2f(ctrlX + ctrlW, ctrlY + ctrlH);
    glVertex2f(ctrlX, ctrlY + ctrlH);
    glEnd();

    glColor3f(0.85f, 0.88f, 0.92f);
    if (isPracticeMode) {
        renderer_render_centered_text(ctrlCenterX, ctrlY + 9.0f, GLUT_BITMAP_HELVETICA_12, "[W/A/S/D] Move  •  [SPACE] Shoot  •  [R] Reset  •  [M] Menu");
    } else {
        renderer_render_centered_text(ctrlCenterX, ctrlY + 9.0f, GLUT_BITMAP_HELVETICA_12, "[W/A/S/D] Move  •  [SPACE] Shoot  •  [N] Pass  •  [M] Menu");
    }

    // =========================================================================
    // 3. TOP CENTER TURN BANNER (Compact, Centered & Clean)
    // =========================================================================
    float topBannerW = 360.0f;
    float topBannerH = 38.0f;
    float topBannerX = (fW - topBannerW) / 2.0f;
    float topBannerY = fH - topBannerH - 12.0f;
    float topBannerCenterX = topBannerX + topBannerW / 2.0f;

    const struct PlayerColor* curCol = player_get_color(&players[currentPlayerIdx]);

    // Dark glass background
    glColor4f(0.05f, 0.07f, 0.12f, 0.94f);
    glBegin(GL_QUADS);
    glVertex2f(topBannerX, topBannerY);
    glVertex2f(topBannerX + topBannerW, topBannerY);
    glVertex2f(topBannerX + topBannerW, topBannerY + topBannerH);
    glVertex2f(topBannerX, topBannerY + topBannerH);
    glEnd();

    // Active glowing border
    if (isPracticeMode) {
        glColor3f(0.25f, 0.90f, 0.95f);
    } else {
        glColor3f(curCol->r, curCol->g, curCol->b);
    }
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(topBannerX, topBannerY);
    glVertex2f(topBannerX + topBannerW, topBannerY);
    glVertex2f(topBannerX + topBannerW, topBannerY + topBannerH);
    glVertex2f(topBannerX, topBannerY + topBannerH);
    glEnd();

    if (isPracticeMode) {
        glColor3f(1.0f, 1.0f, 1.0f);
        renderer_render_centered_text(topBannerCenterX, topBannerY + 12.0f, GLUT_BITMAP_HELVETICA_18, "★ PRACTICE MODE ★");
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);
        char turnStr[128];
        snprintf(turnStr, sizeof(turnStr), "★ %s TURN ★", curCol->name);
        renderer_render_centered_text(topBannerCenterX, topBannerY + 12.0f, GLUT_BITMAP_HELVETICA_18, turnStr);
    }

    // =========================================================================
    // 4. TOP-RIGHT SCOREBOARD (Clean Arcade Score Header)
    // =========================================================================
    if (isPracticeMode) {
        float scoreBoxW = 200.0f;
        float scoreBoxH = 36.0f;
        float scoreBoxX = fW - scoreBoxW - 18.0f;
        float scoreBoxY = fH - scoreBoxH - 12.0f;

        glColor4f(0.06f, 0.08f, 0.13f, 0.90f);
        glBegin(GL_QUADS);
        glVertex2f(scoreBoxX, scoreBoxY);
        glVertex2f(scoreBoxX + scoreBoxW, scoreBoxY);
        glVertex2f(scoreBoxX + scoreBoxW, scoreBoxY + scoreBoxH);
        glVertex2f(scoreBoxX, scoreBoxY + scoreBoxH);
        glEnd();

        glColor3f(0.25f, 0.90f, 0.95f);
        glLineWidth(1.2f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(scoreBoxX, scoreBoxY);
        glVertex2f(scoreBoxX + scoreBoxW, scoreBoxY);
        glVertex2f(scoreBoxX + scoreBoxW, scoreBoxY + scoreBoxH);
        glVertex2f(scoreBoxX, scoreBoxY + scoreBoxH);
        glEnd();

        char pracStr[64];
        snprintf(pracStr, sizeof(pracStr), "SCORE: %d PTS", players[0].score);
        glColor3f(1.0f, 0.90f, 0.25f);
        renderer_render_centered_text(scoreBoxX + scoreBoxW / 2.0f, scoreBoxY + 12.0f, GLUT_BITMAP_HELVETICA_12, pracStr);
    } else {
        float scoreBoxW = 85.0f + (float)numPlayers * 68.0f;
        float scoreBoxH = 36.0f;
        float scoreBoxX = fW - scoreBoxW - 18.0f;
        float scoreBoxY = fH - scoreBoxH - 12.0f;

        glColor4f(0.06f, 0.08f, 0.13f, 0.90f);
        glBegin(GL_QUADS);
        glVertex2f(scoreBoxX, scoreBoxY);
        glVertex2f(scoreBoxX + scoreBoxW, scoreBoxY);
        glVertex2f(scoreBoxX + scoreBoxW, scoreBoxY + scoreBoxH);
        glVertex2f(scoreBoxX, scoreBoxY + scoreBoxH);
        glEnd();

        glColor3f(0.35f, 0.45f, 0.60f);
        glLineWidth(1.2f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(scoreBoxX, scoreBoxY);
        glVertex2f(scoreBoxX + scoreBoxW, scoreBoxY);
        glVertex2f(scoreBoxX + scoreBoxW, scoreBoxY + scoreBoxH);
        glVertex2f(scoreBoxX, scoreBoxY + scoreBoxH);
        glEnd();

        float pItemX = scoreBoxX + 8.0f;
        for (int i = 0; i < numPlayers; ++i) {
            const struct PlayerColor* col = player_get_color(&players[i]);

            if (i == currentPlayerIdx) {
                glColor4f(col->r * 0.35f, col->g * 0.35f, col->b * 0.35f, 0.8f);
                glBegin(GL_QUADS);
                glVertex2f(pItemX - 2, scoreBoxY + 4);
                glVertex2f(pItemX + 54, scoreBoxY + 4);
                glVertex2f(pItemX + 54, scoreBoxY + scoreBoxH - 4);
                glVertex2f(pItemX - 2, scoreBoxY + scoreBoxH - 4);
                glEnd();
            }

            glColor3f(col->r, col->g, col->b);
            char sStr[32];
            snprintf(sStr, sizeof(sStr), "P%d:%d", (i + 1), players[i].score);
            renderer_render_centered_text(pItemX + 26.0f, scoreBoxY + 12.0f, GLUT_BITMAP_HELVETICA_12, sStr);

            if (i < numPlayers - 1) {
                glColor3f(0.35f, 0.40f, 0.50f);
                renderer_render_bitmap_text(pItemX + 55.0f, scoreBoxY + 12.0f, GLUT_BITMAP_HELVETICA_12, "|");
            }
            pItemX += 64.0f;
        }

        // Target Score Badge
        glColor3f(1.0f, 0.85f, 0.20f);
        char goalStr[32];
        snprintf(goalStr, sizeof(goalStr), "GOAL:%d", targetScore);
        renderer_render_centered_text(pItemX + 26.0f, scoreBoxY + 12.0f, GLUT_BITMAP_HELVETICA_12, goalStr);
    }

    // =========================================================================
    // 3. ZONE INDICATOR (Neatly Above Bottom Shot Meters - Centered Text)
    // =========================================================================
    float curDist = ball_calculate_distance_to_hoop(playerX, playerZ);
    bool in3PT = ball_is_inside_three_point_zone(playerX, playerZ);

    float zoneW = 340.0f;
    float zoneH = 30.0f;
    float zoneX = (fW - zoneW) / 2.0f;
    float zoneY = 118.0f;
    float zoneCenterX = zoneX + zoneW / 2.0f;

    glColor4f(0.06f, 0.08f, 0.13f, 0.88f);
    glBegin(GL_QUADS);
    glVertex2f(zoneX, zoneY);
    glVertex2f(zoneX + zoneW, zoneY);
    glVertex2f(zoneX + zoneW, zoneY + zoneH);
    glVertex2f(zoneX, zoneY + zoneH);
    glEnd();

    if (in3PT) {
        glColor3f(1.0f, 0.45f, 0.15f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(zoneX, zoneY);
        glVertex2f(zoneX + zoneW, zoneY);
        glVertex2f(zoneX + zoneW, zoneY + zoneH);
        glVertex2f(zoneX, zoneY + zoneH);
        glEnd();

        char zStr[64];
        snprintf(zStr, sizeof(zStr), "★ 3PT ARC ZONE  •  %.1fm  (+3 PTS)", curDist);
        glColor3f(1.0f, 0.55f, 0.20f);
        renderer_render_centered_text(zoneCenterX, zoneY + 9.0f, GLUT_BITMAP_HELVETICA_12, zStr);
    } else {
        glColor3f(0.20f, 0.80f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(zoneX, zoneY);
        glVertex2f(zoneX + zoneW, zoneY);
        glVertex2f(zoneX + zoneW, zoneY + zoneH);
        glVertex2f(zoneX, zoneY + zoneH);
        glEnd();

        char zStr[64];
        snprintf(zStr, sizeof(zStr), "🏀 2PT FIELD ZONE  •  %.1fm  (+2 PTS)", curDist);
        glColor3f(0.30f, 0.88f, 1.0f);
        renderer_render_centered_text(zoneCenterX, zoneY + 9.0f, GLUT_BITMAP_HELVETICA_12, zStr);
    }

    // =========================================================================
    // 4. SHOT METERS (Bottom Center Segmented Arcade Gauges - Centered Text)
    // =========================================================================
    float dashW = 600.0f;
    float dashH = 88.0f;
    float dashX = (fW - dashW) / 2.0f;
    float dashY = 18.0f;
    float dashCenterX = dashX + dashW / 2.0f;

    // Dashboard background
    glColor4f(0.06f, 0.08f, 0.13f, 0.94f);
    glBegin(GL_QUADS);
    glVertex2f(dashX, dashY);
    glVertex2f(dashX + dashW, dashY);
    glVertex2f(dashX + dashW, dashY + dashH);
    glVertex2f(dashX, dashY + dashH);
    glEnd();

    glColor3f(0.30f, 0.40f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(dashX, dashY);
    glVertex2f(dashX + dashW, dashY);
    glVertex2f(dashX + dashW, dashY + dashH);
    glVertex2f(dashX, dashY + dashH);
    glEnd();

    // Divider Line
    glColor3f(0.20f, 0.25f, 0.35f);
    glBegin(GL_LINES);
    glVertex2f(dashX + dashW / 2.0f, dashY + 12);
    glVertex2f(dashX + dashW / 2.0f, dashY + dashH - 12);
    glEnd();

    // ---------------- Left: Angle Meter ----------------
    float angleGaugeX = dashX + 25.0f;
    float angleGaugeY = dashY + 44.0f;
    float gaugeW = 230.0f;
    float gaugeH = 14.0f;

    char angleStr[64];
    snprintf(angleStr, sizeof(angleStr), "ANGLE: %.1f deg  [▲/▼]", shootingAngle);
    glColor3f(1.0f, 0.85f, 0.25f);
    renderer_render_centered_text(angleGaugeX + gaugeW / 2.0f, angleGaugeY + 18.0f, GLUT_BITMAP_HELVETICA_12, angleStr);

    glColor3f(0.25f, 0.30f, 0.40f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(angleGaugeX, angleGaugeY);
    glVertex2f(angleGaugeX + gaugeW, angleGaugeY);
    glVertex2f(angleGaugeX + gaugeW, angleGaugeY + gaugeH);
    glVertex2f(angleGaugeX, angleGaugeY + gaugeH);
    glEnd();

    float angleFrac = (shootingAngle - 15.0f) / (85.0f - 15.0f);
    int totalSegments = 16;
    int filledSegments = (int)(angleFrac * totalSegments);
    float segW = (gaugeW - 4.0f) / (float)totalSegments;

    for (int s = 0; s < totalSegments; ++s) {
        float sx = angleGaugeX + 2.0f + (float)s * segW;
        if (s < filledSegments) {
            glColor3f(1.0f, 0.75f, 0.15f);
        } else {
            glColor4f(0.15f, 0.18f, 0.25f, 0.6f);
        }
        glBegin(GL_QUADS);
        glVertex2f(sx + 1, angleGaugeY + 2);
        glVertex2f(sx + segW - 1, angleGaugeY + 2);
        glVertex2f(sx + segW - 1, angleGaugeY + gaugeH - 2);
        glVertex2f(sx + 1, angleGaugeY + gaugeH - 2);
        glEnd();
    }

    // ---------------- Right: Power Meter ----------------
    float powerGaugeX = dashX + dashW / 2.0f + 25.0f;
    float powerGaugeY = dashY + 44.0f;

    char powerStr[64];
    snprintf(powerStr, sizeof(powerStr), "POWER: %.1f%%  [◄/►]", shootingPower);
    glColor3f(0.25f, 0.85f, 1.0f);
    renderer_render_centered_text(powerGaugeX + gaugeW / 2.0f, powerGaugeY + 18.0f, GLUT_BITMAP_HELVETICA_12, powerStr);

    glColor3f(0.25f, 0.30f, 0.40f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(powerGaugeX, powerGaugeY);
    glVertex2f(powerGaugeX + gaugeW, powerGaugeY);
    glVertex2f(powerGaugeX + gaugeW, powerGaugeY + gaugeH);
    glVertex2f(powerGaugeX, powerGaugeY + gaugeH);
    glEnd();

    float powerFrac = (shootingPower - 10.0f) / (100.0f - 10.0f);
    int filledPowerSegments = (int)(powerFrac * totalSegments);

    for (int s = 0; s < totalSegments; ++s) {
        float sx = powerGaugeX + 2.0f + (float)s * segW;
        if (s < filledPowerSegments) {
            float segFrac = (float)s / (float)totalSegments;
            if (segFrac < 0.6f) glColor3f(0.15f, 0.85f, 0.95f);
            else if (segFrac < 0.85f) glColor3f(0.95f, 0.85f, 0.20f);
            else glColor3f(1.0f, 0.35f, 0.20f);
        } else {
            glColor4f(0.15f, 0.18f, 0.25f, 0.6f);
        }
        glBegin(GL_QUADS);
        glVertex2f(sx + 1, powerGaugeY + 2);
        glVertex2f(sx + segW - 1, powerGaugeY + 2);
        glVertex2f(sx + segW - 1, powerGaugeY + gaugeH - 2);
        glVertex2f(sx + 1, powerGaugeY + gaugeH - 2);
        glEnd();
    }

    // Bottom Shoot Instruction (Centered)
    glColor3f(0.35f, 0.95f, 0.45f);
    renderer_render_centered_text(dashCenterX, dashY + 14.0f, GLUT_BITMAP_HELVETICA_12, "Press [SPACE] to Shoot Basketball   •   [R / N] Pass Turn");

    // =========================================================================
    // 5. ACTION FEEDBACK (Center Screen Popup on Basket - Centered Text)
    // =========================================================================
    if (scoreBannerTimer > 0.0f) {
        float bannerW = 540.0f;
        float bannerH = 100.0f;
        float bannerX = (fW - bannerW) / 2.0f;
        float bannerY = (fH - bannerH) / 2.0f + 40.0f;
        float bannerCenterX = bannerX + bannerW / 2.0f;

        // Dark Glowing Backdrop
        glColor4f(0.04f, 0.06f, 0.10f, 0.95f);
        glBegin(GL_QUADS);
        glVertex2f(bannerX, bannerY);
        glVertex2f(bannerX + bannerW, bannerY);
        glVertex2f(bannerX + bannerW, bannerY + bannerH);
        glVertex2f(bannerX, bannerY + bannerH);
        glEnd();

        bool isThree = (strstr(scoreBannerText, "THREE") != NULL);
        if (isThree) {
            glColor3f(1.0f, 0.50f, 0.10f);
        } else {
            glColor3f(0.20f, 0.90f, 0.40f);
        }
        glLineWidth(4.0f);

        glBegin(GL_LINE_LOOP);
        glVertex2f(bannerX, bannerY);
        glVertex2f(bannerX + bannerW, bannerY);
        glVertex2f(bannerX + bannerW, bannerY + bannerH);
        glVertex2f(bannerX, bannerY + bannerH);
        glEnd();

        // Main Scoring Announcement (Centered)
        if (isThree) {
            glColor3f(1.0f, 0.85f, 0.20f);
            renderer_render_centered_text(bannerCenterX, bannerY + 58.0f, GLUT_BITMAP_TIMES_ROMAN_24, "★ THREE POINTER! +3 POINTS! ★");
        } else {
            glColor3f(0.30f, 0.95f, 0.50f);
            renderer_render_centered_text(bannerCenterX, bannerY + 58.0f, GLUT_BITMAP_TIMES_ROMAN_24, "★ TWO POINTER! +2 POINTS! ★");
        }

        char subStr[128];
        if (isPracticeMode) {
            snprintf(subStr, sizeof(subStr), "NICE SHOT! BALL RESET INCOMING...");
        } else {
            snprintf(subStr, sizeof(subStr), "SCORED BY %s! PASSING TURN INCOMING...", curCol->name);
        }
        glColor3f(1.0f, 1.0f, 1.0f);
        renderer_render_centered_text(bannerCenterX, bannerY + 24.0f, GLUT_BITMAP_HELVETICA_12, subStr);
    }
}
