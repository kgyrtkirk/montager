#include "config.h"

#include <gtk/gtk.h>

#include <libgimp/gimp.h>

#include "main.h"
#include "render.h"
#include "image.h"
#include "dist_queue.h"
#include "pimage.h"
#include "pmontage.h"

#include <iostream>

#include <boost/chrono.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>

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
