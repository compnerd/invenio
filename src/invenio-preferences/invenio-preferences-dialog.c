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

#include "invenio-preferences-dialog.h"

#include "libinvenio/invenio-category.h"
#include "libinvenio/invenio-configuration.h"


typedef enum InvenioPreferencesCategoryColumn
{
    INVENIO_PREFERENCES_CATEGORY_COLUMN_INDEX,
    INVENIO_PREFERENCES_CATEGORY_COLUMN_ENABLED,
    INVENIO_PREFERENCES_CATEGORY_COLUMN_ICON,
    INVENIO_PREFERENCES_CATEGORY_COLUMN_CATEGORY,
    INVENIO_PREFERENCES_CATEGORY_COLUMNS,
} InvenioPreferencesCategoryColumn;


static const GType InvenioPreferencesCategoryColumnType[INVENIO_PREFERENCES_CATEGORY_COLUMNS] =
{
    [INVENIO_PREFERENCES_CATEGORY_COLUMN_INDEX]     = G_TYPE_INT,
    [INVENIO_PREFERENCES_CATEGORY_COLUMN_ENABLED]   = G_TYPE_BOOLEAN,
    [INVENIO_PREFERENCES_CATEGORY_COLUMN_ICON]      = G_TYPE_OBJECT,
    [INVENIO_PREFERENCES_CATEGORY_COLUMN_CATEGORY]  = G_TYPE_INT,
};

#define COLUMN_TYPE(column)                         (InvenioPreferencesCategoryColumnType[(column)])


typedef struct InvenioPreferences
{
    GtkWidget       *dialog;

    /* Category */
    GtkListStore    *model;
    GtkWidget       *view;

    /* Keyboard Shortcut */
    GtkWidget       *keyboard_shortcut_enabled;
    GtkWidget       *keyboard_shortcut;
} InvenioPreferences;


static gboolean
_closed (GtkWidget  *widget,
         GdkEvent   *event,
         gpointer    user_data)
{
    invenio_configuration_save ();
    gtk_main_quit ();
    return TRUE;
}

static void
_category_enabled_toggled (GtkCellRendererToggle    *cell_renderer,
                           gchar                    *path_str,
                           gpointer                  user_data)
{
    InvenioPreferences *preferences;
    GtkTreePath *path;
    GtkTreeIter iter;

    g_return_if_fail (user_data);

    preferences = (InvenioPreferences *) user_data;

    path = gtk_tree_path_new_from_string (path_str);
    if (gtk_tree_model_get_iter (GTK_TREE_MODEL (preferences->model), &iter, path))
    {
        gboolean active;
        gtk_tree_model_get (GTK_TREE_MODEL (preferences->model), &iter,
                            INVENIO_PREFERENCES_CATEGORY_COLUMN_ENABLED, &active, -1);
        gtk_list_store_set (GTK_LIST_STORE (preferences->model), &iter,
                            INVENIO_PREFERENCES_CATEGORY_COLUMN_ENABLED, active ^ TRUE,
                            -1);
    }
}

static void
_icon_cell_data (GtkTreeViewColumn  *tree_column,
                 GtkCellRenderer    *cell,
                 GtkTreeModel       *model,
                 GtkTreeIter        *iter,
                 gpointer            data)
{
    InvenioPreferences *preferences;
    InvenioCategory category;

    g_return_if_fail (data);

    preferences = (InvenioPreferences *) data;

    gtk_tree_model_get (GTK_TREE_MODEL (preferences->model), iter,
                        INVENIO_PREFERENCES_CATEGORY_COLUMN_CATEGORY, &category, -1);

    g_object_set (G_OBJECT (cell), "pixbuf", invenio_category_to_pixbuf (category), NULL);
}

static void
_category_cell_data (GtkTreeViewColumn  *tree_column,
                     GtkCellRenderer    *cell,
                     GtkTreeModel       *model,
                     GtkTreeIter        *iter,
                     gpointer            data)
{
    InvenioPreferences *preferences;
    InvenioCategory category;

    g_return_if_fail (data);

    preferences = (InvenioPreferences *) data;

    gtk_tree_model_get (GTK_TREE_MODEL (preferences->model), iter,
                        INVENIO_PREFERENCES_CATEGORY_COLUMN_CATEGORY, &category, -1);

    g_object_set (G_OBJECT (cell), "text", invenio_category_to_string (category), NULL);
}

