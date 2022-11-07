#include "config.h"

#include <string.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "main.h"
#include "interface.h"
#include "render.h"
#include "pmontage.h"

/*  Constants  */

#define PROCEDURE_SHOW_HULLS "show_hulls"
#define PROCEDURE_COMPUTE_VORONOI "compute_voronoi"
#define PROCEDURE_CLEANUP_MASKS "cleanup_masks"
#define PROCEDURE_SELECT_EDGES "select_edges"
#define PROCEDURE_CROSSFADE_EDGES "crossfade_edges"

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

  gimp_plugin_menu_register(PROCEDURE_SHOW_HULLS, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_COMPUTE_VORONOI, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_SELECT_EDGES, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_CROSSFADE_EDGES, "<Image>/Filters/Montager/");
  gimp_plugin_menu_register(PROCEDURE_CLEANUP_MASKS, "<Image>/Filters/Montager/");
}

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
  if (strcmp(name, PROCEDURE_SELECT_EDGES) == 0)
  {
    render(image_ID, MontageMode::SELECT_EDGES);
  }
  if (strcmp(name, PROCEDURE_CROSSFADE_EDGES) == 0)
  {
    render(image_ID, MontageMode::CROSSFADE_EDGES);
  }
  gimp_image_undo_group_end(image_ID);

  if (run_mode != GIMP_RUN_NONINTERACTIVE)
    gimp_displays_flush();

  gimp_drawable_detach(drawable);

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
}

MAIN()
