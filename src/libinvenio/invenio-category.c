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

#include "invenio-category.h"

static const gchar * const InvenioCategoryIconName[INVENIO_CATEGORIES] =
{
    [INVENIO_CATEGORY_APPLICATION]  = "application-x-executable",
    [INVENIO_CATEGORY_BOOKMARK]     = "user-bookmarks",
    [INVENIO_CATEGORY_CONTACT]      = "x-office-address-book",
    [INVENIO_CATEGORY_DOCUMENT]     = "x-office-document",
    [INVENIO_CATEGORY_FOLDER]       = "folder",
    [INVENIO_CATEGORY_FONT]         = "font-x-generic",
    [INVENIO_CATEGORY_IMAGE]        = "image-x-generic",
    [INVENIO_CATEGORY_MESSAGE]      = "text-x-generic",
    [INVENIO_CATEGORY_MUSIC]        = "audio-x-generic",
    [INVENIO_CATEGORY_VIDEO]        = "video-x-generic",
};

const gchar *
invenio_category_to_string (const InvenioCategory category)
{
    switch (category)
    {
        case INVENIO_CATEGORY_APPLICATION:
            return "Applications";
        case INVENIO_CATEGORY_BOOKMARK:
            return "Bookmarks";
        case INVENIO_CATEGORY_CONTACT:
            return "Contacts";
        case INVENIO_CATEGORY_DOCUMENT:
            return "Documents";
        case INVENIO_CATEGORY_FOLDER:
            return "Folders";
        case INVENIO_CATEGORY_FONT:
            return "Fonts";
        case INVENIO_CATEGORY_IMAGE:
            return "Images";
        case INVENIO_CATEGORY_MESSAGE:
            return "Messages";
        case INVENIO_CATEGORY_MUSIC:
            return "Music";
        case INVENIO_CATEGORY_VIDEO:
            return "Videos";
        case INVENIO_CATEGORIES:
            g_assert_not_reached ();
    }

    g_assert_not_reached ();
}

GdkPixbuf *
invenio_category_to_pixbuf (const InvenioCategory category)
{
    GdkPixbuf *pixbuf = NULL;
    GtkIconInfo *icon_info;
    gint height, width;
    gint size = 16;

    if (gtk_icon_size_lookup (GTK_ICON_SIZE_SMALL_TOOLBAR, &height, &width))
        size = MIN(height, width);

    icon_info = gtk_icon_theme_lookup_icon (gtk_icon_theme_get_default (),
                                            InvenioCategoryIconName[category],
                                            size,
                                            GTK_ICON_LOOKUP_USE_BUILTIN | GTK_ICON_LOOKUP_GENERIC_FALLBACK);

    if (icon_info)
    {
        pixbuf = gtk_icon_info_load_icon (icon_info, NULL);
        gtk_icon_info_free (icon_info);
    }

    return pixbuf;
}

