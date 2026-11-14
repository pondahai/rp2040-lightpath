/* 光路反射 —— G0 骨架：分條渲染測試畫面。
 *
 * 驗證三件事：
 *   1. hal/display 的 16-bit SPI + DMA 雙 buffer 推條正確（顏色、方向、無錯位）
 *   2. lib/buttons 的去彈跳與連發（方向鍵捲動、A/B 換色）
 *   3. 每幀全畫面重送的速度（最上方的白條長度 = 上一幀花的毫秒數 ×4 px）
 *
 * 畫面：斜向彩色條紋，方向鍵捲動，A/B 切換配色，START 暫停自動捲動。
 */
#include "pico/stdlib.h"
#include "board.h"
#include "display.h"
#include "dpad.h"
#include "buttons.h"

#define STRIP_H 16

static uint16_t strip[2][LCD_W * STRIP_H];

static const uint16_t PALETTES[3][4] = {
    { RD_RGB565(230, 60, 60), RD_RGB565(240, 200, 40),
      RD_RGB565(60, 200, 90), RD_RGB565(50, 120, 230) },
    { RD_RGB565(20, 20, 40),  RD_RGB565(80, 40, 140),
      RD_RGB565(200, 60, 160), RD_RGB565(250, 220, 240) },
    { RD_RGB565(0, 0, 0),     RD_RGB565(255, 255, 255),
      RD_RGB565(255, 0, 0),   RD_RGB565(0, 0, 255) },
};

static void render_strip(int y0, uint16_t *buf, int sx, int sy,
                         const uint16_t *pal, uint32_t frame_ms)
{
    for (int y = 0; y < STRIP_H; y++) {
        int yy = y0 + y;
        uint16_t *row = buf + y * LCD_W;
        for (int x = 0; x < LCD_W; x++)
            row[x] = pal[(((x + sx) + (yy + sy)) >> 4) & 3];

        /* 最上方 4 行：幀時間條 */
        if (yy < 4) {
            int len = (int)frame_ms * 4;
            if (len > LCD_W) len = LCD_W;
            for (int x = 0; x < LCD_W; x++)
                row[x] = x < len ? 0xFFFF : 0x0000;
        }
    }
}

int main(void)
{
    rd_display_init();
    rd_dpad_init();

    rd_buttons btn;
    rd_buttons_init(&btn, RD_BTN_DPAD);

    int sx = 0, sy = 0, pal = 0;
    bool autoscroll = true;
    uint32_t frame_ms = 0;

    for (;;) {
        uint32_t t0 = to_ms_since_boot(get_absolute_time());

        uint8_t fired = rd_buttons_update(&btn, rd_dpad_raw(), t0);
        if (fired & RD_BTN_LEFT)  sx -= 8;
        if (fired & RD_BTN_RIGHT) sx += 8;
        if (fired & RD_BTN_UP)    sy -= 8;
        if (fired & RD_BTN_DOWN)  sy += 8;
        if (fired & RD_BTN_A)     pal = (pal + 1) % 3;
        if (fired & RD_BTN_B)     pal = (pal + 2) % 3;
        if (fired & RD_BTN_START) autoscroll = !autoscroll;
        if (autoscroll) sx++;

        rd_display_window(0, 0, LCD_W, LCD_H);
        int i = 0;
        for (int y = 0; y < LCD_H; y += STRIP_H) {
            render_strip(y, strip[i], sx, sy, PALETTES[pal], frame_ms);
            rd_display_push(strip[i], LCD_W * STRIP_H);
            i ^= 1;
        }
        rd_display_wait();

        frame_ms = to_ms_since_boot(get_absolute_time()) - t0;
    }
}
