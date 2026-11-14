/* 把 game 畫出來。純 C —— 實機 main.c 和 PC 預覽共用同一份。 */
#ifndef LP_SCENE_H
#define LP_SCENE_H

#include <stdint.h>
#include "gfx.h"
#include "game.h"

#define PANEL_X  (BOARD_W * 16)     /* 右側資訊欄起點 = 256 */

/* 盤面以外的狀態 */
typedef struct {
    game g;
    int  all_clear;                 /* 最後一關也過了 */
} scene;

/* 盤面列 y 的格子所在的條 */
rd_dirty scene_dirty_cell(int cy);
rd_dirty scene_dirty_board(void);   /* 盤面 + 資訊欄全部 */

/* rd_render_fn 形式：ctx = scene* */
void scene_render(int y0, int h, uint16_t *buf, void *ctx);

#endif
