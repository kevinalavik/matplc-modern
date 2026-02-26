/*****************************************************************************
******************************************************************************

This is the 'meat' of the application.  Some of the more mundane functions
have been handed off to conf_utils.c to try to keep this neat(er)......

Things left to do include:


	update the Load/Save to include point types, etc.

	Entry validation - make sure the data is legal, mandatory fields are
        enforced, etc.

	Probably a million other things........

******************************************************************************
*****************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "conf_utils.h"
#include <gdk/gdkkeysyms.h>
#include "../../../lib/conffile/conffile_private.h"
#include "../../../lib/misc/ds_util.h"


  GtkWidget *add_module;
  GtkWidget *point_editor;

  gboolean save = FALSE;
  gboolean module_edit = FALSE;
  GtkWidget *Add_Element;
  GtkWidget *reuse_error;
  GtkWidget *top_level;
  GtkCList *element_list, *module_list;
  GtkNotebook *Notebook;
  gint point_row = 0, module_row  = 0;
  gint filewindow_function = LOAD;

  gchar *AddType = "Binary";
/*  Added for ugly hack to work around prob in gtk_clist_get_selection_info */
  gint clist_row;

/*****************************************************************************

on_save1_activate

This function calls up the file selection dialog.  Since we use the same
object for load and save, we set a boolean to tell us which one we are doing
once the OK button is pressed.  I am sure that there are 'cooler' ways of
doing this, but this works for now.

*****************************************************************************/
void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  filewindow_function = SAVE;
  Fileselection1=create_fileselection1();
  gtk_file_selection_set_filename (GTK_FILE_SELECTION(Fileselection1),"plc_point.conf");
  gtk_widget_show (Fileselection1);
}

/*****************************************************************************

on_load1_activate

Display the file selection widget.


*****************************************************************************/
void
on_load1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

  Fileselection1=create_fileselection1();
  filewindow_function = LOAD;
  gtk_file_selection_set_filename (GTK_FILE_SELECTION(Fileselection1),"plc_point.conf");
  gtk_widget_show (Fileselection1);

}

/*****************************************************************************

on_Add_Element_Button_clicked


When the ADD button is pressed on the data entry window, the data is parsed,
ordered, checked for duplication against the existsing records, and inserted
into the appropriate list.


TO DO:  Need to add checking for valid data (no spaces, etc.) and that all
	mandatory fields are filled in.

 
*****************************************************************************/
void
on_Add_Element_Button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *Entry_Field, *Entry1;
  GtkEditable *List[6];
  gint loop, success;
  gchar *data[6];
  int dup_found = 0;
  gchar *dup_check;
  gchar **ptr_dup_check = &dup_check;

/* suck all the data out of the entry window */
  Entry_Field = lookup_widget(GTK_WIDGET (Add_Element), "edit_entry1");
  Entry1 = Entry_Field;
  List[0] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (Add_Element), "edit_entry2");
  List[1] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (Add_Element), "edit_entry3");
  List[2] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (Add_Element), "edit_entry4");
  List[3] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (Add_Element), "edit_entry5");
  List[4] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (Add_Element), "edit_entry6");
  List[5] = GTK_EDITABLE (Entry_Field);

/* Make the pretty list that Clist_Append requires */
  data[0] = gtk_editable_get_chars(List[0],0,-1);
  data[1] = gtk_editable_get_chars(List[1],0,-1);
  data[2] = gtk_editable_get_chars(List[2],0,-1);
  data[3] = gtk_editable_get_chars(List[3],0,-1);
  data[4] = gtk_editable_get_chars(List[4],0,-1);
  data[5] = gtk_editable_get_chars(List[5],0,-1);
  data[6] = AddType;


/*  Check the element list for duplicate names  */
  if (element_list != NULL)  /* Have we added any elements yet? */
   for (loop=0;loop <= point_row;loop++)
    if (gtk_clist_get_text(element_list,loop,0,ptr_dup_check))
      if (strcmp(dup_check,data[0]) == 0)
        {
          dup_found = 1;
	  reuse_error = create_reuse_error();
	  gtk_widget_show(reuse_error);
	}

/*  If no duplicates are found, add the entry to the correct list */
  if (!dup_found)
  {
          success = gtk_clist_append (element_list,data);
          point_row++;
  }

