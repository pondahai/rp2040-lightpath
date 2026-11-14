/* 光路反射 —— G2：可以玩的 5 關。
 *
 *   方向鍵  移動游標        A      旋轉鏡子；過關後進下一關
 *   B       拿起／放下鏡子  START  重來這一關；全破後從第 1 關開始
 *
 * 遊戲邏輯在 game.c、畫面在 scene.c，PC 預覽與測試用的是同一份。
 */
#include "pico/stdlib.h"
#include "display.h"
#include "dpad.h"
#include "buttons.h"
#include "gfx.h"
#include "art.h"
#include "game.h"
#include "scene.h"

static scene sc;

int main(void)
{
    rd_display_init();
    rd_dpad_init();
    art_init();
    game_load(&sc.g, 0);

    rd_buttons btn;
    rd_buttons_init(&btn, RD_BTN_DPAD);

    rd_dirty dirty = RD_DIRTY_ALL;

    for (;;) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        uint8_t fired = rd_buttons_update(&btn, rd_dpad_raw(), now);
        game *g = &sc.g;
        int oy = g->cy;

        if (fired & RD_BTN_START) {
            game_load(g, sc.all_clear ? 0 : g->level);
            sc.all_clear = 0;
            dirty = RD_DIRTY_ALL;
        } else if (sc.all_clear) {
            /* 全破畫面只等 START */
        } else if (game_cleared(g)) {
            if (fired & RD_BTN_A) {
                if (g->level + 1 < LEVEL_COUNT) game_load(g, g->level + 1);
                else sc.all_clear = 1;
                dirty = RD_DIRTY_ALL;
            }
        } else {
            int moved = 0;
            if (fired & RD_BTN_LEFT)  moved |= game_move(g, -1, 0);
            if (fired & RD_BTN_RIGHT) moved |= game_move(g, 1, 0);
            if (fired & RD_BTN_UP)    moved |= game_move(g, 0, -1);
            if (fired & RD_BTN_DOWN)  moved |= game_move(g, 0, 1);
            if (moved)
                dirty |= scene_dirty_cell(oy) | scene_dirty_cell(g->cy);
            /* 光路可能整個改變：重畫全部（約 40 ms，益智遊戲夠用） */
            if ((fired & RD_BTN_A) && game_rotate(g))      dirty = RD_DIRTY_ALL;
            if ((fired & RD_BTN_B) && game_take_place(g))  dirty = RD_DIRTY_ALL;
        }

        if (dirty) {
            rd_display_present(dirty, scene_render, &sc);
            dirty = 0;
        } else {
            sleep_ms(5);
        }
    }
}
