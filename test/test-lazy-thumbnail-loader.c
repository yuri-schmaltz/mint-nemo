/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Nemo
 *
 * Copyright (C) 2026 Nemo Developers
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street - Suite 500,
 * Boston, MA 02110-1335, USA.
 */

#include "../libnemo-private/nemo-lazy-thumbnail-loader.h"
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct {
    GMainLoop *loop;
    GdkPixbuf *result_pixbuf;
    gboolean callback_invoked;
} TestData;

static void
test_callback (GdkPixbuf *pixbuf, gpointer user_data)
{
    TestData *data = user_data;
    
    data->callback_invoked = TRUE;
    
    if (pixbuf != NULL) {
        data->result_pixbuf = g_object_ref (pixbuf);
    }
    
    if (data->loop != NULL) {
        g_main_loop_quit (data->loop);
    }
}

static void
test_loader_creation (void)
{
    NemoLazyThumbnailLoader *loader = nemo_lazy_thumbnail_loader_new (2, 50);
    g_assert_nonnull (loader);
    
    guint cache_hits, cache_misses, pending;
    nemo_lazy_thumbnail_loader_get_stats (loader, &cache_hits, &cache_misses, &pending);
    
    g_assert_cmpuint (cache_hits, ==, 0);
    g_assert_cmpuint (cache_misses, ==, 0);
    g_assert_cmpuint (pending, ==, 0);
    
    g_object_unref (loader);
}

static void
test_cache_hit (void)
{
    NemoLazyThumbnailLoader *loader = nemo_lazy_thumbnail_loader_new (2, 50);
    TestData data = { NULL, NULL, FALSE };
    
    /* Create dummy test image */
    GdkPixbuf *test_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 64, 64);
    char *test_file = g_build_filename (g_get_tmp_dir (), "test-thumb.png", NULL);
    gdk_pixbuf_save (test_pixbuf, test_file, "png", NULL, NULL);
    g_object_unref (test_pixbuf);
    
    char *test_uri = g_filename_to_uri (test_file, NULL, NULL);
    
    /* First request (cache miss) */
    data.loop = g_main_loop_new (NULL, FALSE);
    guint request_id = nemo_lazy_thumbnail_loader_request (loader, test_uri, 64, 0, 
                                                            test_callback, &data);
    
    if (request_id != 0) {
        /* Async, wait for callback */
        GSource *timeout = g_timeout_source_new_seconds (5);
        g_source_set_callback (timeout, (GSourceFunc) g_main_loop_quit, data.loop, NULL);
        g_source_attach (timeout, NULL);
        
        g_main_loop_run (data.loop);
        g_source_destroy (timeout);
        g_source_unref (timeout);
    }
    
    g_assert_true (data.callback_invoked);
    g_assert_nonnull (data.result_pixbuf);
    
    guint cache_hits_before, cache_misses_before;
    nemo_lazy_thumbnail_loader_get_stats (loader, &cache_hits_before, &cache_misses_before, NULL);
    
    if (data.result_pixbuf != NULL) {
        g_object_unref (data.result_pixbuf);
        data.result_pixbuf = NULL;
    }
    data.callback_invoked = FALSE;
    
    /* Second request (cache hit) */
    request_id = nemo_lazy_thumbnail_loader_request (loader, test_uri, 64, 0, 
                                                      test_callback, &data);
    
    g_assert_cmpuint (request_id, ==, 0);  /* Immediate completion */
    g_assert_true (data.callback_invoked);
    g_assert_nonnull (data.result_pixbuf);
    
    guint cache_hits_after;
    nemo_lazy_thumbnail_loader_get_stats (loader, &cache_hits_after, NULL, NULL);
    g_assert_cmpuint (cache_hits_after, >, cache_hits_before);
    
    /* Cleanup */
    if (data.result_pixbuf != NULL) {
        g_object_unref (data.result_pixbuf);
    }
    g_main_loop_unref (data.loop);
    g_free (test_uri);
    g_remove (test_file);
    g_free (test_file);
    g_object_unref (loader);
}

