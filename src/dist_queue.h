#ifndef __DIST_QUEUE_H__
#define __DIST_QUEUE_H__

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
public:

    class entry
    {
        shared_ptr<t_polygon> g;
        t_point last_point;
        double last_dist;

    public:
        entry(shared_ptr<t_polygon> _g) : g(_g),last_dist(-1),last_point(-1,-1)
        {
        }
        entry(const entry &e) : g(e.g)
        {
            printf(" entry(const entry&e)\n");
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

    std::priority_queue<entry, std::vector<entry>, entry_cmp> queue;
    dist_queue(){};

    void add(const entry &e)
    {
        queue.push(e);
    }
    void min(const t_point&p){
        
        queue.top();
        queue.delete
    }
};

void a()
{

    t_polygon poly;
    t_polygon poly2;
    boost::geometry::read_wkt("POLYGON((0 0,1 1,1 0))", poly);
    boost::geometry::read_wkt("POLYGON((10 0,11 1,11 0))", poly2);

    dist_queue a;

    a.add(dist_queue::entry(shared_ptr<t_polygon>(&poly)));
    a.add(dist_queue::entry(shared_ptr<t_polygon>(&poly2)));


    for(int x=0;x<11;x++) {
        t_point p(x,0);
        a.min(p);
    }
    // a.add(2);
}

#endif