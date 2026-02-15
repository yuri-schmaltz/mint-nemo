#include "nemo-icon-text-cache.h"

/* No-op cache fallback: preserves correctness and keeps cache calls safe.
 * Performance may be lower than a full cache implementation.
 */
struct _NemoIconTextCache {
    guint max_entries;
    guint ttl_msec;
};

NemoIconTextCache *
nemo_icon_text_cache_new (guint max_entries,
                          guint ttl_msec)
{
    NemoIconTextCache *cache = g_new0 (NemoIconTextCache, 1);
    cache->max_entries = max_entries;
    cache->ttl_msec = ttl_msec;
    return cache;
}

void
nemo_icon_text_cache_free (NemoIconTextCache *cache)
{
    g_free (cache);
}

void
nemo_icon_text_cache_clear (NemoIconTextCache *cache)
{
    (void) cache;
}

gboolean
nemo_icon_text_cache_lookup (NemoIconTextCache       *cache,
                             const gchar             *filename,
                             gint                     max_text_width,
                             NemoIconTextMeasurement *measurement)
{
    (void) cache;
    (void) filename;
    (void) max_text_width;
    (void) measurement;
    return FALSE;
}

void
nemo_icon_text_cache_insert (NemoIconTextCache             *cache,
                             const gchar                   *filename,
                             gint                           max_text_width,
                             const NemoIconTextMeasurement *measurement)
{
    (void) cache;
    (void) filename;
    (void) max_text_width;
    (void) measurement;
}

void
nemo_icon_text_cache_invalidate_filename (NemoIconTextCache *cache,
                                          const gchar       *filename)
{
    (void) cache;
    (void) filename;
}
