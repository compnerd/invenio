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

#include "invenio-search-window.h"

#define INVENIO_SEARCH_WINDOW_GET_PRIVATE(o)    (G_TYPE_INSTANCE_GET_PRIVATE ((o), INVENIO_TYPE_SEARCH_WINDOW, InvenioSearchWindowPrivate))

typedef enum SearchResultColumn
{
    SEARCH_RESULT_COLUMN_CATEGORY,
    SEARCH_RESULT_COLUMN_ICON,
    SEARCH_RESULT_COLUMN_TITLE,
    SEARCH_RESULT_COLUMN_DESCRIPTION,
    SEARCH_RESULT_COLUMN_URI,
    SEARCH_RESULT_COLUMNS,
} SearchResultColumn;

typedef struct SearchResultsWidget
{
    GtkWidget       *view;
    GtkListStore    *model;
} SearchResultsWidget;

typedef struct InvenioSearchWindowPrivate
{
    GtkWidget           *entry;
    SearchResultsWidget *results;
} InvenioSearchWindowPrivate;

G_DEFINE_TYPE (InvenioSearchWindow, invenio_search_window, GTK_TYPE_WINDOW);

static void
search_window_dispose (GObject *object)
{
    InvenioSearchWindow *window;
    InvenioSearchWindowPrivate *priv;

    window = INVENIO_SEARCH_WINDOW (object);
    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);

    g_object_unref (priv->entry);
    g_object_unref (priv->results->view);
    g_object_unref (priv->results->model);

    G_OBJECT_CLASS (invenio_search_window_parent_class)->dispose (object);
}

static void
search_window_finalize (GObject *object)
{
    InvenioSearchWindow *window;
    InvenioSearchWindowPrivate *priv;

    window = INVENIO_SEARCH_WINDOW (object);
    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);

    g_free (priv->results);

    G_OBJECT_CLASS (invenio_search_window_parent_class)->finalize (object);
}

static void
invenio_search_window_class_init (InvenioSearchWindowClass *klass)
{
    GtkWindowClass *parent_class = GTK_WINDOW_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = search_window_dispose;
    object_class->finalize = search_window_finalize;

    g_type_class_add_private (klass, sizeof (InvenioSearchWindowPrivate));
}

static void
_reset_search (InvenioSearchWindow *window)
{
    InvenioSearchWindowPrivate *priv;

    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);

    gtk_entry_set_text (GTK_ENTRY (priv->entry), "");

    /* TODO Clear Results */
}

static gboolean
search_window_focus_out (GtkWidget      *widget,
                         GdkEventFocus  *event,
                         gpointer        user_data)
{
    InvenioSearchWindow *window;
    InvenioSearchWindowPrivate *priv;

    window = INVENIO_SEARCH_WINDOW (widget);
    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);

    gtk_widget_hide (widget);
    /* XXX Should this reset or just hide? */
    _reset_search (window);

    return TRUE;
}

static gboolean
search_window_key_press (GtkWidget      *widget,
                         GdkEventKey    *event,
                         gpointer        user_data)
{
    InvenioSearchWindow *window;
    InvenioSearchWindowPrivate *priv;

    window = INVENIO_SEARCH_WINDOW (widget);
    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);

    if (event->keyval == GDK_Escape)
    {
        gtk_widget_hide (widget);
        /* XXX Should this reset or just hide? */
        _reset_search (window);
        return FALSE;
    }
    else
    {
        return GTK_WIDGET_CLASS (invenio_search_window_parent_class)->key_press_event (widget, event);
    }

    g_assert_not_reached ();
}

static void
search_window_entry_changed (GtkEditable    *editable,
                             gpointer        user_data)
{
    const gchar *search;
    InvenioSearchWindow *window;
    InvenioSearchWindowPrivate *priv;

    window = INVENIO_SEARCH_WINDOW (user_data);
    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);

    search = gtk_entry_get_text (GTK_ENTRY (priv->entry));

    if (! strlen (search))
    {
        gtk_entry_set_icon_from_stock (GTK_ENTRY (priv->entry),
                                       GTK_ENTRY_ICON_SECONDARY, NULL);
        /* TODO Cancel Previous Search */
        /* TODO Clear Store */
        return;
    }

    gtk_entry_set_icon_from_stock (GTK_ENTRY (priv->entry),
                                   GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);

    /* TODO Perform Queries */
}

