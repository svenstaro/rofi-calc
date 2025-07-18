// rofi-calc
//
// MIT/X11 License
// Copyright (c) 2018 Sven-Hendrik Haase <svenstaro@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <errno.h>
#include <gio/gio.h>
#include <glib.h>
#include <gmodule.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rofi/helper.h>
#include <rofi/mode-private.h>
#include <rofi/mode.h>
#include <rofi/rofi-types.h>

#include <stdint.h>

G_MODULE_EXPORT Mode mode;

typedef struct {
    gboolean no_bold;
    gboolean no_unicode;
    gboolean terse;
    gboolean no_history;
    gboolean no_persist_history;
    gboolean automatic_save_to_history;
    gboolean calc_command_uses_history;
} CALCModeConfig;

// The internal data structure holding the private data of the TEST Mode.
typedef struct {
    char *cmd;
    char *hint_result;
    char *hint_welcome;
    char *calc_error_color;
    char *last_result;
    char *previous_input;
    GPtrArray *history;
    CALCModeConfig config;
} CALCModePrivateData;

// Used in splitting equations into {expression} and {result}.
#define PARENS_LEFT '('
#define PARENS_RIGHT ')'
#define EQUALS_SIGN '='
#define APPROX_SIGN "≈"

// qalc binary name
#define QALC_BINARY_OPTION "-qalc-binary"

// Calc command option
#define CALC_COMMAND_OPTION "calc-command"

// Whether calc command emits a history entry
#define CALC_COMMAND_USES_HISTORY "calc-command-history"

// Option to disable bold results
#define NO_BOLD_OPTION "no-bold"

// Option to disable qalc's unicode mode
#define NO_UNICODE_OPTION "no-unicode"

// Terse option
#define TERSE_OPTION "terse"

// Option to specify result hint
#define HINT_RESULT_OPTION "hint-result"
#define HINT_RESULT_STR "Result: "

// Option to specify welcome hint
#define HINT_WELCOME_OPTION "hint-welcome"
#define HINT_WELCOME_STR "Calculator"

// Option to specify error color
#define CALC_ERROR_COLOR "calc-error-color"
#define CALC_ERROR_COLOR_STR "PaleVioletRed"

// The following keys can be specified in `CALC_COMMAND_FLAG` and
// will be replaced with the left-hand side and right-hand side of
// the equation.
#define EQUATION_LHS_KEY "{expression}"
#define EQUATION_RHS_KEY "{result}"

// History stuff
#define NO_PERSIST_HISTORY_OPTION "no-persist-history"
#define NO_HISTORY_OPTION "no-history"
#define AUTOMATIC_SAVE_TO_HISTORY "automatic-save-to-history"
#define HISTORY_LENGTH 100

// Limit `str` to at most `limit` new lines.
// Returns a new string of either the limited length or the length length.
// However, in both cases, it's a new string.
static gchar *limit_to_n_newlines(gchar *str, uint32_t limit) {
    uint32_t newlines = 0;
    uint32_t last_index = 0;
    uint32_t string_length = strlen(str);

    while (last_index < string_length) {
        if (str[last_index] == '\n') {
            newlines++;
        }

        if (newlines >= limit) {
            gchar *limited_str = g_strndup(str, last_index);
            return limited_str;
        }

        last_index++;
    }

    return g_strdup(str);
}

