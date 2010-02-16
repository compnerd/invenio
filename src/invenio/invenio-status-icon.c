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

#define INVENIO_TYPE_STATUS_ICON            (invenio_status_icon_get_type ())
#define INVENIO_STATUS_ICON(o)              (G_TYPE_CHECK_INSTANCE_CAST ((o), INVENIO_TYPE_STATUS_ICON, InvenioStatusIcon))
#define INVENIO_STATUS_ICON_CLASS(c)        (G_TYPE_CHECK_CLASS_CAST ((c), INVENIO_TYPE_STATUS_ICON, InvenioStatusIconClass))
#define INVENIO_IS_STATUS_ICON(o)           (G_TYPE_CHECK_INSTANCE_TYPE ((o), INVENIO_TYPE_STATUS_ICON))
#define INVENIO_IS_STATUS_ICON_CLASS(c)     (G_TYPE_CHECK_CLASS_TYPE ((c), INVENIO_TYPE_STATUS_ICON))
#define INVENIO_STATUS_ICON_GET_CLASS(o)    (G_TYPE_INSTANCE_GET_CLASS ((o), INVENIO_TYPE_STATUS_ICON))

typedef struct InvenioStatusIcon
{
    GtkStatusIcon parent;
} InvenioStatusIcon;

typedef struct InvenioStatusIconClass
{
    GtkStatusIconClass parent;
} InvenioStatusIconClass;

G_DEFINE_TYPE (InvenioStatusIcon, invenio_status_icon, GTK_TYPE_STATUS_ICON);


static InvenioStatusIcon *icon;


static void
status_icon_activate (GtkStatusIcon *icon)
{
}

static void
status_icon_popup_menu (GtkStatusIcon   *icon,
                        guint            button,
                        guint32          activate_time)
{
}

static void
status_icon_dispose (GObject *object)
{
    InvenioStatusIcon *icon = INVENIO_STATUS_ICON (icon);

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
invenio_status_icon_init (InvenioStatusIcon *icon)
{
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

