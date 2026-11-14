/* PC 預覽：用跟實機同一份 scene_render()，一條一條畫成 320×240，存 PNG。
 *
 *   build：sh examples/lightpath/pc/build.sh（順便跑 test_game）
 *   用法：  preview out.png [關卡 1..N] [按鍵序列]
 *           按鍵：L R U D 移動、A 旋轉、B 拿放、S 重來、N 下一關（過關後）
 *           例：preview out.png 3 "LLLLLLUUUB"
 *         preview --dump-levels lightpath.lvl   把內建關卡寫成關卡檔
 *
 * 只模擬渲染與遊戲邏輯，不模擬 SPI/DMA。
 */
#include <stdio.h>
#include <stdlib.h>
#include "gfx.h"
#include "art.h"
#include "game.h"
#include "scene.h"
#include "png.h"
#include "pack.h"
#include <string.h>

static uint16_t screen[RD_GFX_W * RD_GFX_H];
static scene sc;

static int dump_levels(const char *path)
{
    static char buf[8192];
    size_t n = pack_format(LEVELS, LEVEL_COUNT, buf, sizeof buf);
    FILE *f = fopen(path, "wb");
    if (!n || !f || fwrite(buf, 1, n, f) != n) { fprintf(stderr, "寫不出 %s\n", path); return 1; }
    fclose(f);
    printf("%s  %d 關\n", path, LEVEL_COUNT);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc > 2 && !strcmp(argv[1], "--dump-levels")) return dump_levels(argv[2]);

    const char *out = argc > 1 ? argv[1] : "preview.png";
    int level = argc > 2 ? atoi(argv[2]) - 1 : 0;
    const char *keys = argc > 3 ? argv[3] : "";
    if (level < 0 || level >= LEVEL_COUNT) level = 0;

    art_init();
    game_load(&sc.g, level);
    game *g = &sc.g;
    for (const char *k = keys; *k; k++)
        switch (*k) {
        case 'L': game_move(g, -1, 0); break;
        case 'R': game_move(g, 1, 0);  break;
        case 'U': game_move(g, 0, -1); break;
        case 'D': game_move(g, 0, 1);  break;
        case 'A': game_rotate(g);      break;
        case 'B': game_take_place(g);  break;
        case 'S': game_load(g, g->level); break;
        case 'N':
            if (game_cleared(g)) {
                if (g->level + 1 < LEVEL_COUNT) game_load(g, g->level + 1);
                else sc.all_clear = 1;
            }
            break;
        }

    /* 先填洋紅：render 沒畫到的像素在實機會是殘影，這裡要一眼看得出來 */
    for (int i = 0; i < RD_GFX_W * RD_GFX_H; i++) screen[i] = 0xF81F;
    for (int y = 0; y < RD_GFX_H; y += RD_STRIP_H)
        scene_render(y, RD_STRIP_H, screen + y * RD_GFX_W, &sc);

    if (rd_png_write_rgb565(out, screen, RD_GFX_W, RD_GFX_H)) {
        fprintf(stderr, "寫不出 %s\n", out);
        return 1;
    }
    printf("%s  LV %d  lit %d/%d%s\n", out, g->level + 1, g->lit_count, g->receivers,
           game_cleared(g) ? "  CLEAR" : "");
    return 0;
}
