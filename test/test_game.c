/* 遊戲邏輯的 PC 測試：sh examples/lightpath/pc/build.sh 會一起編並執行。 */
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "art.h"
#include "pack.h"

static int fails;
#define CHECK(cond, ...) do { if (!(cond)) { fails++; \
    printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* 每關的解法：可動鏡子最後的位置與種類（a = /，b = \）。 */
typedef struct { int x, y; char kind; } place;
static const place SOLUTIONS[][4] = {
    { { 8, 5, 'b' } },
    { { 10, 2, 'b' }, { 10, 8, 'a' } },
    { { 2, 3, 'a' } },
    { { 6, 2, 'b' }, { 11, 9, 'b' } },
    { { 7, 6, 'a' }, { 7, 1, 'a' } },
};

static void test_level_rows(void)
{
    for (int l = 0; l < LEVEL_COUNT; l++)
        for (int y = 0; y < BOARD_H; y++)
            CHECK(strlen(LEVELS[l].rows[y]) == BOARD_W,
                  "level %d row %d length %zu", l + 1, y, strlen(LEVELS[l].rows[y]));
}

static void test_solutions(void)
{
    CHECK((int)(sizeof SOLUTIONS / sizeof SOLUTIONS[0]) == LEVEL_COUNT,
          "每關都要有解法");
    for (int l = 0; l < LEVEL_COUNT; l++) {
        game g;
        game_load(&g, l);
        CHECK(g.receivers > 0, "level %d 沒有接收器", l + 1);
        CHECK(!game_cleared(&g), "level %d 開局就過關", l + 1);

        /* 鏡子總數守恆：把可動鏡全收進手上，再照解法擺 */
        int total = g.hand;
        for (int i = 0; i < BOARD_N; i++)
            if (g.cell[i] == T_MIR_S || g.cell[i] == T_MIR_B) {
                g.cell[i] = T_FLOOR;
                total++;
            }
        int used = 0;
        for (int k = 0; k < 4 && SOLUTIONS[l][k].kind; k++) {
            const place *p = &SOLUTIONS[l][k];
            uint8_t *c = &g.cell[p->y * BOARD_W + p->x];
            CHECK(*c == T_FLOOR, "level %d 解法 (%d,%d) 不是空地", l + 1, p->x, p->y);
            *c = p->kind == 'a' ? T_MIR_S : T_MIR_B;
            used++;
        }
        CHECK(used <= total, "level %d 解法用了 %d 面，只有 %d 面", l + 1, used, total);
        game_trace(&g);
        CHECK(game_cleared(&g), "level %d 照解法擺沒過關（亮 %d/%d）",
              l + 1, g.lit_count, g.receivers);
    }
}

static void test_reflect_and_edges(void)
{
    /* 光往東撞 \ 往南，beam 標記：發射器東邊、鏡子西+南、接收器北 */
    level_def d = { {
        "################",
        "#>.b...........#",
        "#..............#",
        "#..R...........#",
        "#..............#", "#..............#", "#..............#", "#..............#",
        "#..............#", "#..............#", "#..............#",
        "################",
    }, 0 };
    game g;
    game_load_def(&g, &d);
    CHECK(g.beam[1 * BOARD_W + 1] == EDGE(DIR_E), "發射器 beam");
    CHECK(g.beam[1 * BOARD_W + 2] == (EDGE(DIR_W) | EDGE(DIR_E)), "直線 beam");
    CHECK(g.beam[1 * BOARD_W + 3] == (EDGE(DIR_W) | EDGE(DIR_S)), "鏡子 beam");
    CHECK(g.beam[3 * BOARD_W + 3] == EDGE(DIR_N), "接收器 beam");
    CHECK(game_cleared(&g), "應該過關");
}

