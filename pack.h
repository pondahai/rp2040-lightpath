/* 關卡檔與存檔的文字格式。純 C，PC 測試直接用；實際讀寫 SD 在 main.c。
 *
 * 關卡檔 lightpath.lvl（SD 卡根目錄；有就取代內建關卡）：
 *
 *     ; 分號開頭是註解
 *     ################      <- 12 行、每行 16 字，字元見 game.h
 *     #..............#
 *     ...
 *     ################
 *     hand 1                <- 可省略，預設 0
 *                           <- 空行分隔關卡
 *
 *   不合格的關卡（行數不對、沒有發射器或接收器、有未知字元）會被跳過並計數。
 *
 * 存檔 lightpath.sav：
 *
 *     level 3               <- 下次開機從第幾關開始（1 起算）
 *     best 1 2              <- 第 1 關最少步數 2
 */
#ifndef LP_PACK_H
#define LP_PACK_H

#include <stddef.h>
#include <stdint.h>
#include "game.h"

#define PACK_MAX 32

typedef struct {
    char      rows[PACK_MAX][BOARD_H][BOARD_W + 1];
    level_def defs[PACK_MAX];
    int       count;
    int       bad;              /* 被跳過的關卡數 */
} level_pack;

void pack_parse(level_pack *p, const char *txt, size_t len);

/* 把一組關卡寫成文字，回傳長度（cap 不夠時截斷，回傳 0） */
size_t pack_format(const level_def *defs, int n, char *buf, size_t cap);

typedef struct {
    int      level;             /* 0 起算 */
    uint16_t best[PACK_MAX];    /* 0 = 還沒過 */
} save_data;

void   save_parse(save_data *s, const char *txt, size_t len);
size_t save_format(const save_data *s, int nlevels, char *buf, size_t cap);

#endif
