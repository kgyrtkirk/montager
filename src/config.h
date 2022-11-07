#define PACKAGE_NAME "gimp-montager"
#define PACKAGE_VERSION "0.1.0"
#define PLUGIN_NAME PACKAGE_NAME

#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

typedef boost::geometry::model::d2::point_xy<int> t_point;
typedef boost::geometry::model::polygon<t_point> t_polygon;
