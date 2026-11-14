/* 光路反射的圖素。開機時用程式畫進 RAM（還沒有圖檔轉換工具）。
 * tile 編號同時就是盤面格子的種類（game.h），render 只多換一個 T_RECV_ON。 */
#ifndef LP_ART_H
#define LP_ART_H

#include <stdint.h>

#define TILE 16

enum {
    T_FLOOR, T_WALL,
    T_EMIT_E, T_EMIT_N, T_EMIT_W, T_EMIT_S,     /* 順序 = 方向 DIR_E..DIR_S */
    T_RECV, T_RECV_ON,
    T_MIR_S, T_MIR_B,                           /* 可動的 /  \ */
    T_FMIR_S, T_FMIR_B,                         /* 固定的 /  \（有底座） */
    T_COUNT
};

#define KEY_MAGENTA 0xF81F          /* sprite 透明色 */

extern uint16_t art_tiles[T_COUNT * TILE * TILE];
extern uint16_t art_cursor[TILE * TILE];

void art_init(void);

#endif
