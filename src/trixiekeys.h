#ifndef __TRIXIEKEYS_H__
#define __TRIXIEKEYS_H__

#include <stdint.h>
#include <linux/input.h>

typedef struct __key_data key_data;
struct __key_data {
    // table to switch to when this key is used as mod. 0 if cannot
    key_data *shift_map;

    // if not 0, this code will be played when key is used as modifier instead of key_code
    // (press - before modified buttons, release - after)
    uint16_t mod_code;

    // when this key is used as a key:
    // if both of the following arguments are 0, key will be ignored
    // if key_code is not 0, key will play as key_code
    // if key_codes is not null, this sequence will be played at some point in time
    uint16_t key_code;
    // pointer to an array of events +key, -key ends with 0
    int *key_codes;

    // switch map permanently
    key_data *lock_map;
};

#define BEGIN_CONFIG key_data *get_map() {
#define END_CONFIG(map) return (map); }

#define CREATE_MAP(map) \
    static key_data map[KEY_CNT]

#define INIT_MAP_IDENTITY(map)               \
    for(int __i = 0; __i < KEY_CNT; __i++) { \
        (map)[__i].shift_map = 0;            \
        (map)[__i].lock_map  = 0;            \
        (map)[__i].mod_code  = 0;            \
        (map)[__i].key_code  = __i;          \
        (map)[__i].key_codes = 0;            \
    }
#define INIT_MAP_IGNORE(map)                 \
    for(int __i = 0; __i < KEY_CNT; __i++) { \
        (map)[__i].shift_map = 0;            \
        (map)[__i].lock_map  = 0;            \
        (map)[__i].mod_code  = 0;            \
        (map)[__i].key_code  = 0;            \
        (map)[__i].key_codes = 0;            \
    }

#define INIT_MAP_FROM_MAP(map, src)                  \
    for(int __i = 0; __i < KEY_CNT; __i++) {         \
        (map)[__i].shift_map = (src)[__i].shift_map; \
        (map)[__i].lock_map  = (src)[__i].lock_map;  \
        (map)[__i].mod_code  = (src)[__i].mod_code;  \
        (map)[__i].key_code  = (src)[__i].key_code;  \
        (map)[__i].key_codes = (src)[__i].key_codes; \
    }

#define CLEAR_KEY(map, orig_key)   \
    (map)[orig_key].shift_map = 0; \
    (map)[orig_key].lock_map  = 0; \
    (map)[orig_key].mod_code  = 0; \
    (map)[orig_key].key_code  = 0; \
    (map)[orig_key].key_codes = 0

#define MAP_KEY(map, orig_key, new_key)   \
    (map)[orig_key].key_code = (new_key); \
    (map)[orig_key].key_codes = 0;        \
    (map)[orig_key].lock_map = 0

#define MAP_KEY_SEQUENCE(map, orig_key, new_keys)             \
    static int *__key_codes_##map_##orig_key = new_keys;      \
    (map)[orig_key].key_codes = __key_codes_##map_##orig_key; \
    (map)[orig_key].key_code = 0;                             \
    (map)[orig_key].lock_map = 0

#define MAP_SHIFT(map, orig_key, new_map) \
    (map)[orig_key].shift_map = new_map

#define MAP_LOCK(map, orig_key, new_map) \
    (map)[orig_key].lock_map = new_map;  \
    (map)[orig_key].key_code = 0;        \
    (map)[orig_key].key_codes = 0

#define MAP_MODIFIER(map, orig_key, new_mod) \
    (map)[orig_key].mod_code = new_mod

#endif // ifndef __TRIXIEKEYS_H__
