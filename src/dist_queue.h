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
        t_polygon g;
        t_point last_point;
        double last_dist;
        double heur_dist;
        double index = 0.0;
        int image_idx;

        entry(t_polygon _g, int _image_idx) : g(_g),image_idx(_image_idx)
        {
            update(t_point(0,0),0);
        }
        // entry(const entry &e) : g(e.g)
        // {
        //     printf(" entry(const entry&e)\n");
        // }
        // entry& operator=(const entry&o) {
        //     g=o.g;
        //     return *this;
        // }
        void update(const t_point&p,double _index) {
            last_point=p;
            last_dist=boost::geometry::distance(last_point, g);
            // printf("up: %d %d => %f\n",p.x(),p.y(),last_dist);
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
                return l->heur_dist > r->heur_dist;
            }
            return l->index > r->index;
        }
    };

    std::priority_queue<entry *, std::vector<entry *>, entry_cmp> queue;
    dist_queue(){};

    void add(entry *e)
    {
        queue.push(e);
    }

    double idx=0;
    t_point last_point;
    entry* min(t_point p)
    {
        idx+=distance(p,last_point);//dist(p,last_point);
        last_point=p;

        entry*curr=queue.top();
        while( (p.x()!=curr->last_point.x()) || (p.y()!=curr->last_point.y())) {
            queue.pop(); // !#@$ why can't this return the top element?

            curr->update( p,idx);
            queue.push(curr);

            curr=queue.top();
        }
        return curr;
    }
    // returns the min radius in which the minima will not change 
    double min_radius(t_point p)
    {
        min(p); // unneccessary; but correctness
        entry*top=queue.top();
        queue.pop();
        entry*sec=min(p);
        queue.push(top);
        return (sec->last_dist - top->last_dist)/2;
    }
};

#endif