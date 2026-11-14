#include <stdio.h>
#include <string.h>
#include "scene.h"
#include "art.h"

#define C_PANEL   RD_RGB565(12, 14, 26)
#define C_TITLE   RD_RGB565(255, 220, 40)
#define C_LABEL   RD_RGB565(150, 160, 190)
#define C_VALUE   RD_WHITE

/* G1 示範盤面：外圈牆、一個發射器、兩個接收器、幾面鏡子 */
static const char *DEMO[BOARD_H] = {
    "################",
    "#..............#",
    "#.E.....\\......#",
    "#..............#",
    "#.....##.......#",
    "#..../.........#",
    "#........R.....#",
    "#..##..........#",
    "#.......\\...R..#",
    "#..............#",
    "#..............#",
    "################",
};

void scene_demo(scene *sc)
{
    memset(sc, 0, sizeof *sc);
    for (int y = 0; y < BOARD_H; y++)
        for (int x = 0; x < BOARD_W; x++) {
            uint8_t t = T_FLOOR;
            switch (DEMO[y][x]) {
            case '#':  t = T_WALL;  break;
            case 'E':  t = T_EMIT;  break;
            case 'R':  t = T_RECV;  break;
            case '/':  t = T_MIR_S; break;
            case '\\': t = T_MIR_B; break;
            }
            sc->map[y * BOARD_W + x] = t;
        }
    sc->cx = 3;
    sc->cy = 2;
    sc->level = 1;
}

static rd_dirty cell_dirty(int cy)
{
    rd_dirty d = 0;
    rd_dirty_mark(&d, cy * TILE, TILE);
    return d;
}

static rd_dirty panel_dirty(void)
{
    rd_dirty d = 0;
    rd_dirty_mark(&d, 40, 24);              /* 數字那兩行 */
    return d;
}

rd_dirty scene_move(scene *sc, int dx, int dy)
{
    int nx = sc->cx + dx, ny = sc->cy + dy;
    if (nx < 0 || nx >= BOARD_W || ny < 0 || ny >= BOARD_H) return 0;
    rd_dirty d = cell_dirty(sc->cy);
    sc->cx = nx;
    sc->cy = ny;
    return d | cell_dirty(ny);
}

rd_dirty scene_rotate(scene *sc)
{
    uint8_t *t = &sc->map[sc->cy * BOARD_W + sc->cx];
    if (*t == T_MIR_S)      *t = T_MIR_B;
    else if (*t == T_MIR_B) *t = T_MIR_S;
    else return 0;
    sc->moves++;
    return cell_dirty(sc->cy) | panel_dirty();
}

void scene_render(int y0, int h, uint16_t *buf, void *ctx)
{
    const scene *sc = ctx;
    rd_strip s = { buf, y0, h };
    char line[12];

    /* 一定要先鋪底：實機的 strip buffer 是重複使用的，沒畫到的地方會留上一條的殘影 */
    rd_fill(&s, RD_BLACK);

    rd_tilemap tm = { art_tiles, sc->map, TILE, TILE, BOARD_W, BOARD_H, 0, 0 };
    rd_draw_tilemap(&s, &tm);

    rd_sprite cur = { art_cursor, (int16_t)(sc->cx * TILE), (int16_t)(sc->cy * TILE),
                      TILE, TILE, 1, KEY_MAGENTA };
    rd_draw_sprites(&s, &cur, 1);

    /* 右側資訊欄 64 px：每行 8 字 */
    rd_rect(&s, PANEL_X, 0, RD_GFX_W - PANEL_X, RD_GFX_H, C_PANEL);
    rd_text(&s, PANEL_X + 12, 8,  "LIGHT", C_TITLE, RD_NOKEY, 1);
    rd_text(&s, PANEL_X + 16, 18, "PATH",  C_TITLE, RD_NOKEY, 1);
    snprintf(line, sizeof line, "LV %3d", sc->level);
    rd_text(&s, PANEL_X + 4, 40, line, C_VALUE, RD_NOKEY, 1);
    snprintf(line, sizeof line, "MV %3d", sc->moves);
    rd_text(&s, PANEL_X + 4, 56, line, C_VALUE, RD_NOKEY, 1);
    rd_text(&s, PANEL_X + 4, 208, "A:TURN", C_LABEL, RD_NOKEY, 1);
    rd_text(&s, PANEL_X + 4, 220, "<>:MOVE", C_LABEL, RD_NOKEY, 1);
}
