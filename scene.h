/* 畫面狀態與 render。純 C —— 實機 main.c 和 PC 預覽共用同一份。 */
#ifndef LP_SCENE_H
#define LP_SCENE_H

#include <stdint.h>
#include "gfx.h"

#define BOARD_W  16
#define BOARD_H  12
#define PANEL_X  (BOARD_W * 16)     /* 右側資訊欄起點 = 256 */

typedef struct {
    uint8_t map[BOARD_W * BOARD_H];
    int     cx, cy;                 /* 游標格 */
    int     level, moves;
} scene;

void scene_demo(scene *sc);

/* 移動游標／旋轉鏡子，回傳哪些條需要重畫 */
rd_dirty scene_move(scene *sc, int dx, int dy);
rd_dirty scene_rotate(scene *sc);

/* rd_render_fn 形式：ctx = scene* */
void scene_render(int y0, int h, uint16_t *buf, void *ctx);

#endif
