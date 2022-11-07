#include "config.h"

#include <libgimp/gimp.h>

#include "main.h"
#include "image.h"
#include "pimage.h"

#include <iostream>


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
	void assign_voronoi();

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
