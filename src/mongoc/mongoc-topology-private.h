/*
 * Copyright 2014 MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MONGOC_TOPOLOGY_PRIVATE_H
#define MONGOC_TOPOLOGY_PRIVATE_H

#include "mongoc-read-prefs-private.h"
#include "mongoc-topology-scanner-private.h"
#include "mongoc-server-description-private.h"
#include "mongoc-topology-description-private.h"
#include "mongoc-thread-private.h"
#include "mongoc-uri.h"

#define MONGOC_TOPOLOGY_MIN_HEARTBEAT_FREQUENCY_MS 60000
#define MONGOC_TOPOLOGY_SOCKET_CHECK_INTERVAL_MS 5000 /* TODO make configurable */
#define MONGOC_TOPOLOGY_HEARTBEAT_FREQUENCY_MS 60000 /* TODO make configurable */

typedef enum {
   MONGOC_TOPOLOGY_BG_OFF,
   MONGOC_TOPOLOGY_BG_RUNNING,
   MONGOC_TOPOLOGY_BG_SHUTTING_DOWN,
} mongoc_topology_bg_state_t;

typedef struct _mongoc_topology_t
{
   mongoc_topology_description_t description;
   const mongoc_uri_t           *uri;
   int                           users;
   mongoc_topology_scanner_t    *scanner;

   int64_t                       last_scan;
   int64_t                       timeout_msec;
   int64_t                       heartbeat_msec;

   mongoc_mutex_t                mutex;
   mongoc_cond_t                 cond_client;
   mongoc_cond_t                 cond_server;
   mongoc_thread_t               thread;

   mongoc_topology_bg_state_t    bg_thread_state;
   bool                          scan_requested;
   bool                          scanning;
   bool                          got_ismaster;
   bool                          shutdown_requested;
   bool                          single_threaded;
   bool                          stale;
} mongoc_topology_t;

mongoc_topology_t *
mongoc_topology_new (const mongoc_uri_t *uri);

void
mongoc_topology_grab (mongoc_topology_t *topology);

void
mongoc_topology_release (mongoc_topology_t *topology);

void
mongoc_topology_destroy (mongoc_topology_t *topology);

mongoc_server_description_t *
mongoc_topology_select (mongoc_topology_t         *topology,
                        mongoc_ss_optype_t         optype,
                        const mongoc_read_prefs_t *read_prefs,
                        int64_t                    timeout_ms,
                        int64_t                    local_threshold_msec,
                        bson_error_t              *error);

mongoc_server_description_t *
mongoc_topology_server_by_id (mongoc_topology_t *topology,
                              uint32_t           id);

void
mongoc_topology_request_scan (mongoc_topology_t *topology);

void
mongoc_topology_background_thread_start (mongoc_topology_t *topology);

void
mongoc_topology_background_thread_stop (mongoc_topology_t *topology);

#endif
