#include <gtk/gtk.h>
#include "conf_utils.h"

#define BINARY 0
#define INTEGER 1
#define UNSIGNED 2
#define FLOAT 3

#define LOAD 0
#define SAVE 1
#define MODULE_BROWSE 2



void
on_file1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
on_binary_elements1_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_Add_Binary_Element_clicked          (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_entry1_key_press_event              (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);
void
on_Add_Element_Button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_integer_elements1_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_unsigned_elements1_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_floating_point_elements1_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
file_select_OK                         (GtkButton       *button,
                                        gpointer         user_data);

void
edit_module                            ();







void parse_file	                       (char 		*filename);


void parse_point                       (char		*token_array[]);


void
on_window1_show                        (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_edit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_delete1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_button_press_event                  (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_Add_Element_key_press_event         (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);
void
on_select_row                          (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_close_edit_entry_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_update_element_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);


void
write_file                             (const char *filename);



void
on_elements1_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_Binary_Button_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_Unsigned_Button_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_Integer_Button_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_Float_Button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_Other_Button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
edit_point                           ();


gboolean
on_update_element_keypress             (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

gboolean
on_column_clicked                      (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);



void
on_add_modules_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_module_close_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_module_add                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_module_change	               (GtkButton       *button,
                                        gpointer         user_data);




void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);



void
on_load1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
module_editor_key_press                (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);


void
on_select_row                          (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_module_accept_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_module_browse_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

