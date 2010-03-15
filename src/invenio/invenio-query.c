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

#include <glib.h>
#include <libtracker-client/tracker-client.h>

#include "invenio-query.h"
#include "invenio-query-result.h"

#define RESULTS_PER_CATEGORY        4

struct InvenioQuery
{
    gchar                   *string;
    InvenioCategory          category;

    InvenioQueryCompleted    callback;
    gpointer                 user_data;

    GError                  *error;
    GSList                  *results;
};


static TrackerClient *client;

#define SPARQL_QUERY_HEADER "SELECT ?urn ?title ?description ?uri ?location WHERE { "
#define SPARQL_QUERY_FOOTER " } ORDER BY DESC (fts:rank (?urn)) OFFSET 0 LIMIT " G_STRINGIFY (RESULTS_PER_CATEGORY)

static const gchar *queries[INVENIO_CATEGORIES] =
{
    [INVENIO_CATEGORY_APPLICATION]  =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Software ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nie:title ?title ;"
                                        "      nfo:softwareCmdLine ?uri ."
                                        " OPTIONAL { ?urn nie:comment ?description }"
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_BOOKMARK]     =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Bookmark ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nie:title ?title ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_CONTACT]      =   SPARQL_QUERY_HEADER
                                        " ?urn a nco:Contact ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nco:fullname ?title ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_DOCUMENT]     =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Document ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fileName ?title ;"
                                        "      nie:url ?uri ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_FOLDER]       =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Folder ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fileName ?title ;"
                                        "      nie:url ?uri ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_FONT]         =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Font ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fontFamily ?title ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_IMAGE]        =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Image ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fileName ?title ;"
                                        "      nie:url ?uri ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_MESSAGE]      =   SPARQL_QUERY_HEADER
                                        " ?urn a nmo:Message ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nie:subject ?title ;"
                                        "      nie:url ?uri ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_MUSIC]        =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Audio ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fileName ?title ;"
                                        "      nie:url ?uri ."
                                        " OPTIONAL { ?urn nie:title ?description }"
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_VIDEO]        =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Video ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fileName ?title ;"
                                        "      nie:url ?uri ."
                                        " OPTIONAL { ?urn nie:title ?description }"
                                        SPARQL_QUERY_FOOTER,
};


InvenioQuery *
invenio_query_new (const InvenioCategory category,
                   const gchar * const keywords)
{
    InvenioQuery *query;

    query = g_slice_new0 (InvenioQuery);

    query->string = g_strdup_printf (queries[category], keywords);
    query->category = category;

    return query;
}

void
invenio_query_free (InvenioQuery *query)
{
    g_free (query->string);

    if (query->error)
        g_error_free (query->error);

    g_slist_foreach (query->results, (GFunc) invenio_query_result_free, NULL);

    g_slice_free (InvenioQuery, query);
}

static void
query_collect_result (gpointer  data,
                      gpointer  user_data)
{
    InvenioQuery *query;
    const gchar **metadata;

    query = (InvenioQuery *) user_data;
    metadata = (const gchar **) data;

    /*
     * NOTE: Metadata content is determined by the SPARQL query.  The order of
     * the output is determined by the SELECT order.  The select order is
     * defined in the SPARQL_QUERY_HEADER macro above.
     */

    query->results =
        g_slist_append (query->results,
                        invenio_query_result_new (metadata[0],      /* urn */
                                                  metadata[1],      /* title */
                                                  metadata[2],      /* description */
                                                  metadata[3],      /* uri */
                                                  metadata[4]));    /* location */
}

static void
query_collect_results (GPtrArray    *results,
                       GError       *error,
                       gpointer      user_data)
{
    InvenioQuery *query;

    query = (InvenioQuery *) user_data;

    if (error)
    {
        query->error = g_error_copy (error);
        g_error_free (error);
    }

    if (! error && results)
    {
        g_ptr_array_foreach (results, query_collect_result, query);

        g_ptr_array_foreach (results, (GFunc) g_strfreev, NULL);
        g_ptr_array_free (results, TRUE);
    }

    query->callback (query, query->error, query->user_data);
}

void
invenio_query_execute_async (InvenioQuery           *query,
                             InvenioQueryCompleted   callback,
                             gpointer                user_data)
{
    query->callback = callback;
    query->user_data = user_data;

    if (G_UNLIKELY (! client))
        client = tracker_client_new (TRACKER_CLIENT_ENABLE_WARNINGS, G_MAXINT);

    tracker_resources_sparql_query_async (client, query->string,
                                          query_collect_results, query);
}

const InvenioCategory
invenio_query_get_category (const InvenioQuery * const query)
{
    return query->category;
}

const GSList *
invenio_query_get_results (const InvenioQuery * const query)
{
    return query->results;
}

