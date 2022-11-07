#pragma once
#include "config.h"

#include <string.h>

#include <libgimp/gimp.h>

#include "main.h"
#include "image.h"

#include <iostream>

#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

class PImage
{
	using point_xy = boost::geometry::model::d2::point_xy<int>;
	using polygon = boost::geometry::model::polygon<point_xy>;
	GimpDrawable *drawable;
	polygon local_hull;
	point_xy pos;
	polygon hull;
	image img;

private:
	void read_image();
	void compute_hull();

	void extract_drawable_infos(gint32 drawable_id)
	{
		// gimp_drawable_get_buffer
		
		drawable = gimp_drawable_get(drawable_id);
		gint x, y;
		gimp_drawable_offsets(drawable->drawable_id, &x, &y);
		pos = point_xy(x, y);
	}

public:
	PImage(gint32 drawable_id)
	{
		extract_drawable_infos(drawable_id);
		read_image();
		compute_hull();
	};
	PImage(const PImage &other)
	{
		extract_drawable_infos(other.drawable->drawable_id);
		img = other.img;
		local_hull = other.local_hull;
		hull = other.hull;
	};

	~PImage()
	{
		gimp_drawable_detach(drawable);
	}
	const polygon &getHull() const
	{
		return hull;
	}

	void paint(const point_xy &p)
	{
		int x = p.x() - pos.x();
		int y = p.y() - pos.y();
		img.paint(t_point(x, y), 254);
	}

	double distance(const point_xy &p) const;
	void show_distance();
	void debug();
	void cleanup();
	void show_hull();
	void flush();
	GimpDrawable *getDrawable()
	{
		return drawable;
	}
};