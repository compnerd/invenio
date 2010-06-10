/* vim: set et fdm=syntax sts=4 sw=4 ts=4 : */
/**
 * Copyright © 2010 Saleem Abdulrasool <compnerd@compnerd.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 **/

#include "invenio-configuration.h"

#include <gio/gio.h>

#define INVENIO_CONFIGURATION_KEYFILE                   "invenio.cfg"
#define INVENIO_CONFIGURATION_GENERAL                   "general"
#define INVENIO_CONFIGURATION_SEARCH                    "search"

#define INVENIO_CONFIGURATION_MENU_SHORTCUT_KEY         "menu-shortcut"
#define INVENIO_CONFIGURATION_MENU_SHORTCUT_KEY_VALUE   "<ctrl>space"
#define INVENIO_CONFIGURATION_MENU_SHORTCUT_KEY_COMMENT "Search menu shortcut key (default: " G_STRINGIFY (INVENIO_CONFIGURATION_MENU_SHORTCUT_KEY_VALUE) ")"

#define INVENIO_CONFIGURATION_SEARCH_CATEGORIES         "search-categories"
#define INVENIO_CONFIGURATION_SEARCH_CATEGORIES_COMMENT "Categories to get results from"


typedef struct InvenioConfiguration
{
    gboolean         loaded;

    GFile           *file;
    GKeyFile        *keyfile;
    gboolean         dirty;

    struct
    {
        gboolean     category_enabled[INVENIO_CATEGORIES];
    } cache;
} InvenioConfiguration;


static InvenioConfiguration configuration;


static void
_load_defaults (void)
{
    if (! g_key_file_has_key (configuration.keyfile,
                              INVENIO_CONFIGURATION_GENERAL,
                              INVENIO_CONFIGURATION_MENU_SHORTCUT_KEY,
                              NULL))
    {
        g_key_file_set_comment (configuration.keyfile,
                                INVENIO_CONFIGURATION_GENERAL,
                                INVENIO_CONFIGURATION_MENU_SHORTCUT_KEY,
                                INVENIO_CONFIGURATION_MENU_SHORTCUT_KEY_COMMENT,
                                NULL);
        g_key_file_set_string (configuration.keyfile,
                               INVENIO_CONFIGURATION_GENERAL,
                               INVENIO_CONFIGURATION_MENU_SHORTCUT_KEY,
                               INVENIO_CONFIGURATION_MENU_SHORTCUT_KEY_VALUE);
        configuration.dirty = TRUE;
    }

    if (! g_key_file_has_key (configuration.keyfile,
                              INVENIO_CONFIGURATION_SEARCH,
                              INVENIO_CONFIGURATION_SEARCH_CATEGORIES,
                              NULL))
    {
        const gchar **search_categories;
        InvenioCategory category;

        search_categories = g_malloc0 (sizeof (gchar *) * INVENIO_CATEGORIES);

        for (category = (InvenioCategory) 0; category != INVENIO_CATEGORIES; category++)
        {
            search_categories[category] = invenio_category_to_string (category);
            configuration.cache.category_enabled[category] = TRUE;
        }

        g_key_file_set_comment (configuration.keyfile,
                                INVENIO_CONFIGURATION_SEARCH,
                                INVENIO_CONFIGURATION_SEARCH_CATEGORIES,
                                INVENIO_CONFIGURATION_SEARCH_CATEGORIES_COMMENT,
                                NULL);
        g_key_file_set_string_list (configuration.keyfile,
                                    INVENIO_CONFIGURATION_SEARCH,
                                    INVENIO_CONFIGURATION_SEARCH_CATEGORIES,
                                    search_categories,
                                    INVENIO_CATEGORIES);
        configuration.dirty = TRUE;

        g_free (search_categories);
    }
}

void
invenio_configuration_load (void)
{
    gchar **search_categories, **category;
    gchar *directory, *filename;
    GError *error = NULL;
    gsize entries;

    directory = g_build_filename (g_get_user_config_dir (), "invenio", NULL);

    if (! g_file_test (directory, G_FILE_TEST_EXISTS))
    {
        if (g_mkdir_with_parents (directory, 0700) == -1)
        {
            g_critical ("Could not create configuration directory");
            g_free (directory);
            return;
        }
    }

    filename = g_build_filename (directory, INVENIO_CONFIGURATION_KEYFILE, NULL);

    if (! configuration.loaded)
    {
        configuration.loaded = TRUE;
        configuration.file = g_file_new_for_path (filename);
        configuration.keyfile = g_key_file_new ();
    }


    g_key_file_load_from_file (configuration.keyfile, filename,
                               G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS,
                               &error);

    if (error)
        g_error_free (error);


    _load_defaults ();


    search_categories = g_key_file_get_string_list (configuration.keyfile,
                                                    INVENIO_CONFIGURATION_SEARCH,
                                                    INVENIO_CONFIGURATION_SEARCH_CATEGORIES,
                                                    &entries,
                                                    NULL);

    for (category = search_categories; entries && *category; category++, entries--)
        configuration.cache.category_enabled[invenio_category_from_string (*category)] = TRUE;


    g_strfreev (search_categories);

    g_free (filename);
    g_free (directory);
}

gchar *
invenio_configuration_get_menu_shortcut (void)
{
    return g_key_file_get_string (configuration.keyfile,
                                  INVENIO_CONFIGURATION_GENERAL,
                                  INVENIO_CONFIGURATION_MENU_SHORTCUT_KEY,
                                  NULL);
}

gboolean
invenio_configuration_get_search_category (const InvenioCategory category)
{
    return configuration.cache.category_enabled[category];
}

void
invenio_configuration_save (void)
{
    GError *error = NULL;
    gchar *filename, *data;
    gsize size;

    if (! configuration.dirty)
        return;

    data = g_key_file_to_data (configuration.keyfile, &size, &error);
    if (error)
    {
        g_warning ("Unable to save configuration: %s", error->message);
        g_error_free (error);
        return;
    }

    filename = g_file_get_path (configuration.file);

    g_file_set_contents (filename, data, size, &error);
    if (error)
    {
        g_warning ("Unable to save configuration: %s", error->message);
        g_error_free (error);
        /* fall-through */
    }

    g_free (data);
    g_free (filename);
}

