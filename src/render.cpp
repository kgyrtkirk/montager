#include "config.h"

#include <gtk/gtk.h>

#include <libgimp/gimp.h>

#include "main.h"
#include "render.h"
#include "image.h"
#include "dist_queue.h"
#include "pimage.h"

#include <iostream>

#include <boost/chrono.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>


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
		if (num_inputs() < 2)
		{
			g_warning("Not enough input layers for this operation; skipping assign_voronoi!");
			return;
		}

		gimp_progress_set_text("assign_voronoi");
		if (images.size() >= 253)
		{
			g_error("more images than supported");
		}
		image aimg;
		aimg.init(width, height);

		dist_queue dq;

		for (int i = 0; i < images.size(); i++)
		{
			dq.add(new dist_queue::entry(images[i].getHull(), i));
		}

		for (int y = 0; y < height; y++)
		{
			gimp_progress_update(((double)y) / height);
			guchar *row = aimg.row(y);

			for (int x0 = 0; x0 < width; x0++)
			{
				int x = (y & 1) ? width - 1 - x0 : x0;

				if (row[x] > 0)
					continue;

				// snake-alike space filling curve; do provide |p-p'|=1 invariant
				point_xy<int> p(x, y);

				auto m = dq.min(p);
				int minPos = m->image_idx;
				double r = dq.min_radius(p);
				aimg.fill_circle(p, r, minPos + 1);
			}
		}

		for (int y = 0; y < height; y++)
		{
			// gimp_progress_update(((double)y) / height);
			guchar *row = aimg.row(y);
			for (int x = 0; x < width; x++)
			{
				point_xy<int> p(x, y);
				images[row[x] - 1].paint(p);
			}
		}
	}
	void show_hulls()
	{
		gimp_progress_set_text("show hulls...");
		gimp_progress_update(0);
		for (int i = 0; i < images.size(); i++)
		{
			images[i].show_hull();
			gimp_progress_update((i + 1) * 1.0 / images.size());
		}
	}
	void cleanup()
	{
		for (auto it = images.begin(); it != images.end(); it++)
		{
			it->cleanup();
		}
	}
	void select_edges()
	{
		gimp_selection_none(image_id);

		int selection_channel = gimp_selection_save(image_id);
		for (auto it = images.begin(); it != images.end(); it++)
		{
			gimp_image_select_item(image_id, GIMP_CHANNEL_OP_REPLACE, it->getDrawable()->drawable_id);
			gimp_selection_shrink(image_id, 3);
			gimp_image_select_item(image_id, GIMP_CHANNEL_OP_ADD, selection_channel);
			gimp_image_remove_channel(image_id, selection_channel);
			selection_channel = gimp_selection_save(image_id);
		}
		gimp_selection_load(selection_channel);
		gimp_item_delete(selection_channel);
	}
	void crossfade(gdouble radius)
	{
		gimp_selection_none(image_id);

		for (auto it = images.begin(); it != images.end(); it++)
		{

			gimp_selection_none(image_id);
			gimp_image_select_item(image_id, GIMP_CHANNEL_OP_REPLACE, it->getDrawable()->drawable_id);

			it->cleanup();
			it->flush();

			// feather works strangely; where >2 this will not work anyway...will get back later

			gdouble off = radius / 4;
			gimp_selection_grow(image_id, off);
			gimp_selection_feather(image_id, radius);

			gimp_drawable_edit_fill(it->getDrawable()->drawable_id, GIMP_WHITE_FILL);
			gimp_selection_shrink(image_id, off);
		}
	}

	void flush()
	{
		for (auto it = images.begin(); it != images.end(); it++)
		{
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
	boost::chrono::high_resolution_clock::time_point t0 = boost::chrono::high_resolution_clock::now();
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
		if (mask < 0)
		{
			// ignore; no mask!
			continue;
		}

		gimp_progress_set_text(name);
		gimp_progress_update(i * 1.0 / num_layers);

		PImage mi(mask);
		mi.debug();
		montage.add(mi);
	}

	boost::chrono::high_resolution_clock::time_point t1 = boost::chrono::high_resolution_clock::now();
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
	case MontageMode::SELECT_EDGES:
		// montage.show_hulls();
		// montage.flush();
		montage.select_edges();
		break;
	case MontageMode::CROSSFADE_EDGES:
		// montage.show_hulls();
		// montage.flush();
		montage.crossfade(30);
		break;
	default:
		g_error("unhandled switch branch");
		break;
	}
	boost::chrono::high_resolution_clock::time_point t2 = boost::chrono::high_resolution_clock::now();
	printf("init time: %d\n", (boost::chrono::duration_cast<boost::chrono::milliseconds>(t1 - t0)));
	printf("execution time: %d\n", (boost::chrono::duration_cast<boost::chrono::milliseconds>(t2 - t1)));

	free(layers);

	g_message("montager finished %dms", boost::chrono::duration_cast<boost::chrono::milliseconds>(t2 - t0));
	gimp_progress_end();
}
