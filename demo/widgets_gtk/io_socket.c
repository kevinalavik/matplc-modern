/*
 * (c) 2003 Stefan Staedtler, Heiko Patz
 *
 * Offered to the public under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 */

/* =====================================================================
 * $Id: io_socket.c,v 1.2 2003/12/01 17:29:04 jorozco Exp $
 *
 * $Log: io_socket.c,v $
 * Revision 1.2  2003/12/01 17:29:04  jorozco
 * Minor bug fix
 *
 * Revision 1.1  2003/11/20 01:36:34  jorozco
 * New widgets from Patz Heiko
 *
 *
 * ===================================================================== 
 * Funktionen:
 * ===================================================================== 
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

#include <string.h>
#include <signal.h>

#include <plc.h>
#include <logic/timer.h>

#include <gtk/gtk.h>

enum t_boolean {false=0,true=1};

/* globals */
GtkWidget	*widget=NULL;  	
GtkWidget	*text=NULL;
GtkWidget	*table;
GtkWidget	*vscrollbar;
GdkFont *font;

GTimer *timer;
int firstscan=1;
enum t_boolean radio1,radio2,radio3;
enum t_boolean check1,check2,check3;
f32 in1,in2,in3,in4;
i32 testp1,testp2,testp3;
	
// Exit-Programm flag (set by the sighandler)
volatile sig_atomic_t do_exit;

/* -------------------- sighandler -------------------- */
void sighandler (
	int signum
	)
{
	do_exit=1;
}


/* function declaration */
plc_pt_t get_pt(const char *pt_name)
{
	plc_pt_t pt_handle;

	pt_handle = plc_pt_by_name(pt_name);

	if (!pt_handle.valid) {
		printf("Could not get valid handle to %s.\n", pt_name);
		exit(1);
	}

	return pt_handle;
}

void pt_init(void){
	in1=plc_get_f32(get_pt("in1"));
	in2=plc_get_f32(get_pt("in2"));
	in3=plc_get_f32(get_pt("in3"));
	in4=plc_get_f32(get_pt("in4"));
	testp1=plc_get(get_pt("testp1"));
	testp2=plc_get(get_pt("testp2"));
	testp3=plc_get(get_pt("testp3"));
	radio1=plc_get(get_pt("radiobutton1"));
	radio2=plc_get(get_pt("radiobutton2"));
	radio3=plc_get(get_pt("radiobutton3"));
	check1=plc_get(get_pt("checkbutton1"));
	check2=plc_get(get_pt("checkbutton2"));
	check3=plc_get(get_pt("checkbutton3"));
}


