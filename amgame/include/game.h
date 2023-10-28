#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>

/* video.c */
void splash();
void screen_update(int x, int y);
void game_progress(int *x, int *y);

/* keyboard.c */
int read_key();
void do_key(int key);
void log_key(int key);
void print_key();


/* klib */
static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}
