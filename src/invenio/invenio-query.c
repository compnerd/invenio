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

#include "libinvenio/invenio-configuration.h"


#define RESULTS_PER_CATEGORY        4


typedef struct InvenioTrackerQuery
{
    gchar                   *query;
    guint                    id;
    gboolean                 valid;

    GSList                  *results;
} InvenioTrackerQuery;

struct InvenioQuery
{
    InvenioTrackerQuery      queries[INVENIO_CATEGORIES];

    InvenioQueryCompleted    callback;
    gpointer                 user_data;

    GError                  *error;
};

typedef struct InvenioQueryRequest
{
    InvenioQuery            *query;
    InvenioCategory          category;
} InvenioQueryRequest;


static TrackerClient *client;

#define SPARQL_QUERY_HEADER "SELECT ?title ?description ?uri ?location WHERE { "
#define SPARQL_QUERY_FOOTER " } ORDER BY DESC (fts:rank (?urn)) OFFSET 0 LIMIT " G_STRINGIFY (RESULTS_PER_CATEGORY)

static const gchar *queries[INVENIO_CATEGORIES] =
{
    [INVENIO_CATEGORY_APPLICATION]  =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Software ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nie:title ?title ;"
                                        "      nfo:softwareCmdLine ?uri ;"
                                        "      nie:url ?location ."
                                        " OPTIONAL { ?urn nie:comment ?description }"
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_BOOKMARK]     =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Bookmark ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nie:title ?title ;"
                                        "      nie:links ?description ;"
                                        "      nie:links ?uri ;"
                                        "      nie:url ?location ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_CONTACT]      =   SPARQL_QUERY_HEADER
                                        " ?urn a nco:Contact ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nco:fullname ?title ;"
                                        "      nco:fullname ?description ;"
                                        "      nie:url ?location ."
                                        " OPTIONAL { ?urn nco:emailAddress ?uri }"
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_DOCUMENT]     =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Document ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fileName ?title ;"
                                        "      nfo:fileName ?description ;"
                                        "      nie:url ?uri ;"
                                        "      nie:url ?location ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_FOLDER]       =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Folder ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fileName ?title ;"
                                        "      nfo:fileName ?description ;"
                                        "      nie:url ?uri ;"
                                        "      nie:url ?location ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_FONT]         =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Font ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fontFamily ?title ;"
                                        "      nfo:fontFamily ?description ;"
                                        "      nie:url ?uri ;"
                                        "      nie:url ?location ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_IMAGE]        =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Image ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fileName ?title ;"
                                        "      nfo:fileName ?description ;"
                                        "      nie:url ?uri ;"
                                        "      nie:url ?location ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_MESSAGE]      =   SPARQL_QUERY_HEADER
                                        " ?urn a nmo:Message ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nmo:messageSubject ?title ;"
                                        "      nmo:messageSubject ?description ;"
                                        "      nie:url ?uri ;"
                                        "      nie:url ?location ."
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_MUSIC]        =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Audio ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fileName ?title ;"
                                        "      nie:url ?uri ;"
                                        "      nie:url ?location ."
                                        " OPTIONAL { ?urn nie:title ?description }"
                                        SPARQL_QUERY_FOOTER,

    [INVENIO_CATEGORY_VIDEO]        =   SPARQL_QUERY_HEADER
                                        " ?urn a nfo:Video ."
                                        " ?urn fts:match \"%s*\" ."
                                        " ?urn nfo:fileName ?title ;"
                                        "      nie:url ?uri ;"
                                        "      nie:url ?location ."
                                        " OPTIONAL { ?urn nie:title ?description }"
                                        SPARQL_QUERY_FOOTER,
};


InvenioQuery *
invenio_query_new (const gchar * const keywords)
{
    InvenioQuery *query;
    InvenioCategory category;

    query = g_slice_new0 (InvenioQuery);

    for (category = (InvenioCategory) 0; category != INVENIO_CATEGORIES; category++)
    {
        query->queries[category].query = g_strdup_printf (queries[category], keywords);
        query->queries[category].valid = FALSE;
    }

    return query;
}

void
invenio_query_free (InvenioQuery *query)
{
    InvenioCategory category;

    for (category = (InvenioCategory) 0; category != INVENIO_CATEGORIES; category++)
    {
        if (query->queries[category].valid)
            tracker_cancel_call (client, query->queries[category].id);

        g_free (query->queries[category].query);
        g_slist_foreach (query->queries[category].results, (GFunc) invenio_query_result_free, NULL);

    }

    if (query->error)
        g_error_free (query->error);

    g_slice_free (InvenioQuery, query);
}

static void
query_collect_result (gpointer  data,
                      gpointer  user_data)
{
    const gchar **metadata;
    InvenioQueryRequest *request;
    InvenioQuery *query;
    InvenioCategory category;

    metadata = (const gchar **) data;
    request = (InvenioQueryRequest *) user_data;
    query = request->query;
    category = request->category;

    /*
     * NOTE: Metadata content is determined by the SPARQL query.  The order of
     * the output is determined by the SELECT order.  The select order is
     * defined in the SPARQL_QUERY_HEADER macro above.
     */
    /*
     * TODO: Convert metadata indexing to enum+macro to get some additional
     * safety when the query results change.
     */

    query->queries[category].results =
        g_slist_prepend (query->queries[category].results,
                         invenio_query_result_new (metadata[0],     /* title */
                                                   metadata[1],     /* description */
                                                   metadata[2],     /* uri */
                                                   metadata[3]));   /* location */
}

static void
query_collect_results (GPtrArray    *results,
                       GError       *error,
                       gpointer      user_data)
{
    InvenioQueryRequest *request;
    InvenioQuery *query;
    InvenioCategory category;

    request = (InvenioQueryRequest *) user_data;
    query = request->query;
    category = request->category;

    query->error = error;
    query->queries[category].valid = FALSE;

    if (! error && results)
    {
        g_ptr_array_foreach (results, query_collect_result, request);
        query->queries[category].results = g_slist_reverse (query->queries[category].results);

        g_ptr_array_foreach (results, (GFunc) g_strfreev, NULL);
        g_ptr_array_free (results, TRUE);
    }

    g_slice_free (InvenioQueryRequest, request);
    query->callback (query, category, query->error, query->user_data);
}

void
invenio_query_execute_async (InvenioQuery           *query,
                             InvenioQueryCompleted   callback,
                             gpointer                user_data)
{
    InvenioQueryRequest *request;
    InvenioCategory category;

    query->callback = callback;
    query->user_data = user_data;

    if (G_UNLIKELY (! client))
        client = tracker_client_new (TRACKER_CLIENT_ENABLE_WARNINGS, G_MAXINT);

    for (category = (InvenioCategory) 0; category != INVENIO_CATEGORIES; category++)
    {
        if (invenio_configuration_get_search_category (category))
        {
            request = g_slice_new0 (InvenioQueryRequest);

            request->query = query;
            request->category = category;

            query->queries[category].id =
                tracker_resources_sparql_query_async (client, query->queries[category].query,
                                                      query_collect_results, request);
            query->queries[category].valid = TRUE;
        }
    }
}

void
invenio_query_cancel (InvenioQuery *query)
{
    InvenioCategory category;

    for (category = (InvenioCategory) 0; category != INVENIO_CATEGORIES; category++)
    {
        if (query->queries[category].valid)
        {
            tracker_cancel_call (client, query->queries[category].id);
            query->queries[category].valid = FALSE;
        }
    }
}

const GSList *
invenio_query_get_results_for_category (const InvenioQuery * const query,
                                        const InvenioCategory      category)
{
    return query->queries[category].results;
}

