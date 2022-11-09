#include "fd_layout.h"
#include <boost/geometry.hpp>
#include <iostream>

using namespace std;
using namespace boost::geometry;
using boost::geometry::strategy::transform::translate_transformer;

fd_layout::entry::entry(t_point _pos, t_polygon _g, int _image_idx, bool _freeze) : position(_pos), g(_g), image_idx(_image_idx), freeze(_freeze)
{
    boost::geometry::centroid(g, center);
    cout << "centroid:" << dsv(center) << endl;
}

t_point fd_layout::entry::force_dir(const entry &o) const
{
    t_point v = (o.position + o.center) - (position + center);
    double l = distance(t_point(0, 0), v);
    if (l <= 0.0)
        return t_point(0, 0);
    return v * (1.0 / l);
}

t_point fd_layout::entry::force_vector(const entry &o) const
{
    double d = distance(absolute_poly(), o.absolute_poly());
    t_point v = (o.position + o.center) - (position + center);
    double l = distance(t_point(0, 0), v);
    cout << d << ";" << l << ".." << dsv(v) << endl;
    if (d <= 0.0 || l <= 0.0)
        return t_point(0, 0);
    return v * (d / l);
}
void fd_layout::entry::reset()
{
    force = t_point(0, 0);
}
void fd_layout::entry::commit()
{
    if (!freeze)
        position = position + force;
    reset();
}
void fd_layout::entry::add_force(const t_point &f)
{
    force = force + f;
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

fd_layout::fd_layout(int width, int height)
{
    max_step = (width + height) / 4;
    auto guards = guard_polys(width, height);
    for (auto it = guards.begin(); it != guards.end(); it++)
    {
        std::cout << ": " << dsv(*it) << std::endl;
        // FIXME: leak
        add(new entry(t_point(0, 0), *it, -1, true));
    }
}
#include <libgimp/gimp.h>

void fd_layout::run()
{
    gimp_progress_set_text("layout");

    int n = 30;
    for (int i = 0; i < n; i++)
    {
		gimp_progress_update(i * 1.0 / n);
        step(max_step * (n - i) / n);
    }
}

void fd_layout::step(double step_size)
{
    for (auto it = elements.begin(); it != elements.end(); it++)
    {
        (*it)->reset();
    }

    for (int i = 0; i < elements.size(); i++)
    {
        for (int j = i + 1; j < elements.size(); j++)
        {
            t_point f = compute_force(elements[i], elements[j]);
            f = f * step_size;
            elements[i]->add_force(f);
            elements[j]->add_force(f * -1.0);
        }
    }
    for (auto it = elements.begin(); it != elements.end(); it++)
    {
        (*it)->commit();
    }
}

t_point fd_layout::compute_force(entry *l, entry *r)
{
    double d = l->distance1(*r);
    auto dir = l->force_dir(*r);
    if (d <= 0.0)
    {
        d = .00000000001 * (random() % 1000);
    }
    return dir * (-10000/(10+d) / max_step);
}

t_polygon guard(int idx, double w, double h)
{
    using namespace boost::geometry::strategy::transform;
    using boost::geometry::model::multi_point;

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
