/* Minimal lazy-thumbnail-loader API used by nemo-icon-container.
 * This fallback keeps compile/runtime safe when the optional module
 * is absent from the source tree.
 */

#ifndef NEMO_LAZY_THUMBNAIL_LOADER_H
#define NEMO_LAZY_THUMBNAIL_LOADER_H

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct _NemoLazyThumbnailLoader NemoLazyThumbnailLoader;

typedef void (*NemoLazyThumbnailLoaderReadyCallback) (GdkPixbuf *pixbuf,
                                                      gpointer   user_data);

NemoLazyThumbnailLoader *nemo_lazy_thumbnail_loader_new (guint max_concurrent,
                                                         guint idle_timeout_msec);
void nemo_lazy_thumbnail_loader_request (NemoLazyThumbnailLoader              *loader,
                                         const gchar                          *uri,
                                         gint                                  size,
                                         guint                                 flags,
                                         NemoLazyThumbnailLoaderReadyCallback  callback,
                                         gpointer                              user_data);
void nemo_lazy_thumbnail_loader_cancel_invisible (NemoLazyThumbnailLoader *loader,
                                                  GList                   *visible_uris);

#endif /* NEMO_LAZY_THUMBNAIL_LOADER_H */
