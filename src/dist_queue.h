#ifndef DIST_QUEUE_H
#define DIST_QUEUE_H

#include <functional>
#include <queue>
#include <vector>
#include <iostream>
#include <string_view>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
// #include <boost/geometry/multi/geometries/multi_point.hpp>

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

using namespace boost::geometry;
using boost::geometry::model::multi_point;
using boost::geometry::model::polygon;
using boost::geometry::model::d2::point_xy;
using namespace boost::geometry::strategy::transform;
using std::shared_ptr;

typedef boost::geometry::model::d2::point_xy<int> t_point;
typedef boost::geometry::model::polygon<t_point> t_polygon;

class dist_queue
{
    class entry
    {
        shared_ptr<t_polygon> g;

    public:
        entry(shared_ptr<t_polygon> _g) : g(_g)
        {
        }
        entry(const entry &e) : g(e.g)
        {
        }
        // entry& operator=(const entry&o) {
        //     g=o.g;
        //     return *this;
        // }
    };

    class entry_cmp
    {
    public:
        bool
        operator()(const entry &l, const entry &r) const
        {
            return true;
        }
    };

public:
    std::priority_queue<entry, std::vector<entry>, entry_cmp> queue;
    dist_queue(){};

    void add(shared_ptr<t_polygon> g)
    {
        queue.push(entry(g));
    }
};

void a()
{

    t_polygon poly;
    boost::geometry::read_wkt(
        "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3)"
        "(4.0 2.0, 4.2 1.4, 4.8 1.9, 4.4 2.2, 4.0 2.0))",
        poly);

    dist_queue a;
    a.add(shared_ptr<t_polygon>(&poly));
    // a.add(2);
}

#endif