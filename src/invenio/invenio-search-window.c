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

typedef struct InvenioSearchWindowPrivate
{
    guint   ignored;
} InvenioSearchWindowPrivate;

G_DEFINE_TYPE (InvenioSearchWindow, invenio_search_window, GTK_TYPE_WINDOW);

static void
search_window_dispose (GObject *object)
{
    InvenioSearchWindow *window;
    InvenioSearchWindowPrivate *priv;

    window = INVENIO_SEARCH_WINDOW (object);
    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);

    G_OBJECT_CLASS (invenio_search_window_parent_class)->dispose (object);
}

static void
search_window_finalize (GObject *object)
{
    InvenioSearchWindow *window;
    InvenioSearchWindowPrivate *priv;

    window = INVENIO_SEARCH_WINDOW (object);
    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);

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
invenio_search_window_init (InvenioSearchWindow *window)
{
    InvenioSearchWindowPrivate *priv;

    priv = INVENIO_SEARCH_WINDOW_GET_PRIVATE (window);
}

GtkWidget *
invenio_search_window_new (void)
{
    return g_object_new (INVENIO_TYPE_SEARCH_WINDOW, NULL);
}