/* Put focus backto the ID entry field to speed data entry */
  gtk_widget_grab_focus (Entry1);
  gtk_clist_sort(element_list);
}

/*****************************************************************************

on_elements1_activate


 Elements selected from the add menu.  This will become the only add
function available.  It will be renamed add elements, and the radio buttons
will be used to determine the data type after the ADD button is pressed.
Currently calls up the add element window, sets the title bar as appropriate
and allows the user to enter data.

*****************************************************************************/
void
on_elements1_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

  GtkWindow *Add_Window;
  GtkWidget *button, *combo;
  GList *items = NULL;
  gchar *dummy = "";
  gchar *module_name = "";
  gchar **module_name_ptr = &module_name;
  int loop, success;


  gtk_notebook_set_page (Notebook,1);

  Add_Element = create_Add_Element ();
  Add_Window = GTK_WINDOW (Add_Element);
  gtk_window_set_title (Add_Window, "Add  Elements");

  button = lookup_widget(Add_Element,"binary_button");
  on_Binary_Button_toggled (GTK_TOGGLE_BUTTON(button),dummy);

  combo = lookup_widget(button,"owner_menu");
  if ((module_list != NULL) && (module_row > 0))
    for (loop = 0;loop <= module_row; loop++)
    {
     success = gtk_clist_get_text (module_list,loop,0, module_name_ptr);
     if (success)
      items = g_list_append (items,module_name);
    }
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), items);
  gtk_widget_show (Add_Element);

}


/*****************************************************************************

write_file

Writes the data to the file.  Each list is gone thru in turn, and in each
list, every row is read, split into component parts, the description field
is padded with " marks, and written out to the file.  Since some of the data
is optional, such as memory location, we check those fields for existing
data, and if found, we include the required keywords for the plc parser.

*****************************************************************************/
void write_file                        (const char *filename)
{

  FILE *File;
  int loop,success;
  gchar *module_field, *PointID, *Description, *Owner, *Memory, *Init, *Length, *point_type;
  gchar **ptr_module_field = &module_field;
  gchar **ptr_ID = &PointID;
  gchar **ptr_des = &Description;
  gchar **ptr_own = &Owner;
  gchar **ptr_mem = &Memory;
  gchar **ptr_init = &Init;
  gchar **ptr_len = &Length;
  gchar **ptr_point_type = &point_type;

  File = fopen(filename,"wt");
  if (File) {
    fprintf(File,"#\n# MatPLC Point Configuration File\n");
    fprintf(File,"# Generated by config-edit\n");
    fprintf(File,"#\n# Test Header Only\n");
    fprintf(File,"#\n# This file should be included into the main matplc.conf file using \n");
    fprintf(File,"#\n# *include plc_point.conf \n#\n#\n");

    fprintf(File,"[PLC]\n");

    if (module_list != NULL)
      for (loop = 0; loop <= module_row; loop++)
      {

        module_field = "";
        success = gtk_clist_get_text(module_list,loop,0,ptr_module_field);
        if (success)
          fprintf(File,"module %s ",module_field);

        success = gtk_clist_get_text(module_list,loop,1,ptr_module_field);
        if (success)
          fprintf(File,"%s ",module_field);

        success = gtk_clist_get_text(module_list,loop,2,ptr_module_field);
        if (success)
          fprintf(File,"%s\n",module_field);
      }

    fprintf(File,"\n\n");

    if (element_list != NULL)
      for(loop = 0;loop <= point_row; loop++)
      {
        PointID = "";
        Description = "";
        Owner = "";
        Memory = "";
        Length = "";
        Init = "";
        point_type = "";
        success = gtk_clist_get_text(element_list,loop,0,ptr_ID);
        success = gtk_clist_get_text(element_list,loop,1,ptr_des);
        success = gtk_clist_get_text(element_list,loop,2,ptr_own);
        success = gtk_clist_get_text(element_list,loop,3,ptr_mem);
        success = gtk_clist_get_text(element_list,loop,4,ptr_init);
        success = gtk_clist_get_text(element_list,loop,5,ptr_len);
        success = gtk_clist_get_text(element_list,loop,6,ptr_point_type);

        fprintf(File,"point %s \"%s\" %s",PointID,Description,Owner);

        if (strcmp(Memory,"") !=0 )
          fprintf(File," at %s",Memory);
        if (strcmp(Length,"")!=0)
        {
          if (strcmp(point_type,"Binary")==0)
            fprintf(File," b");
          if (strcmp(point_type,"Integer")==0)
            fprintf(File," i");
          if (strcmp(point_type,"Unsigned")==0)
            fprintf(File," u");
          if (strcmp(point_type,"Float")==0)
            fprintf(File," f");

          fprintf(File," %s",Length);
        }
        if (strcmp(Init,"") !=0 )
          fprintf(File," init %s",Init);
        fprintf(File,"\n");
      }


  }

  fclose(File);

}



