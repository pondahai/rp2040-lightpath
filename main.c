/* 光路反射 —— G3：音效、SD 卡關卡檔與存檔。
 *
 *   方向鍵  移動游標        A      旋轉鏡子；過關後進下一關
 *   B       拿起／放下鏡子  START  重來這一關；全破後從第 1 關開始
 *
 * SD 卡根目錄（格式見 pack.h）：
 *   lightpath.lvl  有就取代內建關卡
 *   lightpath.sav  目前關卡與每關最少步數；過關時寫入
 * 沒插卡也能玩，只是不存檔。
 *
 * 遊戲邏輯在 game.c、畫面在 scene.c，PC 預覽與測試用的是同一份。
 */
#include <string.h>
#include "pico/stdlib.h"
#include "display.h"
#include "dpad.h"
#include "audio.h"
#include "sd.h"
#include "buttons.h"
#include "gfx.h"
#include "art.h"
#include "game.h"
#include "pack.h"
#include "scene.h"
#include "sfx.h"

#define LEVEL_FILE "lightpath.lvl"
#define SAVE_FILE  "lightpath.sav"

static scene      sc;
static level_pack pack;
static save_data  save;
static char       filebuf[8192];
static int        sd_ok;

static void set_msg(const char *m)
{
    strncpy(sc.msg, m, sizeof sc.msg - 1);
}

static void load_sd(void)
{
    sd_ok = rd_sd_init() == RD_SD_OK;
    if (!sd_ok) { set_msg("NO SD"); return; }

    int n = rd_sd_load(LEVEL_FILE, filebuf, sizeof filebuf);
    if (n > 0) {
        pack_parse(&pack, filebuf, (size_t)n);
        if (pack.count > 0) {
            game_set_levels(pack.defs, pack.count);
            set_msg(pack.bad ? "LVL ERR" : "SD LVL");
        } else {
            set_msg("LVL ERR");     /* 整份都不能用：退回內建關卡 */
        }
    }

    n = rd_sd_load(SAVE_FILE, filebuf, sizeof filebuf);
    if (n > 0) save_parse(&save, filebuf, (size_t)n);
    if (save.level >= game_level_count()) save.level = 0;
}

static void write_save(void)
{
    if (!sd_ok) return;
    size_t n = save_format(&save, game_level_count(), filebuf, sizeof filebuf);
    if (n && rd_sd_save(SAVE_FILE, filebuf, n) != RD_SD_OK)
        set_msg("SAVE ERR");
}

/* 剛過關：記錄步數，存檔指到下一關 */
static void on_clear(game *g)
{
    uint16_t *b = &save.best[g->level];
    if (g->level < PACK_MAX && (*b == 0 || g->moves < *b)) *b = (uint16_t)g->moves;
    int last = g->level + 1 >= game_level_count();
    save.level = last ? 0 : g->level + 1;
    write_save();
    rd_audio_play(last ? SFX_ALL : SFX_CLEAR);
}

int main(void)
{
    rd_display_init();
    rd_dpad_init();
    rd_audio_init();
    art_init();
    load_sd();
    sc.best = save.best;
    game_load(&sc.g, save.level);

    rd_buttons btn;
    rd_buttons_init(&btn, RD_BTN_DPAD);

    rd_dirty dirty = RD_DIRTY_ALL;

    for (;;) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        uint8_t fired = rd_buttons_update(&btn, rd_dpad_raw(), now);
        game *g = &sc.g;
        int oy = g->cy, lit0 = g->lit_count, was_clear = game_cleared(g);

        if (fired && sc.msg[0]) {           /* 開機提示：按任何鍵就收掉 */
            sc.msg[0] = 0;
            rd_dirty_mark(&dirty, 176, 8);
        }

        if (fired & RD_BTN_START) {
            game_load(g, sc.all_clear ? 0 : g->level);
            sc.all_clear = 0;
            dirty = RD_DIRTY_ALL;
        } else if (sc.all_clear) {
            /* 全破畫面只等 START */
        } else if (was_clear) {
            if (fired & RD_BTN_A) {
                if (g->level + 1 < game_level_count()) game_load(g, g->level + 1);
                else sc.all_clear = 1;
                dirty = RD_DIRTY_ALL;
            }
        } else {
            int moved = 0;
            if (fired & RD_BTN_LEFT)  moved |= game_move(g, -1, 0);
            if (fired & RD_BTN_RIGHT) moved |= game_move(g, 1, 0);
            if (fired & RD_BTN_UP)    moved |= game_move(g, 0, -1);
            if (fired & RD_BTN_DOWN)  moved |= game_move(g, 0, 1);
            if (moved) {
                dirty |= scene_dirty_cell(oy) | scene_dirty_cell(g->cy);
                rd_audio_play(SFX_MOVE);
            }
            /* 光路可能整個改變：重畫全部（約 40 ms，益智遊戲夠用） */
            if (fired & RD_BTN_A) {
                if (game_rotate(g)) { dirty = RD_DIRTY_ALL; rd_audio_play(SFX_ROTATE); }
                else rd_audio_play(SFX_DENY);
            }
            if (fired & RD_BTN_B) {
                int h0 = g->hand;
                if (game_take_place(g)) {
                    dirty = RD_DIRTY_ALL;
                    rd_audio_play(g->hand > h0 ? SFX_TAKE : SFX_PLACE);
                } else {
                    rd_audio_play(SFX_DENY);
                }
            }
            if (game_cleared(g))          on_clear(g);
            else if (g->lit_count > lit0) rd_audio_play(SFX_LIT);
        }

        if (dirty) {
            rd_display_present(dirty, scene_render, &sc);
            dirty = 0;
        } else {
            sleep_ms(5);
        }
    }
}
