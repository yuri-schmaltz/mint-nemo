/* nemo-icon-text-cache.h — Caching layer for icon text measurement
 *
 * Performance optimization to reduce Pango layout computation during scrolling.
 * Caches text dimensions by filename hash + layout width.
 *
 * This is used by nemo-icon-canvas-item.c to avoid recalculating text
 * dimensions for every draw operation in large folders (500+ items).
 *
 * License: GPL-2+ (same as Nemo)
 */

#ifndef NEMO_ICON_TEXT_CACHE_H
#define NEMO_ICON_TEXT_CACHE_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct {
	int text_width;
	int text_height;
	int text_dx;
	int text_height_for_layout;
	int text_height_for_entire_text;
} NemoIconTextMeasurement;

typedef struct _NemoIconTextCache NemoIconTextCache;

/* Create/destroy a text measurement cache.
 * max_entries: Maximum number of cached measurements (e.g., 500).
 * ttl_seconds: Time-to-live in seconds; 0 = no expiry.
 */
NemoIconTextCache* nemo_icon_text_cache_new   (guint max_entries, guint ttl_seconds);
void               nemo_icon_text_cache_free  (NemoIconTextCache *cache);

/* Look up a cached measurement by filename and max_width.
 * Returns TRUE and fills *measurement if found; FALSE if cache miss.
 */
gboolean nemo_icon_text_cache_lookup (NemoIconTextCache *cache,
                                      const char *filename,
                                      int max_width,
                                      NemoIconTextMeasurement *measurement);

/* Store a measurement in the cache.
 * filename: The item's filename (used as key).
 * max_width: Layout constraint width.
 * measurement: Pointer to measurement struct to cache.
 */
void nemo_icon_text_cache_insert (NemoIconTextCache *cache,
                                  const char *filename,
                                  int max_width,
                                  const NemoIconTextMeasurement *measurement);

/* Invalidate entries related to a filename.
 * Call when a file is renamed or edited.
 */
void nemo_icon_text_cache_invalidate_filename (NemoIconTextCache *cache,
                                               const char *filename);

/* Clear all cache entries.
 */
void nemo_icon_text_cache_clear (NemoIconTextCache *cache);

/* Get cache statistics for debugging.
 */
typedef struct {
	guint total_entries;
	guint hits;
	guint misses;
} NemoIconTextCacheStats;

NemoIconTextCacheStats
nemo_icon_text_cache_get_stats (NemoIconTextCache *cache);

G_END_DECLS

#endif /* NEMO_ICON_TEXT_CACHE_H */
