#ifndef __RENDER_H__
#define __RENDER_H__


/*  Public functions  */
enum MontageMode {
	SHOW_HULLS,
	VORONOI,
	CLEANUP_MASKS,
	SELECT_EDGES,
	CROSSFADE_EDGES,
};

// void render(gint32 image_ID, MontageMode mode);

// void   render (gint32              image_ID,
// 	       GimpDrawable       *drawable,
// 	       PlugInVals         *vals,
// 	       PlugInImageVals    *image_vals,
// 	       PlugInDrawableVals *drawable_vals);


#endif /* __RENDER_H__ */
