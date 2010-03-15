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

typedef struct InvenioStatusIcon
{
    GtkStatusIcon   *status_icon;

    GtkWidget       *context_menu;
    GtkWidget       *search_window;
} InvenioStatusIcon;


static InvenioStatusIcon *icon;


static void
_icon_activate (GtkStatusIcon   *status_icon,
                gpointer         user_data)
{
    GtkWidget *menu;
    GdkScreen *screen;
    InvenioStatusIcon *icon;
    gint window_width, window_height;
    GdkRectangle geometry;
    gint menu_x, menu_y;
    gboolean pushed_in;
    gint monitor;
    gint x, y;

    g_return_if_fail (status_icon);
    g_return_if_fail (GTK_IS_STATUS_ICON (status_icon));
    g_return_if_fail (user_data);

    icon = (InvenioStatusIcon *) user_data;

    menu = gtk_menu_new ();
    gtk_status_icon_position_menu (GTK_MENU (menu),
                                   &menu_x, &menu_y, &pushed_in,
                                   status_icon);

    screen = gtk_status_icon_get_screen (status_icon);
    monitor = gdk_screen_get_monitor_at_point (screen, menu_x, menu_y);
    gdk_screen_get_monitor_geometry (screen, monitor, &geometry);

    gtk_window_get_size (GTK_WINDOW (icon->search_window),
                         &window_width, &window_height);

    x = CLAMP (menu_x, 0, geometry.width - window_width);
    y = CLAMP (menu_y, 0, geometry.height - window_height);

    gtk_window_move (GTK_WINDOW (icon->search_window), x, y);
    gtk_window_present (GTK_WINDOW (icon->search_window));

    g_object_ref_sink (menu);
}

static void
_icon_popup_menu (GtkStatusIcon *status_icon,
                  guint          button,
                  guint          activate_time,
                  gpointer       user_data)
{
    InvenioStatusIcon *icon;

    icon = (InvenioStatusIcon *) user_data;

    gtk_menu_popup (GTK_MENU (icon->context_menu),
                    NULL, NULL,
                    gtk_status_icon_position_menu, status_icon,
                    button, activate_time);
}

static void
_menu_about_activate (GtkMenuItem   *menu_item,
                      gpointer       user_data)
{
    /* TODO Implement About Dialog */
}

static void
_menu_quit_activate (GtkMenuItem    *menu_item,
                     gpointer        user_data)
{
    gtk_main_quit ();
}

static GtkWidget *
_context_menu_create_for_icon (InvenioStatusIcon *icon)
{
    GtkWidget *menu, *item;

    menu = gtk_menu_new ();

    /* About */
    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (_menu_about_activate), icon);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    /* Quit */
    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (_menu_quit_activate), icon);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    gtk_widget_show_all (GTK_WIDGET (menu));

    return menu;
}

void
invenio_status_icon_create (void)
{
    g_return_if_fail (! icon);

    icon = g_new (InvenioStatusIcon, 1);
    icon->status_icon = gtk_status_icon_new_from_stock (GTK_STOCK_FIND);
    icon->context_menu = _context_menu_create_for_icon (icon);
    icon->search_window = invenio_search_window_get_default ();

    g_signal_connect (G_OBJECT (icon->status_icon), "activate",
                      G_CALLBACK (_icon_activate), icon);
    g_signal_connect (G_OBJECT (icon->status_icon), "popup-menu",
                      G_CALLBACK (_icon_popup_menu), icon);
}

