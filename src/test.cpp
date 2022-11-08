
// #include <glib.h>
// #include </usr/include/glib-2.0/glib/gtestutils.h>
#include "config.h"
#include <glib.h>
#include "dist_queue.h"
#include "fd_layout.h"

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

using namespace std;

typedef struct
{
  //   OtherObject *helper;
} MyObjectFixture;

static void
my_object_fixture_set_up(MyObjectFixture *fixture,
                         gconstpointer user_data)
{
}

static void
my_object_fixture_tear_down(MyObjectFixture *fixture,
                            gconstpointer user_data)
{
}

static void
test_dist_queue_min(MyObjectFixture *fixture,
                    gconstpointer user_data)
{
  t_polygon poly;
  t_polygon poly2;
  boost::geometry::read_wkt("POLYGON((0 0,1 1,1 0))", poly);
  boost::geometry::read_wkt("POLYGON((10 0,11 1,11 0))", poly2);

  dist_queue a;

  dist_queue::entry e1 = dist_queue::entry(poly, 1);
  dist_queue::entry e2 = dist_queue::entry(poly2, 2);
  a.add(&e1);
  a.add(&e2);

  for (int x = 0; x < 10; x++)
  {
    t_point p(x, 10);
    dist_queue::entry *ent = a.min(p);
    // std::cout << x << ": " << dsv(ent->g) << std::endl;
    double dist1 = boost::geometry::distance(ent->g, poly);
    double dist2 = boost::geometry::distance(ent->g, poly2);

    if (x < 7)
      g_assert(dist1 == 0.0);
    else
      g_assert(dist2 == 0.0);
  }
}

static void
test_layout(MyObjectFixture *fixture,
            gconstpointer user_data)
{
  t_polygon poly1;
  t_polygon poly2;
  boost::geometry::read_wkt("POLYGON((0 0,1 1,1 0))", poly1);
  boost::geometry::read_wkt("POLYGON((10 0,11 1,11 0))", poly2);

  fd_layout layout;
  auto e1 = fd_layout::entry(t_point(0, 0), poly1, 1);
  auto e2 = fd_layout::entry(t_point(10, 0), poly1, 1);

  auto d = e1.force_vector(e2);
  cout << dsv(d) << endl;

  layout.add(&e1);
  layout.add(&e2);

  // layout
}

int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");

  g_test_init(&argc, &argv, NULL);

  // Define the tests.
  g_test_add("/my-object/test1", MyObjectFixture, "some-user-data",
             my_object_fixture_set_up, test_dist_queue_min,
             my_object_fixture_tear_down);
  g_test_add("/my-object/test2", MyObjectFixture, "some-user-data",
             my_object_fixture_set_up, test_layout,
             my_object_fixture_tear_down);

  return g_test_run();
}