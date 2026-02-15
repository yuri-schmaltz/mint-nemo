/* Minimal text-cache API used by nemo-icon-canvas-item.
 * This fallback implementation keeps behavior correct when the optional
 * cache module is not present in the source tree.
 */

#ifndef NEMO_ICON_TEXT_CACHE_H
#define NEMO_ICON_TEXT_CACHE_H

#include <glib.h>

typedef struct _NemoIconTextCache NemoIconTextCache;

typedef struct {
    gint text_width;
    gint text_height;
    gint text_dx;
    gint text_height_for_layout;
    gint text_height_for_entire_text;
} NemoIconTextMeasurement;

NemoIconTextCache *nemo_icon_text_cache_new (guint max_entries,
                                             guint ttl_msec);
void               nemo_icon_text_cache_free (NemoIconTextCache *cache);
void               nemo_icon_text_cache_clear (NemoIconTextCache *cache);
gboolean           nemo_icon_text_cache_lookup (NemoIconTextCache            *cache,
                                                const gchar                  *filename,
                                                gint                          max_text_width,
                                                NemoIconTextMeasurement      *measurement);
void               nemo_icon_text_cache_insert (NemoIconTextCache                  *cache,
                                                const gchar                        *filename,
                                                gint                                max_text_width,
                                                const NemoIconTextMeasurement      *measurement);
void               nemo_icon_text_cache_invalidate_filename (NemoIconTextCache *cache,
                                                             const gchar       *filename);

#endif /* NEMO_ICON_TEXT_CACHE_H */
