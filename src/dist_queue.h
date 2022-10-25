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

    public:
        shared_ptr<t_polygon> g;
        t_point last_point;
        double last_dist;
        double heur_dist;
        int index = 0;

        entry(shared_ptr<t_polygon> _g) : g(_g)
        {
            update(t_point(0,0),0);
        }
        entry(const entry &e) : g(e.g)
        {
            printf(" entry(const entry&e)\n");
        }
        // entry& operator=(const entry&o) {
        //     g=o.g;
        //     return *this;
        // }
        void update(const t_point&p,int _index) {
            last_point=p;
            t_point pp=p;
            t_polygon poly;
            last_dist=boost::geometry::distance(pp, *g.get());
            index=_index;
            heur_dist=last_dist+index;
        }
    };

    class entry_cmp
    {
    public:
        bool
        operator()(const entry *l, const entry *r) const
        {
            if (l->heur_dist != r->heur_dist)
            {
                return l->heur_dist < r->heur_dist;
            }
            return l->index < r->index;
        }
    };

    std::priority_queue<entry *, std::vector<entry *>, entry_cmp> queue;
    dist_queue(){};

    void add(entry *e)
    {
        queue.push(e);
    }

    int idx=0;
    void min(const t_point &p)
    {
        entry*curr=queue.top();
        while(p.x()!=curr->last_point.x() || p.y()!=curr->last_point.y()) {
            entry*curr=queue.top();
            queue.pop(); // !#@$ why can't this return the top element?

            curr->update( p,idx);
            queue.push(curr);
        }
        idx++;
    }
};

void a()
{

    t_polygon poly;
    t_polygon poly2;
    boost::geometry::read_wkt("POLYGON((0 0,1 1,1 0))", poly);
    boost::geometry::read_wkt("POLYGON((10 0,11 1,11 0))", poly2);

    dist_queue a;

    dist_queue::entry e1 = dist_queue::entry(shared_ptr<t_polygon>(&poly));
    dist_queue::entry e2 = dist_queue::entry(shared_ptr<t_polygon>(&poly2));
    a.add(&e1);
    a.add(&e2);

    for (int x = 0; x < 11; x++)
    {
        printf("%d >\n",x);
        t_point p(x, 0);
        a.min(p);
    }
    // a.add(2);
}

#endif