#include <SupSy/SL.h>

#define WIDTH 200
#define HEIGHT 150

#include "easyWIN32.h"

static const float VIEW_HEIGHT = 1.6;
static float VIEW_WIDTH = 0;
static const float FOV = PI*0.25;
static const float NCP = 0.03;

static ew32_texture texture;

uint32 toValue(float intensity) {
    uint8 color = floor(SL_clamp(intensity, 0.0, 1.0) * 255 + 0.5);
    return EW32_PACK_RGB(color, color, color);
}
void drawValue(uint x, uint y, float intensity) {
    ((uint32*)texture.buffer)[x + y * WIDTH] = toValue(intensity);
}
void clear() {
    for (uint y = 0; y < HEIGHT; ++y) {
        float d = y / (float)HEIGHT;
        float bg = d > 0 ? d * 0.5 * 0.9 + 0.1 : 0.0;
        for (uint x = 0; x < WIDTH; ++x) {
            drawValue(x, y, bg);
        }
    }
}



typedef struct Material {
    uint type;
    union {
        struct {
            float color;
        };
        struct {
            float* tex;
            uint8 size;
        };
    };
} material;
typedef struct Wall {
    vec2 p1, p2;
    float floor, height;
    material* mat;
} wall;
vec2 wallNormal(wall* w) {
    vec2 dir = norm2(sub2(w->p1, w->p2));
    return Vec2(-dir.y, dir.x);
}
float wallDist(vec2 org, vec2 dir, wall* w) {
    vec2 p1t = sub2(w->p1, org);
    vec2 p2t = sub2(w->p2, org);

    /*
        a1 = (p2t.y - p1t.y) / (p2t.x - p1t.x)
        b1 = p1t.y - (p2t.y - p1t.y) / (p2t.x - p1t.x) * p1t.x

        a2 = dir.y / dir.x
        b2 = 0

        | y = a1*x + b1
        | y = a2*x + b2
        <=>
        | x = -(b1 - b2) / (a1 - a2)
        | y = a2 * x + b2
        <=>
        | x = -b1 / (a1 - a2)
        | y = a2 * x
    */
    float dx = p2t.x - p1t.x, dy = p2t.y - p1t.y;
    float x = 0, y = 0;
    if (dx == 0.0) {
        if (dir.x == 0) return FLOAT_MAX;

        x = p2t.x;
        y = dir.y / dir.x * x;
    }
    else {

        float a1 = dy / dx;

        if (dir.x == 0.0) {
            float b1 = p1t.y - p1t.x * a1;
            return sqrt(b1 * b1);
        }

        float b1 = p1t.y - a1 * p1t.x;
        float a2 = dir.y / dir.x;
        if (a1 == a2) return FLOAT_MAX;

        x = -b1 / (a1 - a2);
        y = a2 * x;
    }

    if (
        (dx != 0 && ((x < p1t.x && x < p2t.x) || (x > p1t.x && x > p2t.x))) ||
        (dy != 0 && ((y < p1t.y && y < p2t.y) || (y > p1t.y && y > p2t.y)))
    ) return FLOAT_MAX;

    if (x * dir.x + y * dir.y < 0) return FLOAT_MAX; // Looking at wall backwards

    return sqrt(x*x + y*y);
}
void wallDraw(wall* w, uint x, float dist, vec2 dir, vec2 org) {
    if (dist < NCP) return;
    float hM = (VIEW_HEIGHT) / dist;
    float hm = (w->height - VIEW_HEIGHT) / dist;

    uint yM = SL_min(floor((0.5 + hM) * HEIGHT), HEIGHT);
    uint ym = SL_max(floor((0.5 - hm) * HEIGHT), 0);
    
    float fog = (1.0 - (dist - NCP) * 0.05);
    float diffuse = (fabs(dot2(dir, wallNormal(w))) * 1.2 + 0.0);
    float light = fog * diffuse;

    if (w->mat->type == 0) {
        light *= w->mat->color;
        for (uint i = ym; i < yM; i++) drawValue(x, i, light);
    }
    else if (w->mat->type == 1) {
        vec2 hitPos = addS2(org, dir, dist + NCP);
        // float u = length2(sub2(hitPos, w->p1)) / length2(sub2(w->p2, w->p1));
        float u = len2(sub2(hitPos, w->p1));
        u = u - floor(u);

        float dv = floor((0.5 - hm) * HEIGHT);
        float dV = 1.0 / (float)(floor((0.5 + hM) * HEIGHT) - dv);
        for (uint i = ym; i < yM; i++) {
            float v = (i - dv) * dV;

            uint UV = floor(u * w->mat->size) + floor(v * w->mat->size) * w->mat->size;
            float color = w->mat->tex[UV];
            drawValue(x, i, color * light);
        }
    }
}
void sceneRender(wall* walls, uint nbWalls, vec2 playerPos, vec2 playerDir) {
    clear();
    vec2 orth = Vec2(-playerDir.y, playerDir.x);
    float viewShift = VIEW_WIDTH / NCP;

    for (uint i = 0; i < WIDTH; i++) {
        struct { uint idx; float dist, height; } wallStack[64] = {0};
        uint wallStackIdx = 0;

        float fact = (1.0 - 2.0 * i / (WIDTH - 1.0)) * viewShift;
        vec2 renderDir = addS2(playerDir, orth, fact);

        for (uint j = 0; j < nbWalls; j++) {
            float dist = wallDist(playerPos, renderDir, walls + j);
            if (dist >= FLOAT_MAX) continue;

            int k = ++wallStackIdx;
            for (; k >= 0; k--) {
                if (wallStack[k].dist > dist) break;
                wallStack[k + 1] = wallStack[k];
            }
            if (k < 0) k = 0;
            wallStack[k].dist = dist;
            wallStack[k].height = walls[j].height;
            wallStack[k].idx = j;
        }

        for (uint j = 0; j < wallStackIdx; j++) wallDraw(walls + wallStack[j].idx, i, wallStack[j].dist / sqrt(1.0 + fact*fact), norm2(renderDir), playerPos);
    }

}

