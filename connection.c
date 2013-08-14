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

#include <libpq-fe.h>
#include "common.h"
#include "connection_priv.h"

static inline postgresql_conn_t *__postgresql_conn_create(duda_request_t *dr,
                                                          postgresql_connect_cb *cb)
{
    postgresql_conn_t *conn = monkey->mem_alloc(sizeof(postgresql_conn_t));
    if (!conn) {
        return NULL;
    }

    conn->dr                   = dr;
    conn->conn                 = NULL;
    conn->fd                   = 0;
    conn->connect_cb           = cb;
    conn->disconnect_cb        = NULL;
    conn->state                = CONN_STATE_CLOSE;
    conn->disconnect_on_finish = 0;
    mk_list_init(&conn->queries);

    return conn;
}

static inline void __postgresql_conn_handle_connect(postgresql_conn_t *conn)
{
    if (!conn->conn) {
        FREE(conn);
        return;
    }

    if (PQstatus(conn->conn) == CONNECTION_BAD) {
        msg->err("PostgreSQL Connect Error: %s", PQerrorMessage(conn->conn));
        if (conn->connect_cb) {
            conn->connect_cb(conn, POSTGRESQL_ERR, conn->dr);
        }
        goto cleanup;
    }

    /* set soecket non-blocking mode */
    int ret = PQsetnonblocking(conn->conn, 1);
    if (ret == -1) {
        msg->err("PostgreSQL Set Non-blocking Error");
        goto cleanup;
    }
    
    conn->fd = PQsocket(conn->conn);
    if (conn->state == CONNECTION_OK || conn->state == CONNECTION_MADE) {
    }

    int events = 0;
    int status = PQconnectPoll(conn->conn);

    if (status == PGRES_POLLING_FAILED) {
        msg->err("PostgreSQL Connect Error: %s", PQerrorMessage(conn->conn));
        if (conn->connect_cb) {
            conn->connect_cb(conn, POSTGRESQL_ERR, conn->dr);
        }
        goto cleanup;
    } else if (status == PGRES_POLLING_OK) {
        /* on connected */
        if (conn->connect_cb) {
            conn->connect_cb(conn, POSTGRESQL_OK, conn->dr);
        }
        conn->state = CONN_STATE_CONNECTED;
    } else {
        if (status & PGRES_POLLING_READING) {
            events |= DUDA_EVENT_READ;
        }
        if (status & PGRES_POLLING_WRITING) {
            events |= DUDA_EVENT_WRITE;
        }
        conn->state = CONN_STATE_CONNECTING;
    }

    event->add(conn->fd, events, DUDA_EVENT_LEVEL_TRIGGERED,
               postgresql_on_read, postgresql_on_write, postgresql_on_error,
               postgresql_on_close, postgresql_on_timeout, NULL);

    struct mk_list *conn_list = global->get(postgresql_conn_list);
    if (!conn_list) {
        conn_list = monkey->mem_alloc(sizeof(struct mk_list));
        if (!conn_list) {
            msg->err("PostgreSQL Connection List Init Error");
            goto cleanup;
        }
        mk_list_init(conn_list);
        global->set(postgresql_conn_list, (void *) conn_list);
    }
    mk_list_add(&conn->_head, conn_list);

cleanup:
    PQfinish(conn->conn);
    FREE(conn);
}

postgresql_conn_t *postgresql_conn_connect(duda_request_t *dr, postgresql_connect_cb *cb,
                                           const char **keys, const char **values,
                                           int expand_dbname)
{
    postgresql_conn_t *conn = __postgresql_conn_create(dr, cb);
    if (!conn) {
        return NULL;
    }

    conn->conn = PQconnectStartParams(keys, values, expand_dbname);

    __postgresql_conn_handle_connect(conn);

    return conn;
}

postgresql_conn_t *postgresql_conn_connect_url(duda_request_t *dr, postgresql_connect_cb *cb,
                                               const char *url)
{
    postgresql_conn_t *conn = __postgresql_conn_create(dr, cb);
    if (!conn) {
        return NULL;
    }

    conn->conn = PQconnectStart(url);

    __postgresql_conn_handle_connect(conn);

    return conn;
}

static void __postgresql_conn_handle_release(postgresql_conn_t *conn, int status)
{
    event->delete(conn->fd);
    if (conn->disconnect_cb) {
        conn->disconnect_cb(conn, status, conn->dr);
    }
    conn->state = CONN_STATE_CLOSED;
    mk_list_del(&conn->_head);
    PQfinish(conn->conn);
    FREE(conn);
}

void postgresql_conn_disconnect(postgresql_conn_t *conn, postgresql_disconnect_cb *cb)
{
    conn->disconnect_cb = disconnect_cb;
    if (conn->state != CONN_STATE_CONNECTED) {
        conn->disconnect_on_finish = 1;
        return;
    }
    __postgresql_conn_handle_release(conn, POSTGRESQL_OK);
}
