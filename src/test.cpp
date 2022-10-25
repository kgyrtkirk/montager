
// #include <glib.h>
// #include </usr/include/glib-2.0/glib/gtestutils.h>
#include "config.h"
#include <glib.h>
#include "dist_queue.h"

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
    t_polygon poly;
    t_polygon poly2;
    boost::geometry::read_wkt("POLYGON((0 0,1 1,1 0))", poly);
    boost::geometry::read_wkt("POLYGON((10 0,11 1,11 0))", poly2);

    dist_queue a;

    dist_queue::entry e1 = dist_queue::entry(poly);
    dist_queue::entry e2 = dist_queue::entry(poly2);
    a.add(&e1);
    a.add(&e2);

    for (int x = 0; x < 11; x++)
    {
        printf("%d >\n",x);
        t_point p(x, 10);
        a.min(p);
    }
    // a.add(2);
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

  // g_test_add ("/my-object/test2", MyObjectFixture, "some-user-data",
  //             my_object_fixture_set_up, test_my_object_test2,
  //             my_object_fixture_tear_down);

  return g_test_run ();
}