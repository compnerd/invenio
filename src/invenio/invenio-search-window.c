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

#include <string.h>

#include <gdk/gdkkeysyms.h>

#include <libwnck/libwnck.h>

#include "invenio-query.h"
#include "invenio-category.h"
#include "invenio-query-result.h"
#include "invenio-search-window.h"

#define INVENIO_SEARCH_WINDOW_WIDTH             (340)


typedef struct InvenioSearchResults
{
    GtkListStore    *model;
    GtkWidget       *view;
    guint            count;
} InvenioSearchResults;

typedef struct InvenioSearchWindow
{
    GtkWidget               *window;
    GtkWidget               *entry;
    InvenioSearchResults    *results;
    gboolean                 ignore_updates;
} InvenioSearchWindow;

typedef enum InvenioSearchResultColumn
{
    INVENIO_SEARCH_RESULT_COLUMN_CATEGORY,
    INVENIO_SEARCH_RESULT_COLUMN_ICON,
    INVENIO_SEARCH_RESULT_COLUMN_TITLE,
    INVENIO_SEARCH_RESULT_COLUMN_DESCRIPTION,
    INVENIO_SEARCH_RESULT_COLUMN_URI,
    INVENIO_SEARCH_RESULT_COLUMN_LOCATION,
    INVENIO_SEARCH_RESULT_COLUMNS,
} InvenioSearchResultColumn;


static const GType InvenioSearchResultColumnType[INVENIO_SEARCH_RESULT_COLUMNS] =
{
    [INVENIO_SEARCH_RESULT_COLUMN_CATEGORY]     = G_TYPE_INT,
    [INVENIO_SEARCH_RESULT_COLUMN_ICON]         = G_TYPE_OBJECT,
    [INVENIO_SEARCH_RESULT_COLUMN_TITLE]        = G_TYPE_STRING,
    [INVENIO_SEARCH_RESULT_COLUMN_DESCRIPTION]  = G_TYPE_STRING,
    [INVENIO_SEARCH_RESULT_COLUMN_URI]          = G_TYPE_STRING,
    [INVENIO_SEARCH_RESULT_COLUMN_LOCATION]     = G_TYPE_STRING,
};

#define COLUMN_TYPE(column)                     (InvenioSearchResultColumnType[(column)])


static void
invenio_search_window_reset_search (InvenioSearchWindow *search_window)
{
    gtk_entry_set_text (GTK_ENTRY (search_window->entry), "");
    gtk_list_store_clear (search_window->results->model);
    search_window->results->count = 0;
}

static gboolean
invenio_search_window_focus_out (GtkWidget      *widget,
                                 GdkEventFocus  *event,
                                 gpointer        user_data)
{
    InvenioSearchWindow *search_window;

    search_window = (InvenioSearchWindow *) user_data;

    /* XXX Should this reset or just hide? */
    gtk_widget_hide (search_window->window);
    invenio_search_window_reset_search (search_window);

    return TRUE;
}

static void
invenio_search_window_select_next_result (InvenioSearchWindow *search_window)
{
    GtkTreeSelection *selection;
    GtkTreePath *path;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (search_window->results->view));

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        path = gtk_tree_model_get_path (GTK_TREE_MODEL (search_window->results->model), &iter);
        gtk_tree_path_next (path);
    }
    else
    {
        path = gtk_tree_path_new_first ();
    }

    gtk_tree_selection_select_path (selection, path);
}


static gboolean
_uri_is_executable (const gchar * const uri)
{
    gchar *scheme;
    gboolean executable;

    /*
     * if there is no scheme or the path is abolute, the URI is probably an
     * executable.  Because that is not a URI, we need to handle that case
     * separately.
     */
    scheme = g_uri_parse_scheme (uri);
    executable = (! scheme || g_path_is_absolute (uri));
    g_free (scheme);

    return executable;
}