/*****************************************************************************

file_select_OK

The user has decided to load the file.  This function destroys the file
select window, wipes out the current data in the lists, and sends the file
name along to be processed by parse_file.


*****************************************************************************/

void
file_select_OK                         (GtkButton       *button,
                                        gpointer         user_data)
{
  char *filename, *tempfile;
  GtkWidget *path_widget;
  GtkEntry *path_entry;


/* need to use a temp variable while we create a safe place to store filename.
   otherwise, filename gets corrupt when the file selection object is destroyed.
   We could wait to destroy it till the end, but I was afraid that large files
   may look like the machine hung if it is parsing with the dialog still open.

*/

  tempfile = gtk_file_selection_get_filename (GTK_FILE_SELECTION(Fileselection1));
  filename = malloc(strlen(tempfile)+1);
  strcpy(filename,tempfile);
  gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));

  if (filewindow_function == SAVE)
  {
    write_file(filename);
  }

  else if (filewindow_function == LOAD)
  {
    gtk_clist_clear(element_list);
    gtk_clist_clear(module_list);

    point_row = 0;
    module_row = 0;

    parse_file (filename);
  }

  else if (filewindow_function == MODULE_BROWSE)
  {

    path_widget = lookup_widget(GTK_WIDGET (add_module), "add_module_path");
    path_entry = GTK_ENTRY (path_widget);
    gtk_entry_set_text (path_entry,filename);

  }




  free(filename);
}


/*****************************************************************************

parse_file

This function reads the file that was passed to it in *filename.  We use the
parse library from the main PLC to read in the data, so what we get is the
exact same thing that the PLC gets.

*****************************************************************************/
void parse_file	                       (char            *filename)
{

  list *section;
  int loop, inner_loop, success;
  list *row;
  gchar *module_data[3];
  gchar *point_data[6];
  gchar *scratchpad = NULL;
  gchar *look_ahead = NULL;

  dict *vars = NULL;
  dict *recs = NULL;
  dict *files = NULL;

  conffile_load_file(filename, &vars, &recs, &files);

  section = dict_get(recs, "PLC:module");
  for (loop = 0; loop<list_len(section); loop++)
  {
    row = list_item(section,loop);
    module_data[0] = list_item(row,0);
    module_data[1] = list_item(row,1);
    module_data[2] = list_item(row,2);

    success = gtk_clist_append(module_list,module_data);
    if (success)
      module_row++;
  }

  section = dict_get(recs, "PLC:point");
  for (loop=0; loop<list_len(section); loop++)
  {
    row = list_item(section,loop);
    point_data[0] = list_item(row,0);
    point_data[1] = list_item(row,1);
    point_data[2] = list_item(row,2);
    point_data[3] = "";
    point_data[4] = "";
    point_data[5] = "";
    point_data[6] = "";
    

    for (inner_loop = 3; list_item(row,inner_loop)!=NULL; inner_loop++)
    {
      scratchpad = list_item(row,inner_loop);
      look_ahead = list_item(row,inner_loop+1);
      if (strcmp(scratchpad,"b")==0)
      {
        point_data[6]="Binary";
        if ((look_ahead != NULL) && (strcmp(look_ahead,"init")!=0))
          point_data[5] = look_ahead;
      }
      if (strcmp(scratchpad,"i")==0)
      {
        point_data[6]="Integer";
        if ((look_ahead != NULL) && (strcmp(look_ahead,"init")!=0))
          point_data[5] = look_ahead;
      }
      if (strcmp(scratchpad,"u")==0)
      {
        point_data[6]="Unsigned";
        if ((look_ahead != NULL) && (strcmp(look_ahead,"init")!=0))
          point_data[5] = look_ahead;
      }
      if (strcmp(scratchpad,"f")==0)
      {
        point_data[6]="Float";
        if ((look_ahead != NULL) && (strcmp(look_ahead,"init")!=0))
          point_data[5] = look_ahead;
      }

      if (strcmp(scratchpad,"at") == 0)
        point_data[3] = look_ahead;

      if (strcmp(scratchpad,"init") == 0)
        point_data[4] = look_ahead;

    }   /*end of inner_loop*/

    success = gtk_clist_append(element_list,point_data);
    if (success)
      point_row++;
  }     /* end of loop*/
}

