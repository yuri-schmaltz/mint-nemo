/* nemo-icon-text-cache.c — Caching layer for icon text measurement
 *
 * Implementation of text measurement cache.
 * Uses a GHashTable keyed by (filename:max_width) pair.
 *
 * License: GPL-2+ (same as Nemo)
 */

#include <config.h>
#include "nemo-icon-text-cache.h"
#include <glib.h>
#include <string.h>

struct _NemoIconTextCache {
	GHashTable *measurements;  /* key = filename:max_width, value = NemoIconTextMeasurement* */
	guint max_entries;
	guint ttl_seconds;
	guint64 hits;
	guint64 misses;
};

typedef struct {
	NemoIconTextMeasurement measurement;
	guint64 timestamp;  /* Time when cached (seconds since epoch) */
} CachedMeasurement;

/* Create a key string from filename and max_width */
static gchar*
make_cache_key (const char *filename, int max_width)
{
	return g_strdup_printf ("%s:%d", filename, max_width);
}

/* Free a cached measurement */
static void
cached_measurement_free (gpointer data)
{
	g_free (data);
}

NemoIconTextCache*
nemo_icon_text_cache_new (guint max_entries, guint ttl_seconds)
{
	NemoIconTextCache *cache;

	cache = g_new0 (NemoIconTextCache, 1);
	cache->measurements = g_hash_table_new_full (g_str_hash, g_str_equal,
	                                              g_free,
	                                              cached_measurement_free);
	cache->max_entries = max_entries > 0 ? max_entries : 500;
	cache->ttl_seconds = ttl_seconds;
	cache->hits = 0;
	cache->misses = 0;

	return cache;
}

void
nemo_icon_text_cache_free (NemoIconTextCache *cache)
{
	if (cache == NULL) {
		return;
	}

	g_hash_table_unref (cache->measurements);
	g_free (cache);
}

gboolean
nemo_icon_text_cache_lookup (NemoIconTextCache *cache,
                             const char *filename,
                             int max_width,
                             NemoIconTextMeasurement *measurement)
{
	CachedMeasurement *cached;
	gchar *key;
	guint64 now;

	g_return_val_if_fail (cache != NULL, FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (measurement != NULL, FALSE);

	key = make_cache_key (filename, max_width);
	cached = g_hash_table_lookup (cache->measurements, key);

	if (cached != NULL) {
		/* Check TTL if enabled */
		if (cache->ttl_seconds > 0) {
			now = g_get_monotonic_time () / 1000000;  /* Convert µs to seconds */
			if (now - cached->timestamp > cache->ttl_seconds) {
				/* Entry expired */
				g_hash_table_remove (cache->measurements, key);
				g_free (key);
				cache->misses++;
				return FALSE;
			}
		}

		/* Copy cached measurement */
		*measurement = cached->measurement;
		cache->hits++;
		g_free (key);
		return TRUE;
	}

	cache->misses++;
	g_free (key);
	return FALSE;
}

void
nemo_icon_text_cache_insert (NemoIconTextCache *cache,
                             const char *filename,
                             int max_width,
                             const NemoIconTextMeasurement *measurement)
{
	CachedMeasurement *cached;
	gchar *key;

	g_return_if_fail (cache != NULL);
	g_return_if_fail (filename != NULL);
	g_return_if_fail (measurement != NULL);

	/* Simple eviction: remove oldest when cache is full */
	if (g_hash_table_size (cache->measurements) >= cache->max_entries) {
		/* For simplicity, clear the entire cache. In production, use LRU. */
		g_hash_table_remove_all (cache->measurements);
	}

	key = make_cache_key (filename, max_width);
	cached = g_new0 (CachedMeasurement, 1);
	cached->measurement = *measurement;
	cached->timestamp = g_get_monotonic_time () / 1000000;  /* µs to seconds */

	g_hash_table_insert (cache->measurements, key, cached);
}

void
nemo_icon_text_cache_invalidate_filename (NemoIconTextCache *cache,
                                          const char *filename)
{
	GHashTableIter iter;
	gpointer key_ptr, value_ptr;

	g_return_if_fail (cache != NULL);
	g_return_if_fail (filename != NULL);

	/* Remove all entries matching this filename */
	g_hash_table_iter_init (&iter, cache->measurements);
	while (g_hash_table_iter_next (&iter, &key_ptr, &value_ptr)) {
		const char *key = (const char *) key_ptr;
		if (g_str_has_prefix (key, filename)) {
			g_hash_table_iter_remove (&iter);
		}
	}
}

void
nemo_icon_text_cache_clear (NemoIconTextCache *cache)
{
	g_return_if_fail (cache != NULL);
	g_hash_table_remove_all (cache->measurements);
}

NemoIconTextCacheStats
nemo_icon_text_cache_get_stats (NemoIconTextCache *cache)
{
	NemoIconTextCacheStats stats = { 0 };

	if (cache != NULL) {
		stats.total_entries = g_hash_table_size (cache->measurements);
		stats.hits = cache->hits;
		stats.misses = cache->misses;
	}

	return stats;
}
