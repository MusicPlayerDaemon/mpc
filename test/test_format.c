#include "song_format.h"

#include <mpd/client.h>

#include <check.h>

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

static void
feed_song(struct mpd_song *song, const char *name, const char *value)
{
	const struct mpd_pair pair = { name, value };
	mpd_song_feed(song, &pair);
}

static struct mpd_song *
construct_song(const char *file, ...)
{
	const struct mpd_pair pair = { "file", file };
	struct mpd_song *song = mpd_song_begin(&pair);
	assert(song != NULL);

	va_list ap;
	va_start(ap, file);
	const char *name;
	while ((name = va_arg(ap, const char *)) != NULL) {
		const char *value = va_arg(ap, const char *);
		assert(value != NULL);
		feed_song(song, name, value);
	}

	return song;
}

static const char *const default_file = "foo.ogg";
static const char *const default_artist = "Foo";
static const char *const default_title = "Bar";
static const char *const default_format =
	"[%name%: &[%artist% - ]%title%]|%name%|[%artist% - ]%title%|%file%";

static struct mpd_song *
construct_default_song(void)
{
	return construct_song(default_file,
			      "Artist", default_artist,
			      "Title", default_title,
			      NULL);
}

static void
assert_format(const struct mpd_song *song, const char *format,
	      const char *expected)
{
	char *p = format_song(song, format);
	if (expected == NULL)
		ck_assert_ptr_eq(p, NULL);
	else
		ck_assert_str_eq(p, expected);
	free(p);
}

START_TEST(test_empty)
{
	struct mpd_song *song = construct_song("foo.ogg", NULL);
	assert_format(song, "", NULL);
	mpd_song_free(song);
}
END_TEST

START_TEST(test_basic)
{
	struct mpd_song *song = construct_default_song();
	assert_format(song, "%file%", default_file);
	assert_format(song, "%artist%", default_artist);
	assert_format(song, "%title%", default_title);
	assert_format(song, "%albumartist%", "");
	mpd_song_free(song);
}
END_TEST

START_TEST(test_group)
{
	struct mpd_song *song = construct_default_song();
	assert_format(song, "artist=['%artist%']", "artist='Foo'");
	assert_format(song, "albumartist=['%albumartist%']", "albumartist=");
	mpd_song_free(song);
}
END_TEST

START_TEST(test_fallback)
{
	struct mpd_song *song = construct_default_song();
	assert_format(song, "%artist%|%albumartist%", default_artist);
	assert_format(song, "%albumartist%|%artist%", default_artist);
	mpd_song_free(song);
}
END_TEST

START_TEST(test_default)
{
	struct mpd_song *song = construct_song(default_file, NULL);
	assert_format(song, default_format, default_file);

	feed_song(song, "Name", "abc");
	assert_format(song, default_format, "abc");

	feed_song(song, "Title", default_title);
	assert_format(song, default_format, "abc: Bar");

	feed_song(song, "Artist", default_artist);
	assert_format(song, default_format, "abc: Foo - Bar");

	mpd_song_free(song);
}
END_TEST

START_TEST(test_escape)
{
	struct mpd_song *song = construct_default_song();
	assert_format(song, "\\a\\b\\t\\n\\v\\f\\r\\[\\]", "\a\b\t\n\v\f\r[]");
	assert_format(song, "###%#[#]#|#&", "#%[]|&");
	mpd_song_free(song);
}
END_TEST

static Suite *
create_suite(void)
{
	Suite *s = suite_create("format");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_empty);
	tcase_add_test(tc_core, test_basic);
	tcase_add_test(tc_core, test_group);
	tcase_add_test(tc_core, test_fallback);
	tcase_add_test(tc_core, test_default);
	tcase_add_test(tc_core, test_escape);
	suite_add_tcase(s, tc_core);
	return s;
}

int
main(void)
{
	Suite *s = create_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	int number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