/*****************************************************************************

on_window1_show

When the point editor is first brought up, we need to gather some pointers
to various elements.

*****************************************************************************/
void
on_window1_show                        (GtkWidget       *widget,
                                        gpointer         user_data)
{

  element_list = GTK_CLIST(lookup_widget(GTK_WIDGET (widget), "element_list"));
  gtk_clist_set_sort_type(element_list,GTK_SORT_ASCENDING);
  gtk_clist_set_compare_func(element_list,NULL);
  gtk_clist_set_auto_sort (element_list,TRUE);


  module_list = GTK_CLIST(lookup_widget(GTK_WIDGET (widget), "module_list"));
  gtk_clist_set_sort_type(module_list,GTK_SORT_ASCENDING);
  gtk_clist_set_compare_func(module_list,NULL);
  gtk_clist_set_auto_sort (module_list,TRUE);

  Notebook = GTK_NOTEBOOK(lookup_widget(GTK_WIDGET(widget),"notebook1"));

}

/*****************************************************************************

Edit elements that already exist

*****************************************************************************/

void
on_edit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

  int page;


  page = gtk_notebook_get_current_page(Notebook);
  switch (page)
  {

    case 0:
    {
      edit_module();
      break;
    }

    case 1:
    {

     edit_point();
     break;
    }
  }
}


void
edit_point ()

{

  GtkWidget *entry_field, *edit_window, *button;
  gint success;
  gchar *text;
  gchar **text_ptr = &text;
  GtkEntry *entry_window;



  edit_window = create_edit_entry ();
  gtk_widget_show(edit_window);

  success = gtk_clist_get_text(element_list,clist_row,0,text_ptr);
  if (success)
  {
    entry_field = lookup_widget(GTK_WIDGET (edit_window), "edit_entry1");
    entry_window = GTK_ENTRY (entry_field);
    gtk_entry_set_text (entry_window,text);
  }

    success = gtk_clist_get_text(element_list,clist_row,1,text_ptr);
  if (success)
  {
    entry_field = lookup_widget(GTK_WIDGET (edit_window), "edit_entry2");
    entry_window = GTK_ENTRY (entry_field);
    gtk_entry_set_text (entry_window,text);
  }

    success = gtk_clist_get_text(element_list,clist_row,2,text_ptr);
  if (success)
  {
    entry_field = lookup_widget(GTK_WIDGET (edit_window), "edit_entry3");
    entry_window = GTK_ENTRY (entry_field);
    gtk_entry_set_text (entry_window,text);
  }
    success = gtk_clist_get_text(element_list,clist_row,3,text_ptr);
  if (success)
  {
    entry_field = lookup_widget(GTK_WIDGET (edit_window), "edit_entry4");
    entry_window = GTK_ENTRY (entry_field);
    gtk_entry_set_text (entry_window,text);
  }
    success = gtk_clist_get_text(element_list,clist_row,4,text_ptr);
  if (success)
  {
    entry_field = lookup_widget(GTK_WIDGET (edit_window), "edit_entry5");
    entry_window = GTK_ENTRY (entry_field);
    gtk_entry_set_text (entry_window,text);
  }
    success = gtk_clist_get_text(element_list,clist_row,5,text_ptr);
  if (success)
  {
    entry_field = lookup_widget(GTK_WIDGET (edit_window), "edit_entry6");
    entry_window = GTK_ENTRY (entry_field);
    gtk_entry_set_text (entry_window,text);
  }


  AddType = "Binary";

    success = gtk_clist_get_text(element_list,clist_row,6,text_ptr);
  if (success)
  {
    if (strcmp(text,"Binary") == 0)
    {
      button = lookup_widget(GTK_WIDGET (edit_window),"binary_button");
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    }

    if (strcmp(text,"Integer") == 0)
    {
      button = lookup_widget(GTK_WIDGET (edit_window),"integer_button");
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    }

    if (strcmp(text,"Unsigned") == 0)
    {
      button = lookup_widget(GTK_WIDGET (edit_window),"unsigned_button");
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    }

    if (strcmp(text,"Float") == 0)
    {
      button = lookup_widget(GTK_WIDGET (edit_window),"float_button");
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    }


    if (strcmp(text,"Other") == 0)
    {
      button = lookup_widget(GTK_WIDGET (edit_window),"other_button");
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    }
  }

}

