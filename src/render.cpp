#include "config.h"

#include <gtk/gtk.h>

#include <libgimp/gimp.h>

#include "main.h"
#include "render.h"

#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

using namespace boost::geometry;
using boost::geometry::model::multi_point;
using boost::geometry::model::polygon;
using boost::geometry::model::d2::point_xy;
using namespace boost::geometry::strategy::transform;
using std::shared_ptr;

class PImage
{
	GimpDrawable *drawable;
	shared_ptr<guchar> img;
	polygon<point_xy<int>> local_hull;
	polygon<point_xy<int>> hull;

private:
	void read_image()
	{
		gint bpp = drawable->bpp;
		gint w = drawable->width;
		gint h = drawable->height;
		size_t size = w * h * bpp;
		if (bpp != 1)
		{
			g_error("bpp is expected to be 1 at this point; however its: %d", bpp);
		}
		shared_ptr<guchar> p0((guchar *)g_malloc(size));
		img = p0;

		GimpPixelRgn region;
		gimp_pixel_rgn_init(&region, drawable, 0, 0, w, h, FALSE, FALSE);
		gimp_pixel_rgn_get_rect(&region, img.get(), 0, 0, w, h);
	}
	void compute_hull()
	{
		multi_point<point_xy<int>> points;
		gint w = drawable->width;
		gint h = drawable->height;

		guchar *img = this->img.get();
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				if (img[y * w + x] >= 255)
				{
					append(points, make<point_xy<int>>(x, y));
				}
			}
		}

		convex_hull(points, local_hull);

		// make global hull
		gint x, y;
		gimp_drawable_offsets(drawable->drawable_id, &x, &y);

		translate_transformer<int, 2, 2> translate(x, y);
		transform(local_hull, hull, translate);
	}

public:
	PImage(gint32 drawable_id)
	{
		drawable = gimp_drawable_get(drawable_id);
		read_image();
		compute_hull();
	};
	PImage(const PImage &other)
	{
		drawable = gimp_drawable_get(other.drawable->drawable_id);
		img = other.img;
		local_hull = other.local_hull;
		hull = other.hull;
	};

	~PImage()
	{
		gimp_drawable_detach(drawable);
		// g_free(img);
	}

	void show_distance()
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
				point_xy<int> p(x, y);
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
	void debug()
	{
		using boost::geometry::dsv;
		std::cout << dsv(hull) << std::endl;
	}
	void flush()
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
};

/*  Public functions  */

void render(gint32 image_ID,
			GimpDrawable *drawable,
			PlugInVals *vals,
			PlugInImageVals *image_vals,
			PlugInDrawableVals *drawable_vals)
{

	gimp_progress_init(PLUGIN_NAME);

	int num_layers;
	gint *layers = gimp_image_get_layers(image_ID, &num_layers);

	std::vector<PImage> images;
	for (int i = 0; i < num_layers; i++)
	{

		gint32 layer = layers[i];
		gchar *name = gimp_drawable_get_name(layers[i]);
		gint32 mask = gimp_layer_get_mask(layers[i]);

		gimp_progress_set_text(name);
		gimp_progress_update(i * 1.0 / num_layers);

		PImage mi(mask);
		mi.debug();
		// mi.show_distance();
		// mi.flush();
		images.push_back(mi);
	}

	// for(auto it=images.begin(); it)

	// get shape:
	// * get_mask
	// * gimp_image_select_color
	//

	// per-pixel distance ?

	// gimp_layer_add_alpha

	// gimp_drawable_fill(drawable->drawable_id,GIMP_WHITE_FILL);

	free(layers);

	g_message("White-white?@!123");
}