static void
_launch_uri (const gchar * const uri, GdkScreen *screen)
{
    GAppInfo *info;
    GError *error = NULL;
    GdkAppLaunchContext *context;

    context = gdk_app_launch_context_new ();
    gdk_app_launch_context_set_screen (context, screen);

    if (_uri_is_executable (uri))
    {
        info = g_app_info_create_from_commandline (uri, NULL, G_APP_INFO_CREATE_NONE, &error);
        if (error)
        {
            g_warning ("Could not create GAppInfo for command '%s': %s",
                    uri, error->message);
            g_error_free (error);
        }
        else
        {
            if (! g_app_info_launch (info, NULL, G_APP_LAUNCH_CONTEXT (context), &error))
            {
                g_warning ("Could not launch application for uri '%s': %s",
                        uri, error->message);
                g_error_free (error);
            }
        }
        g_object_unref (info);
    }
    else
    {
        if (! g_app_info_launch_default_for_uri (uri, G_APP_LAUNCH_CONTEXT (context), &error))
        {
            g_warning ("Could not launch application for uri '%s': %s",
                    uri, error->message);
            g_error_free (error);
        }
    }

    g_object_unref (context);
}

static void
invenio_search_window_activate_selected_result (InvenioSearchWindow *search_window,
                                                const gboolean alternate_action)
{
    gchar *uri;
    GtkTreePath *path;
    GtkTreeSelection *selection;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (search_window->results->view));

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        path = gtk_tree_model_get_path (GTK_TREE_MODEL (search_window->results->model), &iter);

        if (alternate_action)
        {
            char *temp;

            gtk_tree_model_get (GTK_TREE_MODEL (search_window->results->model), &iter,
                                INVENIO_SEARCH_RESULT_COLUMN_LOCATION, &uri, -1);

            temp = g_path_get_dirname (uri);
            g_free (uri);
            uri = temp;
        }
        else
        {
            gtk_tree_model_get (GTK_TREE_MODEL (search_window->results->model), &iter,
                                INVENIO_SEARCH_RESULT_COLUMN_URI, &uri, -1);
        }

        _launch_uri (uri, gtk_widget_get_screen (search_window->window));

        g_free (uri);
    }
}

static void
invenio_search_window_select_previous_result (InvenioSearchWindow *search_window)
{
    GtkTreeSelection *selection;
    gchar *path_string;
    GtkTreePath *path;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (search_window->results->view));

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        path = gtk_tree_model_get_path (GTK_TREE_MODEL (search_window->results->model), &iter);
        gtk_tree_path_prev (path);
    }
    else
    {
        path_string = g_strdup_printf ("%d", search_window->results->count - 1);
        path = gtk_tree_path_new_from_string (path_string);
        g_free (path_string);
    }

    gtk_tree_selection_select_path (selection, path);
}

static gboolean
invenio_search_window_key_press (GtkWidget      *widget,
                                 GdkEventKey    *event,
                                 gpointer        user_data)
{
    InvenioSearchWindow *search_window;

    search_window = (InvenioSearchWindow *) user_data;

    switch (event->keyval)
    {
        case GDK_Down:
            invenio_search_window_select_next_result (search_window);
            return FALSE;

        case GDK_Escape:
            /* XXX Should this reset or just hide? */
            gtk_widget_hide (search_window->window);
            invenio_search_window_reset_search (search_window);
            return FALSE;

        case GDK_Return:
            invenio_search_window_activate_selected_result (search_window,
                                                            event->state & GDK_CONTROL_MASK);
            return FALSE;

        case GDK_Up:
            invenio_search_window_select_previous_result (search_window);
            return FALSE;

        default:
            break;
    }

    return GTK_WIDGET_GET_CLASS (search_window->window)->key_press_event (widget, event);
}

static void
invenio_search_window_active_workspace_changed (WnckScreen      *screen,
                                                WnckWorkspace   *previously_active_space,
                                                gpointer         user_data)
{
    InvenioSearchWindow *search_window;

    search_window = (InvenioSearchWindow *) user_data;

    /* XXX Should this reset or just hide? */
    gtk_widget_hide (search_window->window);
    invenio_search_window_reset_search (search_window);
}

static void
_insert_result (GtkListStore        *store,
                GtkTreeIter         *iter,
                InvenioCategory      category,
                InvenioQueryResult  *result)
{
    gtk_list_store_set (store, iter,
                        INVENIO_SEARCH_RESULT_COLUMN_CATEGORY, category,
                        INVENIO_SEARCH_RESULT_COLUMN_TITLE, invenio_query_result_get_title (result),
                        INVENIO_SEARCH_RESULT_COLUMN_DESCRIPTION, invenio_query_result_get_description (result),
                        INVENIO_SEARCH_RESULT_COLUMN_URI, invenio_query_result_get_uri (result),
                        INVENIO_SEARCH_RESULT_COLUMN_LOCATION, invenio_query_result_get_location (result),
                        -1);
}

