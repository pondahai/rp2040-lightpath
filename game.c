#include <string.h>
#include "game.h"
#include "art.h"

static const int DX[4] = { 1, 0, -1, 0 };
static const int DY[4] = { 0, -1, 0, 1 };

static int is_slash(uint8_t t)  { return t == T_MIR_S || t == T_FMIR_S; }
static int is_back(uint8_t t)   { return t == T_MIR_B || t == T_FMIR_B; }
static int is_emit(uint8_t t)   { return t >= T_EMIT_E && t <= T_EMIT_S; }
static int is_movable(uint8_t t){ return t == T_MIR_S || t == T_MIR_B; }

/* /：東<->北、西<->南    \：東<->南、西<->北 */
static int reflect(uint8_t t, int d)
{
    static const int SLASH[4] = { DIR_N, DIR_E, DIR_S, DIR_W };
    static const int BACK[4]  = { DIR_S, DIR_W, DIR_N, DIR_E };
    if (is_slash(t)) return SLASH[d];
    if (is_back(t))  return BACK[d];
    return d;
}

void game_trace(game *g)
{
    /* visited[i] 每個方向一個 bit：同一格同一方向第二次經過 = 進入迴圈 */
    uint8_t visited[BOARD_N];
    memset(g->beam, 0, sizeof g->beam);
    memset(g->lit, 0, sizeof g->lit);
    memset(visited, 0, sizeof visited);
    g->lit_count = 0;

    for (int i = 0; i < BOARD_N; i++) {
        if (!is_emit(g->cell[i])) continue;
        int d = g->cell[i] - T_EMIT_E;
        int x = i % BOARD_W, y = i / BOARD_W;
        g->beam[i] |= EDGE(d);

        for (;;) {
            x += DX[d];
            y += DY[d];
            if (x < 0 || x >= BOARD_W || y < 0 || y >= BOARD_H) break;
            int j = y * BOARD_W + x;
            uint8_t t = g->cell[j];
            if (t == T_WALL || is_emit(t)) break;

            g->beam[j] |= EDGE((d + 2) & 3);            /* 從反方向的邊進來 */
            if (t == T_RECV) {
                if (!g->lit[j]) { g->lit[j] = 1; g->lit_count++; }
                break;
            }
            d = reflect(t, d);
            if (visited[j] & (1u << d)) break;
            visited[j] |= (uint8_t)(1u << d);
            g->beam[j] |= EDGE(d);                       /* 從這個邊出去 */
        }
    }
}

void game_load_def(game *g, const level_def *def)
{
    memset(g, 0, sizeof *g);
    for (int y = 0; y < BOARD_H; y++)
        for (int x = 0; x < BOARD_W; x++) {
            uint8_t t = T_FLOOR;
            switch (def->rows[y][x]) {
            case '#':  t = T_WALL;   break;
            case '>':  t = T_EMIT_E; break;
            case '^':  t = T_EMIT_N; break;
            case '<':  t = T_EMIT_W; break;
            case 'v':  t = T_EMIT_S; break;
            case 'R':  t = T_RECV;   g->receivers++; break;
            case '/':  t = T_FMIR_S; break;
            case '\\': t = T_FMIR_B; break;
            case 'a':  t = T_MIR_S;  break;
            case 'b':  t = T_MIR_B;  break;
            }
            g->cell[y * BOARD_W + x] = t;
        }
    g->hand = def->hand;
    g->cx = BOARD_W / 2;
    g->cy = BOARD_H / 2;
    game_trace(g);
}

void game_load(game *g, int level)
{
    game_load_def(g, &LEVELS[level]);
    g->level = level;
}

int game_move(game *g, int dx, int dy)
{
    int nx = g->cx + dx, ny = g->cy + dy;
    /* 游標只在內圈走（外圈全是牆） */
    if (nx < 1 || nx >= BOARD_W - 1 || ny < 1 || ny >= BOARD_H - 1) return 0;
    g->cx = nx;
    g->cy = ny;
    return 1;
}

int game_rotate(game *g)
{
    uint8_t *t = &g->cell[g->cy * BOARD_W + g->cx];
    if (*t == T_MIR_S)      *t = T_MIR_B;
    else if (*t == T_MIR_B) *t = T_MIR_S;
    else return 0;
    g->moves++;
    game_trace(g);
    return 1;
}

int game_take_place(game *g)
{
    uint8_t *t = &g->cell[g->cy * BOARD_W + g->cx];
    if (is_movable(*t)) {
        *t = T_FLOOR;
        g->hand++;
    } else if (*t == T_FLOOR && g->hand > 0) {
        *t = T_MIR_S;
        g->hand--;
    } else {
        return 0;
    }
    g->moves++;
    game_trace(g);
    return 1;
}

int game_cleared(const game *g)
{
    return g->receivers > 0 && g->lit_count == g->receivers;
}
