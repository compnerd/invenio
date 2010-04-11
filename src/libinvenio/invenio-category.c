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

#include "invenio-category.h"

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

