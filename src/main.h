#pragma once

typedef struct
{
  gint dummy1;
  gint dummy2;
  gint dummy3;
  guint seed;
  gboolean random_seed;
} PlugInVals;

typedef struct
{
  gint32 image_id;
} PlugInImageVals;

typedef struct
{
  gint32 drawable_id;
} PlugInDrawableVals;

typedef struct
{
  gboolean chain_active;
} PlugInUIVals;

/*  Default values  */

extern const PlugInVals default_vals;
extern const PlugInImageVals default_image_vals;
extern const PlugInDrawableVals default_drawable_vals;
extern const PlugInUIVals default_ui_vals;
