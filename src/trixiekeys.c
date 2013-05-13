#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>

#include "trixiekeys.h"

#define DEBUG

#define VAL_PRESS 1
#define VAL_RELEASE 0
#define VAL_REPEAT 2

#define EVENT_SIZE sizeof(struct input_event)
#define ARR_LEN(x) (sizeof(x) / sizeof(x[0]))

key_data *get_map(); // defined in config.c

key_data *permanent_map;
key_data *current_map;

int candidate = 0;
int modifier = 0;

key_data *press_map[KEY_CNT] = {0};

int event_buf[KEY_CNT]; // we physically cannot hold more buttons than we have at max
int event_buf_len = 0;

int hdev_fd = -1;
int udev_fd = -1;

static int want_to_exit;

void sighandler(int signum) {
    switch (signum) {
        case SIGTERM:
        case SIGINT:
            want_to_exit = 1;
            break;

        default:
            /* TODO: print some usefull info on SIGUSR1/SIGUSR2 */
            break;

    }
}

void die(const char* msg) {
    printf("%s\n", msg);
    if (hdev_fd != -1) {
        ioctl(hdev_fd, EVIOCGRAB, 0);
        close(hdev_fd);
    }
    if (udev_fd != -1) {
        close(udev_fd);
    }
    exit(1);
}

int read_event(const int fd, struct input_event * ev) {
    int res = EVENT_SIZE == read(fd, ev, EVENT_SIZE);
#ifdef DEBUG
    printf("R %d: %2hd %7d %3hd\n", fd, ev->type, ev->value, ev->code);
#endif
    return res;
}

void write_event(const int fd, const struct input_event * ev) {
    write(fd, ev, EVENT_SIZE);
#ifdef DEBUG
    printf("W %d: %2hd %7d %3hd\n", fd, ev->type, ev->value, ev->code);
#endif
}

void send_event(const int type, const int value, const int code, const int auto_syn) {
    static struct timeval prev_tv;
    static struct input_event ev;

    #define TV_LESS(a, b) ((a).tv_sec < (b).tv_sec || ((a).tv_sec == (b).tv_sec && (a).tv_usec < (b).tv_usec))
    #define TV_NEXT_USEC(tv) do {      \
        (tv).tv_usec++;                \
        if ((tv).tv_usec >= 1000000) { \
            (tv).tv_usec = 0;          \
            (tv).tv_sec++;             \
        }                              \
    } while(0)

    // write event itself
    ev.type = type;
    ev.value = value;
    ev.code = code;

    gettimeofday(&ev.time, NULL);

    if (!TV_LESS(prev_tv, ev.time)) {
        ev.time = prev_tv;
        TV_NEXT_USEC(ev.time);
    }

    write_event(udev_fd, &ev);

    if (auto_syn) {
        // and write sync event
        ev.type = EV_SYN;
        ev.value = 0;
        ev.code = 0;

        TV_NEXT_USEC(ev.time);

        write_event(udev_fd, &ev);
    }

    // remember previous time
    prev_tv = ev.time;

    #undef TV_LESS
    #undef TV_NEXT_USEC
}

#define SEND(ev) send_event(EV_KEY, (ev) > 0 ? VAL_PRESS : VAL_RELEASE, (ev) > 0 ? (ev) : -(ev), 1)

void flush_buffer() {
    for (int i = 0; i < event_buf_len; i++) {
        SEND(event_buf[i]);
    }
    event_buf_len = 0;
}

