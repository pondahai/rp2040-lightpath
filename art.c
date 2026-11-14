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
#define C_MIRROR  RD_RGB565(200, 220, 240)
#define C_MIR_DK  RD_RGB565(110, 130, 150)
#define C_CURSOR  RD_RGB565(255, 220, 40)

static uint16_t *tile(int t) { return &art_tiles[t * TILE * TILE]; }

static void floor_into(uint16_t *p)
{
    for (int y = 0; y < TILE; y++)
        for (int x = 0; x < TILE; x++)
            p[y * TILE + x] = (x == 0 || y == 0) ? C_GRID : C_FLOOR;
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

    /* 發射器：紅色方塊，右邊一支槍管（G2 會依方向旋轉） */
    p = tile(T_EMIT);
    floor_into(p);
    for (int y = 3; y <= 12; y++)
        for (int x = 2; x <= 10; x++)
            p[y * TILE + x] = (y == 3 || x == 2) ? C_EMIT_HI : C_EMIT;
    for (int y = 6; y <= 9; y++)
        for (int x = 11; x <= 15; x++)
            p[y * TILE + x] = C_EMIT_HI;

    /* 接收器：綠色圓環 */
    p = tile(T_RECV);
    floor_into(p);
    for (int y = 0; y < TILE; y++)
        for (int x = 0; x < TILE; x++) {
            int dx = 2 * x - 15, dy = 2 * y - 15, d = dx * dx + dy * dy;
            if (d <= 13 * 13 && d >= 7 * 7) p[y * TILE + x] = C_RECV;
            else if (d < 7 * 7)             p[y * TILE + x] = C_RECV_IN;
        }

    /* 鏡子：兩像素寬的亮線 + 一側暗邊（鏡背） */
    uint16_t *s = tile(T_MIR_S), *b = tile(T_MIR_B);
    floor_into(s);
    floor_into(b);
    for (int i = 1; i < TILE - 1; i++) {
        int j = TILE - 1 - i;
        s[j * TILE + i] = C_MIRROR;                       /* / */
        if (i + 1 < TILE) s[j * TILE + i + 1] = C_MIRROR;
        if (j + 1 < TILE) s[(j + 1) * TILE + i] = C_MIR_DK;
        b[i * TILE + i] = C_MIRROR;                       /* \ */
        if (i + 1 < TILE) b[i * TILE + i + 1] = C_MIRROR;
        if (i + 1 < TILE) b[(i + 1) * TILE + i] = C_MIR_DK;
    }

    /* 游標：四個角的 L 形框，其餘透明 */
    for (int y = 0; y < TILE; y++)
        for (int x = 0; x < TILE; x++) {
            int ex = x < 2 || x > 13, ey = y < 2 || y > 13;
            int nx = x < 6 || x > 9,  ny = y < 6 || y > 9;
            art_cursor[y * TILE + x] =
                ((ex && ny) || (ey && nx)) ? C_CURSOR : KEY_MAGENTA;
        }
}
