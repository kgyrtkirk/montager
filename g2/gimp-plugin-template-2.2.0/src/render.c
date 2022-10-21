/* GIMP Plug-in Template
 * Copyright (C) 2000  Michael Natterer <mitch@gimp.org> (the "Author").
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the Author of the
 * Software shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the Author.
 */

#include "config.h"

#include <gtk/gtk.h>

#include <libgimp/gimp.h>

#include "main.h"
#include "render.h"

#include "plugin-intl.h"

void read_image(gint32 drawable_id)
{
	GimpDrawable *drawable = gimp_drawable_get(drawable_id);
	gint bpp = drawable->bpp; //(drawable->drawable_id);
	gint w = drawable->width;
	gint h = drawable->height;

	GimpPixelRgn region;
	gimp_pixel_rgn_init(&region, drawable, 0, 0, w, h, FALSE, FALSE);
	size_t size = w * h * bpp;
	guchar *img = g_malloc(size);
	printf("maskbpp :%d %d\n", bpp, size);

	gimp_pixel_rgn_get_rect(&region, img, 0, 0, w, h);
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (i * w + j < size)
				img[i * w + j] = 255;
		}
	}
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

	g_message(_("White-white!"));
}
