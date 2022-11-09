#include "config.h"

#include <string.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#pragma GCC diagnostic pop

#include "main.h"
#include "interface.h"
#include "render.h"

/*  Constants  */

#define PROCEDURE_SHOW_HULLS "show_hulls"
#define PROCEDURE_COMPUTE_VORONOI "compute_voronoi"
#define PROCEDURE_CLEANUP_MASKS "cleanup_masks"
#define PROCEDURE_SELECT_EDGES "select_edges"
#define PROCEDURE_CROSSFADE_EDGES "crossfade_edges"
#define PROCEDURE_AUTO_LAYOUT "auto_layout"

#define DATA_KEY_VALS "plug_in_template"
#define DATA_KEY_UI_VALS "plug_in_template_ui"

#define PARASITE_KEY "plug-in-template-options"

/*  Local function prototypes  */

static void query(void);
static void run(const gchar *name,
                gint nparams,
                const GimpParam *param,
                gint *nreturn_vals,
                GimpParam **return_vals);

/*  Local variables  */

const PlugInVals default_vals =
    {
        30,
        1,
        2,
        0,
        FALSE};

const PlugInImageVals default_image_vals =
    {
        0};

const PlugInDrawableVals default_drawable_vals =
    {
        0};

const PlugInUIVals default_ui_vals =
    {
        TRUE};

static PlugInVals vals;
static PlugInImageVals image_vals;
static PlugInDrawableVals drawable_vals;
static PlugInUIVals ui_vals;

GimpPlugInInfo PLUG_IN_INFO =
    {
        NULL,  /* init_proc  */
        NULL,  /* quit_proc  */
        query, /* query_proc */
        run,   /* run_proc   */
};

