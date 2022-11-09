
// #include <glib.h>
// #include </usr/include/glib-2.0/glib/gtestutils.h>
#include "config.h"
#include <glib.h>
#include "dist_queue.h"
#include "fd_layout.h"

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

using namespace std;
using namespace montager;

static void
test_dist_queue_min(void *, gconstpointer)
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
test_layout_entry(void *, gconstpointer)
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

  auto e1 = fd_layout::entry(t_point(0, 0), poly1, 1);
  auto e2 = fd_layout::entry(t_point(10, 0), poly1, 1);
  auto e3 = fd_layout::entry(t_point(10, 0), poly2, 1);
  auto g0 = fd_layout::entry(t_point(0, 0), guards[0], -1);

  auto w = e1.distance1(g0);
  auto d = e1.force_dir(g0);
  auto f = d * (100 / (w + 1));
  std::cout << ": " << dsv(f) << std::endl;

  // g_assert(d.x() == 0.0);
  // g_assert(d.y() == 0.0);
}

static void
test_layout(void *, gconstpointer)
{
  t_polygon poly1;
  t_polygon poly2;
  boost::geometry::read_wkt("POLYGON((0 0,1 1,1 0))", poly1);
  boost::geometry::read_wkt("POLYGON((10 0,11 1,11 0))", poly2);

  fd_layout layout(100, 100);
  auto e1 = fd_layout::entry(t_point(0, 0), poly1, 1);
  auto e2 = fd_layout::entry(t_point(0, 0), poly1, 1);
  layout.add(&e1);

  layout.run();
  cout << "pos:" << dsv(e1.position) << endl;

  g_assert(-1 < e1.position.x() -50.0 && e1.position.x() -50.0 < 1);
  g_assert(-1 < e1.position.y() -50.0 && e1.position.y() -50.0 < 1);
}

// same polygon at same location
static void
test_layout2(void *, gconstpointer)
{
  t_polygon poly1;
  t_polygon poly2;
  boost::geometry::read_wkt("POLYGON((0 0,1 1,1 0))", poly1);
  boost::geometry::read_wkt("POLYGON((10 0,11 1,11 0))", poly2);

  fd_layout layout(200, 100);
  auto e1 = fd_layout::entry(t_point(0, 0), poly1, 1);
  auto e2 = fd_layout::entry(t_point(0, 0), poly1, 1);
  layout.add(&e1);
  layout.add(&e2);

  layout.run();
  cout << "pos:" << dsv(e1.position) << endl;

  // g_assert(-1 < e1.position.x() -100.0 && e1.position.x() -50.0 < 1);
  // g_assert(-1 < e1.position.y() -50.0 && e1.position.y() -50.0 < 1);
}

int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");

  g_test_init(&argc, &argv, NULL);

#define TEST(METHOD) \
  g_test_add("/" #METHOD, void, 0, 0, METHOD, 0);

  TEST(test_dist_queue_min)
  TEST(test_layout_entry)
  TEST(test_layout)
  TEST(test_layout2)

  return g_test_run();
}