static float* TEXTURES[] = {
    (float[]) {
        0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.8, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.8,
        0.0, 0.8, 1.0, 1.0, 1.0, 1.0, 0.8, 0.5, 0.0, 0.8, 1.0, 1.0, 1.0, 1.0, 0.8, 0.5,
        0.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
        0.0, 0.1, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0, 0.0, 0.1, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 0.8, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.8, 0.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 0.8, 0.5, 0.0, 0.8, 1.0, 1.0, 1.0, 1.0, 0.8, 0.5, 0.0, 0.8, 1.0, 1.0,
        0.5, 0.5, 0.5, 0.5, 0.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.0, 0.5, 0.5, 0.5,
        0.1, 0.0, 0.0, 0.0, 0.0, 0.1, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0, 0.0, 0.1, 0.1, 0.1,
        0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.8, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.8,
        0.0, 0.8, 1.0, 1.0, 1.0, 1.0, 0.8, 0.5, 0.0, 0.8, 1.0, 1.0, 1.0, 1.0, 0.8, 0.5,
        0.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
        0.0, 0.1, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0, 0.0, 0.1, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 0.8, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.8, 0.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 0.8, 0.5, 0.0, 0.8, 1.0, 1.0, 1.0, 1.0, 0.8, 0.5, 0.0, 0.8, 1.0, 1.0,
        0.5, 0.5, 0.5, 0.5, 0.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.0, 0.5, 0.5, 0.5,
        0.1, 0.0, 0.0, 0.0, 0.0, 0.1, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0, 0.0, 0.1, 0.1, 0.1,
        
    },
    (float[]) {
        0.0, 1.0,
        1.0, 0.0
    }
};

int ew32_main(int argc, char** argv) {
    texture = (ew32_texture){.width = WIDTH, .height = HEIGHT, .bitDepth = 32, .buffer = malloc(sizeof(uint32) * WIDTH * HEIGHT)};
    EW32_SetTexture(texture);

    VIEW_WIDTH = tan(FOV * 0.5) * NCP;
    
    float angle = PI * 0.5;
    vec2 position = vec2_zero;

    material materials[] = {
        (material){.type = 0, .color = 0.5},
        (material){.type = 0, .color = 0.5},
        (material){.type = 1, .size = 16, .tex = TEXTURES[0]}
    };
    const uint nbMaterials = sizeof(materials) / sizeof(material);

    wall walls[] = {
        (wall){.p1 = Vec2(-5, 5), .p2 = Vec2(5, 5), .floor = 0.0, .height = 2.0, .mat = materials + 2},
        (wall){.p1 = Vec2(-5, 5), .p2 = Vec2(-5, -5), .floor = 0.0, .height = 2.0, .mat = materials + 2},
        (wall){.p1 = Vec2(-10, -10), .p2 = Vec2(-10, -12), .floor = 0.0, .height = 5.0, .mat = materials + 0},
        (wall){.p1 = Vec2(-10, -12), .p2 = Vec2(-12, -15), .floor = 0.0, .height = 5.0, .mat = materials + 2}
    };
    const uint nbWalls = sizeof(walls) / sizeof(wall);

    while (!EW32_ShouldClose()) {
        EW32_StartFrame();
        if (EW32_inputIsKeyDown(EW32_KEY_ESCAPE)) EW32_SetShouldClose(true);

        double dt = EW32_GetDeltaTime();

        // PLAYER MOVEMENT
        angle += (EW32_inputIsKeyDown('K') - EW32_inputIsKeyDown('M')) * 1.5 * dt;

        vec2 dir = Vec2(cos(angle), sin(angle));
        vec2 orth = Vec2(-dir.y, dir.x);
        position = addS2(position, dir, (EW32_inputIsKeyDown('Z') - EW32_inputIsKeyDown('S')) * 2.5 * (1 + EW32_inputIsKeyDown(EW32_KEY_SHIFT)) * dt);
        position = addS2(position, orth, -(EW32_inputIsKeyDown('D') - EW32_inputIsKeyDown('Q')) * 2.5 * (1 + EW32_inputIsKeyDown(EW32_KEY_SHIFT)) * dt);
        
        // SCENE RENDERING
        sceneRender(walls, nbWalls, position, dir);

        EW32_EndFrame();
    }

    return 0;
}