#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf_utils.h"
#include "interface.h"
#include "support.h"



/*

Handle close buttons and program exit

*/

void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data)
{

	gtk_exit(0);

}


void
on_button9_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}


void
on_Close_Binary_Add_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}

void
on_button3_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}

void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)

{

  gtk_main_quit();

}


/*****************************************************************************

Display the About window.

*****************************************************************************/
void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

  About = create_About();
  gtk_widget_show (About);

}


/*****************************************************************************

file_select_cancel

User changed their mind about loading / saving the file.  destroy the file
selection widget and go on with life.

*****************************************************************************/

void
file_select_cancel                     (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}