static void
invenio_search_window_update_results_for_category (InvenioSearchWindow  *search_window,
                                                   InvenioCategory       category,
                                                   const GSList         *results)
{
    const GSList *entry;
    InvenioCategory value;
    GtkTreeIter position;
    GtkTreeIter iter;
    gboolean valid;

    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (search_window->results->model), &iter);

    while (valid)
    {
        gtk_tree_model_get (GTK_TREE_MODEL (search_window->results->model), &iter,
                            INVENIO_SEARCH_RESULT_COLUMN_CATEGORY, &value, -1);

        if (value == category)
        {
            for (entry = results; entry; entry = g_slist_next (entry))
            {
                _insert_result (search_window->results->model, &iter, category, entry->data);

                valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (search_window->results->model), &iter);
                if (! valid)
                    break;

                gtk_tree_model_get (GTK_TREE_MODEL (search_window->results->model), &iter,
                                    INVENIO_SEARCH_RESULT_COLUMN_CATEGORY, &value, -1);
                if (value != category)
                    break;
            }

            while ((entry = g_slist_next (entry)))
            {
                if (valid)
                    gtk_list_store_insert_before (search_window->results->model, &position, &iter);
                else
                    gtk_list_store_append (search_window->results->model, &position);

                _insert_result (search_window->results->model, &position, category, entry->data);
                search_window->results->count++;
            }

            while (valid)
            {
                gtk_tree_model_get (GTK_TREE_MODEL (search_window->results->model), &iter,
                                    INVENIO_SEARCH_RESULT_COLUMN_CATEGORY, &value, -1);
                if (value != category)
                    break;

                valid = gtk_list_store_remove (search_window->results->model, &iter);
                search_window->results->count--;
            }

            return;
        }

        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (search_window->results->model), &iter);
    }

    for (entry = results; entry; entry = g_slist_next (entry))
    {
        gtk_list_store_append (search_window->results->model, &iter);
        _insert_result (search_window->results->model, &iter, category, entry->data);
        search_window->results->count++;
    }
}

static void
invenio_search_window_clear_results_for_category (InvenioSearchWindow   *search_window,
                                                  InvenioCategory        category)
{
    InvenioCategory value;
    GtkTreeIter iter;
    gboolean valid;

    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (search_window->results->model), &iter);

    while (valid)
    {
        gtk_tree_model_get (GTK_TREE_MODEL (search_window->results->model), &iter,
                            INVENIO_SEARCH_RESULT_COLUMN_CATEGORY, &value, -1);

        if (value == category)
        {
            valid = gtk_list_store_remove (search_window->results->model, &iter);
            search_window->results->count--;
        }
        else
            valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (search_window->results->model), &iter);
    }
}

static void
invenio_search_window_update_results_for_query (InvenioQuery    *query,
                                                GError          *error,
                                                gpointer         user_data)
{
    const GSList *results;
    InvenioSearchWindow *search_window;

    search_window = (InvenioSearchWindow *) user_data;

    if (error)
    {
        g_critical ("Failed to execute query for category '%s': %s",
                    invenio_category_to_string (invenio_query_get_category (query)),
                    error->message);
        g_error_free (error);
        goto out;
    }

    if (search_window->ignore_updates)
        goto out;

    results = invenio_query_get_results (query);

    if (results)
        invenio_search_window_update_results_for_category (search_window,
                                                           invenio_query_get_category (query),
                                                           results);
    else
        invenio_search_window_clear_results_for_category (search_window,
                                                          invenio_query_get_category (query));

out:
    invenio_query_free (query);
}

static void
invenio_search_window_entry_changed (GtkEditable    *editable,
                                     gpointer        user_data)
{
    const gchar *search;
    InvenioSearchWindow *search_window;
    InvenioCategory category;

    search_window = (InvenioSearchWindow *) user_data;

    search = gtk_entry_get_text (GTK_ENTRY (search_window->entry));

    if (! strlen (search))
    {
        gtk_entry_set_icon_from_stock (GTK_ENTRY (search_window->entry),
                                       GTK_ENTRY_ICON_SECONDARY, NULL);
        invenio_search_window_reset_search (search_window);
        search_window->ignore_updates = TRUE;
        return;
    }

    search_window->ignore_updates = FALSE;
    gtk_entry_set_icon_from_stock (GTK_ENTRY (search_window->entry),
                                   GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);

    for (category = (InvenioCategory) 0; category != INVENIO_CATEGORIES; ++category)
        invenio_query_execute_async (invenio_query_new (category, search),
                                     invenio_search_window_update_results_for_query,
                                     search_window);
}

