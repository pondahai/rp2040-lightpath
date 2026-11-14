#include <stdio.h>
#include "scene.h"
#include "art.h"

#define C_PANEL   RD_RGB565(12, 14, 26)
#define C_TITLE   RD_RGB565(255, 220, 40)
#define C_LABEL   RD_RGB565(150, 160, 190)
#define C_VALUE   RD_WHITE
#define C_CLEAR   RD_RGB565(140, 255, 120)
#define C_BEAM    RD_RGB565(255, 60, 40)
#define C_BEAM_HI RD_RGB565(255, 220, 200)

rd_dirty scene_dirty_cell(int cy)
{
    rd_dirty d = 0;
    rd_dirty_mark(&d, cy * TILE, TILE);
    return d;
}

rd_dirty scene_dirty_board(void)
{
    return RD_DIRTY_ALL;
}

/* 一格裡的光：從中心往每個有標記的邊畫一段。外側 4px 紅、中間 2px 亮芯 */
static void draw_beam(const rd_strip *s, int x0, int y0, uint8_t edges)
{
    static const int DX[4] = { 1, 0, -1, 0 };
    static const int DY[4] = { 0, -1, 0, 1 };
    for (int d = 0; d < 4; d++) {
        if (!(edges & EDGE(d))) continue;
        for (int pass = 0; pass < 2; pass++) {
            int w = pass ? 2 : 4, o = pass ? 7 : 6;
            uint16_t c = pass ? C_BEAM_HI : C_BEAM;
            int x, y, bw, bh;
            if (DX[d]) {            /* 水平段：中心到左或右邊 */
                x  = DX[d] > 0 ? x0 + o : x0;
                bw = DX[d] > 0 ? TILE - o : o + w;
                y  = y0 + o;
                bh = w;
            } else {                /* 垂直段 */
                bw = w;
                y = DY[d] > 0 ? y0 + o : y0;
                bh = DY[d] > 0 ? TILE - o : o + w;
                x = x0 + o;
            }
            rd_rect(s, x, y, bw, bh, c);
        }
    }
}

void scene_render(int y0, int h, uint16_t *buf, void *ctx)
{
    const scene *sc = ctx;
    const game *g = &sc->g;
    rd_strip s = { buf, y0, h };
    char line[32];

    /* 一定要先鋪底：實機的 strip buffer 是重複使用的，沒畫到的地方會留上一條的殘影 */
    rd_fill(&s, RD_BLACK);

    /* 只在跟這條重疊的盤面列上畫 */
    uint8_t rowmap[BOARD_W];
    for (int cy = 0; cy < BOARD_H; cy++) {
        int py = cy * TILE;
        if (py + TILE <= y0 || py >= y0 + h) continue;
        for (int cx = 0; cx < BOARD_W; cx++) {
            int i = cy * BOARD_W + cx;
            rowmap[cx] = (g->cell[i] == T_RECV && g->lit[i]) ? T_RECV_ON : g->cell[i];
        }
        rd_tilemap tm = { art_tiles, rowmap, TILE, TILE, BOARD_W, 1, 0, (int16_t)py };
        rd_draw_tilemap(&s, &tm);
        for (int cx = 0; cx < BOARD_W; cx++) {
            uint8_t e = g->beam[cy * BOARD_W + cx];
            if (e) draw_beam(&s, cx * TILE, py, e);
        }
    }

    rd_sprite cur = { art_cursor, (int16_t)(g->cx * TILE), (int16_t)(g->cy * TILE),
                      TILE, TILE, 1, KEY_MAGENTA };
    rd_draw_sprites(&s, &cur, 1);

    /* 右側資訊欄 64 px：每行 8 字 */
    rd_rect(&s, PANEL_X, 0, RD_GFX_W - PANEL_X, RD_GFX_H, C_PANEL);
    rd_text(&s, PANEL_X + 12, 8,  "LIGHT", C_TITLE, RD_NOKEY, 1);
    rd_text(&s, PANEL_X + 16, 18, "PATH",  C_TITLE, RD_NOKEY, 1);
    snprintf(line, sizeof line, "LV %d/%d", g->level + 1, game_level_count());
    rd_text(&s, PANEL_X + 4, 40, line, C_VALUE, RD_NOKEY, 1);
    snprintf(line, sizeof line, "MV %3d", g->moves);
    rd_text(&s, PANEL_X + 4, 56, line, C_VALUE, RD_NOKEY, 1);
    snprintf(line, sizeof line, "HAND %d", g->hand);
    rd_text(&s, PANEL_X + 4, 72, line, C_VALUE, RD_NOKEY, 1);
    snprintf(line, sizeof line, "LIT %d/%d", g->lit_count, g->receivers);
    rd_text(&s, PANEL_X + 4, 88, line, g->lit_count ? C_CLEAR : C_LABEL, RD_NOKEY, 1);

    if (sc->best && sc->best[g->level]) {
        snprintf(line, sizeof line, "BEST %d", sc->best[g->level]);
        rd_text(&s, PANEL_X + 4, 104, line, C_LABEL, RD_NOKEY, 1);
    }
    if (sc->msg[0])
        rd_text(&s, PANEL_X, 176, sc->msg, C_TITLE, RD_NOKEY, 1);

    if (sc->all_clear) {
        rd_text(&s, PANEL_X + 4, 120, "ALL", C_CLEAR, RD_NOKEY, 2);
        rd_text(&s, PANEL_X + 0, 140, "CLEAR", C_CLEAR, RD_NOKEY, 1);
        rd_text(&s, PANEL_X, 156, "ST:AGAIN", C_LABEL, RD_NOKEY, 1);
    } else if (game_cleared(g)) {
        rd_text(&s, PANEL_X + 4, 120, "CLEAR!", C_CLEAR, RD_NOKEY, 1);
        rd_text(&s, PANEL_X + 4, 136, "A:NEXT", C_LABEL, RD_NOKEY, 1);
    }

    rd_text(&s, PANEL_X + 4, 196, "A:TURN", C_LABEL, RD_NOKEY, 1);
    rd_text(&s, PANEL_X + 4, 208, "B:TAKE", C_LABEL, RD_NOKEY, 1);
    rd_text(&s, PANEL_X, 220, "ST:RETRY", C_LABEL, RD_NOKEY, 1);
}
