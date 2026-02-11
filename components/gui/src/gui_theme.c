#include "gui_theme.h"

static const gui_theme_t s_dark = {
    .mode = GUI_THEME_DARK,
    .background = 0x1A1A2EU,
    .primary = 0x16213EU,
    .accent = 0x0F3460U,
    .text = 0xE0E0E0U,
    .success = 0x00C853U,
    .warning = 0xFFD600U,
    .error = 0xFF1744U,
    .attack = 0xFF5722U,
};

static const gui_theme_t s_light = {
    .mode = GUI_THEME_LIGHT,
    .background = 0xFAFAFAU,
    .primary = 0xEDEDEDU,
    .accent = 0x1976D2U,
    .text = 0x212121U,
    .success = 0x00C853U,
    .warning = 0xFFD600U,
    .error = 0xFF1744U,
    .attack = 0xFF5722U,
};

static const gui_theme_t *s_active = &s_dark;

void gui_theme_apply(gui_theme_mode_t mode) {
    s_active = (mode == GUI_THEME_LIGHT) ? &s_light : &s_dark;
}

const gui_theme_t *gui_theme_get(void) {
    return s_active;
}

gui_theme_mode_t gui_theme_get_mode(void) {
    return s_active->mode;
}
