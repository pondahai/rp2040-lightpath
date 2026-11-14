/* PC 預覽：用跟實機同一份 scene_render()，一條一條畫成 320×240，存 PNG。
 *
 *   build：examples/lightpath/pc/build.sh
 *   用法：  preview out.png [按鍵序列]
 *           按鍵：L R U D A，例如 "RRDA" = 右右下、旋轉
 *
 * 只模擬渲染與遊戲邏輯，不模擬 SPI/DMA；條是一條一條畫的，
 * 所以條與條之間的裁切錯誤在這裡一樣看得到。
 */
#include <stdio.h>
#include "gfx.h"
#include "art.h"
#include "scene.h"
#include "png.h"

static uint16_t screen[RD_GFX_W * RD_GFX_H];

int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : "preview.png";
    const char *keys = argc > 2 ? argv[2] : "";
    scene sc;

    art_init();
    scene_demo(&sc);
    for (const char *k = keys; *k; k++)
        switch (*k) {
        case 'L': scene_move(&sc, -1, 0); break;
        case 'R': scene_move(&sc, 1, 0);  break;
        case 'U': scene_move(&sc, 0, -1); break;
        case 'D': scene_move(&sc, 0, 1);  break;
        case 'A': scene_rotate(&sc);      break;
        }

    /* 先填洋紅：render 沒畫到的像素在實機會是殘影，這裡要一眼看得出來 */
    for (int i = 0; i < RD_GFX_W * RD_GFX_H; i++) screen[i] = 0xF81F;

    for (int y = 0; y < RD_GFX_H; y += RD_STRIP_H)
        scene_render(y, RD_STRIP_H, screen + y * RD_GFX_W, &sc);

    if (rd_png_write_rgb565(out, screen, RD_GFX_W, RD_GFX_H)) {
        fprintf(stderr, "寫不出 %s\n", out);
        return 1;
    }
    printf("%s\n", out);
    return 0;
}
