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
    char *last_result;
    unsigned int array_length;
} MYPLUGINModePrivateData;


static void get_myplugin (  Mode *sw )
{
    /**
     * Get the entries to display.
     * this gets called on plugin initialization.
     */
    MYPLUGINModePrivateData *pd = (MYPLUGINModePrivateData *) mode_get_private_data ( sw );
    pd->array_length = 1;
    pd->last_result = g_strdup("");
}


static int myplugin_mode_init ( Mode *sw )
{
    /**
     * Called on startup when enabled (in modi list)
     */
    if ( mode_get_private_data ( sw ) == NULL ) {
        MYPLUGINModePrivateData *pd = g_malloc0 ( sizeof ( *pd ) );
        mode_set_private_data ( sw, (void *) pd );
        // Load content.
        get_myplugin ( sw );
    }
    return TRUE;
}
static unsigned int myplugin_mode_get_num_entries ( const Mode *sw )
{
    const MYPLUGINModePrivateData *pd = (const MYPLUGINModePrivateData *) mode_get_private_data ( sw );
    return pd->array_length;
}

static ModeMode myplugin_mode_result ( Mode *sw, int mretv, char **input, unsigned int selected_line )
{
    ModeMode           retv  = MODE_EXIT;
    MYPLUGINModePrivateData *pd = (MYPLUGINModePrivateData *) mode_get_private_data ( sw );
    if ( mretv & MENU_NEXT ) {
        retv = NEXT_DIALOG;
    } else if ( mretv & MENU_PREVIOUS ) {
        retv = PREVIOUS_DIALOG;
    } else if ( mretv & MENU_QUICK_SWITCH ) {
        retv = ( mretv & MENU_LOWER_MASK );
    } else if ( ( mretv & MENU_OK ) ) {
        retv = RELOAD_DIALOG;
    } else if ( ( mretv & MENU_ENTRY_DELETE ) == MENU_ENTRY_DELETE ) {
        retv = RELOAD_DIALOG;
    }
    g_message("result");
    return retv;
}

static void myplugin_mode_destroy ( Mode *sw )
{
    MYPLUGINModePrivateData *pd = (MYPLUGINModePrivateData *) mode_get_private_data ( sw );
    if ( pd != NULL ) {
        g_free ( pd );
        mode_set_private_data ( sw, NULL );
    }
}

static char *_get_display_value ( const Mode *sw, unsigned int selected_line, G_GNUC_UNUSED int *state, G_GNUC_UNUSED GList **attr_list, int get_entry )
{
    MYPLUGINModePrivateData *pd = (MYPLUGINModePrivateData *) mode_get_private_data ( sw );

    // Only return the string if requested, otherwise only set state.
    return get_entry ? g_strdup(pd->last_result): NULL;
}

/**
 * @param sw The mode object.
 * @param tokens The tokens to match against.
 * @param index  The index in this plugin to match against.
 *
 * Match the entry.
 *
 * @param returns try when a match.
 */
static int myplugin_token_match ( const Mode *sw, rofi_int_matcher **tokens, unsigned int index )
{
    MYPLUGINModePrivateData *pd = (MYPLUGINModePrivateData *) mode_get_private_data ( sw );

    // Call default matching function.
    return helper_token_match ( tokens, pd->last_result);
}

static void process_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    g_autoptr(GError) error = NULL;
    MYPLUGINModePrivateData *pd = (MYPLUGINModePrivateData *) mode_get_private_data ( user_data );
    g_message("cb");
    GSubprocess* process = (GSubprocess*) source_object;
    g_subprocess_wait_finish(process, res, &error);

    if (error != NULL)
    {
        g_error ("Process errored with: %s", error->message);
    }
    gint exit_status = g_subprocess_get_exit_status(process);
    g_message("Exited with status: ", exit_status);
}

static char * _preprocess_input ( Mode *sw, const char *input )
{
    g_autoptr(GError) error = NULL;
    g_message("current input: %s", input);
    MYPLUGINModePrivateData *pd = (MYPLUGINModePrivateData *) mode_get_private_data ( sw );

    const gchar * const argv[] = { "/usr/bin/qalc", "+u8", "-s", "update_exchange_rates 1days", input, NULL };
    GSubprocess * process = g_subprocess_newv(argv, G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error);

    if (error != NULL)
    {
        g_error ("Spawning child failed: %s", error->message);
    }

    GInputStream * p_stdout = g_subprocess_get_stdout_pipe(process);
    g_autoptr(GString) stdout_str = g_string_new(NULL);

    char stdout_buf;
    while (g_input_stream_read(p_stdout, &stdout_buf, 1, NULL, &error) != 0) {
        if (error != NULL)
        {
            g_error ("Error reading stdout: %s", error->message);
        }

        g_string_append_c(stdout_str, stdout_buf);
    }
    g_message("%s", stdout_str->str);
    pd->last_result = g_strdup(stdout_str->str);

    g_subprocess_wait_async(process, NULL, process_cb, sw);

    return g_strdup ( input );
}


Mode mode =
{
    .abi_version        = ABI_VERSION,
    .name               = "calc",
    .cfg_name_key       = "display-myplugin",
    ._init              = myplugin_mode_init,
    ._get_num_entries   = myplugin_mode_get_num_entries,
    ._result            = myplugin_mode_result,
    ._destroy           = myplugin_mode_destroy,
    ._token_match       = myplugin_token_match,
    ._get_display_value = _get_display_value,
    ._get_message       = NULL,
    ._get_completion    = NULL,
    ._preprocess_input  = _preprocess_input,
    .private_data       = NULL,
    .free               = NULL,
};
