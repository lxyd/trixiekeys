#include "trixiekeys.h"

BEGIN_CONFIG

CREATE_MAP(defmap);
INIT_MAP_IDENTITY(defmap);

CREATE_MAP(cplkmap);
INIT_MAP_IGNORE(cplkmap);

MAP_MODIFIER(defmap, KEY_CAPSLOCK, cplkmap);

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

END_CONFIG(defmap)