static void
test_cancel_request (void)
{
    NemoLazyThumbnailLoader *loader = nemo_lazy_thumbnail_loader_new (1, 50);
    TestData data = { NULL, NULL, FALSE };
    
    /* Queue many requests */
    for (int i = 0; i < 10; i++) {
        char *uri = g_strdup_printf ("file:///tmp/test%d.jpg", i);
        nemo_lazy_thumbnail_loader_request (loader, uri, 128, 0, NULL, NULL);
        g_free (uri);
    }
    
    guint pending_before;
    nemo_lazy_thumbnail_loader_get_stats (loader, NULL, NULL, &pending_before);
    g_assert_cmpuint (pending_before, >, 0);
    
    /* Cancel invisible (keep only first 3) */
    GList *visible = NULL;
    visible = g_list_append (visible, "file:///tmp/test0.jpg");
    visible = g_list_append (visible, "file:///tmp/test1.jpg");
    visible = g_list_append (visible, "file:///tmp/test2.jpg");
    
    nemo_lazy_thumbnail_loader_cancel_invisible (loader, visible);
    
    /* Give worker threads time to process cancellations */
    g_usleep (100000);  /* 100ms */
    
    g_list_free (visible);
    g_object_unref (loader);
}

static void
test_clear_cache (void)
{
    NemoLazyThumbnailLoader *loader = nemo_lazy_thumbnail_loader_new (2, 50);
    
    /* Create dummy image */
    GdkPixbuf *test_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 64, 64);
    char *test_file = g_build_filename (g_get_tmp_dir (), "test-clear.png", NULL);
    gdk_pixbuf_save (test_pixbuf, test_file, "png", NULL, NULL);
    g_object_unref (test_pixbuf);
    
    char *test_uri = g_filename_to_uri (test_file, NULL, NULL);
    
    TestData data = { NULL, NULL, FALSE };
    data.loop = g_main_loop_new (NULL, FALSE);
    
    /* Load into cache */
    guint request_id = nemo_lazy_thumbnail_loader_request (loader, test_uri, 64, 0, 
                                                            test_callback, &data);
    if (request_id != 0) {
        GSource *timeout = g_timeout_source_new_seconds (5);
        g_source_set_callback (timeout, (GSourceFunc) g_main_loop_quit, data.loop, NULL);
        g_source_attach (timeout, NULL);
        
        g_main_loop_run (data.loop);
        g_source_destroy (timeout);
        g_source_unref (timeout);
    }
    
    if (data.result_pixbuf != NULL) {
        g_object_unref (data.result_pixbuf);
        data.result_pixbuf = NULL;
    }
    
    guint cache_hits_before;
    nemo_lazy_thumbnail_loader_get_stats (loader, &cache_hits_before, NULL, NULL);
    
    /* Clear cache */
    nemo_lazy_thumbnail_loader_clear_cache (loader);
    
    /* Next request should be cache miss */
    data.callback_invoked = FALSE;
    request_id = nemo_lazy_thumbnail_loader_request (loader, test_uri, 64, 0, 
                                                      test_callback, &data);
    
    if (request_id != 0) {
        GSource *timeout = g_timeout_source_new_seconds (5);
        g_source_set_callback (timeout, (GSourceFunc) g_main_loop_quit, data.loop, NULL);
        g_source_attach (timeout, NULL);
        
        g_main_loop_run (data.loop);
        g_source_destroy (timeout);
        g_source_unref (timeout);
    }
    
    guint cache_hits_after;
    nemo_lazy_thumbnail_loader_get_stats (loader, &cache_hits_after, NULL, NULL);
    g_assert_cmpuint (cache_hits_after, ==, cache_hits_before);  /* No new hits */
    
    /* Cleanup */
    if (data.result_pixbuf != NULL) {
        g_object_unref (data.result_pixbuf);
    }
    g_main_loop_unref (data.loop);
    g_free (test_uri);
    g_remove (test_file);
    g_free (test_file);
    g_object_unref (loader);
}

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    
    g_test_add_func ("/lazy-thumbnail-loader/creation", test_loader_creation);
    g_test_add_func ("/lazy-thumbnail-loader/cache-hit", test_cache_hit);
    g_test_add_func ("/lazy-thumbnail-loader/cancel", test_cancel_request);
    g_test_add_func ("/lazy-thumbnail-loader/clear-cache", test_clear_cache);
    
    return g_test_run ();
}