static void
search_window_entry_icon_release (GtkEntry              *entry,
                                  GtkEntryIconPosition   icon_pos,
                                  GdkEventButton        *event,
                                  gpointer               user_data)
{
    InvenioSearchWindow *window;
    InvenioSearchWindowPrivate *priv;

    window = INVENIO_SEARCH_WINDOW (user_data);
    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);

    switch (icon_pos)
    {
        case GTK_ENTRY_ICON_PRIMARY:
            g_assert_not_reached ();
        case GTK_ENTRY_ICON_SECONDARY:
            _reset_search (window);
            break;
    }
}

static void
search_results_render_category_name (GtkTreeViewColumn  *tree_column,
                                     GtkCellRenderer    *cell,
                                     GtkTreeModel       *tree_model,
                                     GtkTreeIter        *iter,
                                     gpointer            user_data)
{
}

static void
search_results_render_icon (GtkTreeViewColumn   *tree_column,
                            GtkCellRenderer     *cell,
                            GtkTreeModel        *tree_model,
                            GtkTreeIter         *iter,
                            gpointer             user_data)
{
}

static void
invenio_search_window_init (InvenioSearchWindow *window)
{
    InvenioSearchWindowPrivate *priv;
    GtkTreeViewColumn *column;
    GtkWidget *hbox, *vbox;
    GtkCellRenderer *cell;

    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);

    /* window settings */
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_widget_set_events (GTK_WIDGET (window), GDK_FOCUS_CHANGE_MASK);
    g_signal_connect (G_OBJECT (window), "focus-out-event",
                      G_CALLBACK (search_window_focus_out), window);
    g_signal_connect (G_OBJECT (window), "key-press-event",
                      G_CALLBACK (search_window_key_press), window);

    /* entry */
    priv->entry = gtk_entry_new ();
    g_signal_connect (G_OBJECT (priv->entry), "changed",
                      G_CALLBACK (search_window_entry_changed), window);
    g_signal_connect (G_OBJECT (priv->entry), "icon-release",
                      G_CALLBACK (search_window_entry_icon_release), window);

    hbox = gtk_hbox_new (FALSE, 2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
    gtk_box_pack_start (GTK_BOX (hbox),
                        gtk_label_new ("Search: "), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), priv->entry, TRUE, TRUE, 0);

    /* results */
    priv->results = g_new0 (SearchResultsWidget, 1);

    priv->results->model =
        gtk_list_store_new (SEARCH_RESULT_COLUMNS,
                            G_TYPE_INT,             /* Category */
                            GDK_TYPE_PIXBUF,        /* Icon */
                            G_TYPE_STRING,          /* Title */
                            G_TYPE_STRING,          /* Description */
                            G_TYPE_STRING);         /* URI */

    priv->results->view = gtk_tree_view_new ();
    gtk_tree_view_set_model (GTK_TREE_VIEW (priv->results->view),
                             GTK_TREE_MODEL (priv->results->model));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->results->view),
                                       FALSE);

    /* Column: Category */
    column = gtk_tree_view_column_new ();
    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             search_results_render_category_name,
                                             window, NULL);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
    gtk_tree_view_column_set_sort_column_id (column,
                                             SEARCH_RESULT_COLUMN_CATEGORY);
    gtk_tree_view_column_set_title (column, "Category");
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->results->view), column);

    /* Column: Icon + Title */
    column = gtk_tree_view_column_new ();
    cell = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             search_results_render_icon,
                                             window, NULL);

    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "xpad", 4, "ypad", 1, NULL);
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_add_attribute (column, cell, "text",
                                        SEARCH_RESULT_COLUMN_TITLE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
    gtk_tree_view_column_set_sort_column_id (column,
                                             SEARCH_RESULT_COLUMN_TITLE);
    gtk_tree_view_column_set_title (column, "Result");
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->results->view), column);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), priv->results->view, TRUE, TRUE, 0);

    /* window contents */
    gtk_widget_show_all (GTK_WIDGET (vbox));

    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show (hbox);
}

GtkWidget *
invenio_search_window_new (void)
{
    return g_object_new (INVENIO_TYPE_SEARCH_WINDOW, NULL);
}

