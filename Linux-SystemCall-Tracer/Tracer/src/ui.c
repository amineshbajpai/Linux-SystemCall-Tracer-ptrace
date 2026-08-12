/*------------------------------------------------------------------
 * ui.c
 *
 * Purely cosmetic terminal-presentation helpers.
 * See include/ui.h for the design rationale.
 *
 * NOTE: Every function here only calls printf()/vprintf(). Nothing
 * in this file touches ptrace, process tracking, syscall detection,
 * register collection, or the JSON log file.
 *-----------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "../include/ui.h"

/* Inner width (in single box-drawing characters) used for the
 * horizontal borders of single-line boxes. */
#define UI_BOX_WIDTH 64

static void repeat_utf8(const char *glyph, int count)
{
    for (int i = 0; i < count; i++)
        fputs(glyph, stdout);
}

void ui_print_banner(void)
{
    printf("%s", UI_BOLD_CYAN);
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║             LINUX SYSTEM CALL TRACER                          ║\n");
    printf("║          Real-Time Process & Syscall Monitor                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("%s", UI_RESET);
}

void ui_box_top(const char *color, const char *title)
{
    int prefix_len = 3 + (int)strlen(title) + 1; /* "┌─ " + title + " " */
    int dashes = UI_BOX_WIDTH - prefix_len;
    if (dashes < 0)
        dashes = 0;

    printf("%s┌─ %s%s%s ", color, UI_BOLD, title, UI_RESET);
    printf("%s", color);
    repeat_utf8("─", dashes);
    printf("┐%s\n", UI_RESET);
}

void ui_box_bottom(const char *color)
{
    printf("%s└", color);
    repeat_utf8("─", UI_BOX_WIDTH + 1);
    printf("┘%s\n", UI_RESET);
}

void ui_box_row(const char *color, const char *label, const char *fmt, ...)
{
    char value[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(value, sizeof(value), fmt, args);
    va_end(args);

    printf("%s│%s %s%-14s%s: %s%s%s\n",
           color, UI_RESET,
           UI_BOLD, label, UI_RESET,
           UI_WHITE, value, UI_RESET);
}

void ui_box_note(const char *color, const char *text)
{
    printf("%s│%s %s%s%s\n", color, UI_RESET, UI_DIM, text, UI_RESET);
}

void ui_dbox_top(const char *color, const char *title)
{
    int prefix_len = 3 + (int)strlen(title) + 1; /* "╔═ " + title + " " */
    int dashes = UI_BOX_WIDTH - prefix_len;
    if (dashes < 0)
        dashes = 0;

    printf("%s╔═ %s%s%s ", color, UI_BOLD, title, UI_RESET);
    printf("%s", color);
    repeat_utf8("═", dashes);
    printf("╗%s\n", UI_RESET);
}

void ui_dbox_bottom(const char *color)
{
    printf("%s╚", color);
    repeat_utf8("═", UI_BOX_WIDTH + 1);
    printf("╝%s\n", UI_RESET);
}

void ui_dbox_row(const char *color, const char *fmt, ...)
{
    char value[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(value, sizeof(value), fmt, args);
    va_end(args);

    printf("%s║%s  %s%s%s\n", color, UI_RESET, UI_BOLD_WHITE, value, UI_RESET);
}

void ui_error(const char *fmt, ...)
{
    char value[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(value, sizeof(value), fmt, args);
    va_end(args);

    printf("%s[ERROR]%s %s\n", UI_BOLD_RED, UI_RESET, value);
}

void ui_warning(const char *fmt, ...)
{
    char value[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(value, sizeof(value), fmt, args);
    va_end(args);

    printf("%s[WARNING]%s %s\n", UI_BOLD_YELLOW, UI_RESET, value);
}

void ui_info(const char *fmt, ...)
{
    char value[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(value, sizeof(value), fmt, args);
    va_end(args);

    printf("%s[INFO]%s %s\n", UI_BOLD_CYAN, UI_RESET, value);
}

void ui_success(const char *fmt, ...)
{
    char value[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(value, sizeof(value), fmt, args);
    va_end(args);

    printf("%s[OK]%s %s\n", UI_BOLD_GREEN, UI_RESET, value);
}