/*****************************************************************************

on_delete1_activate

deletes the selected item from the list view.  Depending on which notebook
page we are on, we delete the module or point or whatever.  Very similar to
how we select what we want to edit.

*****************************************************************************/

void
on_delete1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

  int page;


  page = gtk_notebook_get_current_page(Notebook);
  switch (page)
  {

    case 0:
    {
      gtk_clist_remove(module_list,clist_row);
      module_row--;
      break;
    }

    case 1:
    {
     gtk_clist_remove(element_list,clist_row);
     point_row--;
     break;
    }
  }




gtk_clist_remove(element_list,clist_row);
point_row--;
}

/*****************************************************************************

edit_module

This section populates the module entry window with the data from the current
selection for editing.  If re-using the entry window for editing works, I'll
retro the point entry/editor, deprecate the editor, and re-use the entry for
that as well......

*****************************************************************************/


void edit_module  ()
{

  GtkWidget *entry_field;
  gint success;
  gchar *text;
  gchar **text_ptr = &text;
  GtkEntry *entry_window;

  add_module = create_add_module();
  gtk_widget_show (add_module);

  module_edit = TRUE;

  success = gtk_clist_get_text(module_list,clist_row,0,text_ptr);
  if (success)
  {
    entry_field = lookup_widget(GTK_WIDGET (add_module), "add_module_name");
    entry_window = GTK_ENTRY (entry_field);
    gtk_entry_set_text (entry_window,text);
  }


  success = gtk_clist_get_text(module_list,clist_row,1,text_ptr);
  if (success)
  {
    entry_field = lookup_widget(GTK_WIDGET (add_module), "add_module_path");
    entry_window = GTK_ENTRY (entry_field);
    gtk_entry_set_text (entry_window,text);
  }

  success = gtk_clist_get_text(module_list,clist_row,2,text_ptr);
  if (success)
  {
    entry_field = lookup_widget(GTK_WIDGET (add_module), "add_module_parameters");
    entry_window = GTK_ENTRY (entry_field);
    gtk_entry_set_text (entry_window,text);
  }

}

/*****************************************************************************

on_button_press_event

process the mouse button clicks on the notebook object.  This allows us to
pop the context menu up, and possibly add edit as a default action when
double clicking an element.

*****************************************************************************/

gboolean
on_button_press_event                  (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  GtkMenu *popup;
  int page;

  if (event->button == 3)
  {
    popup = GTK_MENU(create_Notebook_Popup());
    gtk_menu_popup (popup, NULL, NULL, NULL, NULL, event->button, event->time);
  }


  if (event->type == GDK_2BUTTON_PRESS)
  {
    page = gtk_notebook_get_current_page(Notebook);
    switch (page)
    {
      case 0:
      {
        edit_module();
        break;
      }

      case 1:
      {
        edit_point();
        break;
      }
    }
  }


  return FALSE;
}

/*****************************************************************************

on_Add_Element_key_press_event

when in any of the text entry fields of the add element window, we monitor
each keypress, looking for an enter key on the main keyboard or the key pad.
when one occurs, we run the function as if the ADD button had been pressed.

*****************************************************************************/


gboolean
on_Add_Element_key_press_event         (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
GtkButton *button;

  if ((event->keyval == GDK_Return) || (event->keyval == GDK_KP_Enter))
  {
    button = GTK_BUTTON(lookup_widget(GTK_WIDGET(widget),"Add_Element_Button"));
    on_Add_Element_Button_clicked(button,user_data);
  }
  return FALSE;
}


/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

THIS IS HERE ONLY UNTIL I CAN GET THE gtk_clist_get_selection_info FUNCTION
WORKING PROPERLY.

I REALIZE THIS IS AN UGLY HACK!!!!!

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/


void
on_select_row                          (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  clist_row = row;
}

void
on_close_edit_entry_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}



/*****************************************************************************

on_update_element_button_clicked

update the point that has been edited.  This will go away, and the point
add window will be re-used as the editor, just like we did with the module
add/edit window.


*****************************************************************************/