static void test_loop_terminates(void)
{
    /* 光路自己交叉（經過 (3,4) 兩次、方向不同）。光學可逆，從發射器出發的光
     * 不會真的進入無窮迴圈，但 visited 保護要確保萬一規則改了也停得下來 */
    level_def d = { {
        "################",
        "#..............#",
        "#..a...b.......#",
        "#..............#",
        "#>.....a.......#",
        "#..b...........#",
        "#..............#", "#..............#", "#..............#",
        "#.............R#", "#..............#",
        "################",
    }, 0 };
    game g;
    game_load_def(&g, &d);      /* 不當機、能回來就算過 */
    CHECK(!game_cleared(&g), "迴圈關不該過");
}

static void test_take_place(void)
{
    game g;
    game_load(&g, 2);           /* 第 3 關：手上 1 面 */
    g.cx = 2; g.cy = 3;
    CHECK(game_take_place(&g) && g.hand == 0, "放下鏡子");
    CHECK(game_cleared(&g), "第 3 關放 (2,3) 應過關");
    CHECK(game_rotate(&g) && !game_cleared(&g), "轉成 \\ 就不過");
    CHECK(game_take_place(&g) && g.hand == 1, "拿回鏡子");
    CHECK(g.moves == 3, "步數 %d", g.moves);
    g.cx = 2; g.cy = 8;         /* 發射器上不能放 */
    CHECK(!game_take_place(&g), "發射器上不能放");
}

static void test_pack_roundtrip(void)
{
    static char buf[8192];
    static level_pack p;
    size_t n = pack_format(LEVELS, LEVEL_COUNT, buf, sizeof buf);
    CHECK(n > 0, "pack_format");
    pack_parse(&p, buf, n);
    CHECK(p.count == LEVEL_COUNT && p.bad == 0, "roundtrip count %d bad %d", p.count, p.bad);
    for (int l = 0; l < p.count && l < LEVEL_COUNT; l++) {
        CHECK(p.defs[l].hand == LEVELS[l].hand, "level %d hand", l + 1);
        for (int y = 0; y < BOARD_H; y++)
            CHECK(!strcmp(p.defs[l].rows[y], LEVELS[l].rows[y]), "level %d row %d", l + 1, y);
    }
}

static void test_pack_errors(void)
{
    static level_pack p;
    /* CRLF、註解、一關少一行、一關沒有接收器、一關正常 */
    const char *txt =
        "; comment\r\n"
        "################\r\n#>............R#\r\n"    /* 只有 2 行 */
        "\r\n";
    pack_parse(&p, txt, strlen(txt));
    CHECK(p.count == 0 && p.bad == 1, "短關卡 count %d bad %d", p.count, p.bad);

    static char buf[4096];
    size_t n = pack_format(LEVELS, 1, buf, sizeof buf);
    char *r = strchr(buf, 'R');             /* 拿掉第 1 關唯一的接收器 */
    *r = '.';
    pack_parse(&p, buf, n);
    CHECK(p.count == 0 && p.bad == 1, "無接收器 count %d bad %d", p.count, p.bad);

    n = pack_format(LEVELS, 2, buf, sizeof buf);
    *strrchr(buf, '#') = 'x';               /* 第 2 關最後一行有未知字元 */
    pack_parse(&p, buf, n);
    CHECK(p.count == 1 && p.bad == 1, "壞掉的關不影響前一關 count %d bad %d", p.count, p.bad);
}

static void test_save(void)
{
    save_data s, t;
    char buf[256];
    memset(&s, 0, sizeof s);
    s.level = 3;
    s.best[0] = 2;
    s.best[4] = 17;
    size_t n = save_format(&s, 5, buf, sizeof buf);
    save_parse(&t, buf, n);
    CHECK(t.level == 3 && t.best[0] == 2 && t.best[4] == 17 && t.best[1] == 0,
          "save roundtrip");
    save_parse(&t, "garbage\nbest 99 3\nlevel 0\n", 26);
    CHECK(t.level == 0 && t.best[0] == 0, "壞存檔不爆");
}

int main(void)
{
    test_pack_roundtrip();
    test_pack_errors();
    test_save();
    test_level_rows();
    test_solutions();
    test_reflect_and_edges();
    test_loop_terminates();
    test_take_place();
    printf(fails ? "%d 項失敗\n" : "全部通過\n", fails);
    return fails != 0;
}