// Append `input` to history.
static void append_str_to_history(gchar *input) {
    GError *error = NULL;
    gchar *history_dir = g_build_filename(g_get_user_data_dir(), "rofi", NULL);
    gchar *history_file =
        g_build_filename(history_dir, "rofi_calc_history", NULL);
    gchar *history_contents;
    gboolean old_history_was_read = FALSE;

    g_mkdir_with_parents(history_dir, 0755);

    if (g_file_test(history_file,
                    G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
        g_file_get_contents(history_file, &history_contents, NULL, &error);
        old_history_was_read = TRUE;

        if (error != NULL) {
            g_error("Error while reading the history file: %s", error->message);
            g_error_free(error);
        }
    } else {
        // Empty history, initialize it.
        history_contents = "";
    }

    gchar *new_history = g_strjoin("\n", history_contents, input, NULL);
    g_strstrip(new_history);

    gchar *limited_str = g_strreverse(
        limit_to_n_newlines(g_strreverse(new_history), HISTORY_LENGTH));

    g_file_set_contents(history_file, limited_str, -1, &error);

    if (error != NULL) {
        g_error("Error while writing the history file: %s", error->message);
        g_error_free(error);
    }

    g_free(limited_str);
    g_free(new_history);
    if (old_history_was_read) {
        g_free(history_contents);
    }
    g_free(history_file);
    g_free(history_dir);
}

// Count number of new lines in a string.
static uint32_t get_number_of_newlines(gchar *string, gsize length) {
    uint32_t lines = 0;
    for (uint32_t i = 0; i < length; i++) {
        if (string[i] == '\n') {
            lines++;
        }
    }

    return lines;
}

// Delete a certain line number from history.
static void delete_line_from_history(uint32_t line) {
    GError *error = NULL;
    gchar *history_dir = g_build_filename(g_get_user_data_dir(), "rofi", NULL);
    gchar *history_file =
        g_build_filename(history_dir, "rofi_calc_history", NULL);
    gchar *history_contents;
    gsize history_length;
    gboolean old_history_was_read = FALSE;

    if (g_file_test(history_file,
                    G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
        g_file_get_contents(history_file, &history_contents, &history_length,
                            &error);
        old_history_was_read = TRUE;

        if (error != NULL) {
            g_error("Error while reading the history file: %s", error->message);
            g_error_free(error);
        }
    } else {
        // Empty history, do nothing and exit early.
        return;
    }

    uint32_t newlines =
        get_number_of_newlines(history_contents, history_length);
    GString *new_history = g_string_new("");
    uint32_t current_line = 0;
    uint32_t line_to_delete = newlines - line;
    for (gsize c = 0; c < history_length; c++) {
        if (history_contents[c] == '\n') {
            current_line++;
        }

        // Skip any copying of history if the line we're on is the line we're
        // trying to get rid of.
        if (current_line == line_to_delete) {
            continue;
        }

        new_history = g_string_append_c(new_history, history_contents[c]);
    }

    gchar *new_history_str = g_string_free(new_history, FALSE);
    g_file_set_contents(history_file, new_history_str, -1, &error);

    if (error != NULL) {
        g_error("Error while writing the history file: %s", error->message);
        g_error_free(error);
    }

    g_free(new_history_str);
    if (old_history_was_read) {
        g_free(history_contents);
    }
    g_free(history_file);
    g_free(history_dir);
}

// sets config values from rofi config file and command line
// command line options have higher priority than config file
static void set_config(Mode *sw) {
    CALCModePrivateData *pd = (CALCModePrivateData *)mode_get_private_data(sw);
    ConfigEntry *config_file = rofi_config_find_widget(sw->name, NULL, TRUE);

    pd->config.no_bold = FALSE;
    pd->config.no_unicode = FALSE;
    pd->config.terse = FALSE;
    pd->config.no_history = FALSE;
    pd->config.no_persist_history = FALSE;
    pd->config.automatic_save_to_history = FALSE;
    pd->config.calc_command_uses_history = FALSE;

    pd->hint_result = HINT_RESULT_STR;
    pd->hint_welcome = HINT_WELCOME_STR;
    pd->calc_error_color = CALC_ERROR_COLOR_STR;

    if (config_file != NULL) {
        Property *no_bold = rofi_theme_find_property(config_file, P_BOOLEAN,
                                                     NO_BOLD_OPTION, TRUE);
        if (no_bold != NULL && (no_bold->type == P_BOOLEAN)) {
            pd->config.no_bold = no_bold->value.b;
        }

        Property *terse = rofi_theme_find_property(config_file, P_BOOLEAN,
                                                   TERSE_OPTION, TRUE);
        if (terse != NULL && (terse->type == P_BOOLEAN)) {
            pd->config.terse = terse->value.b;
        }

        Property *no_unicode = rofi_theme_find_property(
            config_file, P_BOOLEAN, NO_UNICODE_OPTION, TRUE);
        if (no_unicode != NULL && (no_unicode->type == P_BOOLEAN)) {
            pd->config.no_unicode = no_unicode->value.b;
        }

        Property *cmd_option = rofi_theme_find_property(
            config_file, P_STRING, CALC_COMMAND_OPTION, TRUE);
        if (cmd_option != NULL &&
            (cmd_option->type == P_STRING && cmd_option->value.s)) {
            pd->cmd = g_strdup(cmd_option->value.s);
        }

        Property *hint_result_option = rofi_theme_find_property(
            config_file, P_STRING, HINT_RESULT_OPTION, TRUE);
        if (hint_result_option != NULL &&
            (hint_result_option->type == P_STRING &&
             hint_result_option->value.s)) {
            pd->hint_result = g_strdup(hint_result_option->value.s);
        }

        Property *hint_welcome_option = rofi_theme_find_property(
            config_file, P_STRING, HINT_WELCOME_OPTION, TRUE);
        if (hint_welcome_option != NULL &&
            (hint_welcome_option->type == P_STRING &&
             hint_welcome_option->value.s)) {
            pd->hint_welcome = g_strdup(hint_welcome_option->value.s);
        }

        Property *calc_error_color_option = rofi_theme_find_property(
            config_file, P_STRING, CALC_ERROR_COLOR, TRUE);
        if (calc_error_color_option != NULL &&
            (calc_error_color_option->type == P_STRING &&
             calc_error_color_option->value.s)) {
            pd->calc_error_color = g_strdup(calc_error_color_option->value.s);
        }

        Property *no_history = rofi_theme_find_property(
            config_file, P_BOOLEAN, NO_HISTORY_OPTION, TRUE);
        if (no_history != NULL && (no_history->type == P_BOOLEAN)) {
            pd->config.no_history = no_history->value.b;
        }

        Property *no_persist_history = rofi_theme_find_property(
            config_file, P_BOOLEAN, NO_PERSIST_HISTORY_OPTION, TRUE);
        if (no_persist_history != NULL &&
            (no_persist_history->type == P_BOOLEAN)) {
            pd->config.no_persist_history = no_persist_history->value.b;
        }

        Property *automatic_save_to_history = rofi_theme_find_property(
            config_file, P_BOOLEAN, AUTOMATIC_SAVE_TO_HISTORY, TRUE);
        if (automatic_save_to_history != NULL &&
            (automatic_save_to_history->type == P_BOOLEAN)) {
            pd->config.automatic_save_to_history =
                automatic_save_to_history->value.b;
        }

        Property *calc_command_uses_history = rofi_theme_find_property(
            config_file, P_BOOLEAN, CALC_COMMAND_USES_HISTORY, TRUE);
        if (calc_command_uses_history != NULL &&
            (calc_command_uses_history->type == P_BOOLEAN)) {
            pd->config.calc_command_uses_history =
                calc_command_uses_history->value.b;
        }
    }

    // command line options
    if (find_arg("-" NO_BOLD_OPTION) > -1)
        pd->config.no_bold = TRUE;

    if (find_arg("-" TERSE_OPTION) > -1)
        pd->config.terse = TRUE;

    if (find_arg("-" NO_UNICODE_OPTION) > -1)
        pd->config.no_unicode = TRUE;

    if (find_arg("-" NO_HISTORY_OPTION) > -1)
        pd->config.no_history = TRUE;

    if (find_arg("-" NO_PERSIST_HISTORY_OPTION) > -1)
        pd->config.no_persist_history = TRUE;

    if (find_arg("-" AUTOMATIC_SAVE_TO_HISTORY) > -1)
        pd->config.automatic_save_to_history = TRUE;

    if (find_arg("-" CALC_COMMAND_USES_HISTORY) > -1)
        pd->config.calc_command_uses_history = TRUE;

    char *cmd = NULL;
    if (find_arg_str("-" CALC_COMMAND_OPTION, &cmd)) {
        pd->cmd = g_strdup(cmd);
    }

    char *hint_result = NULL;
    if (find_arg_str("-" HINT_RESULT_OPTION, &hint_result)) {
        pd->hint_result = g_strdup(hint_result);
    }

    char *hint_welcome = NULL;
    if (find_arg_str("-" HINT_WELCOME_OPTION, &hint_welcome)) {
        pd->hint_welcome = g_strdup(hint_welcome);
    }

    char *calc_error_color = NULL;
    if (find_arg_str(CALC_ERROR_COLOR, &calc_error_color)) {
        pd->calc_error_color = g_strdup(calc_error_color);
    }
}

// Get the entries to display.
// This gets called on plugin initialization.
static void get_calc(Mode *sw) {
    CALCModePrivateData *pd = (CALCModePrivateData *)mode_get_private_data(sw);
    pd->last_result = g_strdup("");
    pd->history = g_ptr_array_new();
    pd->previous_input = g_strdup(""); // providing initial value

    set_config(sw);

    if (!pd->config.no_history && !pd->config.no_persist_history) {
        // Load old history if it exists.
        GError *error = NULL;
        gchar *history_file = g_build_filename(g_get_user_data_dir(), "rofi",
                                               "rofi_calc_history", NULL);
        gchar *history_contents;

        if (g_file_test(history_file,
                        G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
            g_file_get_contents(history_file, &history_contents, NULL, &error);

            if (error != NULL) {
                g_error("Error while reading the history file: %s",
                        error->message);
                g_error_free(error);
            }

            char *newline = strtok(history_contents, "\n");

            while (newline != NULL) {
                g_ptr_array_add(pd->history, newline);

                newline = strtok(NULL, "\n");
            }
        }

        g_free(history_file);
    }
}

// Called on startup when enabled (in modi list)
static int calc_mode_init(Mode *sw) {
    if (mode_get_private_data(sw) == NULL) {
        CALCModePrivateData *pd = g_malloc0(sizeof(*pd));
        mode_set_private_data(sw, (void *)pd);
        // Load content.
        get_calc(sw);
    }

    return TRUE;
}

static unsigned int calc_mode_get_num_entries(const Mode *sw) {
    const CALCModePrivateData *pd =
        (const CALCModePrivateData *)mode_get_private_data(sw);

    // Add +1 because we put a static message into the history array as
    // well.
    return pd->history->len + 1;
}

static gboolean is_error_string(char *str) {

    if (g_strrstr(str, "warning:") != NULL ||
        g_strrstr(str, "error:") != NULL) {
        return TRUE;
    }
    return FALSE;
}

static int get_real_history_index(GPtrArray *history,
                                  unsigned int selected_line) {
    return history->len - selected_line;
}

static void append_last_result_to_history(CALCModePrivateData *pd) {
    if (!is_error_string(pd->last_result) && strlen(pd->last_result) > 0) {
        char *history_entry = g_strdup_printf("%s", pd->last_result);

        // Replace newlines with semicolons so one entry isn't split into
        // multiple entries
        for (unsigned int i = 0; i < strlen(history_entry); i++) {
            if (history_entry[i] == '\n') {
                history_entry[i] = ';';
            }
        }

        g_ptr_array_add(pd->history, (gpointer)history_entry);
        if (!pd->config.no_persist_history) {
            append_str_to_history(history_entry);
        }
    }
}

// Split the equation result into the left (expression) and right (result)
// side of the equals sign.
//
// Note that both sides can themselves contain equals sign, consider the
// simple example of `20x + 40 = 100`. This means we cannot naively split on
// the '=' character.
static char **split_equation(Mode *sw, char *string) {
    CALCModePrivateData *pd = (CALCModePrivateData *)mode_get_private_data(sw);
    char **result = malloc(2 * sizeof(char *));

    if (pd->config.terse) {
        result[0] = NULL;
        result[1] = g_strdup(string); // with -terse, string _is_ the result
        return result;
    }

    int parens_depth = 0;
    char *curr = string + strlen(string);
    int delimiter_len = 0;

    // Iterate through and track our level of nestedness, stopping when
    // we've hit an equals sign not inside other parentheses.
    // At this point we can set the NULL character to split the string
    // into `string` and `curr + delimiter_len`.
    while (curr != string) {
        curr--;
        if (*curr == PARENS_RIGHT) {
            parens_depth++;
        } else if (*curr == PARENS_LEFT) {
            parens_depth--;
        } else if (parens_depth == 0) {
            if (*curr == EQUALS_SIGN) {
                delimiter_len = 1;
                break;
            } else if (!strncmp(curr, APPROX_SIGN, strlen(APPROX_SIGN))) {
                delimiter_len = strlen(APPROX_SIGN);
                break;
            }
        }
    }

    if (curr == string) {
        // No equals signs were found. Shouldn't happen, but if it does
        // treat the entire expression as the result.
        result[0] = NULL;
        result[1] = g_strdup(string);
    } else {
        // We found an equals sign; set it to null to split the string in
        // two.
        *curr = '\0';

        // Strip trailing whitespace with `g_strchomp()` from the left.
        // Strip leading whitespace with `g_strchug()` from the right.
        result[0] = g_strchomp(string);
        result[1] = g_strchug(curr + delimiter_len);
    }

    return result;
}

static void execsh(Mode *sw, char *cmd, char *entry) {
    // If no command was provided, simply print the entry
    if (cmd == NULL) {
        printf("%s\n", entry);
        return;
    }

    // Otherwise, we will execute -calc-command
    char **parts = split_equation(sw, entry);
    char *user_cmd = helper_string_replace_if_exists(
        cmd, EQUATION_LHS_KEY, parts[0], EQUATION_RHS_KEY, parts[1], NULL);
    g_free(parts);

    // don't escape these utf-8 runes which appear in qalc output, the escape
    // sequences are not recognized by shell (#108)
    static const char *unescaped_chars = "·−×√²³⊻↊↋¬°";
    gchar *escaped_cmd = g_strescape(user_cmd, unescaped_chars);
    gchar *complete_cmd = g_strdup_printf("/bin/sh -c \"%s\"", escaped_cmd);
    g_free(user_cmd);
    g_free(escaped_cmd);

    helper_execute_command(NULL, complete_cmd, FALSE, NULL);
    g_free(complete_cmd);
}

static ModeMode calc_mode_result(Mode *sw, int menu_entry,
                                 G_GNUC_UNUSED char **input,
                                 unsigned int selected_line) {
    ModeMode retv = MODE_EXIT;
    CALCModePrivateData *pd = (CALCModePrivateData *)mode_get_private_data(sw);
    if (menu_entry & MENU_CUSTOM_COMMAND) {
        retv = (menu_entry & MENU_LOWER_MASK);
    } else if ((menu_entry & MENU_OK) &&
               (selected_line == 0 && !pd->config.no_history)) {
        append_last_result_to_history(pd);
        retv = RELOAD_DIALOG;
    } else if ((menu_entry & MENU_OK) &&
               (selected_line > 0 || pd->config.no_history)) {
        char *entry;
        if (pd->config.no_history)
            entry = pd->last_result;
        else
            entry = g_ptr_array_index(
                pd->history,
                get_real_history_index(pd->history, selected_line));

        execsh(sw, pd->cmd, entry);
        retv = MODE_EXIT;
    } else if (menu_entry & MENU_CUSTOM_INPUT) {
        if (!is_error_string(pd->last_result) && strlen(pd->last_result) > 0) {
            if (!pd->config.no_history &&
                find_arg(CALC_COMMAND_USES_HISTORY) != -1) {
                char *history_entry = g_strdup_printf("%s", pd->last_result);
                g_ptr_array_add(pd->history, (gpointer)history_entry);
                if (!pd->config.no_persist_history) {
                    append_str_to_history(history_entry);
                }
            }

            execsh(sw, pd->cmd, pd->last_result);
            retv = MODE_EXIT;
        } else {
            retv = RELOAD_DIALOG;
        }
    } else if (menu_entry & MENU_ENTRY_DELETE) {
        if (selected_line > 0) {
            g_ptr_array_remove_index(
                pd->history,
                get_real_history_index(pd->history, selected_line));
            if (!pd->config.no_persist_history && !pd->config.no_history) {
                delete_line_from_history(selected_line - 1);
            }
        }
        retv = RELOAD_DIALOG;
    }

    g_debug("selected_line: %i", selected_line);
    g_debug("ding: %x", menu_entry);
    g_debug("MENU_OK: %x", menu_entry & MENU_OK);
    g_debug("MENU_CANCEL: %x", menu_entry & MENU_CANCEL);
    g_debug("MENU_CUSTOM_INPUT: %x", menu_entry & MENU_CUSTOM_INPUT);
    g_debug("MENU_ENTRY_DELETE: %x", menu_entry & MENU_ENTRY_DELETE);
    g_debug("MENU_CUSTOM_COMMAND: %x", menu_entry & MENU_CUSTOM_COMMAND);
    g_debug("MENU_CUSTOM_ACTION: %x", menu_entry & MENU_CUSTOM_ACTION);
    g_debug("MENU_LOWER_MASK: %x", menu_entry & MENU_LOWER_MASK);
    return retv;
}

static void calc_mode_destroy(Mode *sw) {
    CALCModePrivateData *pd = (CALCModePrivateData *)mode_get_private_data(sw);
    if (pd->config.automatic_save_to_history) {
        append_last_result_to_history(pd);
    }

    if (pd != NULL) {
        g_free(pd);
        mode_set_private_data(sw, NULL);
    }
}

static char *calc_get_display_value(const Mode *sw, unsigned int selected_line,
                                    G_GNUC_UNUSED int *state,
                                    G_GNUC_UNUSED GList **attr_list,
                                    int get_entry) {
    CALCModePrivateData *pd = (CALCModePrivateData *)mode_get_private_data(sw);

    if (!get_entry) {
        return NULL;
    }

    if (selected_line == 0) {
        if (!pd->config.no_history)
            return g_strdup("Add to history");
        else
            return g_strdup("");
    }
    unsigned int real_index =
        get_real_history_index(pd->history, selected_line);
    return g_strdup(g_ptr_array_index(pd->history, real_index));
}

static int calc_token_match(G_GNUC_UNUSED const Mode *sw,
                            G_GNUC_UNUSED rofi_int_matcher **tokens,
                            G_GNUC_UNUSED unsigned int index) {
    return TRUE;
}

// It's a hacky way of making rofi show new window titles.
extern void rofi_view_reload(void);

static void process_cb(GObject *source_object, GAsyncResult *res,
                       gpointer user_data) {
    GError *error = NULL;
    GSubprocess *process = (GSubprocess *)source_object;
    GInputStream *stdout_stream = g_subprocess_get_stdout_pipe(process);
    char **last_result = (char **)user_data;

    g_subprocess_wait_check_finish(process, res, &error);

    if (error != NULL) {
        // With qalculate >= 5.0.0, exit status 1 can mean bad (or
        // incomplete) input
        if (error->domain != G_SPAWN_EXIT_ERROR || error->code != 1) {
            g_error("Process errored with: %s", error->message);
        }
        g_error_free(error);
        error = NULL;
    }

    gsize bytes_read = 0;
    unsigned int stdout_bufsize = 4096;
    char stdout_buf[stdout_bufsize];
    g_input_stream_read_all(stdout_stream, stdout_buf, stdout_bufsize,
                            &bytes_read, NULL, &error);

    if (error != NULL) {
        g_error("Process errored with: %s", error->message);
        g_error_free(error);
        error = NULL;
    }

    *last_result = g_strndup(stdout_buf, bytes_read - 1);
    g_input_stream_close(stdout_stream, NULL, &error);

    if (error != NULL) {
        g_error("Process errored with: %s", error->message);
        g_error_free(error);
        error = NULL;
    }

    rofi_view_reload();
}

static char *calc_preprocess_input(Mode *sw, const char *input) {
    GError *error = NULL;
    CALCModePrivateData *pd = (CALCModePrivateData *)mode_get_private_data(sw);

    if (strcmp(input, pd->previous_input) == 0) {
        return g_strdup(pd->previous_input);
    }

    g_free(pd->previous_input);
    pd->previous_input = g_strdup(input);

    char *qalc_binary = "qalc";
    if (find_arg(QALC_BINARY_OPTION) >= 0) {
        find_arg_str(QALC_BINARY_OPTION, &qalc_binary);
    }

    // Build array of strings that is later fed into a subprocess to actually
    // start qalc with proper parameters.
    GPtrArray *argv = g_ptr_array_new();
    g_ptr_array_add(argv, qalc_binary);
    g_ptr_array_add(argv, "-s");
    g_ptr_array_add(argv, "update_exchange_rates 1days");
    if (pd->config.terse) {
        g_ptr_array_add(argv, "-t");
    }
    if (!pd->config.no_unicode) {
        g_ptr_array_add(argv, "+u8");
    }
    g_ptr_array_add(argv, (gchar *)input);
    g_ptr_array_add(argv, NULL);

    GSubprocess *process = g_subprocess_newv(
        (const gchar **)(argv->pdata),
        G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_MERGE,
        &error);
    g_ptr_array_free(argv, TRUE);

    if (error != NULL) {
        g_error("Spawning child failed: %s", error->message);
        g_error_free(error);
    }

    g_subprocess_wait_check_async(process, NULL, process_cb,
                                  (gpointer)&pd->last_result);

    return g_strdup(input);
}

static char *calc_get_message(const Mode *sw) {
    CALCModePrivateData *pd = (CALCModePrivateData *)mode_get_private_data(sw);
    if (is_error_string(pd->last_result)) {
        return g_markup_printf_escaped("<span foreground='%s'>%s</span>",
                                       pd->calc_error_color, pd->last_result);
    }

    if (*pd->last_result) {

        if (!pd->config.no_bold)
            return g_markup_printf_escaped("%s<b>%s</b>", pd->hint_result,
                                           pd->last_result);
        else
            return g_markup_printf_escaped("%s%s", pd->hint_result,
                                           pd->last_result);
    } else {
        return g_markup_printf_escaped("%s", pd->hint_welcome);
    }
}

Mode mode = {
    .abi_version = ABI_VERSION,
    .name = "calc",
    .cfg_name_key = "display-calc",
    ._init = calc_mode_init,
    ._get_num_entries = calc_mode_get_num_entries,
    ._result = calc_mode_result,
    ._destroy = calc_mode_destroy,
    ._token_match = calc_token_match,
    ._get_display_value = calc_get_display_value,
    ._get_message = calc_get_message,
    ._preprocess_input = calc_preprocess_input,
    .private_data = NULL,
    .free = NULL,
#if ABI_VERSION >= 7u
    // Only recent rofi versions have this so we'll have to use this ifdef
    .type = MODE_TYPE_SWITCHER,
#endif
};
