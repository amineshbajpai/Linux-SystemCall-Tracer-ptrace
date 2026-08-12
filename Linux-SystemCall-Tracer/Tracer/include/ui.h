#ifndef UI_H
#define UI_H

/*------------------------------------------------------------------
 * ui.h
 *
 * Purely cosmetic terminal-presentation helpers for the Linux
 * System Call Tracer.
 *
 * IMPORTANT: This module contains ONLY printf()-based formatting
 * helpers (colors, borders, spacing). It has no knowledge of, and
 * no effect on, ptrace(), waitpid(), process tracking, syscall
 * detection, register collection, or JSON logging. It exists solely
 * so the rest of the codebase can produce nicer-looking terminal
 * output without duplicating ANSI escape sequences everywhere.
 *-----------------------------------------------------------------*/

/* ---- Text styles ---- */
#define UI_RESET        "\033[0m"
#define UI_BOLD         "\033[1m"
#define UI_DIM          "\033[2m"

/* ---- Standard colors ---- */
#define UI_RED          "\033[31m"
#define UI_GREEN        "\033[32m"
#define UI_YELLOW       "\033[33m"
#define UI_BLUE         "\033[34m"
#define UI_MAGENTA      "\033[35m"
#define UI_CYAN         "\033[36m"
#define UI_WHITE        "\033[37m"

/* ---- Bold colors (used for headers / emphasis) ---- */
#define UI_BOLD_RED     "\033[1;31m"
#define UI_BOLD_GREEN   "\033[1;32m"
#define UI_BOLD_YELLOW  "\033[1;33m"
#define UI_BOLD_BLUE    "\033[1;34m"
#define UI_BOLD_MAGENTA "\033[1;35m"
#define UI_BOLD_CYAN    "\033[1;36m"
#define UI_BOLD_WHITE   "\033[1;37m"

/* Startup banner */
void ui_print_banner(void);

/* Single-line box drawing (used for SYSCALL ENTRY / EXIT / REGISTERS) */
void ui_box_top(const char *color, const char *title);
void ui_box_bottom(const char *color);
void ui_box_row(const char *color, const char *label, const char *fmt, ...);
void ui_box_note(const char *color, const char *text);

/* Double-line box drawing (used for process lifecycle events) */
void ui_dbox_top(const char *color, const char *title);
void ui_dbox_bottom(const char *color);
void ui_dbox_row(const char *color, const char *fmt, ...);

/* Inline status helpers */
void ui_error(const char *fmt, ...);
void ui_warning(const char *fmt, ...);
void ui_info(const char *fmt, ...);
void ui_success(const char *fmt, ...);

#endif /* UI_H */
