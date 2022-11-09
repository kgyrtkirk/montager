#include "config.h"
#include "main.h"
#include "interface.h"

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>


/*  Constants  */
#define SCALE_WIDTH 180
#define SPIN_BUTTON_WIDTH 75
#define RANDOM_SEED_WIDTH 100

gboolean
dialog(PlugInVals *vals)
{
  GtkWidget *dlg;
  GtkWidget *main_vbox;
  GtkWidget *frame;
  GtkWidget *table;
  GtkObject *adj;
  gint row;
  gboolean run = FALSE;

  gimp_ui_init(PLUGIN_NAME, TRUE);

  dlg = gimp_dialog_new("GIMP Plug-In Template11", PLUGIN_NAME,
                        NULL, (GtkDialogFlags)0,
                        gimp_standard_help_func, "plug-in-template",

                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                        GTK_STOCK_OK, GTK_RESPONSE_OK,

                        NULL);

  main_vbox = gtk_vbox_new(FALSE, 12);
  gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 12);
  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dlg)->vbox), main_vbox);

  /*  gimp_scale_entry_new() examples  */

  frame = gimp_frame_new("ScaleEntry Examples");
  gtk_box_pack_start(GTK_BOX(main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show(frame);

  table = gtk_table_new(3, 3, FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(table), 6);
  gtk_table_set_row_spacings(GTK_TABLE(table), 2);
  gtk_container_add(GTK_CONTAINER(frame), table);
  gtk_widget_show(table);

  row = 0;

  adj = gimp_scale_entry_new(GTK_TABLE(table), 0, row++,
                             "Feather:", SCALE_WIDTH, SPIN_BUTTON_WIDTH,
                             vals->dummy1, 0, 1000, 1, 10, 0,
                             TRUE, 0, 0,
                             "feather", NULL);
  g_signal_connect(adj, "value_changed",
                   G_CALLBACK(gimp_int_adjustment_update),
                   &vals->dummy1);

  /*  Show the main containers  */

  gtk_widget_show(main_vbox);
  gtk_widget_show(dlg);

  run = (gimp_dialog_run(GIMP_DIALOG(dlg)) == GTK_RESPONSE_OK);


  gtk_widget_destroy(dlg);

  return run;
}
