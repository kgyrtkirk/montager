#include "fd_layout.h"
#include <boost/geometry.hpp>
#include <iostream>

using namespace std;
using namespace boost::geometry;
using boost::geometry::strategy::transform::translate_transformer;

fd_layout::entry::entry(t_point _pos, t_polygon _g, int _image_idx) : position(_pos), g(_g), image_idx(_image_idx)
{
    boost::geometry::centroid(g, center);
    cout << "centroid:" << dsv(center) << endl;
}

t_point fd_layout::entry::force_dir(const entry &o) const
{
    t_point v =  (o.position + o.center) - (position + center);
    double l = distance(t_point(0, 0), v);
    if (l <= 0.0)
        return t_point(0, 0);
    return v * (1.0 / l);
}

t_point fd_layout::entry::force_vector(const entry &o) const
{
    double d = distance(absolute_poly(), o.absolute_poly());
    t_point v =  (o.position + o.center) - (position + center);
    double l = distance(t_point(0, 0), v);
    cout << d << ";" << l << ".." << dsv(v) << endl;
    if (d <= 0.0 || l <= 0.0)
        return t_point(0, 0);
    return v * (d / l);
}

double fd_layout::entry::distance1(const entry &o) const
{
    return distance(absolute_poly(), o.absolute_poly());
}

t_polygon fd_layout::entry::absolute_poly() const
{
    t_polygon abs_poly;

    translate_transformer<double, 2, 2> translate(position.x(), position.y());
    transform(g, abs_poly, translate);
    return abs_poly;
}
t_point fd_layout::entry::absolute_center() const
{
    t_point abs_pos;

    translate_transformer<double, 2, 2> translate(position.x(), position.y());
    transform(position, abs_pos, translate);
    return abs_pos;
}

