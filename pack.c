#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pack.h"

/* 逐行走：回傳下一行的起點，*n 是這行長度（不含 \r\n） */
static const char *next_line(const char *p, const char *end, const char **line, size_t *n)
{
    *line = p;
    while (p < end && *p != '\n') p++;
    *n = (size_t)(p - *line);
    if (*n && (*line)[*n - 1] == '\r') (*n)--;
    return p < end ? p + 1 : p;
}

static int board_char(char c)
{
    return c && strchr("#.>^<vR/\\ab", c) != NULL;
}

/* 收完一關：檢查並登錄 */
static void finish(level_pack *p, int rows, int hand, int *open)
{
    if (!*open) return;
    *open = 0;
    if (p->count >= PACK_MAX) { p->bad++; return; }
    int emit = 0, recv = 0;
    if (rows == BOARD_H)
        for (int y = 0; y < BOARD_H; y++)
            for (int x = 0; x < BOARD_W; x++) {
                char c = p->rows[p->count][y][x];
                if (strchr(">^<v", c)) emit++;
                if (c == 'R') recv++;
            }
    if (rows != BOARD_H || !emit || !recv) { p->bad++; return; }

    level_def *d = &p->defs[p->count];
    for (int y = 0; y < BOARD_H; y++) d->rows[y] = p->rows[p->count][y];
    d->hand = hand;
    p->count++;
}

void pack_parse(level_pack *p, const char *txt, size_t len)
{
    const char *s = txt, *end = txt + len, *line;
    size_t n;
    int rows = 0, hand = 0, open = 0, broken = 0;

    p->count = p->bad = 0;
    while (s < end) {
        s = next_line(s, end, &line, &n);
        if (n && line[0] == ';') continue;
        if (n == 0) {                                   /* 空行：關卡結束 */
            if (broken) { p->bad++; broken = open = 0; }
            finish(p, rows, hand, &open);
            rows = hand = 0;
            continue;
        }
        if (n > 5 && !strncmp(line, "hand ", 5)) {
            hand = atoi(line + 5);
            if (hand < 0 || hand > 99) broken = 1;
            open = 1;
            continue;
        }
        open = 1;
        int ok = n == BOARD_W && rows < BOARD_H && p->count < PACK_MAX;
        for (size_t i = 0; ok && i < n; i++) ok = board_char(line[i]);
        if (!ok) { broken = 1; continue; }
        memcpy(p->rows[p->count][rows], line, BOARD_W);
        p->rows[p->count][rows][BOARD_W] = 0;
        rows++;
    }
    if (broken) p->bad++;
    else finish(p, rows, hand, &open);
}

size_t pack_format(const level_def *defs, int n, char *buf, size_t cap)
{
    size_t at = 0;
    int w = snprintf(buf, cap, "; lightpath 關卡檔，格式見 examples/lightpath/pack.h\n\n");
    if (w < 0 || (size_t)w >= cap) return 0;
    at = (size_t)w;
    for (int i = 0; i < n; i++) {
        w = snprintf(buf + at, cap - at, "; level %d\n", i + 1);
        if (w < 0 || (size_t)w >= cap - at) return 0;
        at += (size_t)w;
        for (int y = 0; y < BOARD_H; y++) {
            w = snprintf(buf + at, cap - at, "%s\n", defs[i].rows[y]);
            if (w < 0 || (size_t)w >= cap - at) return 0;
            at += (size_t)w;
        }
        w = snprintf(buf + at, cap - at, "hand %d\n\n", defs[i].hand);
        if (w < 0 || (size_t)w >= cap - at) return 0;
        at += (size_t)w;
    }
    return at;
}

void save_parse(save_data *s, const char *txt, size_t len)
{
    const char *p = txt, *end = txt + len, *line;
    size_t n;
    char tmp[40];

    memset(s, 0, sizeof *s);
    while (p < end) {
        p = next_line(p, end, &line, &n);
        if (n >= sizeof tmp) continue;
        memcpy(tmp, line, n);
        tmp[n] = 0;
        int a, b;
        if (sscanf(tmp, "level %d", &a) == 1 && a >= 1) s->level = a - 1;
        else if (sscanf(tmp, "best %d %d", &a, &b) == 2 &&
                 a >= 1 && a <= PACK_MAX && b > 0 && b < 65536)
            s->best[a - 1] = (uint16_t)b;
    }
}

size_t save_format(const save_data *s, int nlevels, char *buf, size_t cap)
{
    int w = snprintf(buf, cap, "level %d\n", s->level + 1);
    if (w < 0 || (size_t)w >= cap) return 0;
    size_t at = (size_t)w;
    for (int i = 0; i < nlevels && i < PACK_MAX; i++) {
        if (!s->best[i]) continue;
        w = snprintf(buf + at, cap - at, "best %d %d\n", i + 1, s->best[i]);
        if (w < 0 || (size_t)w >= cap - at) return 0;
        at += (size_t)w;
    }
    return at;
}
