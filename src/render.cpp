#include "config.h"

#include <gtk/gtk.h>

#include <libgimp/gimp.h>

#include "main.h"
#include "render.h"
#include "dist_queue.h"

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

// single channel image
class image {
	gint32 width;
	gint32 height;
	shared_ptr<guchar> img;

	void safe_paint(int x, int y, int v)
	{
		guchar *p = &img.get()[y * width + x];
		if (*p < 255)
		{
			*p = v;
		}
	}
public:
	void init(gint32 w,gint32 h){
width=w;
height=h;
		size_t size = w * h ;
		shared_ptr<guchar> p0((guchar *)g_malloc(size));
		img = p0;
		memset(img.get(), 0, size);
	}
	guchar* get() {
		img.get();
	}

	void paint(const point_xy<int> &p,int value)
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
	void fill_circle(const point_xy<int> &p, double radius, int value)
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
			paint(p,value);
			return;
		}

		int c_x = p.x();
		int c_y = p.y();
		int r2=r*r;

		for (int y0 = -r; y0 <= r; y0++)
		{
			for (int x0 = -r; x0 <= r; x0++)
			{
				if(x0*x0+y0*y0 > r2) {
					continue;
				}
				int x=x0+c_x;
				int y=y0+c_y;
				if (0 <= x && x < width &&
					0 <= y && y < height)
				{
					safe_paint(x, y, value);
				}
			}
		}
	}
	guchar* row(int r) {
		return &img.get()[r*width];
	}


};

class PImage
{
	GimpDrawable *drawable;
	polygon<point_xy<int>> local_hull;
	point_xy<int> pos;
	polygon<point_xy<int>> hull;
	image	img;

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
		img.init(w,h);

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
			for (int y = 0; y < h; y++)
		{
			int g = -1;
			for (int x = 0; x < w; x++)
			{
				if (img[y * w + x] >= 255)
				{
					if (g == -1)
						append(points, make<point_xy<int>>(x, y));
					g = x;
				}
			}
			append(points, make<point_xy<int>>(g, y));
		}

		convex_hull(points, local_hull);

		// make global hull
		translate_transformer<int, 2, 2> translate(pos.x(), pos.y());
		transform(local_hull, hull, translate);
	}
	void extract_drawable_infos(gint32 drawable_id)
	{
		drawable = gimp_drawable_get(drawable_id);
		gint x, y;
		gimp_drawable_offsets(drawable->drawable_id, &x, &y);
		pos = point_xy<int>(x, y);
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
	const polygon<point_xy<int>>&getHull() const {
		return hull;
	}

	void paint(const point_xy<int> &p)
	{
		int x = p.x() - pos.x();
		int y = p.y() - pos.y();
		img.paint(t_point(x,y),254);
	}

	double distance(const point_xy<int> &p) const
	{
		return boost::geometry::distance(p, hull);
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
	void cleanup()
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
	void show_hull()
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

				point_xy<int> p(x, y);
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

class PMontage
{
	std::vector<PImage> images;
	gint32 image_id;
	gint32 width;
	gint32 height;

public:
	PMontage(gint32 _image_id) : image_id(_image_id)
	{

		width = gimp_image_width(image_id);
		height = gimp_image_height(image_id);
	}
	void add(const PImage &image)
	{
		images.push_back(image);
	}
	int num_inputs()
	{
		return images.size();
	}
	// this is a bit different than Voronoi diagram; but the concept is close-enough
	// in this case the generators are not just points - but polygons
	void assign_voronoi()
	{
		if(num_inputs() < 2) {
			g_warning("Not enough input layers for this operation; skipping assign_voronoi!");
			return;
		}

		gimp_progress_set_text("assign_voronoi");
		if (images.size() >= 253)
		{
			g_error("more images than supported");
		}
		image aimg;
		aimg.init(width,height);

		dist_queue	dq;

		for(int i=0;i<images.size();i++) {
			dq.add(new dist_queue::entry(images[i].getHull(),i));
		}

		for (int y = 0; y < height; y++)
		{
			gimp_progress_update(((double)y) / height);
			guchar*	row=aimg.row(y);

			for (int x0 = 0; x0 < width; x0++)
			{
				int x=(y&1) ? width-1-x0 : x0;

				if (row[x] > 0)
					continue;

				// snake-alike space filling curve; do provide |p-p'|=1 invariant
				point_xy<int> p(x, y);

				auto m=dq.min(p);
				int minPos=m->image_idx;
				double r = dq.min_radius(p);

				// printf("fill-circ: %d,%d	%f	%d", x, y, r, minPos);
				aimg.fill_circle(p, r/2, minPos + 1);
			}
		}
		
		for (int y = 0; y < height; y++)
		{
			// gimp_progress_update(((double)y) / height);
			guchar*	row=aimg.row(y);
			for (int x = 0; x < width; x++) {
				point_xy<int> p(x, y);
				images[row[x]-1].paint(p);
			}
		}

	}
	void show_hulls()
	{
		for (auto it = images.begin(); it != images.end(); it++)
		{
			it->show_hull();
		}
	}
	void cleanup()
	{
		for (auto it = images.begin(); it != images.end(); it++)
		{
			it->cleanup();
		}
	}
	void flush()
	{
		for (auto it = images.begin(); it != images.end(); it++)
		{
			// 	it->show_distance();
			it->flush();
		}
	}
};

/*  Public functions  */
void render(gint32 image_ID, MontageMode mode)
{
	gimp_progress_init(PLUGIN_NAME);

	int num_layers;
	gint *layers = gimp_image_get_layers(image_ID, &num_layers);

	PMontage montage(image_ID);
	for (int i = 0; i < num_layers; i++)
	{
		gint32 layer = layers[i];
		if (!gimp_drawable_get_visible(layer))
		{
			continue;
		}
		if (!gimp_layer_get_mask(layer))
		{
			continue;
		}
		gchar *name = gimp_drawable_get_name(layers[i]);
		gint32 mask = gimp_layer_get_mask(layers[i]);
		printf("mask: %d", mask);
		if (mask < 0)
		{
			// ignore; no mask!
			continue;
		}

		gimp_progress_set_text(name);
		gimp_progress_update(i * 1.0 / num_layers);

		PImage mi(mask);
		mi.debug();
		// mi.show_distance();
		// mi.flush();
		montage.add(mi);
	}


	switch (mode)
	{
	case MontageMode::CLEANUP_MASKS:
		montage.cleanup();
		montage.flush();
		break;
	case MontageMode::VORONOI:
		montage.assign_voronoi();
		montage.flush();
		break;
	case MontageMode::SHOW_HULLS:
		montage.show_hulls();
		montage.flush();
		break;
	default:
		g_error("unhandled switch branch");
		break;
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
	gimp_progress_end();
}
