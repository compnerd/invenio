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

static const gchar * const InvenioCategoryString[INVENIO_CATEGORIES] =
{
    [INVENIO_CATEGORY_APPLICATION]  = "Applications",
    [INVENIO_CATEGORY_BOOKMARK]     = "Bookmarks",
    [INVENIO_CATEGORY_CONTACT]      = "Contacts",
    [INVENIO_CATEGORY_DOCUMENT]     = "Documents",
    [INVENIO_CATEGORY_FOLDER]       = "Folders",
    [INVENIO_CATEGORY_FONT]         = "Fonts",
    [INVENIO_CATEGORY_IMAGE]        = "Images",
    [INVENIO_CATEGORY_MESSAGE]      = "Messages",
    [INVENIO_CATEGORY_MUSIC]        = "Music",
    [INVENIO_CATEGORY_VIDEO]        = "Videos",
};

const gchar *
invenio_category_to_string (const InvenioCategory category)
{
    return InvenioCategoryString[category];
}

InvenioCategory
invenio_category_from_string (const gchar * const category)
{
    int i;

    for (i = 0; i < INVENIO_CATEGORIES; i++)
        if (strcmp (InvenioCategoryString[i], category) == 0)
            return (InvenioCategory) i;

    return INVENIO_CATEGORIES;
}

GdkPixbuf *
invenio_category_to_pixbuf (const InvenioCategory category)
{
    GdkPixbuf *pixbuf = NULL;
    GtkIconInfo *icon_info;
    gint height, width;
    gint size = -1;

    if (gtk_icon_size_lookup (GTK_ICON_SIZE_BUTTON, &height, &width))
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