static void
invenio_search_window_entry_icon_release (GtkEntry              *entry,
                                          GtkEntryIconPosition   icon_pos,
                                          GdkEventButton        *event,
                                          gpointer               user_data)
{
    InvenioSearchWindow *search_window;

    search_window = (InvenioSearchWindow *) user_data;

    switch (icon_pos)
    {
        case GTK_ENTRY_ICON_PRIMARY:
            g_assert_not_reached ();
        case GTK_ENTRY_ICON_SECONDARY:
            invenio_search_window_reset_search (search_window);
            break;
    }
}

static void
_category_cell_data (GtkTreeViewColumn  *tree_column,
                     GtkCellRenderer    *cell,
                     GtkTreeModel       *model,
                     GtkTreeIter        *iter,
                     gpointer            data)
{
    GtkTreePath *path;
    InvenioSearchWindow *search_window;
    InvenioCategory category, value;
    gboolean visible = TRUE;
    GtkTreeIter entry;

    search_window = (InvenioSearchWindow *) data;

    gtk_tree_model_get (GTK_TREE_MODEL (search_window->results->model), iter,
                        INVENIO_SEARCH_RESULT_COLUMN_CATEGORY, &category, -1);

    path = gtk_tree_model_get_path (GTK_TREE_MODEL (search_window->results->model), iter);

    if (gtk_tree_path_prev (path))
    {
        if (gtk_tree_model_get_iter (GTK_TREE_MODEL (search_window->results->model), &entry, path))
        {
            gtk_tree_model_get (GTK_TREE_MODEL (search_window->results->model), &entry,
                                INVENIO_SEARCH_RESULT_COLUMN_CATEGORY, &value, -1);

            if (value == category)
                visible = FALSE;
        }
    }

    g_object_set (cell,
                  "text", invenio_category_to_string (category),
                  "visible", visible,
                  NULL);

    gtk_tree_path_free (path);
}

static void
_icon_cell_data (GtkTreeViewColumn  *tree_column,
                 GtkCellRenderer    *cell,
                 GtkTreeModel       *model,
                 GtkTreeIter        *iter,
                 gpointer            data)
{
    GFile *file;
    gchar *id, *uri;
    GFileInfo *info;
    GIcon *icon = NULL;
    GError *error = NULL;
    InvenioSearchWindow *search_window;

    search_window = (InvenioSearchWindow *) data;

    gtk_tree_model_get (GTK_TREE_MODEL (search_window->results->model), iter,
                        INVENIO_SEARCH_RESULT_COLUMN_URI, &uri, -1);

    if (_uri_is_executable (uri))
    {
        /* TODO Try to get the application icon */
        icon = g_themed_icon_new (GTK_STOCK_EXECUTE);
    }
    else
    {
        file = g_file_new_for_uri (uri);

        info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_ICON,
                                  G_FILE_QUERY_INFO_NONE, NULL, &error);
        if (error)
        {
            g_warning ("Could not query file info for uri '%s': %s",
                       uri, error->message);
            g_error_free (error);
        }
        else
        {
            /*
             * The icon is owned by the info and will be unref'ed when the info
             * is unref'ed.  We need to copy the icon in order to hold a valid
             * icon.
             */
            if ((id = g_icon_to_string (g_file_info_get_icon (info))))
            {
                icon = g_icon_new_for_string (id, &error);
                if (error)
                {
                    g_warning ("Could not create GIcon for uri '%s': %s",
                               uri, error->message);
                    g_error_free (error);
                }

                g_free (id);
            }
        }

        if (info)
            g_object_unref (info);

        g_object_unref (file);
    }

    g_object_set (G_OBJECT (cell), "gicon", icon, "visible", TRUE, NULL);

    if (icon)
        g_object_unref (icon);

    g_free (uri);
}

