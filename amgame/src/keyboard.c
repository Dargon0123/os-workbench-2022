#include <game.h>

#define KEYNAME(key) \
  [AM_KEY_##key] = #key,
static const char *key_names[] = {
  AM_KEYS(KEYNAME)
};

int read_key() {
    AM_INPUT_KEYBRD_T event = { .keycode = AM_KEY_NONE };
    ioe_read(AM_INPUT_KEYBRD, &event);
    if (event.keycode != AM_KEY_NONE && event.keydown) {
        log_key(event.keycode);
        return event.keycode;
    }
    return AM_KEY_NONE;
}


void log_key(int key) {
    printf("Key pressed: %s\r\n", key_names[key]);
}

void print_key() {
  AM_INPUT_KEYBRD_T event = { .keycode = AM_KEY_NONE };
  ioe_read(AM_INPUT_KEYBRD, &event);
  if (event.keycode != AM_KEY_NONE && event.keydown) {
    do_key(event.keycode);
    log_key(event.keycode);
  }
}
