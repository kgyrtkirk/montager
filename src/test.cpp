
#include <glib.h>


#include <locale.h>

typedef struct {
//   OtherObject *helper;
} MyObjectFixture;

static void
my_object_fixture_set_up (MyObjectFixture *fixture,
                          gconstpointer user_data)
{
}

static void
my_object_fixture_tear_down (MyObjectFixture *fixture,
                             gconstpointer user_data)
{
}

static void
test_my_object_test1 (MyObjectFixture *fixture,
                      gconstpointer user_data)
{
//   g_assert_cmpstr (my_object_get_property (fixture->obj), ==, "initial-value");
}

static void
test_my_object_test2 (MyObjectFixture *fixture,
                      gconstpointer user_data)
{
//   my_object_do_some_work_using_helper (fixture->obj, fixture->helper);
//   g_assert_cmpstr (my_object_get_property (fixture->obj), ==, "updated-value");
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);

  // Define the tests.
  g_test_add ("/my-object/test1", MyObjectFixture, "some-user-data",
              my_object_fixture_set_up, test_my_object_test1,
              my_object_fixture_tear_down);
  g_test_add ("/my-object/test2", MyObjectFixture, "some-user-data",
              my_object_fixture_set_up, test_my_object_test2,
              my_object_fixture_tear_down);

  g_test_add ("/my-object/test2", MyObjectFixture, "some-user-data",
              my_object_fixture_set_up, test_my_object_test2,
              my_object_fixture_tear_down);

  return g_test_run ();
}