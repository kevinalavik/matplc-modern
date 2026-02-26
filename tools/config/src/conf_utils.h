#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "interface.h"
#include "support.h"



  GtkWidget *Fileselection1;
  GtkWidget *About;


/*

Handle window closures and program exit

*/

void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data);

void
on_button9_clicked                     (GtkButton       *button,
                                        gpointer         user_data);


void
on_Close_Binary_Add_clicked            (GtkButton       *button,
                                        gpointer         user_data);
void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
on_button3_clicked                     (GtkButton       *button,
                                        gpointer         user_data);


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
file_select_cancel                     (GtkButton       *button,
                                        gpointer         user_data);





