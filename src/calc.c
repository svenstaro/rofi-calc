/**
 * rofi-plugin-template
 *
 * MIT/X11 License
 * Copyright (c) 2017 Qball Cow <qball@gmpclient.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <gmodule.h>
#include <gio/gio.h>

#include <rofi/mode.h>
#include <rofi/helper.h>
#include <rofi/mode-private.h>

#include <stdint.h>

G_MODULE_EXPORT Mode mode;

/**
 * The internal data structure holding the private data of the TEST Mode.
 */
typedef struct
{
    char* last_result;
    unsigned int array_length;
    GPtrArray* history;
} CALCModePrivateData;

static void get_calc(Mode* sw)
{
    /**
     * Get the entries to display.
     * this gets called on plugin initialization.
     */
    CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);
    pd->array_length = 1;
    pd->last_result = g_strdup("");
    pd->history = g_ptr_array_new();
}

static int calc_mode_init(Mode* sw)
{
    /**
     * Called on startup when enabled (in modi list)
     */
    if (mode_get_private_data(sw) == NULL) {
        CALCModePrivateData* pd = g_malloc0(sizeof(*pd));
        mode_set_private_data(sw, (void*)pd);
        // Load content.
        get_calc(sw);
    }
    return TRUE;
}
static unsigned int calc_mode_get_num_entries(const Mode* sw)
{
    const CALCModePrivateData* pd = (const CALCModePrivateData*)mode_get_private_data(sw);
    return pd->array_length;
}

static ModeMode calc_mode_result(Mode* sw, int mretv, char** input, unsigned int selected_line)
{
    ModeMode retv = MODE_EXIT;
    CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);
    if (mretv & MENU_NEXT) {
        g_message("next");
        retv = NEXT_DIALOG;
    } else if (mretv & MENU_PREVIOUS) {
        g_message("previous");
        retv = PREVIOUS_DIALOG;
    } else if (mretv & MENU_QUICK_SWITCH) {
        g_message("switch");
        retv = (mretv & MENU_LOWER_MASK);
    } else if ((mretv & MENU_OK)) {
        g_message("enter");
        g_ptr_array_add(pd->history, (gpointer) pd->last_result);
        retv = RELOAD_DIALOG;
    } else if ((mretv & MENU_ENTRY_DELETE) == MENU_ENTRY_DELETE) {
        g_message("delete");
        retv = RELOAD_DIALOG;
    }
    return retv;
}

static void calc_mode_destroy(Mode* sw)
{
    CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);
    if (pd != NULL) {
        g_free(pd);
        mode_set_private_data(sw, NULL);
    }
}

static char* calc_get_display_value(const Mode* sw, unsigned int selected_line, int* state, G_GNUC_UNUSED GList** attr_list, int get_entry)
{
    CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);

    if (!get_entry) {
        return NULL;
    }
    g_message("selected line: %i", selected_line);
    /* char* lol = g_ptr_array_index(pd->history, selected_line); */
    /* g_message("array: %s", lol); */
    return g_strdup("lol");
    return g_strdup(g_ptr_array_index(pd->history, selected_line));
}

static int calc_token_match(const Mode* sw, rofi_int_matcher** tokens, unsigned int index)
{
    /* CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw); */
    return TRUE;
}

static void process_cb(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    g_autoptr(GError) error = NULL;
    CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(user_data);
    GSubprocess* process = (GSubprocess*)source_object;
    g_subprocess_wait_check_finish(process, res, &error);

    if (error != NULL) {
        g_error("Process errored with: %s", error->message);
    }
}

// It's a hacky way of making rofi show new window titles.
extern void rofi_view_reload(void);

static char* calc_preprocess_input(Mode* sw, const char* input)
{
    g_autoptr(GError) error = NULL;
    CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);

    const gchar* const argv[] = { "/usr/bin/qalc", "+u8", "-s", "update_exchange_rates 1days", input, NULL };
    GSubprocess* process = g_subprocess_newv(argv, G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_MERGE, &error);

    if (error != NULL) {
        g_error("Spawning child failed: %s", error->message);
    }

    GInputStream* p_stdout = g_subprocess_get_stdout_pipe(process);
    g_autoptr(GString) stdout_str = g_string_new(NULL);

    char stdout_buf;
    while (g_input_stream_read(p_stdout, &stdout_buf, 1, NULL, &error) != 0) {
        if (error != NULL) {
            g_error("Error reading stdout: %s", error->message);
        }

        // We only want to get the first line.
        if(stdout_buf == '\n') {
            break;
        }

        g_string_append_c(stdout_str, stdout_buf);

    }
    pd->last_result = g_strdup(stdout_str->str);

    g_subprocess_wait_check_async(process, NULL, process_cb, sw);

    rofi_view_reload();
    return g_strdup(input);
}

static char *calc_get_message ( const Mode *sw )
{
    CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);
    if (g_strrstr(pd->last_result, "warning:") != NULL || g_strrstr(pd->last_result, "error:") != NULL) {
        return g_markup_printf_escaped("<span foreground='PaleVioletRed'>%s</span>", pd->last_result);
    }
    return g_markup_printf_escaped("<b>%s</b>", pd->last_result);
}

Mode mode =
{
    .abi_version        = ABI_VERSION,
    .name               = "calc",
    .cfg_name_key       = "display-calc",
    ._init              = calc_mode_init,
    ._get_num_entries   = calc_mode_get_num_entries,
    ._result            = calc_mode_result,
    ._destroy           = calc_mode_destroy,
    ._token_match       = calc_token_match,
    ._get_display_value = calc_get_display_value,
    ._get_message       = calc_get_message,
    ._preprocess_input  = calc_preprocess_input,
    .private_data       = NULL,
    .free               = NULL,
};
