#include <game.h>

/* 刷新率 */
#define FPS 10
static int next_frame = 0;
static int x = 0, y = 0;

static inline int up_time() {
    return io_read(AM_TIMER_UPTIME).us / 1000;
}

void wait_for_frame() {
    int cur = up_time();
    while (cur < next_frame) {
        cur = up_time();
    }
    next_frame = cur;
}


void do_key(int key) {    
    switch (key) {
    /* left <-- */
    case AM_KEY_A :
        x--;
        break;
    /* down  */
    case AM_KEY_S :
        y++;
        break;
    /* right -->  */
    case AM_KEY_D :
        x++;
        break;
    /* up  */
    case AM_KEY_W :
        y--;
        break;
    /* quit  */
    case AM_KEY_ESCAPE :
        halt(1);
        break;
    case AM_KEY_R :
        x = 0; 
        y = 0;
        break;
    default :
        break;
    }    
}

// Operating system is a C program!
int main(const char *args) {
    int key = AM_KEY_NONE;    

    ioe_init();
    puts("mainargs = \"");
    puts(args); // make run mainargs=xxx
    puts("\"\r\n");

    splash();
    printf("Press any key to see its key code...\r\n");
       
    while (1) {
        wait_for_frame();
        while ((key = read_key()) != AM_KEY_NONE) {
            do_key(key);
        }
        game_progress(&x, &y);
        screen_update(x, y);

        next_frame += 1000 / FPS;
    }
    return 0;
}
