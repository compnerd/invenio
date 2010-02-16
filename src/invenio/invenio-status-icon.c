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

#include <gtk/gtk.h>

#include "invenio-status-icon.h"
#include "invenio-search-window.h"

#define INVENIO_TYPE_STATUS_ICON            (invenio_status_icon_get_type ())
#define INVENIO_STATUS_ICON(o)              (G_TYPE_CHECK_INSTANCE_CAST ((o), INVENIO_TYPE_STATUS_ICON, InvenioStatusIcon))
#define INVENIO_STATUS_ICON_CLASS(c)        (G_TYPE_CHECK_CLASS_CAST ((c), INVENIO_TYPE_STATUS_ICON, InvenioStatusIconClass))
#define INVENIO_IS_STATUS_ICON(o)           (G_TYPE_CHECK_INSTANCE_TYPE ((o), INVENIO_TYPE_STATUS_ICON))
#define INVENIO_IS_STATUS_ICON_CLASS(c)     (G_TYPE_CHECK_CLASS_TYPE ((c), INVENIO_TYPE_STATUS_ICON))
#define INVENIO_STATUS_ICON_GET_CLASS(o)    (G_TYPE_INSTANCE_GET_CLASS ((o), INVENIO_TYPE_STATUS_ICON))

typedef struct InvenioStatusIcon
{
    GtkStatusIcon    parent;

    GtkWidget       *context_menu;
    GtkWidget       *search_window;
} InvenioStatusIcon;

typedef struct InvenioStatusIconClass
{
    GtkStatusIconClass  parent;
} InvenioStatusIconClass;

G_DEFINE_TYPE (InvenioStatusIcon, invenio_status_icon, GTK_TYPE_STATUS_ICON);


static InvenioStatusIcon *icon;


static void
status_icon_activate (GtkStatusIcon *icon)
{
    InvenioStatusIcon *instance;
    GtkWidget *menu, *window;
    gboolean pushed_in;
    gint x, y;

    instance = INVENIO_STATUS_ICON (icon);

    menu = gtk_menu_new ();
    gtk_status_icon_position_menu (GTK_MENU (menu), &x, &y, &pushed_in, icon);

    window = instance->search_window;
    gtk_window_move (GTK_WINDOW (window), x, y);
    gtk_widget_show (GTK_WIDGET (window));

    g_object_ref_sink (menu);
}

static void
status_icon_popup_menu (GtkStatusIcon   *icon,
                        guint            button,
                        guint32          activate_time)
{
    InvenioStatusIcon *instance = INVENIO_STATUS_ICON (icon);

    gtk_menu_popup (GTK_MENU (instance->context_menu),
                    NULL, NULL,
                    gtk_status_icon_position_menu, icon,
                    button, activate_time);
}

static void
status_icon_dispose (GObject *object)
{
    InvenioStatusIcon *icon = INVENIO_STATUS_ICON (icon);

    g_object_unref (icon->context_menu);
    g_object_unref (icon->search_window);

    G_OBJECT_CLASS (invenio_status_icon_parent_class)->dispose (object);
}

static void
status_icon_finalize (GObject *object)
{
    InvenioStatusIcon *icon = INVENIO_STATUS_ICON (icon);

    G_OBJECT_CLASS (invenio_status_icon_parent_class)->finalize (object);
}

static void
invenio_status_icon_class_init (InvenioStatusIconClass *klass)
{
    GtkStatusIconClass *parent_class = GTK_STATUS_ICON_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    parent_class->activate = status_icon_activate;
    parent_class->popup_menu = status_icon_popup_menu;

    object_class->dispose = status_icon_dispose;
    object_class->finalize = status_icon_finalize;
}

static void
status_icon_about (GtkMenuItem  *menu_item,
                   gpointer      user_data)
{
    /* TODO Implement About Dialog */
}

static void
status_icon_quit (GtkMenuItem   *menu_item,
                  gpointer       user_data)
{
    gtk_main_quit ();
}

static void
_create_context_menu (InvenioStatusIcon *icon)
{
    GtkWidget *item;

    icon->context_menu = gtk_menu_new ();

    /* About */
    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (status_icon_about), icon);
    gtk_menu_shell_append (GTK_MENU_SHELL (icon->context_menu), item);

    /* Quit */
    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (status_icon_quit), icon);
    gtk_menu_shell_append (GTK_MENU_SHELL (icon->context_menu), item);

    gtk_widget_show_all (GTK_WIDGET (icon->context_menu));
}

static void
invenio_status_icon_init (InvenioStatusIcon *icon)
{
    _create_context_menu (icon);
    icon->search_window = invenio_search_window_new ();
    gtk_status_icon_set_from_stock (GTK_STATUS_ICON (icon), GTK_STOCK_FIND);
}

static InvenioStatusIcon *
invenio_status_icon_new (void)
{
    return g_object_new (INVENIO_TYPE_STATUS_ICON, NULL);
}


void
invenio_status_icon_initialise (void)
{
    g_return_if_fail (! icon);

    icon = invenio_status_icon_new ();
}

