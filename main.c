/* 光路反射 —— G1：tilemap + sprite + 文字 + dirty strip。
 *
 * 方向鍵移動游標，A 旋轉游標下的鏡子。只重送有變動的條。
 * 畫面內容由 scene.c 決定，PC 預覽（pc/preview.c）用的是同一份。
 */
#include "pico/stdlib.h"
#include "display.h"
#include "dpad.h"
#include "buttons.h"
#include "gfx.h"
#include "art.h"
#include "scene.h"

static scene sc;

int main(void)
{
    rd_display_init();
    rd_dpad_init();
    art_init();
    scene_demo(&sc);

    rd_buttons btn;
    rd_buttons_init(&btn, RD_BTN_DPAD);

    rd_dirty dirty = RD_DIRTY_ALL;

    for (;;) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        uint8_t fired = rd_buttons_update(&btn, rd_dpad_raw(), now);

        if (fired & RD_BTN_LEFT)  dirty |= scene_move(&sc, -1, 0);
        if (fired & RD_BTN_RIGHT) dirty |= scene_move(&sc, 1, 0);
        if (fired & RD_BTN_UP)    dirty |= scene_move(&sc, 0, -1);
        if (fired & RD_BTN_DOWN)  dirty |= scene_move(&sc, 0, 1);
        if (fired & RD_BTN_A)     dirty |= scene_rotate(&sc);

        if (dirty) {
            rd_display_present(dirty, scene_render, &sc);
            dirty = 0;
        } else {
            sleep_ms(5);
        }
    }
}