/* This function is called when gtk is idle (free of events) */
gboolean 
scanloop(gpointer data)
{	
	gchar *buffer;
	guint widget_id;

	/* begin of scan */
	plc_scan_beg();
	plc_update();

	if(firstscan){
		/* initialize points*/
		pt_init();
		font = gdk_font_load ("-*-courier-medium-r-*-*-14-*-*-*-*-*-*-*");

		if( (widget_id=plc_get(get_pt("statusbar"))) != 0 ){
			//printf("IO_SOCKET: Window ID=%d\n",widget_id);fflush(stdout);
			
			/* plug to socket widget */
			widget = gtk_plug_new(widget_id);

			/* Create your own widget */

			/* create a new text window. */
			text=gtk_text_new(NULL,NULL);
			/* Editable off */
			gtk_text_set_editable(GTK_TEXT(text),FALSE);
			/* Wrap off */
			gtk_text_set_line_wrap(GTK_TEXT(text),TRUE);
			gtk_text_set_word_wrap(GTK_TEXT(text),FALSE);

			/*-- Create the vertical and horizontal scrollbars --*/
			vscrollbar = gtk_vscrollbar_new(GTK_TEXT (text) -> vadj);
			/* Horizontal scrollbar is not supported */
			/* hscrollbar = gtk_hscrollbar_new(GTK_TEXT (text) -> hadj); */

			/*-- Create the packing table --*/
			table = gtk_table_new(2, 2, FALSE);
			/*-- Add the items to the table --*/
			gtk_table_attach(GTK_TABLE(table), text, 0, 1, 0, 1, GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
			gtk_table_attach(GTK_TABLE(table), vscrollbar, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK , 0, 0);
			/* Horizontal scrollbar is not supported 
			gtk_table_attach(GTK_TABLE(table), hscrollbar, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_FILL, 0, 0);
			*/
			
			/* Add scrolled window to plugged Widget */
			gtk_container_add(GTK_CONTAINER(widget),table);

			/* Show all windows */
			gtk_widget_show_all(widget);

			firstscan=0;
		}else{
			printf("IO_SOCKET: Can`t get window-ID\n");fflush(stdout);
		}
	}	

	/* insert text into text widget, if state changed */
	if((plc_get_f32(get_pt("in1")))!=in1){
		in1=plc_get_f32(get_pt("in1"));
		buffer=g_strdup_printf("In-Field 1    : Time = %8.3fs New Value = %5.2f\n",
								g_timer_elapsed(timer,NULL),in1);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get_f32(get_pt("in2")))!=in2){
		in2=plc_get_f32(get_pt("in2"));
		buffer=g_strdup_printf("In-Field 2    : Time = %8.3fs New Value = %5.2f\n",
								g_timer_elapsed(timer,NULL),in2);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get_f32(get_pt("in3")))!=in3){
		in3=plc_get_f32(get_pt("in3"));
		buffer=g_strdup_printf("In-Field 3    : Time = %8.3fs New Value = %5.2f\n",
								g_timer_elapsed(timer,NULL),in3);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get_f32(get_pt("in4")))!=in4){
		in4=plc_get_f32(get_pt("in4"));
		buffer=g_strdup_printf("In-Field 4    : Time = %8.3fs New Value = %5.2f\n",
								g_timer_elapsed(timer,NULL),in4);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get(get_pt("testp1")))!=testp1){
		testp1=plc_get(get_pt("testp1"));
		buffer=g_strdup_printf("Option_Menu P1: Time = %8.3fs New Value = %d\n",
								g_timer_elapsed(timer,NULL),testp1);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get(get_pt("testp2")))!=testp2){
		testp2=plc_get(get_pt("testp2"));
		buffer=g_strdup_printf("Option_Menu P2: Time = %8.3fs New Value = %d\n",
								g_timer_elapsed(timer,NULL),testp2);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get(get_pt("testp3")))!=testp3){
		testp3=plc_get(get_pt("testp3"));
		buffer=g_strdup_printf("Option_Menu P3: Time = %8.3fs New Value = %d\n",
								g_timer_elapsed(timer,NULL),testp3);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get(get_pt("radiobutton1")))!=radio1){
		radio1=plc_get(get_pt("radiobutton1"));
		buffer=g_strdup_printf("RadioButton 1 : Time = %8.3fs New Value = %d\n",
								g_timer_elapsed(timer,NULL),radio1);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get(get_pt("radiobutton2")))!=radio2){
		radio2=plc_get(get_pt("radiobutton2"));
		buffer=g_strdup_printf("RadioButton 2 : Time = %8.3fs New Value = %d\n",
								g_timer_elapsed(timer,NULL),radio2);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get(get_pt("radiobutton3")))!=radio3){
		radio3=plc_get(get_pt("radiobutton3"));
		buffer=g_strdup_printf("RadioButton 3 : Time = %8.3fs New Value = %d\n",
								g_timer_elapsed(timer,NULL),radio3);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get(get_pt("checkbutton1")))!=check1){
		check1=plc_get(get_pt("checkbutton1"));
		buffer=g_strdup_printf("CheckButton 1 : Time = %8.3fs New Value = %d\n",
								g_timer_elapsed(timer,NULL),check1);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get(get_pt("checkbutton2")))!=check2){
		check2=plc_get(get_pt("checkbutton2"));
		buffer=g_strdup_printf("CheckButton 2 : Time = %8.3fs New Value = %d\n",
								g_timer_elapsed(timer,NULL),check2);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}
	if((plc_get(get_pt("checkbutton3")))!=check3){
		check3=plc_get(get_pt("checkbutton3"));
		buffer=g_strdup_printf("CheckButton 3 : Time = %8.3fs New Value = %d\n",
								g_timer_elapsed(timer,NULL),check3);
		gtk_text_insert(GTK_TEXT(text),font,NULL,NULL,buffer,strlen(buffer));
		g_free(buffer);
	}

	if(do_exit){
		if(GTK_IS_WIDGET (widget)){
			gtk_widget_destroy(widget);
			gtk_main_quit ();
		}
	}	

	/* end of scan */
	plc_update();
	plc_scan_end();

  return TRUE;
}

/*
========================================================================
	M A I N
========================================================================
*/

int main(int argc, char **argv) 
{
	
    //const char 		*author = "Patz/Staedtler";

	const char *module_name = "io_socket";

	/* printf("Initializing.\n"); */
	if (plc_init(module_name, argc, argv) < 0) {
		printf("Error initializing PLC\n");
		return -1;
	}
	
	if(signal(SIGINT, sighandler) == SIG_IGN){ 
		printf ("ERROR; signal handler cannot be initialized\n");
	}
	if(signal(SIGTERM, sighandler) == SIG_IGN){
		printf ("ERROR; signal handler cannot be initialized\n");
	}

	timer=g_timer_new();
	g_timer_start(timer);

	gtk_init (&argc, &argv);

	/* run the main loop */
	if(!g_idle_add((GSourceFunc)scanloop, NULL)){
		printf("Error cannot add scanloop function\n");
		return -1;
	}
	
	gtk_main ();
	
	return(0);
}
