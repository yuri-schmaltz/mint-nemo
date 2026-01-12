/* nemo-icon-lazy-loader.h — Lazy-loading and thumbnail rendering optimization
 *
 * This module provides off-screen icon lazy-loading capability to improve
 * performance when navigating large folders (500+ items).
 *
 * Strategy:
 * - Render only visible icons immediately
 * - Enqueue off-screen icons for background loading
 * - Prioritize icons near the viewport
 * - Use thread pool for thumbnail loading
 *
 * License: GPL-2+ (same as Nemo)
 */

#ifndef NEMO_ICON_LAZY_LOADER_H
#define NEMO_ICON_LAZY_LOADER_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _NemoIconLazyLoader NemoIconLazyLoader;

typedef void (*NemoIconLoadCallback) (gpointer item_data, gboolean success, gpointer user_data);

/* Create a lazy loader with thread pool.
 * num_threads: Number of worker threads for thumbnail loading (e.g., 2–4).
 */
NemoIconLazyLoader* nemo_icon_lazy_loader_new   (guint num_threads);
void                nemo_icon_lazy_loader_free  (NemoIconLazyLoader *loader);

/* Check if item is visible in viewport.
 * Returns TRUE if item is within viewport bounds.
 */
gboolean nemo_icon_lazy_loader_is_visible (NemoIconLazyLoader *loader,
                                           int item_x, int item_y,
                                           int item_width, int item_height);

/* Enqueue an icon for lazy-loading.
 * callback: Called when thumbnail is loaded or failed.
 * priority: Lower values = higher priority (0 is immediate).
 */
void nemo_icon_lazy_loader_enqueue (NemoIconLazyLoader *loader,
                                    gpointer item_data,
                                    NemoIconLoadCallback callback,
                                    gpointer callback_data,
                                    int priority);

/* Cancel pending loads for an item.
 */
void nemo_icon_lazy_loader_cancel (NemoIconLazyLoader *loader,
                                   gpointer item_data);

/* Update viewport bounds to adjust loading priority.
 * Call during scroll to prioritize visible items.
 */
void nemo_icon_lazy_loader_set_viewport (NemoIconLazyLoader *loader,
                                         int viewport_x, int viewport_y,
                                         int viewport_width, int viewport_height);

/* Pause/resume loading (e.g., during drag/drop).
 */
void nemo_icon_lazy_loader_pause  (NemoIconLazyLoader *loader);
void nemo_icon_lazy_loader_resume (NemoIconLazyLoader *loader);

/* Statistics for debugging.
 */
typedef struct {
	guint queued_items;
	guint in_progress;
	guint loaded;
	guint failed;
} NemoIconLazyLoaderStats;

NemoIconLazyLoaderStats
nemo_icon_lazy_loader_get_stats (NemoIconLazyLoader *loader);

G_END_DECLS

#endif /* NEMO_ICON_LAZY_LOADER_H */