void process_event(const struct input_event * iev) {
    key_data *k;
    switch (iev->value) {
        case VAL_PRESS:
            if (candidate) {
                modifier = candidate;
                candidate = 0;
                if (press_map[modifier]->shift_map) {
                    current_map = press_map[modifier]->shift_map;
                }
                if (press_map[modifier]->mod_code) {
                    SEND(press_map[modifier]->mod_code);
                }
            }
            k = &current_map[iev->code];
            press_map[iev->code] = k;
            flush_buffer();
            if ((k->shift_map || k->mod_code) && !modifier) {
#ifdef DEBUG
                printf("MODIFIER CANDIDATE: %d %d\n", k->mod_code, k->shift_map != NULL);
#endif
                candidate = iev->code;
            } else {
                if (k->lock_map) {
                    // do nothing
                    //permanent_map = k->lock_map; will be done on release
                } else if (k->key_code) {
                    SEND(k->key_code);
                } else if (k->key_codes) {
                    int *p = k->key_codes;
                    while (*p) {
                        SEND(*p);
                        p++;
                    }
                }
            }
            break;
        case VAL_RELEASE:
            k = press_map[iev->code];
            if (!k) {
                SEND( - iev->code); // key was pressed before we started
            } else if (iev->code == modifier) {
                modifier = 0;
                current_map = permanent_map;
                if (k->mod_code) {
                    SEND( - k->mod_code);
                }
                flush_buffer();
            } else if (iev->code == candidate) {
                candidate = 0;
                if (k->key_code) {
                    SEND(k->key_code);
                    flush_buffer();
                    SEND( - k->key_code);
                } else {
                    flush_buffer();
                }
            } else if (candidate) {
                if (k->key_code) {
                    event_buf[event_buf_len++] = -k->key_code;
                }
            } else {
                if (k->lock_map) {
                    current_map = permanent_map = k->lock_map;
                } else if (k->key_code) {
                    SEND( - k->key_code);
                }
            }
            press_map[iev->code] = NULL;
            break;

        case VAL_REPEAT:
            k = press_map[iev->code];
            if (!k) {
                // key was pressed before we started
                send_event(EV_KEY, VAL_REPEAT, iev->code, 1);
            } else if (!candidate && k->key_code) {
                send_event(EV_KEY, VAL_REPEAT, k->key_code, 1);
            } // else: drop event
            break;
        default:
#ifdef DEBUG
            printf("Unknown event to process\n");
#endif
            break;

    }
}

int find_dev_by_name(const char *devdir_path, const char *target_name, char* out_path, const int max_n) {
    DIR *dir;
    struct dirent *ent;
    int found = 0;
    const int max_fn = max_n - strlen(devdir_path);

    dir = opendir(devdir_path);
    if (!dir) {
        die("Couldn't open input device directory");
    }

    while (!found && (ent = readdir(dir))) {
        struct stat s;
        strncpy(out_path, devdir_path, max_n);
        strncat(out_path, ent->d_name, max_fn);

        if (stat(out_path, &s) < 0) {
            continue;
        }
        if (S_ISCHR(s.st_mode)) {
            int fd = open(out_path, O_RDONLY);

            if (fd >= 0) {
                char dev_name[256];
                ioctl(fd, EVIOCGNAME(sizeof(dev_name)), dev_name);

                // check if we've found our device
                found = 0 == strcmp(dev_name, target_name);

                close(fd);
            }
        }
    }

    closedir(dir);

    return found;
}

