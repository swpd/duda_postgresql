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

#ifndef POSTGRESQL_UTIL_H
#define POSTGRESQL_UTIL_H

char *postgresql_util_escape_literal(postgresql_conn_t *conn, const char *str,
                                     size_t length);

char *postgresql_util_escape_identifier(postgresql_conn_t *conn, const char *str,
                                        size_t length);

unsigned char *postgresql_util_escape_binary(postgresql_conn_t *conn,
                                             const unsigned char *from,
                                             size_t from_length,
                                             size_t *to_length);

unsigned char *postgresql_util_unescape_binary(const unsigned char *from,
                                               size_t *to_length);

void postgresql_util_free(void *ptr);

#endif
