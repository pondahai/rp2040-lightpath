#include "art.h"
#include "gfx.h"

uint16_t art_tiles[T_COUNT * TILE * TILE];
uint16_t art_cursor[TILE * TILE];

#define C_FLOOR   RD_RGB565(24, 28, 48)
#define C_GRID    RD_RGB565(40, 46, 74)
#define C_WALL    RD_RGB565(96, 96, 110)
#define C_WALL_HI RD_RGB565(140, 140, 156)
#define C_WALL_LO RD_RGB565(56, 56, 66)
#define C_EMIT    RD_RGB565(200, 40, 40)
#define C_EMIT_HI RD_RGB565(255, 120, 100)
#define C_RECV    RD_RGB565(60, 140, 90)
#define C_RECV_IN RD_RGB565(20, 60, 36)
#define C_LIT     RD_RGB565(140, 255, 120)
#define C_LIT_IN  RD_RGB565(240, 255, 220)
#define C_MIRROR  RD_RGB565(200, 220, 240)
#define C_MIR_DK  RD_RGB565(110, 130, 150)
#define C_PLATE   RD_RGB565(58, 62, 80)
#define C_BOLT    RD_RGB565(120, 124, 140)
#define C_CURSOR  RD_RGB565(255, 220, 40)

static uint16_t *tile(int t) { return &art_tiles[t * TILE * TILE]; }

static void floor_into(uint16_t *p)
{
    for (int y = 0; y < TILE; y++)
        for (int x = 0; x < TILE; x++)
            p[y * TILE + x] = (x == 0 || y == 0) ? C_GRID : C_FLOOR;
}

/* 把朝東的圖轉成其他方向：rot = 1 北、2 西、3 南（逆時針 90° 的次數） */
static void rotate_into(uint16_t *dst, const uint16_t *src, int rot)
{
    for (int y = 0; y < TILE; y++)
        for (int x = 0; x < TILE; x++) {
            int nx = x, ny = y;
            if (rot == 1) { nx = y;            ny = TILE - 1 - x; }
            if (rot == 2) { nx = TILE - 1 - x; ny = y; }
            if (rot == 3) { nx = TILE - 1 - y; ny = x; }
            dst[ny * TILE + nx] = src[y * TILE + x];
        }
}

static void ring_into(uint16_t *p, uint16_t ring, uint16_t core)
{
    floor_into(p);
    for (int y = 0; y < TILE; y++)
        for (int x = 0; x < TILE; x++) {
            int dx = 2 * x - 15, dy = 2 * y - 15, d = dx * dx + dy * dy;
            if (d <= 13 * 13 && d >= 7 * 7) p[y * TILE + x] = ring;
            else if (d < 7 * 7)             p[y * TILE + x] = core;
        }
}

/* slash = 1 畫 /，0 畫 \ */
static void mirror_into(uint16_t *p, int slash, int fixed)
{
    floor_into(p);
    if (fixed) {
        for (int y = 2; y <= 13; y++)
            for (int x = 2; x <= 13; x++)
                p[y * TILE + x] = C_PLATE;
        p[3 * TILE + 3] = p[3 * TILE + 12] = p[12 * TILE + 3] = p[12 * TILE + 12] = C_BOLT;
    }
    for (int i = 1; i < TILE - 1; i++) {
        int y = slash ? TILE - 1 - i : i;
        p[y * TILE + i] = C_MIRROR;
        p[y * TILE + i + 1] = C_MIRROR;
        if (y + 1 < TILE) p[(y + 1) * TILE + i] = C_MIR_DK;
    }
}

void art_init(void)
{
    uint16_t *p;

    floor_into(tile(T_FLOOR));

    /* 牆：上亮下暗的方塊，中間一道磚縫 */
    p = tile(T_WALL);
    for (int y = 0; y < TILE; y++)
        for (int x = 0; x < TILE; x++) {
            uint16_t c = C_WALL;
            if (y == 0 || x == 0) c = C_WALL_HI;
            if (y == TILE - 1 || x == TILE - 1) c = C_WALL_LO;
            if (y == 7 || (y < 7 && x == 8) || (y > 7 && x == 3)) c = C_WALL_LO;
            p[y * TILE + x] = c;
        }

    /* 發射器：紅色方塊 + 朝東的槍管，再轉出其他三個方向 */
    p = tile(T_EMIT_E);
    floor_into(p);
    for (int y = 3; y <= 12; y++)
        for (int x = 2; x <= 10; x++)
            p[y * TILE + x] = (y == 3 || x == 2) ? C_EMIT_HI : C_EMIT;
    for (int y = 6; y <= 9; y++)
        for (int x = 11; x <= 15; x++)
            p[y * TILE + x] = C_EMIT_HI;
    for (int r = 1; r <= 3; r++)
        rotate_into(tile(T_EMIT_E + r), p, r);

    ring_into(tile(T_RECV), C_RECV, C_RECV_IN);
    ring_into(tile(T_RECV_ON), C_LIT, C_LIT_IN);

    mirror_into(tile(T_MIR_S), 1, 0);
    mirror_into(tile(T_MIR_B), 0, 0);
    mirror_into(tile(T_FMIR_S), 1, 1);
    mirror_into(tile(T_FMIR_B), 0, 1);

    /* 游標：四個角的 L 形框，其餘透明 */
    for (int y = 0; y < TILE; y++)
        for (int x = 0; x < TILE; x++) {
            int ex = x < 2 || x > 13, ey = y < 2 || y > 13;
            int nx = x < 6 || x > 9,  ny = y < 6 || y > 9;
            art_cursor[y * TILE + x] =
                ((ex && ny) || (ey && nx)) ? C_CURSOR : KEY_MAGENTA;
        }
}