static void
query(void)
{
  // gchar *help_path;
  // gchar *help_uri;

#pragma GCC diagnostic ignored "-Wwrite-strings"
  static GimpParamDef args[] =
      {
          {GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive"},
          {GIMP_PDB_IMAGE, "image", "Input image"},
          {GIMP_PDB_DRAWABLE, "drawable", "Input drawable"},
          {GIMP_PDB_INT32, "dummy", "dummy1"},
          {GIMP_PDB_INT32, "dummy", "dummy2"},
          {GIMP_PDB_INT32, "dummy", "dummy3"},
          {GIMP_PDB_INT32, "seed", "Seed value (used only if randomize is FALSE)"},
          {GIMP_PDB_INT32, "randomize", "Use a random seed (TRUE, FALSE)"}};
#pragma GCC diagnostic pop

  gimp_install_procedure(PROCEDURE_SHOW_HULLS,
                         "Shows the convex hulls of the layers",
                         "Help",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "2022",
                         "Show convex hulls",
                         "RGB*, GRAY*, INDEXED*",
                         GIMP_PLUGIN,
                         G_N_ELEMENTS(args), 0,
                         args, NULL);

  gimp_install_procedure(PROCEDURE_COMPUTE_VORONOI,
                         "Computes the Voronoi assignment of each layer based on the convex hull of the full opacity pixels in the layer mask",
                         "Help",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "2022",
                         "Voronoi generation",
                         "RGB*, GRAY*, INDEXED*",
                         GIMP_PLUGIN,
                         G_N_ELEMENTS(args), 0,
                         args, NULL);

  gimp_install_procedure(PROCEDURE_CLEANUP_MASKS,
                         "Cleans up the masks - so that the original area of importance is shown",
                         "Help",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "2022",
                         "Cleanup masks",
                         "RGB*, GRAY*, INDEXED*",
                         GIMP_PLUGIN,
                         G_N_ELEMENTS(args), 0,
                         args, NULL);

  gimp_install_procedure(PROCEDURE_SELECT_EDGES,
                         "Selects the in-between edges of the pieces",
                         "Help",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "2022",
                         "Select edges",
                         "RGB*, GRAY*, INDEXED*",
                         GIMP_PLUGIN,
                         G_N_ELEMENTS(args), 0,
                         args, NULL);

  gimp_install_procedure(PROCEDURE_CROSSFADE_EDGES,
                         "Crossfades the edges between the voronoi fields",
                         "Help",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "2022",
                         "Crossfade edges",
                         "RGB*, GRAY*, INDEXED*",
                         GIMP_PLUGIN,
                         G_N_ELEMENTS(args), 0,
                         args, NULL);

  gimp_install_procedure(PROCEDURE_AUTO_LAYOUT,
                         "creates a layout for the active images",
                         "Help",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "Zoltan Haindrich <kirk@rxd.hu>",
                         "2022",
                         "Auto layout",
                         "RGB*, GRAY*, INDEXED*",
                         GIMP_PLUGIN,
                         G_N_ELEMENTS(args), 0,
                         args, NULL);

  gimp_plugin_menu_register(PROCEDURE_SHOW_HULLS, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_COMPUTE_VORONOI, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_SELECT_EDGES, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_CROSSFADE_EDGES, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_CLEANUP_MASKS, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_AUTO_LAYOUT, "<Image>/Filters/Montager/");
}

static void
run(const gchar *name,
    gint n_params,
    const GimpParam *param,
    gint *nreturn_vals,
    GimpParam **return_vals)
{
  static GimpParam values[1];
  GimpDrawable *drawable;
  gint32 image_ID;
  GimpRunMode run_mode;
  GimpPDBStatusType status = GIMP_PDB_SUCCESS;

  *nreturn_vals = 1;
  *return_vals = values;

  run_mode = (GimpRunMode)param[0].data.d_int32;
  image_ID = param[1].data.d_int32;
  drawable = gimp_drawable_get(param[2].data.d_drawable);

  /*  Initialize with default values  */
  if (gimp_get_data_size(DATA_KEY_VALS) != sizeof(vals) || !gimp_get_data(DATA_KEY_VALS, &vals))
  {
    vals = default_vals;
  }

  if (run_mode == GIMP_RUN_INTERACTIVE)
  {
    if (strcmp(name, PROCEDURE_CROSSFADE_EDGES) == 0)
    {
      if (dialog(image_ID, drawable, &vals, &image_vals, &drawable_vals, &ui_vals))
      {
        gimp_set_data(DATA_KEY_VALS, &vals, sizeof(vals));
      }
      else
      {
        status = GIMP_PDB_CANCEL;
      }
    }
  }

  if (status == GIMP_PDB_SUCCESS)
  {
    gimp_image_undo_group_start(image_ID);
    if (strcmp(name, PROCEDURE_SHOW_HULLS) == 0)
    {
      render(image_ID, MontageMode::SHOW_HULLS, &vals);
    }
    if (strcmp(name, PROCEDURE_COMPUTE_VORONOI) == 0)
    {
      render(image_ID, MontageMode::VORONOI, &vals);
    }
    if (strcmp(name, PROCEDURE_CLEANUP_MASKS) == 0)
    {
      render(image_ID, MontageMode::CLEANUP_MASKS, &vals);
    }
    if (strcmp(name, PROCEDURE_SELECT_EDGES) == 0)
    {
      render(image_ID, MontageMode::SELECT_EDGES, &vals);
    }
    if (strcmp(name, PROCEDURE_CROSSFADE_EDGES) == 0)
    {
      render(image_ID, MontageMode::CROSSFADE_EDGES, &vals);
    }
    if (strcmp(name, PROCEDURE_AUTO_LAYOUT) == 0)
    {
      render(image_ID, MontageMode::AUTO_LAYOUT, &vals);
    }
    gimp_image_undo_group_end(image_ID);
  }

  if (run_mode != GIMP_RUN_NONINTERACTIVE)
    gimp_displays_flush();

  gimp_drawable_detach(drawable);


  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
}

MAIN()