static void
_load (InvenioPreferences *preferences)
{
    const gchar *keybinding;
    gchar *keybinding_label;
    GdkModifierType accelerator_mods;
    InvenioCategory category;
    guint i, accelerator_key;
    GtkTreeIter iter;

    for (i = 1, category = (InvenioCategory) 0; category < INVENIO_CATEGORIES; category++, i++)
    {
        gtk_list_store_append (GTK_LIST_STORE (preferences->model), &iter);
        gtk_list_store_set (GTK_LIST_STORE (preferences->model), &iter,
                            INVENIO_PREFERENCES_CATEGORY_COLUMN_INDEX, i,
                            INVENIO_PREFERENCES_CATEGORY_COLUMN_ENABLED, TRUE,
                            INVENIO_PREFERENCES_CATEGORY_COLUMN_ICON, NULL,
                            INVENIO_PREFERENCES_CATEGORY_COLUMN_CATEGORY, category,
                            -1);
    }

    keybinding = invenio_configuration_get_menu_shortcut ();
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (preferences->keyboard_shortcut_enabled),
                                  keybinding ? TRUE : FALSE);
    if (keybinding)
    {
        gtk_accelerator_parse (keybinding, &accelerator_key, &accelerator_mods);
        keybinding_label = gtk_accelerator_get_label (accelerator_key, accelerator_mods);
        gtk_entry_set_text (GTK_ENTRY (preferences->keyboard_shortcut), keybinding_label);
        g_free (keybinding_label);
    }
}

static InvenioPreferences *
invenio_preferences_create (void)
{
    InvenioPreferences *preferences;
    GtkWidget *vbox, *hbox, *label;
    GtkWidget *scrolled_window;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    preferences = g_new0 (InvenioPreferences, 1);

    preferences->dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_object_set (G_OBJECT (preferences->dialog), "border-width", 15);
    gtk_window_set_title (GTK_WINDOW (preferences->dialog), "invenio Preferences");
    g_signal_connect (G_OBJECT (preferences->dialog), "delete-event",
                      G_CALLBACK (_closed), preferences);

    /* model */
    preferences->model = gtk_list_store_new (INVENIO_PREFERENCES_CATEGORY_COLUMNS,
                                             COLUMN_TYPE(INVENIO_PREFERENCES_CATEGORY_COLUMN_INDEX),
                                             COLUMN_TYPE(INVENIO_PREFERENCES_CATEGORY_COLUMN_ENABLED),
                                             COLUMN_TYPE(INVENIO_PREFERENCES_CATEGORY_COLUMN_ICON),
                                             COLUMN_TYPE(INVENIO_PREFERENCES_CATEGORY_COLUMN_CATEGORY));

    /* view */
    preferences->view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (preferences->model));
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (preferences->view), FALSE);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (preferences->view), FALSE);
    gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (preferences->view), GTK_TREE_VIEW_GRID_LINES_VERTICAL);
    gtk_tree_view_set_reorderable (GTK_TREE_VIEW (preferences->view), TRUE);

    /* Column: Index */
    column = gtk_tree_view_column_new ();

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_renderer_set_alignment (cell, 0.5, 0.5);
    g_object_set (G_OBJECT (cell), "width-chars", 4);
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_add_attribute (column, cell, "text", INVENIO_PREFERENCES_CATEGORY_COLUMN_INDEX);

    gtk_tree_view_append_column (GTK_TREE_VIEW (preferences->view), column);

    /* Column: Enabled + Icon + Title */
    column = gtk_tree_view_column_new ();

    cell = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_set_padding (cell, 2, 0);
    g_signal_connect (G_OBJECT (cell), "toggled", G_CALLBACK (_category_enabled_toggled), preferences);
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_add_attribute (column, cell, "active", INVENIO_PREFERENCES_CATEGORY_COLUMN_ENABLED);

    cell = gtk_cell_renderer_pixbuf_new ();
    gtk_cell_renderer_set_padding (cell, 2, 0);
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_cell_data_func (column, cell, _icon_cell_data, preferences, NULL);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (column, cell, _category_cell_data, preferences, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (preferences->view), column);


    vbox = gtk_vbox_new (FALSE, 5);

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label),
                          "<b>Drag categories to change the order in which results appear.</b>");
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

    label = gtk_label_new ("Only selected categories will appear in search results.");
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                         GTK_SHADOW_IN);
    gtk_container_add (GTK_CONTAINER (scrolled_window), preferences->view);
    gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, FALSE, FALSE, 5);

    hbox = gtk_hbox_new (FALSE, 5);
    preferences->keyboard_shortcut_enabled = gtk_check_button_new_with_label ("Keyboard Shortcut: ");
    preferences->keyboard_shortcut = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox), preferences->keyboard_shortcut_enabled, TRUE, TRUE, 0);
    gtk_box_pack_end (GTK_BOX (hbox), preferences->keyboard_shortcut, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (preferences->dialog), vbox);

    _load (preferences);

    return preferences;
}

GtkWidget *
invenio_preferences_dialog_get_default (void)
{
    static InvenioPreferences *preferences = NULL;

    if (preferences)
        return preferences->dialog;

    invenio_configuration_load ();
    preferences = invenio_preferences_create ();

    return preferences->dialog;
}

