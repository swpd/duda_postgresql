/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2013, Zeying Xie <swpdtz at gmail dot com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef POSTGRESQL_QUERY_PRIV_H
#define POSTGRESQL_QUERY_PRIV_H

#include "query.h"

struct postgresql_query {
    const char *query_str;
    PGresult *result;
    int n_fields;
    char **fields;

    postgresql_query_result_cb *result_cb;
    postgresql_query_row_cb *row_cb;
    postgresql_query_end_cb *end_cb;
    void *privdata;

    struct mk_list _head;
};

postgresql_query_t *postgresql_query_init(const char *query_str,
                                          postgresql_query_result_cb *result_cb,
                                          postgresql_query_row_cb *row_cb,
                                          postgresql_query_end_cb *end_cb,
                                          void *privdata);

void postgresql_query_free(postgresql_query_t *query);

#endif