void
on_update_element_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *Entry_Field;
  GtkEditable *List[6];
  gchar *data[7];

/* suck all the data out of the entry window */

  Entry_Field = lookup_widget(GTK_WIDGET (button), "edit_entry1");
  List[0] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (button), "edit_entry2");
  List[1] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (button), "edit_entry3");
  List[2] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (button), "edit_entry4");
  List[3] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (button), "edit_entry5");
  List[4] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (button), "edit_entry6");
  List[5] = GTK_EDITABLE (Entry_Field);


/* Make the pretty list that Clist_set requires */

  data[0] = gtk_editable_get_chars(List[0],0,-1);
  data[1] = gtk_editable_get_chars(List[1],0,-1);
  data[2] = gtk_editable_get_chars(List[2],0,-1);
  data[3] = gtk_editable_get_chars(List[3],0,-1);
  data[4] = gtk_editable_get_chars(List[4],0,-1);
  data[5] = gtk_editable_get_chars(List[5],0,-1);
  data[6] = AddType;

      gtk_clist_remove(element_list,clist_row);
      gtk_clist_insert(element_list, clist_row,data);
      gtk_clist_select_row(element_list,clist_row,1);


  gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));


}


/*****************************************************************************

on_xxxxxxx_Button_toggled

These functions monitor the radio buttons, and set the type as it is selected
If anything besides other is selected, we disable text entry on the bit width
field, and automatically enter it for them.

I bet this could all be done with one function too.....


*****************************************************************************/


void
on_Binary_Button_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  GtkWidget *width_field;

  if (gtk_toggle_button_get_active    (togglebutton))
  {
    width_field = lookup_widget(GTK_WIDGET (togglebutton), "edit_entry6");
    gtk_entry_set_text(GTK_ENTRY(width_field),"1");
    AddType = "Binary";
    gtk_editable_set_editable       (GTK_EDITABLE(width_field),FALSE);
  }

}

void
on_Unsigned_Button_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  GtkWidget *width_field;
  if (gtk_toggle_button_get_active    (togglebutton))
  {
    AddType = "Unsigned";
    width_field = lookup_widget(GTK_WIDGET (togglebutton), "edit_entry6");
    gtk_entry_set_text(GTK_ENTRY(width_field),"32");
    gtk_editable_set_editable       (GTK_EDITABLE(width_field),FALSE);
  }
}

void
on_Integer_Button_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  GtkWidget *width_field;
  if (gtk_toggle_button_get_active    (togglebutton))
  {
    AddType = "Integer";
    width_field = lookup_widget(GTK_WIDGET (togglebutton), "edit_entry6");
    gtk_entry_set_text(GTK_ENTRY(width_field),"32");
    gtk_editable_set_editable       (GTK_EDITABLE(width_field),FALSE);
  }
}

void
on_Float_Button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  GtkWidget *width_field;
  if (gtk_toggle_button_get_active    (togglebutton))
  {
    AddType = "Float";
    width_field = lookup_widget(GTK_WIDGET (togglebutton), "edit_entry6");
    gtk_entry_set_text(GTK_ENTRY(width_field),"32");
    gtk_editable_set_editable       (GTK_EDITABLE(width_field),FALSE);
  }
}

void
on_Other_Button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  GtkWidget *width_field;
  if (gtk_toggle_button_get_active    (togglebutton))
  {
    AddType = "Other";
    width_field = lookup_widget(GTK_WIDGET (togglebutton), "edit_entry6");
    gtk_entry_set_text(GTK_ENTRY(width_field),"");
    gtk_editable_set_editable       (GTK_EDITABLE(width_field),TRUE);
  }
}


gboolean
on_update_element_keypress             (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
GtkButton *button;


  if ((event->keyval == GDK_Return) || (event->keyval == GDK_KP_Enter))
  {
    button = GTK_BUTTON(lookup_widget(GTK_WIDGET(widget),"update_element_button"));
    on_update_element_button_clicked(button,user_data);
  }

  return FALSE;
}

/*****************************************************************************

on_column_clicked

This function gets called if one of the columns is clicked on the CList
display.  When it is, the sort column is changed, and a re-sort is forced.
The column number that gets clicked is passed as an argument in user_data
so that we can use one function for all columns.
*****************************************************************************/


