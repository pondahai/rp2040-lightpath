/* 光路反射的圖素。開機時用程式畫進 RAM（G1 階段先不做圖檔轉換工具）。 */
#ifndef LP_ART_H
#define LP_ART_H

#include <stdint.h>

#define TILE 16

enum {
    T_FLOOR, T_WALL, T_EMIT, T_RECV, T_MIR_S /* / */, T_MIR_B /* \ */,
    T_COUNT
};

#define KEY_MAGENTA 0xF81F          /* sprite 透明色 */

extern uint16_t art_tiles[T_COUNT * TILE * TILE];
extern uint16_t art_cursor[TILE * TILE];

void art_init(void);

#endif
