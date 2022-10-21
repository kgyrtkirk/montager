#include "config.h"

#include <gtk/gtk.h>

#include <libgimp/gimp.h>

#include "main.h"
#include "render.h"


#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

class Point {
	int x;
	int y;
};

class ConvexHull {
	int last_x=0;

public:
	void add(int x,int y) {

	}

};

void read_image(gint32 drawable_id)
{
    using boost::geometry::model::d2::point_xy;
    using boost::geometry::append;
    using boost::geometry::make;
	
	boost::geometry::model::multi_point<point_xy<int> > points;
	
	GimpDrawable *drawable = gimp_drawable_get(drawable_id);
	gint bpp = drawable->bpp; //(drawable->drawable_id);
	gint w = drawable->width;
	gint h = drawable->height;

	GimpPixelRgn region;
	gimp_pixel_rgn_init(&region, drawable, 0, 0, w, h, FALSE, FALSE);
	size_t size = w * h * bpp;
	guchar *img = (guchar *)g_malloc(size);
	// printf("maskbpp :%d %d\n", bpp, size);

	gimp_pixel_rgn_get_rect(&region, img, 0, 0, w, h);

	// for (int i = 0; i < 10; i++)
	// {
	// 	for (int j = 0; j < 10; j++)
	// 	{
	// 		if (i * w + j < size)
	// 			img[i * w + j] = 255;
	// 	}
	// }

	// ConvexHull hull;

	for(int x=0;x<w;x++) {
		for(int y=0;y<h;y++) {
			if(img[y*w+x]>=255) {
				append(points, make<point_xy<int> >(x,y));
			}
		}
	}

    boost::geometry::model::polygon<point_xy<int> > hull;
    boost::geometry::convex_hull(points,hull);

	using boost::geometry::dsv;
	std::cout << dsv(hull) << std::endl;
	// memset(img, 255, size);
	gimp_pixel_rgn_init(&region, drawable, 0, 0, w, h, TRUE, TRUE);
	gimp_pixel_rgn_set_rect(&region, img, 0, 0, w, h);
	gimp_drawable_flush(drawable);
	gimp_drawable_merge_shadow(drawable->drawable_id, TRUE);
	gimp_drawable_update(drawable->drawable_id, 0, 0, w, h);
	gimp_drawable_detach(drawable);
	g_free(img);
}

/*  Public functions  */

void render(gint32 image_ID,
			GimpDrawable *drawable,
			PlugInVals *vals,
			PlugInImageVals *image_vals,
			PlugInDrawableVals *drawable_vals)
{

	int num_layers;
	gint *layers = gimp_image_get_layers(image_ID, &num_layers);
	// gimp_image_get_layer_position

	for (int i = 0; i < num_layers; i++)
	{
		gint32 layer = layers[i];
		gchar *name = gimp_drawable_get_name(layers[i]);
		gint32 mask = gimp_layer_get_mask(layers[i]);
		gint x, y;
		// gimp_pixel_rgn_get_rect
		// gimp_pixel
		gimp_drawable_offsets(layer, &x, &y);
		gimp_drawable_offsets(mask, &x, &y);
		printf("name: %s  ; mask :%d  @%d,%d\n", name, mask, x, y);

		read_image(mask);
		//		read_image(layer);
		// gimp_drawable_get_pixel
		// gimp_drawable_get
	}

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
