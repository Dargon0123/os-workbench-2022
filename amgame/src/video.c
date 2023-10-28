#include <game.h>

#define SIDE 16
static int w, h;

// typedef struct { 
//     _Bool present, has_accel; 
//     int width, height, vmemsz; 
// } AM_GPU_CONFIG_T;

static void init() {
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;
}

static void draw_tile(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  /* 把小块颜色初始化 */
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}

void splash() {
    init();
    for (int x = 0; x * SIDE <= w; x++) {
        for (int y = 0; y * SIDE <= h; y++) {
            if ((x & 1) ^ (y & 1)) {
                draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0x000000); // white
            }
        }
    }
    // draw_tile(0, 0, SIDE, SIDE, 0x00ffff); // white
}

void screen_update(int row, int col) {
    for (int x = 0; x * SIDE <= w; x++) {
        for (int y = 0; y * SIDE <= h; y++) {
            draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0x000000); // white
        }
    }
    draw_tile(row * SIDE, col * SIDE, SIDE, SIDE, 0x00ff00);
}

void game_progress(int *x, int *y) {
    if (*x < 0) *x = (w / SIDE) -1;
    else if (*x >= w / SIDE) *x = 0;
    if (*y < 0) *y = (h / SIDE) -1;
    else if (*y >= h / SIDE) *y = 0;
    return;    
}
