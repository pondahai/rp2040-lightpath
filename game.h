/* 光路反射的遊戲邏輯。純 C，不碰硬體也不碰繪圖 —— PC 測試直接用。
 *
 * 盤面格子用 art.h 的 tile 編號（T_FLOOR、T_WALL …）。
 *
 * 規則：
 *   發射器朝一個方向射光；光遇到鏡子轉 90°，遇到牆、發射器、接收器、盤面邊緣就停。
 *   照到的接收器亮起，全部亮起即過關。
 *   可動的鏡子能旋轉（A）、拿起（B）；手上有鏡子時在空地按 B 放下一面 /。
 *   固定的鏡子（有底座）不能動。
 */
#ifndef LP_GAME_H
#define LP_GAME_H

#include <stdint.h>

#define BOARD_W  16
#define BOARD_H  12
#define BOARD_N  (BOARD_W * BOARD_H)

enum { DIR_E, DIR_N, DIR_W, DIR_S };

/* beam[] 每格的位元：光從格子中心連到哪幾個邊（1 << DIR_*） */
#define EDGE(d) (1u << (d))

typedef struct {
    uint8_t cell[BOARD_N];
    uint8_t beam[BOARD_N];
    uint8_t lit[BOARD_N];       /* 接收器是否被照到 */
    int     level;              /* 0 起算 */
    int     hand;               /* 手上的鏡子數 */
    int     moves;
    int     receivers, lit_count;
    int     cx, cy;             /* 游標 */
} game;

typedef struct {
    const char *rows[BOARD_H];
    int hand;
} level_def;

/* 字元：# 牆  . 地板  > ^ < v 發射器  R 接收器
 *       / \ 固定鏡  a = 可動 /  b = 可動 \ */
extern const level_def LEVELS[];
extern const int LEVEL_COUNT;

void game_load(game *g, int level);
void game_load_def(game *g, const level_def *def);   /* 測試用 */

/* 重算光路（載入與每次動作後會自動呼叫） */
void game_trace(game *g);

int  game_move(game *g, int dx, int dy);   /* 回傳 1 = 有移動 */
int  game_rotate(game *g);                 /* 回傳 1 = 盤面有變 */
int  game_take_place(game *g);
int  game_cleared(const game *g);

#endif
