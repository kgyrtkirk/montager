#include "fd_layout.h"
#include <boost/geometry.hpp>
#include <iostream>

using namespace std;
using namespace boost::geometry;
fd_layout::entry::entry(t_point _pos, t_polygon _g, int _image_idx) : position(_pos), g(_g), image_idx(_image_idx)
{
    boost::geometry::centroid(g, center);
    cout << "a" << endl;
    cout << dsv(center) << endl;
}
