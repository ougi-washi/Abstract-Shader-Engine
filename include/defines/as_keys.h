// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

// These numbers are matching the glfw implementation

#define AS_RELEASE                0
#define AS_PRESS                  1
#define AS_REPEAT                 2

#define AS_HAT_CENTERED           0
#define AS_HAT_UP                 1
#define AS_HAT_RIGHT              2
#define AS_HAT_DOWN               4
#define AS_HAT_LEFT               8
#define AS_HAT_RIGHT_UP           (AS_HAT_RIGHT | AS_HAT_UP)
#define AS_HAT_RIGHT_DOWN         (AS_HAT_RIGHT | AS_HAT_DOWN)
#define AS_HAT_LEFT_UP            (AS_HAT_LEFT  | AS_HAT_UP)
#define AS_HAT_LEFT_DOWN          (AS_HAT_LEFT  | AS_HAT_DOWN)
#define AS_KEY_UNKNOWN            -1

  /* Printable keys */
#define AS_KEY_SPACE              32
#define AS_KEY_APOSTROPHE         39  /* ' */
#define AS_KEY_COMMA              44  /* , */
#define AS_KEY_MINUS              45  /* - */
#define AS_KEY_PERIOD             46  /* . */
#define AS_KEY_SLASH              47  /* / */
#define AS_KEY_0                  48
#define AS_KEY_1                  49
#define AS_KEY_2                  50
#define AS_KEY_3                  51
#define AS_KEY_4                  52
#define AS_KEY_5                  53
#define AS_KEY_6                  54
#define AS_KEY_7                  55
#define AS_KEY_8                  56
#define AS_KEY_9                  57
#define AS_KEY_SEMICOLON          59  /* ; */
#define AS_KEY_EQUAL              61  /* = */
#define AS_KEY_A                  65
#define AS_KEY_B                  66
#define AS_KEY_C                  67
#define AS_KEY_D                  68
#define AS_KEY_E                  69
#define AS_KEY_F                  70
#define AS_KEY_G                  71
#define AS_KEY_H                  72
#define AS_KEY_I                  73
#define AS_KEY_J                  74
#define AS_KEY_K                  75
#define AS_KEY_L                  76
#define AS_KEY_M                  77
#define AS_KEY_N                  78
#define AS_KEY_O                  79
#define AS_KEY_P                  80
#define AS_KEY_Q                  81
#define AS_KEY_R                  82
#define AS_KEY_S                  83
#define AS_KEY_T                  84
#define AS_KEY_U                  85
#define AS_KEY_V                  86
#define AS_KEY_W                  87
#define AS_KEY_X                  88
#define AS_KEY_Y                  89
#define AS_KEY_Z                  90
#define AS_KEY_LEFT_BRACKET       91  /* [ */
#define AS_KEY_BACKSLASH          92  /* \ */
#define AS_KEY_RIGHT_BRACKET      93  /* ] */
#define AS_KEY_GRAVE_ACCENT       96  /* ` */
#define AS_KEY_WORLD_1            161 /* non-US #1 */
#define AS_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define AS_KEY_ESCAPE             256
#define AS_KEY_ENTER              257
#define AS_KEY_TAB                258
#define AS_KEY_BACKSPACE          259
#define AS_KEY_INSERT             260
#define AS_KEY_DELETE             261
#define AS_KEY_RIGHT              262
#define AS_KEY_LEFT               263
#define AS_KEY_DOWN               264
#define AS_KEY_UP                 265
#define AS_KEY_PAGE_UP            266
#define AS_KEY_PAGE_DOWN          267
#define AS_KEY_HOME               268
#define AS_KEY_END                269
#define AS_KEY_CAPS_LOCK          280
#define AS_KEY_SCROLL_LOCK        281
#define AS_KEY_NUM_LOCK           282
#define AS_KEY_PRINT_SCREEN       283
#define AS_KEY_PAUSE              284
#define AS_KEY_F1                 290
#define AS_KEY_F2                 291
#define AS_KEY_F3                 292
#define AS_KEY_F4                 293
#define AS_KEY_F5                 294
#define AS_KEY_F6                 295
#define AS_KEY_F7                 296
#define AS_KEY_F8                 297
#define AS_KEY_F9                 298
#define AS_KEY_F10                299
#define AS_KEY_F11                300
#define AS_KEY_F12                301
#define AS_KEY_F13                302
#define AS_KEY_F14                303
#define AS_KEY_F15                304
#define AS_KEY_F16                305
#define AS_KEY_F17                306
#define AS_KEY_F18                307
#define AS_KEY_F19                308
#define AS_KEY_F20                309
#define AS_KEY_F21                310
#define AS_KEY_F22                311
#define AS_KEY_F23                312
#define AS_KEY_F24                313
#define AS_KEY_F25                314
#define AS_KEY_KP_0               320
#define AS_KEY_KP_1               321
#define AS_KEY_KP_2               322
#define AS_KEY_KP_3               323
#define AS_KEY_KP_4               324
#define AS_KEY_KP_5               325
#define AS_KEY_KP_6               326
#define AS_KEY_KP_7               327
#define AS_KEY_KP_8               328
#define AS_KEY_KP_9               329
#define AS_KEY_KP_DECIMAL         330
#define AS_KEY_KP_DIVIDE          331
#define AS_KEY_KP_MULTIPLY        332
#define AS_KEY_KP_SUBTRACT        333
#define AS_KEY_KP_ADD             334
#define AS_KEY_KP_ENTER           335
#define AS_KEY_KP_EQUAL           336
#define AS_KEY_LEFT_SHIFT         340
#define AS_KEY_LEFT_CONTROL       341
#define AS_KEY_LEFT_ALT           342
#define AS_KEY_LEFT_SUPER         343
#define AS_KEY_RIGHT_SHIFT        344
#define AS_KEY_RIGHT_CONTROL      345
#define AS_KEY_RIGHT_ALT          346
#define AS_KEY_RIGHT_SUPER        347
#define AS_KEY_MENU               348

#define AS_KEY_LAST               AS_KEY_MENU
