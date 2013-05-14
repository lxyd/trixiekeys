#include "trixiekeys.h"

#define KEY_NEW_L3_SHIFT KEY_RIGHTALT
#define KEY_NEW_COMPOSE KEY_SCROLLLOCK

BEGIN_CONFIG

// defmap - usual keyboard map with the following changes:
// - CAPSLOCK shifts to cplkmap
// - SCROLLLOCK is moved to make future combile of the compose key and level3_shift possible

CREATE_MAP(defmap);
INIT_MAP_IDENTITY(defmap);

MAP_KEY(defmap, KEY_NEW_L3_SHIFT, KEY_NEW_COMPOSE);
MAP_MODIFIER(defmap, KEY_NEW_L3_SHIFT, KEY_NEW_L3_SHIFT);

// l3map - map intended to support RALT+ENTER, RALT+LEFT etc when RALT is L3_SHIFT

CREATE_MAP(l3map);
INIT_MAP_IDENTITY(l3map);

MAP_SHIFT(defmap, KEY_NEW_L3_SHIFT, l3map);

MAP_KEY_SEQUENCE(l3map, KEY_ENTER, +KEY_LEFTALT, +KEY_ENTER, -KEY_ENTER, -KEY_LEFTALT);
MAP_KEY_SEQUENCE(l3map, KEY_TAB,   +KEY_LEFTALT, +KEY_TAB,   -KEY_TAB,   -KEY_LEFTALT);
MAP_KEY_SEQUENCE(l3map, KEY_LEFT,  +KEY_LEFTALT, +KEY_LEFT,  -KEY_LEFT,  -KEY_LEFTALT);
MAP_KEY_SEQUENCE(l3map, KEY_RIGHT, +KEY_LEFTALT, +KEY_RIGHT, -KEY_RIGHT, -KEY_LEFTALT);
MAP_KEY_SEQUENCE(l3map, KEY_UP,    +KEY_LEFTALT, +KEY_UP,    -KEY_UP,    -KEY_LEFTALT);
MAP_KEY_SEQUENCE(l3map, KEY_DOWN,  +KEY_LEFTALT, +KEY_DOWN,  -KEY_DOWN,  -KEY_LEFTALT);

// cplkmap - modified map to which CAPSLOCK shifts

CREATE_MAP(cplkmap);
INIT_MAP_IDENTITY(cplkmap);

MAP_SHIFT(defmap, KEY_CAPSLOCK, cplkmap);

MAP_KEY(cplkmap, KEY_A, KEY_LEFTCTRL);
MAP_KEY(cplkmap, KEY_S, KEY_LEFTSHIFT);

MAP_KEY(cplkmap, KEY_LEFTBRACE, KEY_ESC);

MAP_KEY(cplkmap, KEY_H, KEY_LEFT);
MAP_KEY(cplkmap, KEY_J, KEY_DOWN);
MAP_KEY(cplkmap, KEY_K, KEY_UP);
MAP_KEY(cplkmap, KEY_L, KEY_RIGHT);

MAP_KEY(cplkmap, KEY_N, KEY_PAGEDOWN);
MAP_KEY(cplkmap, KEY_P, KEY_PAGEUP);

MAP_KEY(cplkmap, KEY_COMMA, KEY_HOME);
MAP_KEY(cplkmap, KEY_DOT, KEY_END);

MAP_KEY(cplkmap, KEY_U, KEY_BACKSPACE);
MAP_KEY(cplkmap, KEY_O, KEY_DELETE);

// cplkmap_lock - equal to cplkmap but have the return back capability

CREATE_MAP(cplkmap_lock);
INIT_MAP_FROM_MAP(cplkmap_lock, cplkmap);

MAP_LOCK(cplkmap, KEY_F, cplkmap_lock);

MAP_LOCK(cplkmap_lock, KEY_F, defmap);
MAP_LOCK(cplkmap_lock, KEY_ESC, defmap);
MAP_LOCK(cplkmap_lock, KEY_CAPSLOCK, defmap);
MAP_LOCK(cplkmap_lock, KEY_BACKSPACE, defmap);

END_CONFIG(defmap)
