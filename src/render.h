#ifndef __RENDER_H__
#define __RENDER_H__

#include "main.h"

/*  Public functions  */
enum MontageMode
{
	SHOW_HULLS,
	VORONOI,
	CLEANUP_MASKS,
	SELECT_EDGES,
	CROSSFADE_EDGES,
	AUTO_LAYOUT,
};

void render(gint32 image_ID, MontageMode mode, PlugInVals *vals);

// void   render (gint32              image_ID,
// 	       GimpDrawable       *drawable,
// 	       PlugInVals         *vals,
// 	       PlugInImageVals    *image_vals,
// 	       PlugInDrawableVals *drawable_vals);

#endif /* __RENDER_H__ */
