#include "nemo-lazy-thumbnail-loader.h"
#include "nemo-thumbnails.h"

/* Fallback implementation.
 * Returning NULL from _new() keeps nemo-icon-container on the existing
 * nemo_thumbnail_prioritize() path when this optional module is unavailable.
 */
struct _NemoLazyThumbnailLoader {
    guint unused;
};

NemoLazyThumbnailLoader *
nemo_lazy_thumbnail_loader_new (guint max_concurrent,
                                guint idle_timeout_msec)
{
    (void) max_concurrent;
    (void) idle_timeout_msec;
    return NULL;
}

void
nemo_lazy_thumbnail_loader_request (NemoLazyThumbnailLoader             *loader,
                                    const gchar                         *uri,
                                    gint                                 size,
                                    guint                                flags,
                                    NemoLazyThumbnailLoaderReadyCallback callback,
                                    gpointer                             user_data)
{
    (void) loader;
    (void) size;
    (void) flags;

    if (uri != NULL) {
        nemo_thumbnail_prioritize (uri);
    }

    if (callback != NULL) {
        callback (NULL, user_data);
    }
}

void
nemo_lazy_thumbnail_loader_cancel_invisible (NemoLazyThumbnailLoader *loader,
                                             GList                   *visible_uris)
{
    (void) loader;
    (void) visible_uris;
}
