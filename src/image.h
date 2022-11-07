#pragma once
#include "config.h"

#include <gtk/gtk.h>

#include <libgimp/gimp.h>
#include <iostream>
#include <memory>
#include <boost/geometry/geometries/point_xy.hpp>

// single channel image
class image
{
	// template<class T>
	// using point_xy<T> = boost::geometry::model::d2::point_xy<T>;
	// typename boost::geometry::model::d2::point_xy<T> point_xy<T>;
	typedef boost::geometry::model::d2::point_xy<int> point_xy;
	
	gint32 width;
	gint32 height;
	std::shared_ptr<guchar> img;

	void safe_paint(int x, int y, int v)
	{
		guchar *p = &img.get()[y * width + x];
		if (*p < 255)
		{
			*p = v;
		}
	}

public:
	void init(gint32 w, gint32 h)
	{
		width = w;
		height = h;
		size_t size = w * h;
		std::shared_ptr<guchar> p0((guchar *)g_malloc(size));
		img = p0;
		memset(img.get(), 0, size);
	}
	guchar *get()
	{
		img.get();
	}

	void paint(const point_xy &p, int value)
	{
		int x = p.x();
		int y = p.y();
		if (0 <= x && x < width &&
			0 <= y && y < height)
		{
			safe_paint(x, y, value);
		}
		else
		{
			// gimp_drawable_get_name(layers[i]);
			g_warning_once("Belonging point not available on canvas!");
		}
	}
	void fill_circle(const point_xy &p, double radius, int value)
	{
		int r = radius;
		if (r > 60000)
		{
			g_error("radius is pretty big - is everything alright?");
			r = 60000;
		}
		if (r > width)
		{
			r = width;
		}
		if (r < 1)
		{
			paint(p, value);
			return;
		}

		int c_x = p.x();
		int c_y = p.y();
		int r2 = r * r;

		for (int y0 = -r; y0 <= r; y0++)
		{
			for (int x0 = -r; x0 <= r; x0++)
			{
				if (x0 * x0 + y0 * y0 > r2)
				{
					continue;
				}
				int x = x0 + c_x;
				int y = y0 + c_y;
				if (0 <= x && x < width &&
					0 <= y && y < height)
				{
					safe_paint(x, y, value);
				}
			}
		}
	}
	guchar *row(int r)
	{
		return &img.get()[r * width];
	}
};
