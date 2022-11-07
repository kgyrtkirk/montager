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
	void fill_circle(const point_xy &p, double radius, int value);
	guchar *row(int r)
	{
		return &img.get()[r * width];
	}
};
