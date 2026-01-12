/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Nemo
 *
 * Copyright (C) 2026 Nemo Developers
 *
 * Nemo is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Nemo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; see the file COPYING.  If not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street - Suite 500,
 * Boston, MA 02110-1335, USA.
 */

#include "nemo-lazy-thumbnail-loader.h"
#include <gio/gio.h>
#include <string.h>

struct _NemoLazyThumbnailLoader {
    GObject parent_instance;
    
    GThreadPool *thread_pool;
    GMutex mutex;
    GHashTable *pending_requests;  /* request_id -> NemoLazyThumbnailRequest */
    GHashTable *cache;             /* uri:size -> GdkPixbuf */
    GQueue *lru_queue;             /* LRU order for cache eviction */
    guint next_request_id;
    guint cache_size_bytes;
    guint cache_max_bytes;
    guint cache_hits;
    guint cache_misses;
};

struct _NemoLazyThumbnailRequest {
    guint id;
    char *uri;
    guint size;
    guint priority;
    NemoLazyThumbnailCallback callback;
    gpointer user_data;
    NemoLazyThumbnailLoader *loader;
    gboolean cancelled;
};

G_DEFINE_TYPE (NemoLazyThumbnailLoader, nemo_lazy_thumbnail_loader, G_TYPE_OBJECT)

/* --- Helpers --- */

static char *
make_cache_key (const char *uri, guint size)
{
    return g_strdup_printf ("%s:%u", uri, size);
}

static guint
estimate_pixbuf_size (GdkPixbuf *pixbuf)
{
    if (pixbuf == NULL) {
        return 0;
    }
    
    guint width = gdk_pixbuf_get_width (pixbuf);
    guint height = gdk_pixbuf_get_height (pixbuf);
    guint n_channels = gdk_pixbuf_get_n_channels (pixbuf);
    
    return width * height * n_channels;
}

static gboolean
idle_callback_wrapper (gpointer user_data)
{
    NemoLazyThumbnailRequest *request = user_data;
    
    if (!request->cancelled && request->callback != NULL) {
        /* Lookup in cache (may have been loaded by another thread) */
        char *cache_key = make_cache_key (request->uri, request->size);
        
        g_mutex_lock (&request->loader->mutex);
        GdkPixbuf *pixbuf = g_hash_table_lookup (request->loader->cache, cache_key);
        if (pixbuf != NULL) {
            g_object_ref (pixbuf);
        }
        g_mutex_unlock (&request->loader->mutex);
        
        request->callback (pixbuf, request->user_data);
        
        if (pixbuf != NULL) {
            g_object_unref (pixbuf);
        }
        
        g_free (cache_key);
    }
    
    /* Cleanup request */
    g_mutex_lock (&request->loader->mutex);
    g_hash_table_remove (request->loader->pending_requests, GUINT_TO_POINTER (request->id));
    g_mutex_unlock (&request->loader->mutex);
    
    g_free (request->uri);
    g_free (request);
    
    return G_SOURCE_REMOVE;
}

