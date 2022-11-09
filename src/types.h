#pragma once

#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

typedef boost::geometry::model::d2::point_xy<double> t_point;
typedef boost::geometry::model::d2::point_xy<int> t_point2i;
typedef boost::geometry::model::polygon<t_point> t_polygon;

t_point operator+(const t_point &lh, const t_point &rh);
t_point operator-(const t_point &lh, const t_point &rh);
t_point operator*(const t_point &lh, const double &rh);

