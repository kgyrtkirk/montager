
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

t_polygon guard(int idx, double w, double h)
{
  const int L = 50;
  t_polygon p, p_temp;
  multi_point<t_point> points;
  points.push_back(t_point(-1, 1));
  points.push_back(t_point(1, 1));
  points.push_back(t_point(L, L));
  points.push_back(t_point(-L, L));
  convex_hull(points, p);
  p_temp = p;
  boost::geometry::transform(p_temp, p, rotate_transformer<boost::geometry::degree, double, 2, 2>(90.0 * idx));
  p_temp = p;
  transform(p_temp, p, translate_transformer<double, 2, 2>(1, 1));
  p_temp = p;
  transform(p_temp, p, scale_transformer<double, 2, 2>(w / 2, h / 2));
  return p;
}

vector<t_polygon> guard_polys(int w, int h)
{
  vector<t_polygon> ret;
  ret.push_back(guard(2, w, h));
  ret.push_back(guard(3, w, h));
  ret.push_back(guard(0, w, h));
  ret.push_back(guard(1, w, h));
  return ret;
}

static void
test_layout_entry(MyObjectFixture *fixture,
                  gconstpointer user_data)
{
  t_polygon poly1;
  t_polygon poly2;
  boost::geometry::read_wkt("POLYGON((0 0,1 1,1 0))", poly1);
  boost::geometry::read_wkt("POLYGON((10 0,11 0,11 1))", poly2);

  vector<t_polygon> guards = guard_polys(1000, 500);
  for (auto it = guards.begin(); it != guards.end(); it++)
  {
    std::cout << ": " << dsv(*it) << std::endl;
  }

  // boost::geometry::read_wkt("POLYGON((-10 -10,10 -10,10 10,-10 10),(-100 -100,100 -100,100 100,-100 100))", poly2);
  // boost::geometry::read_wkt("POLYGON((-10 -10,10 -10,10 10,-10 10))", poly2);

  fd_layout layout;
  auto e1 = fd_layout::entry(t_point(0, 0), poly1, 1);
  auto e2 = fd_layout::entry(t_point(10, 0), poly1, 1);
  auto e3 = fd_layout::entry(t_point(10, 0), poly2, 1);

  auto d = e1.force_vector(e2);

  g_assert(d.x() == 9.0);
  g_assert(d.y() == 0.0);
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

  layout.add(&e1);
  layout.add(&e2);
}

int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");

  g_test_init(&argc, &argv, NULL);

  // Define the tests.
  g_test_add("/dist_queue/min", MyObjectFixture, "some-user-data",
             my_object_fixture_set_up, test_dist_queue_min,
             my_object_fixture_tear_down);
  g_test_add("/layout/entry", MyObjectFixture, "some-user-data",
             my_object_fixture_set_up, test_layout_entry,
             my_object_fixture_tear_down);
  g_test_add("/layout/main", MyObjectFixture, "some-user-data",
             my_object_fixture_set_up, test_layout,
             my_object_fixture_tear_down);

  return g_test_run();
}