gboolean
on_column_clicked                      (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
int sort_column = (int)user_data;

  gtk_clist_set_sort_column(element_list,sort_column);
  gtk_clist_sort(element_list);
  return FALSE;
}


void
on_add_modules_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_notebook_set_page (Notebook,0);
  add_module = create_add_module();
  gtk_widget_show(add_module);

  module_edit = FALSE;

}


void
on_module_close_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}





/*****************************************************************************

on_module_add

The accept button was pressed in the module list editor window, and
module_edit was FALSE, so that means we are adding new modules to the list.
This function scans for duplications and adds the new entry to the list.
*****************************************************************************/
void
on_module_add                          (GtkButton       *button,
                                        gpointer         user_data)
{



  GtkWidget *entry_field, *entry1;
  GtkEditable *List[3];
  gint loop, success;
  gchar *data[3];
  int dup_found = 0;
  gchar *dup_check;
  gchar **ptr_dup_check = &dup_check;

/* suck all the data out of the entry window */

  entry_field = lookup_widget(GTK_WIDGET (button), "add_module_name");
  entry1 = entry_field;
  List[0] = GTK_EDITABLE (entry_field);
  entry_field = lookup_widget(GTK_WIDGET (button), "add_module_path");
  List[1] = GTK_EDITABLE (entry_field);
  entry_field = lookup_widget(GTK_WIDGET (button), "add_module_parameters");
  List[2] = GTK_EDITABLE (entry_field);

/* Make the pretty list that Clist_Append requires */

  data[0] = gtk_editable_get_chars(List[0],0,-1);
  data[1] = gtk_editable_get_chars(List[1],0,-1);
  data[2] = gtk_editable_get_chars(List[2],0,-1);


/*  Check the module list for duplicate names  */

  if (module_list != NULL)  /* Have we added any modules yet? */
   for (loop=0;loop <= module_row;loop++)
    if (gtk_clist_get_text(module_list,loop,0,ptr_dup_check))
      if (strcmp(dup_check,data[0]) == 0)
        {
          dup_found = 1;
	  reuse_error = create_reuse_error();
	  gtk_widget_show(reuse_error);
	}

/*  If no duplicates are found, add the entry to the correct list */

  if (!dup_found)
  {
          success = gtk_clist_append (module_list,data);
          module_row++;
  }

/* Put focus backto the ID entry field to speed data entry */

  gtk_widget_grab_focus (entry1);
  gtk_clist_sort(module_list);

}

/*****************************************************************************

module_editor_key_press

This simply watches what the use is typing in the module list editor window,
and if a ENTER or RETURN is encountered, simulate an 'accept' button click.

*****************************************************************************/


gboolean
module_editor_key_press                (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
GtkButton *button;


  if ((event->keyval == GDK_Return) || (event->keyval == GDK_KP_Enter))
  {
    button = GTK_BUTTON(lookup_widget(GTK_WIDGET(widget),"module_add"));
    on_module_accept_clicked(button,user_data);
  }

  return FALSE;

}


void on_module_change                  (GtkButton       *button,
                                        gpointer         user_data)
{

  GtkWidget *Entry_Field;
  GtkEditable *List[3];
  gchar *data[3];

/* suck all the data out of the edit window */

  Entry_Field = lookup_widget(GTK_WIDGET (button), "add_module_name");
  List[0] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (button), "add_module_path");
  List[1] = GTK_EDITABLE (Entry_Field);
  Entry_Field = lookup_widget(GTK_WIDGET (button), "add_module_parameters");
  List[2] = GTK_EDITABLE (Entry_Field);


/* Make the pretty list that Clist_set requires */

  data[0] = gtk_editable_get_chars(List[0],0,-1);
  data[1] = gtk_editable_get_chars(List[1],0,-1);
  data[2] = gtk_editable_get_chars(List[2],0,-1);


      gtk_clist_remove(module_list,clist_row);
      gtk_clist_insert(module_list, clist_row,data);
      gtk_clist_select_row(module_list,clist_row,1);


  gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(button)));



}



void
on_module_accept_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{

if (module_edit == FALSE)
  on_module_add (button,user_data);
else
  on_module_change(button,user_data);


}

void
on_module_browse_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)

{
  filewindow_function = MODULE_BROWSE;
  Fileselection1=create_fileselection1();
  gtk_widget_show (Fileselection1);

}