static InvenioSearchWindow *
invenio_search_window_create (void)
{
    InvenioSearchWindow *search_window;
    GtkWidget *label, *hbox, *vbox;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    WnckScreen *screen;

    search_window = g_new (InvenioSearchWindow, 1);

    /* XXX This should be GTK_WINDOW_POPUP, but that does not behave */
    search_window->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint (GTK_WINDOW (search_window->window),
                              GDK_WINDOW_TYPE_HINT_UTILITY);
    /* XXX The following 2 should not be required when GTK_WINDOW_POPUP works */
    gtk_window_set_decorated (GTK_WINDOW (search_window->window), FALSE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (search_window->window), TRUE);
    gtk_window_set_resizable (GTK_WINDOW (search_window->window), FALSE);
    gtk_widget_set_size_request (GTK_WIDGET (search_window->window),
                                 INVENIO_SEARCH_WINDOW_WIDTH, -1);

    gtk_widget_add_events (GTK_WIDGET (search_window->window),
                           GDK_FOCUS_CHANGE_MASK);
    g_signal_connect (G_OBJECT (search_window->window), "focus-out-event",
                      G_CALLBACK (invenio_search_window_focus_out),
                      search_window);
    g_signal_connect (G_OBJECT (search_window->window), "key-press-event",
                      G_CALLBACK (invenio_search_window_key_press),
                      search_window);

    screen = wnck_screen_get_default ();
    g_signal_connect (G_OBJECT (screen), "active-workspace-changed",
                      G_CALLBACK (invenio_search_window_active_workspace_changed),
                      search_window);

    /* label */
    label = gtk_label_new ("Search: ");

    /* entry */
    search_window->entry = gtk_entry_new ();
    g_signal_connect (G_OBJECT (search_window->entry), "changed",
                      G_CALLBACK (invenio_search_window_entry_changed),
                      search_window);
    g_signal_connect (G_OBJECT (search_window->entry), "icon-release",
                      G_CALLBACK (invenio_search_window_entry_icon_release),
                      search_window);

    /* search area */
    hbox = gtk_hbox_new (FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), search_window->entry, TRUE, TRUE, 0);

    /* results */
    search_window->results = g_new0 (InvenioSearchResults, 1);

    search_window->results->model =
        gtk_list_store_new (INVENIO_SEARCH_RESULT_COLUMNS,
                            COLUMN_TYPE(INVENIO_SEARCH_RESULT_COLUMN_CATEGORY),
                            COLUMN_TYPE(INVENIO_SEARCH_RESULT_COLUMN_ICON),
                            COLUMN_TYPE(INVENIO_SEARCH_RESULT_COLUMN_TITLE),
                            COLUMN_TYPE(INVENIO_SEARCH_RESULT_COLUMN_DESCRIPTION),
                            COLUMN_TYPE(INVENIO_SEARCH_RESULT_COLUMN_URI),
                            COLUMN_TYPE(INVENIO_SEARCH_RESULT_COLUMN_LOCATION));
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (search_window->results->model),
                                          INVENIO_SEARCH_RESULT_COLUMN_CATEGORY,
                                          GTK_SORT_ASCENDING);

    search_window->results->view =
        gtk_tree_view_new_with_model (GTK_TREE_MODEL (search_window->results->model));
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (search_window->results->view), FALSE);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (search_window->results->view), FALSE);
    gtk_tree_view_set_tooltip_column (GTK_TREE_VIEW (search_window->results->view),
                                      INVENIO_SEARCH_RESULT_COLUMN_DESCRIPTION);
    gtk_widget_set_can_focus (GTK_WIDGET (search_window->results->view), FALSE);

    /* Column: Category */
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_cell_data_func (column, cell, _category_cell_data, search_window, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (search_window->results->view), column);

    /* Column: Icon + Title */
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);

    cell = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_cell_data_func (column, cell, _icon_cell_data, search_window, NULL);

    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_add_attribute (column, cell, "text", INVENIO_SEARCH_RESULT_COLUMN_TITLE);

    gtk_tree_view_append_column (GTK_TREE_VIEW (search_window->results->view), column);

    /* results view */
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), search_window->results->view, TRUE, TRUE, 0);

    /* window contents */
    gtk_widget_show_all (vbox);
    gtk_container_add (GTK_CONTAINER (search_window->window), vbox);

    gtk_widget_realize (GTK_WIDGET (search_window->window));

    return search_window;
}

GtkWidget *
invenio_search_window_get_default (void)
{
    static InvenioSearchWindow *search_window;

    if (search_window)
        return search_window->window;

    search_window = invenio_search_window_create ();

    return search_window->window;
}

