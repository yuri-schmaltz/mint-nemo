#include <glib.h>
#include "libnemo-private/nemo-icon-text-cache.h"

static void test_cache_basic(void) {
    NemoIconTextCache *cache = nemo_icon_text_cache_new(16, 0);
    NemoIconTextMeasurement m;

    gboolean hit = nemo_icon_text_cache_lookup(cache, "file1.txt", 110, &m);
    g_assert_false(hit);

    NemoIconTextMeasurement ins = { .text_width = 42, .text_height = 11, .text_dx = 0, .text_height_for_layout = 11, .text_height_for_entire_text = 11 };
    nemo_icon_text_cache_insert(cache, "file1.txt", 110, &ins);

    hit = nemo_icon_text_cache_lookup(cache, "file1.txt", 110, &m);
    g_assert_true(hit);
    g_assert_cmpint(m.text_width, ==, 42);
    g_assert_cmpint(m.text_height, ==, 11);

    nemo_icon_text_cache_free(cache);
}

static void test_cache_invalidate(void) {
    NemoIconTextCache *cache = nemo_icon_text_cache_new(16, 0);
    NemoIconTextMeasurement ins = { .text_width = 10, .text_height = 5, .text_dx = 0, .text_height_for_layout = 5, .text_height_for_entire_text = 5 };
    nemo_icon_text_cache_insert(cache, "file2.txt", 96, &ins);

    NemoIconTextMeasurement m;
    gboolean hit = nemo_icon_text_cache_lookup(cache, "file2.txt", 96, &m);
    g_assert_true(hit);

    nemo_icon_text_cache_invalidate_filename(cache, "file2.txt");
    hit = nemo_icon_text_cache_lookup(cache, "file2.txt", 96, &m);
    g_assert_false(hit);

    nemo_icon_text_cache_free(cache);
}

int main(int argc, char **argv) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/nemo/cache/basic", test_cache_basic);
    g_test_add_func("/nemo/cache/invalidate", test_cache_invalidate);
    return g_test_run();
}
