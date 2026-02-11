#ifndef GUI_THEME_H
#define GUI_THEME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GUI_THEME_DARK = 0,
    GUI_THEME_LIGHT,
} gui_theme_mode_t;

typedef struct {
    gui_theme_mode_t mode;
    uint32_t background;
    uint32_t primary;
    uint32_t accent;
    uint32_t text;
    uint32_t success;
    uint32_t warning;
    uint32_t error;
    uint32_t attack;
} gui_theme_t;

void gui_theme_apply(gui_theme_mode_t mode);
const gui_theme_t *gui_theme_get(void);
gui_theme_mode_t gui_theme_get_mode(void);

#ifdef __cplusplus
}
#endif

#endif  // GUI_THEME_H
