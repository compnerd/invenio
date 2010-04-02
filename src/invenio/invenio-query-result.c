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

#include "invenio-query-result.h"

struct InvenioQueryResult
{
    gchar *title;
    gchar *description;
    gchar *uri;
    gchar *location;
};

InvenioQueryResult *
invenio_query_result_new (const gchar * const title,
                          const gchar * const description,
                          const gchar * const uri,
                          const gchar * const location)
{
    InvenioQueryResult *result;

    result = g_slice_new0 (InvenioQueryResult);

    /* XXX Is there a more robust way to check for undefined values? */

    if (strcmp (title, "title_u") != 0)
        result->title = g_strdup (title);

    if (strcmp (description, "description_u") != 0)
        result->description = g_strdup (description);

    if (strcmp (uri, "uri_u") != 0)
        result->uri = g_strdup (uri);

    if (strcmp (location, "location_u") != 0)
        result->location = g_strdup (location);

    return result;
};

void
invenio_query_result_free (InvenioQueryResult *result)
{
    if (result->title)
        g_free (result->title);

    if (result->description)
        g_free (result->description);

    if (result->uri)
        g_free (result->uri);

    if (result->location)
        g_free (result->location);

    g_slice_free (InvenioQueryResult, result);
}


const gchar *
invenio_query_result_get_title (const InvenioQueryResult * const result)
{
    return result->title;
}

const gchar *
invenio_query_result_get_description (const InvenioQueryResult * const result)
{
    return result->description;
}

const gchar *
invenio_query_result_get_uri (const InvenioQueryResult * const result)
{
    return result->uri;
}

const gchar *
invenio_query_result_get_location (const InvenioQueryResult * const result)
{
    return result->location;
}