static void
thread_worker (gpointer data, gpointer user_data)
{
    NemoLazyThumbnailRequest *request = data;
    NemoLazyThumbnailLoader *loader = request->loader;
    
    if (request->cancelled) {
        g_free (request->uri);
        g_free (request);
        return;
    }
    
    /* Check cache first */
    char *cache_key = make_cache_key (request->uri, request->size);
    
    g_mutex_lock (&loader->mutex);
    GdkPixbuf *cached = g_hash_table_lookup (loader->cache, cache_key);
    if (cached != NULL) {
        loader->cache_hits++;
        g_mutex_unlock (&loader->mutex);
        
        g_idle_add (idle_callback_wrapper, request);
        g_free (cache_key);
        return;
    }
    loader->cache_misses++;
    g_mutex_unlock (&loader->mutex);
    
    /* Load thumbnail */
    GError *error = NULL;
    GFile *file = g_file_new_for_uri (request->uri);
    char *path = g_file_get_path (file);
    g_object_unref (file);
    
    GdkPixbuf *pixbuf = NULL;
    if (path != NULL && !request->cancelled) {
        pixbuf = gdk_pixbuf_new_from_file_at_size (path, 
                                                     request->size, 
                                                     request->size, 
                                                     &error);
        if (error != NULL) {
            g_debug ("Failed to load thumbnail for %s: %s", 
                     request->uri, error->message);
            g_error_free (error);
        }
    }
    g_free (path);
    
    if (pixbuf != NULL && !request->cancelled) {
        /* Insert into cache */
        guint pixbuf_size = estimate_pixbuf_size (pixbuf);
        
        g_mutex_lock (&loader->mutex);
        
        /* Evict old entries if cache full */
        while (loader->cache_size_bytes + pixbuf_size > loader->cache_max_bytes &&
               g_queue_get_length (loader->lru_queue) > 0) {
            char *old_key = g_queue_pop_head (loader->lru_queue);
            GdkPixbuf *old_pixbuf = g_hash_table_lookup (loader->cache, old_key);
            
            if (old_pixbuf != NULL) {
                loader->cache_size_bytes -= estimate_pixbuf_size (old_pixbuf);
                g_hash_table_remove (loader->cache, old_key);
            }
            
            g_free (old_key);
        }
        
        /* Insert new entry */
        g_hash_table_insert (loader->cache, g_strdup (cache_key), g_object_ref (pixbuf));
        g_queue_push_tail (loader->lru_queue, g_strdup (cache_key));
        loader->cache_size_bytes += pixbuf_size;
        
        g_mutex_unlock (&loader->mutex);
    }
    
    g_free (cache_key);
    
    /* Schedule callback in main thread */
    g_idle_add (idle_callback_wrapper, request);
    
    if (pixbuf != NULL) {
        g_object_unref (pixbuf);
    }
}

/* --- GObject Implementation --- */

static void
nemo_lazy_thumbnail_loader_finalize (GObject *object)
{
    NemoLazyThumbnailLoader *loader = NEMO_LAZY_THUMBNAIL_LOADER (object);
    
    g_thread_pool_free (loader->thread_pool, TRUE, TRUE);
    g_hash_table_destroy (loader->pending_requests);
    g_hash_table_destroy (loader->cache);
    g_queue_free_full (loader->lru_queue, g_free);
    g_mutex_clear (&loader->mutex);
    
    G_OBJECT_CLASS (nemo_lazy_thumbnail_loader_parent_class)->finalize (object);
}

static void
nemo_lazy_thumbnail_loader_class_init (NemoLazyThumbnailLoaderClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = nemo_lazy_thumbnail_loader_finalize;
}

static void
nemo_lazy_thumbnail_loader_init (NemoLazyThumbnailLoader *loader)
{
    g_mutex_init (&loader->mutex);
    loader->next_request_id = 1;
    loader->cache_hits = 0;
    loader->cache_misses = 0;
    loader->cache_size_bytes = 0;
}

/* --- Public API --- */

NemoLazyThumbnailLoader *
nemo_lazy_thumbnail_loader_new (guint num_threads, guint cache_size_mb)
{
    NemoLazyThumbnailLoader *loader = g_object_new (NEMO_TYPE_LAZY_THUMBNAIL_LOADER, NULL);
    
    loader->thread_pool = g_thread_pool_new (thread_worker, NULL, num_threads, FALSE, NULL);
    loader->pending_requests = g_hash_table_new (g_direct_hash, g_direct_equal);
    loader->cache = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
    loader->lru_queue = g_queue_new ();
    loader->cache_max_bytes = cache_size_mb * 1024 * 1024;
    
    return loader;
}

