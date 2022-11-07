#include "pimage.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>

using namespace boost::geometry;
using boost::geometry::model::multi_point;
using namespace boost::geometry::strategy::transform;

void PImage::compute_hull()
{
    multi_point<point_xy> points;
    gint w = drawable->width;
    gint h = drawable->height;

    guchar *img = this->img.get();
    for (int y = 0; y < h; y++)
    {
        int g = -1;
        for (int x = 0; x < w; x++)
        {
            if (img[y * w + x] >= 255)
            {
                if (g == -1)
                    append(points, make<point_xy>(x, y));
                g = x;
            }
        }
        if (g >= 0)
            append(points, make<point_xy>(g, y));
    }

    convex_hull(points, local_hull);

    // make global hull
    translate_transformer<int, 2, 2> translate(pos.x(), pos.y());
    transform(local_hull, hull, translate);
}


	double PImage::distance(const point_xy &p) const
	{
		return boost::geometry::distance(p, hull);
	}
	void PImage::show_distance()
	{
		gint bpp = drawable->bpp;
		gint w = drawable->width;
		gint h = drawable->height;
		guchar *img = this->img.get();

		size_t size = w * h * bpp;
		memset(img, 0, size);
		for (auto y = 0; y < h; y++)
		{
			for (auto x = 0; x < w; x++)
			{
				point_xy p(x, y);
				double d = boost::geometry::distance(p, local_hull);
				guchar v;
				if (d <= 0.0)
					v = 255;
				else
					v = std::max(0.0, 254.0 - d);
				img[p.y() * w + p.x()] = v;
			}
		}
	}
	void PImage::debug()
	{
		using boost::geometry::dsv;
		std::cout << dsv(hull) << std::endl;
	}
	void PImage::cleanup()
	{
		gint w = drawable->width;
		gint h = drawable->height;
		guchar *img = this->img.get();

		for (auto y = 0; y < h; y++)
		{
			for (auto x = 0; x < w; x++)
			{
				int v = img[y * w + x];
				if (v < 255)
					v = 0;
				img[y * w + x] = v;
			}
		}
	}
	void PImage::show_hull()
	{
		gint w = drawable->width;
		gint h = drawable->height;
		guchar *img = this->img.get();

		for (auto y = 0; y < h; y++)
		{
			for (auto x = 0; x < w; x++)
			{
				int v = img[y * w + x];
				if (v >= 255)
					continue;

				point_xy p(x, y);
				if (within(p, local_hull))
				{
					v = 254;
				}
				else
				{
					v = 0;
				}
				img[y * w + x] = v;
			}
		}
	}
	void PImage::flush()
	{
		gint w = drawable->width;
		gint h = drawable->height;

		GimpPixelRgn region;
		gimp_pixel_rgn_init(&region, drawable, 0, 0, w, h, TRUE, TRUE);
		gimp_pixel_rgn_set_rect(&region, img.get(), 0, 0, w, h);
		gimp_drawable_flush(drawable);
		gimp_drawable_merge_shadow(drawable->drawable_id, TRUE);
		gimp_drawable_update(drawable->drawable_id, 0, 0, w, h);
	}
