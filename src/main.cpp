#include "config.h"

#include <string.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "main.h"
#include "interface.h"
#include "render.h"

/*  Constants  */

#define PROCEDURE_SHOW_HULLS "show_hulls"
#define PROCEDURE_COMPUTE_VORONOI "compute_voronoi"
#define PROCEDURE_CLEANUP_MASKS "cleanup_masks"

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
        0,
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

  // gimp_plugin_domain_register (PLUGIN_NAME, LOCALEDIR);

  // help_path = g_build_filename (DATADIR, "help", NULL);
  // help_uri = g_filename_to_uri (help_path, NULL, NULL);
  // g_free (help_path);

  // gimp_plugin_help_register ("http://developer.gimp.org/plug-in-template/help",
  //                            help_uri);

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

  gimp_plugin_menu_register(PROCEDURE_SHOW_HULLS, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_COMPUTE_VORONOI, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_CLEANUP_MASKS, "<Image>/Filters/Montager/");
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
  vals = default_vals;
  image_vals = default_image_vals;
  drawable_vals = default_drawable_vals;
  ui_vals = default_ui_vals;

  gimp_image_undo_group_start(image_ID);
  if (strcmp(name, PROCEDURE_SHOW_HULLS) == 0)
  {
    render(image_ID, MontageMode::SHOW_HULLS);
  }
  if (strcmp(name, PROCEDURE_COMPUTE_VORONOI) == 0)
  {
    render(image_ID, MontageMode::VORONOI);
  }
  if (strcmp(name, PROCEDURE_CLEANUP_MASKS) == 0)
  {
    render(image_ID, MontageMode::CLEANUP_MASKS);
  }
  gimp_image_undo_group_end(image_ID);

  if (run_mode != GIMP_RUN_NONINTERACTIVE)
    gimp_displays_flush();

  gimp_drawable_detach(drawable);

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
}

MAIN()
