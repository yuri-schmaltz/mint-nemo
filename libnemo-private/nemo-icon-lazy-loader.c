/* nemo-icon-lazy-loader.c — Lazy-loading implementation
 *
 * Simple thread-pool based lazy-loader for off-screen icon rendering.
 *
 * License: GPL-2+ (same as Nemo)
 */

#include <config.h>
#include "nemo-icon-lazy-loader.h"
#include <glib.h>

typedef struct {
	gpointer item_data;
	NemoIconLoadCallback callback;
	gpointer callback_data;
	int priority;
	guint64 enqueue_time;
} LoadTask;

struct _NemoIconLazyLoader {
	GThreadPool *pool;
	GQueue *queue;
	GMutex lock;
	
	int viewport_x, viewport_y;
	int viewport_width, viewport_height;
	
	gboolean paused;
	
	guint64 stats_loaded;
	guint64 stats_failed;
};

static void
load_task_free (LoadTask *task)
{
	if (task != NULL) {
		g_free (task);
	}
}

static gint
compare_tasks_by_priority (gconstpointer a, gconstpointer b, gpointer user_data)
{
	const LoadTask *ta = (const LoadTask *) a;
	const LoadTask *tb = (const LoadTask *) b;
	
	/* Lower priority value = higher priority in queue */
	if (ta->priority != tb->priority) {
		return ta->priority - tb->priority;
	}
	
	/* If same priority, prefer newer (more recent in queue) */
	return (ta->enqueue_time > tb->enqueue_time) ? -1 : 1;
}

/* Dummy worker function (no-op in this basic implementation)
 * In production, this would load the thumbnail asynchronously.
 */
static void
load_worker (gpointer task_data, gpointer user_data)
{
	LoadTask *task = (LoadTask *) task_data;
	NemoIconLazyLoader *loader = (NemoIconLazyLoader *) user_data;
	
	if (task == NULL || loader == NULL) {
		return;
	}
	
	/* Simulate asynchronous work (in reality, would load thumbnail) */
	g_usleep (100);  /* 100 µs delay */
	
	/* Call callback on main thread if available */
	if (task->callback != NULL) {
		/* Note: In production, must use g_idle_add() to call on main thread */
		task->callback (task->item_data, TRUE, task->callback_data);
	}
	
	g_atomic_int_add ((gint *) &loader->stats_loaded, 1);
}

NemoIconLazyLoader*
nemo_icon_lazy_loader_new (guint num_threads)
{
	NemoIconLazyLoader *loader;
	
	loader = g_new0 (NemoIconLazyLoader, 1);
	loader->queue = g_queue_new ();
	g_mutex_init (&loader->lock);
	
	/* Create thread pool for thumbnail loading */
	loader->pool = g_thread_pool_new (load_worker, loader,
	                                   num_threads > 0 ? num_threads : 2,
	                                   FALSE,  /* exclusive = FALSE (threads can share) */
	                                   NULL);
	
	loader->paused = FALSE;
	loader->stats_loaded = 0;
	loader->stats_failed = 0;
	
	return loader;
}

void
nemo_icon_lazy_loader_free (NemoIconLazyLoader *loader)
{
	if (loader == NULL) {
		return;
	}
	
	/* Wait for all pending tasks to complete */
	if (loader->pool != NULL) {
		g_thread_pool_free (loader->pool, FALSE, TRUE);
	}
	
	/* Free queue */
	if (loader->queue != NULL) {
		g_queue_free_full (loader->queue, (GDestroyNotify) load_task_free);
	}
	
	g_mutex_clear (&loader->lock);
	g_free (loader);
}

gboolean
nemo_icon_lazy_loader_is_visible (NemoIconLazyLoader *loader,
                                  int item_x, int item_y,
                                  int item_width, int item_height)
{
	if (loader == NULL) {
		return FALSE;
	}
	
	/* Simple AABB (axis-aligned bounding box) intersection test */
	return !(item_x + item_width < loader->viewport_x ||
	         item_x > loader->viewport_x + loader->viewport_width ||
	         item_y + item_height < loader->viewport_y ||
	         item_y > loader->viewport_y + loader->viewport_height);
}

void
nemo_icon_lazy_loader_enqueue (NemoIconLazyLoader *loader,
                               gpointer item_data,
                               NemoIconLoadCallback callback,
                               gpointer callback_data,
                               int priority)
{
	LoadTask *task;
	
	g_return_if_fail (loader != NULL);
	
	task = g_new0 (LoadTask, 1);
	task->item_data = item_data;
	task->callback = callback;
	task->callback_data = callback_data;
	task->priority = priority;
	task->enqueue_time = g_get_monotonic_time ();
	
	g_mutex_lock (&loader->lock);
	
	if (!loader->paused) {
		/* Insert into queue sorted by priority */
		g_queue_insert_sorted (loader->queue, task, compare_tasks_by_priority, NULL);
		
		/* Push to thread pool */
		if (loader->pool != NULL) {
			LoadTask *queued = (LoadTask *) g_queue_pop_head (loader->queue);
			if (queued != NULL) {
				g_thread_pool_push (loader->pool, queued, NULL);
			}
		}
	} else {
		/* If paused, just queue it */
		g_queue_insert_sorted (loader->queue, task, compare_tasks_by_priority, NULL);
	}
	
	g_mutex_unlock (&loader->lock);
}

void
nemo_icon_lazy_loader_cancel (NemoIconLazyLoader *loader,
                              gpointer item_data)
{
	GList *link;
	
	g_return_if_fail (loader != NULL);
	
	g_mutex_lock (&loader->lock);
	
	/* Find and remove from queue */
	link = g_queue_find_custom (loader->queue, item_data,
	                             (GCompareFunc) g_direct_equal);
	if (link != NULL) {
		LoadTask *task = (LoadTask *) link->data;
		g_queue_delete_link (loader->queue, link);
		load_task_free (task);
	}
	
	g_mutex_unlock (&loader->lock);
}

void
nemo_icon_lazy_loader_set_viewport (NemoIconLazyLoader *loader,
                                    int viewport_x, int viewport_y,
                                    int viewport_width, int viewport_height)
{
	g_return_if_fail (loader != NULL);
	
	g_mutex_lock (&loader->lock);
	
	loader->viewport_x = viewport_x;
	loader->viewport_y = viewport_y;
	loader->viewport_width = viewport_width;
	loader->viewport_height = viewport_height;
	
	g_mutex_unlock (&loader->lock);
}

void
nemo_icon_lazy_loader_pause (NemoIconLazyLoader *loader)
{
	g_return_if_fail (loader != NULL);
	g_mutex_lock (&loader->lock);
	loader->paused = TRUE;
	g_mutex_unlock (&loader->lock);
}

void
nemo_icon_lazy_loader_resume (NemoIconLazyLoader *loader)
{
	g_return_if_fail (loader != NULL);
	g_mutex_lock (&loader->lock);
	loader->paused = FALSE;
	g_mutex_unlock (&loader->lock);
}

NemoIconLazyLoaderStats
nemo_icon_lazy_loader_get_stats (NemoIconLazyLoader *loader)
{
	NemoIconLazyLoaderStats stats = { 0 };
	
	if (loader != NULL) {
		g_mutex_lock (&loader->lock);
		stats.queued_items = g_queue_get_length (loader->queue);
		stats.loaded = g_atomic_int_get ((gint *) &loader->stats_loaded);
		stats.failed = g_atomic_int_get ((gint *) &loader->stats_failed);
		g_mutex_unlock (&loader->lock);
	}
	
	return stats;
}