guint
nemo_lazy_thumbnail_loader_request (NemoLazyThumbnailLoader   *loader,
                                     const char                *file_uri,
                                     guint                      size,
                                     guint                      priority,
                                     NemoLazyThumbnailCallback  callback,
                                     gpointer                   user_data)
{
    g_return_val_if_fail (NEMO_IS_LAZY_THUMBNAIL_LOADER (loader), 0);
    g_return_val_if_fail (file_uri != NULL, 0);
    
    /* Check cache first (fast path) */
    char *cache_key = make_cache_key (file_uri, size);
    
    g_mutex_lock (&loader->mutex);
    GdkPixbuf *cached = g_hash_table_lookup (loader->cache, cache_key);
    if (cached != NULL) {
        loader->cache_hits++;
        g_object_ref (cached);
        g_mutex_unlock (&loader->mutex);
        
        if (callback != NULL) {
            callback (cached, user_data);
        }
        
        g_object_unref (cached);
        g_free (cache_key);
        return 0;  /* Immediate completion, no request ID */
    }
    g_mutex_unlock (&loader->mutex);
    
    g_free (cache_key);
    
    /* Create request */
    NemoLazyThumbnailRequest *request = g_new0 (NemoLazyThumbnailRequest, 1);
    request->uri = g_strdup (file_uri);
    request->size = size;
    request->priority = priority;
    request->callback = callback;
    request->user_data = user_data;
    request->loader = loader;
    request->cancelled = FALSE;
    
    g_mutex_lock (&loader->mutex);
    request->id = loader->next_request_id++;
    g_hash_table_insert (loader->pending_requests, 
                         GUINT_TO_POINTER (request->id), 
                         request);
    g_mutex_unlock (&loader->mutex);
    
    /* Queue for background processing */
    g_thread_pool_push (loader->thread_pool, request, NULL);
    
    return request->id;
}

void
nemo_lazy_thumbnail_loader_cancel (NemoLazyThumbnailLoader *loader,
                                    guint                    request_id)
{
    g_return_if_fail (NEMO_IS_LAZY_THUMBNAIL_LOADER (loader));
    
    g_mutex_lock (&loader->mutex);
    NemoLazyThumbnailRequest *request = g_hash_table_lookup (loader->pending_requests,
                                                              GUINT_TO_POINTER (request_id));
    if (request != NULL) {
        request->cancelled = TRUE;
    }
    g_mutex_unlock (&loader->mutex);
}

void
nemo_lazy_thumbnail_loader_cancel_invisible (NemoLazyThumbnailLoader *loader,
                                              GList                   *visible_uris)
{
    g_return_if_fail (NEMO_IS_LAZY_THUMBNAIL_LOADER (loader));
    
    GHashTable *visible_set = g_hash_table_new (g_str_hash, g_str_equal);
    for (GList *l = visible_uris; l != NULL; l = l->next) {
        g_hash_table_add (visible_set, l->data);
    }
    
    g_mutex_lock (&loader->mutex);
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init (&iter, loader->pending_requests);
    
    while (g_hash_table_iter_next (&iter, &key, &value)) {
        NemoLazyThumbnailRequest *request = value;
        if (!g_hash_table_contains (visible_set, request->uri)) {
            request->cancelled = TRUE;
        }
    }
    g_mutex_unlock (&loader->mutex);
    
    g_hash_table_destroy (visible_set);
}

void
nemo_lazy_thumbnail_loader_clear_cache (NemoLazyThumbnailLoader *loader)
{
    g_return_if_fail (NEMO_IS_LAZY_THUMBNAIL_LOADER (loader));
    
    g_mutex_lock (&loader->mutex);
    g_hash_table_remove_all (loader->cache);
    g_queue_clear_full (loader->lru_queue, g_free);
    loader->cache_size_bytes = 0;
    g_mutex_unlock (&loader->mutex);
}

void
nemo_lazy_thumbnail_loader_get_stats (NemoLazyThumbnailLoader *loader,
                                       guint                   *cache_hits,
                                       guint                   *cache_misses,
                                       guint                   *pending_requests)
{
    g_return_if_fail (NEMO_IS_LAZY_THUMBNAIL_LOADER (loader));
    
    g_mutex_lock (&loader->mutex);
    
    if (cache_hits != NULL) {
        *cache_hits = loader->cache_hits;
    }
    
    if (cache_misses != NULL) {
        *cache_misses = loader->cache_misses;
    }
    
    if (pending_requests != NULL) {
        *pending_requests = g_hash_table_size (loader->pending_requests);
    }
    
    g_mutex_unlock (&loader->mutex);
}