//
// USAGE: trixiekeys {input_device_path | input_device_name} [output_device_name]
//
int main(int argc, char* argv[]) {
    struct uinput_user_dev odev_data;
    struct input_event ev;
    char hdev_name[256] = "Unknown";
    char udev_name[256] = "TrixieKeys keyboard";
    char hdev_path[256];
    char udev_path[256] = "/dev/uinput";
    char devdir_path[256] = "/dev/input/";

    /*
    char* event_types[EV_CNT] = { NULL };
    #define ADD_EVENT_TYPE(x) (event_types[x] = #x)
    ADD_EVENT_TYPE(EV_SYN);
    ADD_EVENT_TYPE(EV_SYN);
    ADD_EVENT_TYPE(EV_KEY);
    ADD_EVENT_TYPE(EV_REL);
    ADD_EVENT_TYPE(EV_ABS);
    ADD_EVENT_TYPE(EV_MSC);
    ADD_EVENT_TYPE(EV_SW);
    ADD_EVENT_TYPE(EV_LED);
    ADD_EVENT_TYPE(EV_SND);
    ADD_EVENT_TYPE(EV_REP);
    ADD_EVENT_TYPE(EV_FF);
    ADD_EVENT_TYPE(EV_PWR);
    ADD_EVENT_TYPE(EV_FF_STATUS);
    ADD_EVENT_TYPE(EV_MAX);
    #undef ADD_EVENT_TYPE
    #define EVENT_TYPE(x) (((x) >= 0 && (x) < ARR_LEN(event_types) && event_types[(x)]) ? event_types[(x)] : "EV_UNKNOWN")
    */

	signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);

    if (argc < 2 || argc > 3) {
        die("USAGE: trixiekeys {input_device_path | input_device_name} [output_device_name]");
    }

    if (argc == 3) {
        strncpy(udev_name, argv[2], sizeof(udev_name)-1);
    }

    // 1. modprobe uinput

    if (0 != system("modprobe uinput")) {
        die("Couldn't modprobe uinput module.");
    }

    // 2. find the input device

    if (argv[1][0] == '/') {
        strncpy(hdev_path, argv[1], sizeof(hdev_path)-1);
    } else if (!find_dev_by_name(devdir_path, argv[1], hdev_path, sizeof(hdev_path)-1)) {
        die("Couldn't find a device with the specified name");
    }

    // 3. grab hardware device

    hdev_fd = open(hdev_path, O_RDWR);// | O_NDELAY);
    if (hdev_fd < 0) {
        die("Couldn't open source event device.");
    }

    ioctl(hdev_fd, EVIOCGNAME(sizeof(hdev_name)), hdev_name);
    printf("Reading From : %s (%s)\n", hdev_path, hdev_name);

    if (0 != ioctl(hdev_fd, EVIOCGRAB, 1)) {
        die("Coudn't get exclusive access to the hardware device.");
    }

    // 4. setup uinput device

    udev_fd = open(udev_path, O_RDWR);// | O_NDELAY);
    if (udev_fd < 0) {
        die("Couldn't open uinput device.");
    }

    memset(&odev_data, 0, sizeof(odev_data));
    strncpy(odev_data.name, udev_name, UINPUT_MAX_NAME_SIZE);
    odev_data.id.version = 4;
    odev_data.id.bustype = BUS_USB;

    ioctl(udev_fd, UI_SET_EVBIT, EV_MSC);
    ioctl(udev_fd, UI_SET_MSCBIT, MSC_SCAN);

    ioctl(udev_fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < KEY_CNT; i++) {
        ioctl(udev_fd, UI_SET_KEYBIT, i);
    }
    ioctl(udev_fd, UI_SET_EVBIT, EV_LED);
    for (int i = 0; i < LED_CNT; i++) {
        ioctl(udev_fd, UI_SET_LEDBIT, i);
    }

    write(udev_fd, &odev_data, sizeof(odev_data));

    if (0 != ioctl(udev_fd, UI_DEV_CREATE)) {
        die("Couldn't create UINPUT device.");
    }

    // 5. obtain user's config

    permanent_map = current_map = get_map();

    // 6. become daemon

	daemon(0, 1);

    // 7. create fd_set to select from input devices

    fd_set read_fd_set;
    const int nfds = (hdev_fd > udev_fd ? hdev_fd : udev_fd) + 1;
    FD_ZERO(&read_fd_set);
    FD_SET(hdev_fd, &read_fd_set);
    FD_SET(udev_fd, &read_fd_set);

    // 8. enter main loop

    int skip_next_syn = 0;
    while (!want_to_exit) {
        if (select(nfds, &read_fd_set, NULL, NULL, NULL) < 0) {
            die("Couldn't read nor wait for event\n");
        }
        

        // just pass LED commands that come to our keyboard
        if (FD_ISSET(udev_fd, &read_fd_set)) {
            
            read_event(udev_fd, &ev);

            //printf("U: %s %7d %3hd %30ld\n", EVENT_TYPE(ev.type), ev.value, ev.code, ev.time.tv_usec);

            if (ev.type == EV_LED) {
                write_event(hdev_fd, &ev);
            }

        }

        if (FD_ISSET(hdev_fd, &read_fd_set)) {
            
            read_event(hdev_fd, &ev);

            //printf("H: %s %7d %3hd %30ld\n", EVENT_TYPE(ev.type), ev.value, ev.code, ev.time.tv_usec);

            switch (ev.type) {

                case EV_KEY:
                    skip_next_syn = 1;
                    process_event(&ev);
                    break;

                case EV_SYN:
                    if (!skip_next_syn) {
                        send_event(ev.type, ev.value, ev.code, 0);
                    }
                    skip_next_syn = 0;
                    break;

                case EV_LED:
                    skip_next_syn = 0;
                    break;

                default:
                    send_event(ev.type, ev.value, ev.code, 0);
                    skip_next_syn = 0;
                    break;

            }

        }

        FD_ZERO(&read_fd_set);
        FD_SET(hdev_fd, &read_fd_set);
        FD_SET(udev_fd, &read_fd_set);

    }

    printf("Exiting.\n");
    ioctl(hdev_fd, EVIOCGRAB, 0);

    close(hdev_fd);
    close(udev_fd);

    return 0;
}
