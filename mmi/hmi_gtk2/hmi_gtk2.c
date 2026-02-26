/*
 * (c) 2001 Juan Carlos Orozco
 *
 * PLC point type conversions taken from /mmi/curses/vitrine.c from 
 * Jiri Baum and Mario de Sousa
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

/*
 * See hmi_gtk2.h for TODO list and how to add widgets tips.
 */


#include <string.h>
#include <fnmatch.h>
//#include <libgnomeui-2.0/gnome.h>
#include <gtk-2.0/gtk/gtk.h>
#include <glade/glade.h>

#include <plc.h>

#include "hmi_gtk2.h"


/* Function to get a plc point variable */
/* -------------------- get_pt -------------------- */
plc_pt_t get_pt(const char *pt_name)
{
  plc_pt_t pt_handle;

  pt_handle = plc_pt_by_name(pt_name);

  if (!pt_handle.valid) {
	plc_log_errmsg(0,"get_pt(): Could not get valid handle to %s.",pt_name);
	pt_handle = plc_pt_null();
  }

  return pt_handle;
}


/* Print digital value in a label widget */
void
label_digital_out(GtkWidget *w, gboolean state, char *on, char *off)
{
  char s[10];

  if(state)
  {
    if(on)
      sprintf(s, "%s", on);
    else
      sprintf(s, "X");
  }
  else
  {
    if(off)
      sprintf(s, "%s", off);
    else
      sprintf(s, "-");
  }
  gtk_label_set_text(GTK_LABEL(w), s);
} 

/* Load a GtkImage widget image */
void
load_gtk_image(GtkWidget *widget, char *name){
  gtk_image_set_from_file(GTK_IMAGE(widget), name);
}

/* Connect the different widget types */
void
label_widget(connection_t *connection){
  union {
    u32 u;
    f32 f;
  } tmp;
#define u32_tmp (tmp.u)
  char str_buff[BUFF_SIZE];
  char *sFormat=NULL;
  
  if(connection->data_type == bool_dt)
    label_digital_out(connection->widget, plc_get(connection->pt), 
		      connection->on, connection->off);
  else{
    u32_tmp = plc_get(connection->pt);
	if ( connection->LabelTab != NULL ) {
		sFormat=connection->LabelTab->formatstring;
	} else {
		sFormat=NULL;
	}
	if (sFormat==NULL) {
    	switch (connection->data_type) {
    	case f32_dt: snprintf(str_buff, BUFF_SIZE, "%f", *((f32 *)&u32_tmp)); break;
    	case  i8_dt: snprintf(str_buff, BUFF_SIZE, "%d", *(( i8 *)&u32_tmp)); break;
    	case  u8_dt: snprintf(str_buff, BUFF_SIZE, "%d", *(( u8 *)&u32_tmp)); break;
    	case i16_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((i16 *)&u32_tmp)); break;
    	case u16_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((u16 *)&u32_tmp)); break;
    	case i32_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((i32 *)&u32_tmp)); break;
    	case u32_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((u32 *)&u32_tmp)); break;
    	default : break;
    	}; /* switch() */
		
	} else {
    	switch (connection->data_type) {
    	case f32_dt: snprintf(str_buff, BUFF_SIZE, sFormat, *((f32 *)&u32_tmp)); break;
    	case  i8_dt: snprintf(str_buff, BUFF_SIZE, sFormat, *(( i8 *)&u32_tmp)); break;
    	case  u8_dt: snprintf(str_buff, BUFF_SIZE, sFormat, *(( u8 *)&u32_tmp)); break;
    	case i16_dt: snprintf(str_buff, BUFF_SIZE, sFormat, *((i16 *)&u32_tmp)); break;
    	case u16_dt: snprintf(str_buff, BUFF_SIZE, sFormat, *((u16 *)&u32_tmp)); break;
    	case i32_dt: snprintf(str_buff, BUFF_SIZE, sFormat, *((i32 *)&u32_tmp)); break;
    	case u32_dt: snprintf(str_buff, BUFF_SIZE, sFormat, *((u32 *)&u32_tmp)); break;
    	default : break;
    	}; 
	}
    gtk_label_set_text(GTK_LABEL(connection->widget), str_buff);
  }
#undef u32_tmp
}

/* This widget only takes float with 0.0 to 100.0 value range.
 * We could add the other types and convert them to 0.0 to 100.0 float values 
 *   using their min and max.
 *
 * 13.03.2003: Staedtler
 *	Range is now limited by the widget settings. This is more
 *	flexible and the widget does the job in a better way.
 */
void
progress_widget(connection_t *connection){
  f32 f32_tmp;

  f32_tmp = 0.0;
  switch (connection->data_type) {
  case f32_dt: 
    f32_tmp = plc_get_f32(connection->pt);
    break;
  case  i8_dt: break;
  case  u8_dt: break;
  case i16_dt: break;
  case u16_dt: break;
  case i32_dt: break;
  case u32_dt: break;
  default : break;
  }; /* switch() */
  gtk_progress_set_value(GTK_PROGRESS(connection->widget), f32_tmp);
}

void
gtk_image_widget(connection_t *connection){
  char *image_name;
  char *image;
  int  iPtab=0;
  union {
    u32 u;
    f32 f;
  } tmp;
#define u32_tmp (tmp.u)
 
  /* Check previous state before doing */
  if(plc_get(connection->pt) == connection->variable->value) return;

#ifdef DEBUG_gtk_image_widget
  printf("Widget=%s\n", gtk_widget_get_name(connection->widget));
#endif

  /* Save new value */
  connection->variable->value = plc_get(connection->pt);
  
  u32_tmp = plc_get(connection->pt);
  switch (connection->data_type) {
  case bool_dt: if (u32_tmp) iPtab=1; else iPtab=0; break;
  case f32_dt: iPtab=*(( f32 *)&u32_tmp); break;
  case  i8_dt: iPtab=*(( i8  *)&u32_tmp); break;
  case  u8_dt: iPtab=*(( u8  *)&u32_tmp); break;
  case i16_dt: iPtab=*(( i16 *)&u32_tmp); break;
  case u16_dt: iPtab=*(( u16 *)&u32_tmp); break;
  case i32_dt: iPtab=*(( i32 *)&u32_tmp); break;
  case u32_dt: iPtab=*(( u32 *)&u32_tmp); break;
  default : break;
  }; /* switch() */  

#ifdef DEBUG_gtk_image_widget
  printf("gtk_image_widget: iPtab=%d\n",iPtab);	  	
#endif
  
  image=NULL;
  if (connection->ImageTab != NULL) {
	if ((connection->ImageTab->nbImage)>0) {
  		iPtab=iPtab % connection->ImageTab->nbImage;
		image=connection->ImageTab->Image[iPtab];
	}
  } 
  else {
  	if (iPtab) image=connection->on;
	else image=connection->off;
  }
  if (image!=NULL) {
	  image_name = g_strdup_printf("%s.xpm",image);
#ifdef DEBUG_gtk_image_widget
	  printf("gtk_image_widget: image_name=%s\n",image_name);
#endif
	  load_gtk_image(connection->widget, image_name);
	  g_free(image_name);
  }
#undef u32_tmp
}


/* Data entry widgets */ 
void
toggle_button_widget(connection_t *connection){
  if(connection->data_type == bool_dt){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(connection->widget), 
				 (gboolean)connection->variable->value);
    plc_set(connection->pt, connection->variable->value); 
  }
}

void
button_widget(connection_t *connection){
  if(connection->data_type == bool_dt){
	  //MYDEBUG
	  //printf("Button Val %d\n", connection->variable->value);
    plc_set(connection->pt, connection->variable->value); 
  }
}

void 
entry_widget(connection_t *connection){
  /* Copy the Value from Widget to plc */
  plc_set(connection->pt, connection->variable->value);
}

/* Initialize the entry widget text with init value from plc
   This Function is called only once by parse connection,
   when window->first_scan=1 */
void 
entry_widget_init(connection_t *connection){
    union {
	  u32 u;
	  f32 f;
	} tmp;
#define u32_tmp (tmp.u)
	char str_buff[BUFF_SIZE];

	u32_tmp = connection->variable->value;
	switch (connection->data_type) {
	case f32_dt: snprintf(str_buff, BUFF_SIZE, "%f", *((f32 *)&u32_tmp)); break;
	case  i8_dt: snprintf(str_buff, BUFF_SIZE, "%d", *(( i8 *)&u32_tmp)); break;
	case  u8_dt: snprintf(str_buff, BUFF_SIZE, "%d", *(( u8 *)&u32_tmp)); break;
	case i16_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((i16 *)&u32_tmp)); break;
	case u16_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((u16 *)&u32_tmp)); break;
	case i32_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((i32 *)&u32_tmp)); break;
	case u32_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((u32 *)&u32_tmp)); break;
	case bool_dt: snprintf(str_buff, BUFF_SIZE, "%01d", *((i8 *)&u32_tmp)); break;
	default : break;
	}; /* switch() */
    gtk_entry_set_text(GTK_ENTRY(connection->widget), str_buff);
#undef u32_tmp
}


void
scale_widget(connection_t *connection){
	switch (connection->data_type) {
	case f32_dt:
	case u32_dt:
	case u16_dt:
	case u8_dt:
	case i32_dt:
	case i16_dt:
	case i8_dt:
		plc_set(connection->pt, connection->variable->value);
		break;
	default:
		break;
	}
}

/* Initialize the scale widget with init value from plc
   This Function is called only once by parse connection,
   when window->first_scan=1 */
void
scale_widget_init(connection_t *connection){
	u32 u32_tmp;
	u16 u16_tmp;
	u8   u8_tmp;
	i32 i32_tmp;
	i16 i16_tmp;
	i8   i8_tmp;
	f32 f32_tmp=0;
	GtkAdjustment *adjustment;

	adjustment=gtk_range_get_adjustment(GTK_RANGE(connection->widget));

	switch (connection->variable->type) {
	case f32_dt:
		f32_tmp =  *((f32 *)&(connection->variable->value));
		break;
	case u32_dt:
		u32_tmp =  *((u32 *)&(connection->variable->value));
		f32_tmp=u32_tmp;
		break;
	case u16_dt:
		u16_tmp =  *((u16 *)&(connection->variable->value));
		f32_tmp=u16_tmp;
		break;
	case u8_dt:
		u8_tmp =  *((u8 *)&(connection->variable->value));
		f32_tmp=u8_tmp;
		break;
	case i32_dt:
		i32_tmp =  *((i32 *)&(connection->variable->value));
		f32_tmp=i32_tmp;
		break;
	case i16_dt:
		i16_tmp =  *((i16 *)&(connection->variable->value));
		f32_tmp=i16_tmp;
		break;
	case i8_dt:
		i8_tmp =  *((i8 *)&(connection->variable->value));
		f32_tmp=i8_tmp;
		break;
	default:
		break;
	}

	adjustment->value = f32_tmp;
	gtk_adjustment_value_changed(adjustment);
}

void
spin_button_widget(connection_t *connection){
	switch (connection->data_type) {
	case f32_dt:
	case u32_dt:
	case u16_dt:
	case u8_dt:
	case i32_dt:
	case i16_dt:
	case i8_dt:
		plc_set(connection->pt, connection->variable->value);
		break;
	default:
		break;
	}
}

/* Initialize the spinbutton widget with init value from plc
   This Function is called only once by parse connection,
   when window->first_scan=1 */
void
spin_button_widget_init(connection_t *connection){
	u32 u32_tmp;
	u16 u16_tmp;
	u8   u8_tmp;
	i32 i32_tmp;
	i16 i16_tmp;
	i8   i8_tmp;
	f32 f32_tmp=0;


	switch (connection->variable->type) {
	case f32_dt:
		f32_tmp =  *((f32 *)&(connection->variable->value));
		break;
	case u32_dt:
		u32_tmp =  *((u32 *)&(connection->variable->value));
		f32_tmp=u32_tmp;
		break;
	case u16_dt:
		u16_tmp =  *((u16 *)&(connection->variable->value));
		f32_tmp=u16_tmp;
		break;
	case u8_dt:
		u8_tmp =  *((u8 *)&(connection->variable->value));
		f32_tmp=u8_tmp;
		break;
	case i32_dt:
		i32_tmp =  *((i32 *)&(connection->variable->value));
		f32_tmp=i32_tmp;
		break;
	case i16_dt:
		i16_tmp =  *((i16 *)&(connection->variable->value));
		f32_tmp=i16_tmp;
		break;
	case i8_dt:
		i8_tmp =  *((i8 *)&(connection->variable->value));
		f32_tmp=i8_tmp;
		break;
	default:
		break;
	}

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(connection->widget),f32_tmp);
}


//#ifdef GTKEXTRA
/*
	hmi_GtkPlotCanvas_check_sections()
	Check for necessary definitions in matplc.conf
*/
//void hmi_GtkPlotCanvas_check_sections(connection_t *connection){
//	char *widgetname;
//	int nbofrows=0;
//	int i=0,j;
//	int found;
//
//	const char * section_tab[] =
//	{									
//		"mode",	
//		"plotctrl",	
//		"channel",	
//		"-"
//	};

	
//#define DEBUG_CHECK_SECTIONS
//#undef DEBUG_CHECK_SECTIONS


//	#ifdef DEBUG_CHECK_SECTIONS
//		printf("hmi_GtkPlotCanvas_check_sections()\n");
//	#endif
//	widgetname=gtk_widget_get_name(connection->widget);

//	if( widgetname[0] == '_' ){
//		widgetname++;

//		while (section_tab[i][0] != '-'){
//			#ifdef DEBUG_CHECK_SECTIONS
//				printf("Searching for SECTION [%s] in matplc.conf ...",section_tab[i]);
//			#endif
//			nbofrows=conffile_get_table_rows(widgetname);
//			found=0;
//			for(j=0;j<nbofrows;j++){
//				if((g_strcasecmp(conffile_get_table(widgetname,j,0),section_tab[i]))==0){
//					found++;
//					break;
//				}
//			}
//			if(!found){
//				#ifdef DEBUG_CHECK_SECTIONS
//					printf(" not found\n");
//				#else
//					printf("ERROR: You have to set %s in matplc.conf\n",section_tab[i]);
//				#endif
//				exit(1);
//			}else{
//				#ifdef DEBUG_CHECK_SECTIONS
//					printf(" found\n");
//				#endif
//				i++;
//			}
//		}
//	}
//}

///*
//	hmi_GtkPlotCanvas_set_defaults()
//	set default values
//*/
//void hmi_GtkPlotCanvas_set_defaults(connection_t *connection){
//	char * character;
//	
//#define DEBUG_SET_DEFAULTS
//#undef DEBUG_SET_DEFAULTS
//
//	#ifdef DEBUG_SET_DEFAULTS
//		printf("hmi_GtkPlotCanvas_set_defaults()\n");
//	#endif
//	
//	connection->plot->plotstart=false;
//	connection->plot->plotcount=0;
//
//	connection->plot->conf_title		= notconfigured;
//	connection->plot->conf_widget		= notconfigured;
//	connection->plot->conf_axis			= notconfigured;
//	connection->plot->conf_color		= notconfigured;
//	connection->plot->conf_timer		= notconfigured;
//	connection->plot->conf_plotctrl		= notconfigured;
//	connection->plot->conf_plotx		= notconfigured;
//	connection->plot->conf_channel		= notconfigured;
//	connection->plot->conf_channelcolor	= notconfigured;
//	connection->plot->conf_mode			= notconfigured;
//	connection->plot->conf_symboltype	= notconfigured;
//	connection->plot->conf_symbolstyle	= notconfigured;
//	connection->plot->conf_linestyle	= notconfigured;
	//~ connection->plot->conf_linewidth	= notconfigured;
	//~ connection->plot->conf_pointconnect	= notconfigured;

	//~ character=(char *)"lightyellow";
	//~ connection->plot->plot_bg_color = (char *)g_malloc(strlen(character));
	//~ strcpy(connection->plot->plot_bg_color,character);

	//~ character=(char *)"white";
	//~ connection->plot->legend_bg_color = (char *)g_malloc(strlen(character));
	//~ strcpy(connection->plot->legend_bg_color,character);

	//~ /* Title */
	//~ character=(char *)"Title";
	//~ connection->plot->title = (char *)g_malloc(strlen(character));
	//~ strcpy(connection->plot->title,character);

	//~ /* x_title */
	//~ character=(char *)"x";
	//~ connection->plot->x_title = (char *)g_malloc(strlen(character));
	//~ strcpy(connection->plot->x_title,character);

	//~ /* y_title */
	//~ character=(char *)"y";
	//~ connection->plot->y_title = (char *)g_malloc(strlen(character));
	//~ strcpy(connection->plot->y_title,character);

	//~ /* size_plot_x */
	//~ connection->plot->size_plot_x = 0.80;

	//~ /* size_plot_y */
	//~ connection->plot->size_plot_y = 0.80;

	//~ /* pos_draw_x */
	//~ connection->plot->pos_draw_x = 0.10;

	//~ /* pos_draw_y */
	//~ connection->plot->pos_draw_y = 0.1;


	//~ /* xmin */
	//~ connection->plot->xmin = DEFAULT_XMIN;

	//~ /* xmax */
	//~ connection->plot->xmax = DEFAULT_XMAX;

	//~ /* ymin */
	//~ connection->plot->ymin = DEFAULT_YMIN;

	//~ /* ymax */
	//~ connection->plot->ymax = DEFAULT_YMAX;

	//~ /* sampletime */
	//~ connection->plot->sampletime = DEFAULT_SAMPLETIME;

//~ }

//~ /*
	//~ hmi_GtkPlotCanvas_check_dependence()
	//~ check, if all variables are defined
//~ */
//~ void hmi_GtkPlotCanvas_check_dependence(connection_t *connection){
	//~ int j;
	//~ channel_t *channelPtr;
	
//~ #define DEBUG_CHECK_DEPENDENCE
//~ #undef DEBUG_CHECK_DEPENDENCE

	//~ #ifdef DEBUG_CHECK_DEPENDENCE
		//~ printf("hmi_GtkPlotCanvas_check_dependence()\n");
	//~ #endif

	//~ if(connection->plot->plotmode==plotxymode){
		//~ if((connection->plot->conf_plotx)==notconfigured){
			//~ printf("ERROR: In mode <plotxy> you have to set plotx-points in matplc.conf\n");
			//~ exit(1);
		//~ }
	//~ }
	
	//~ if((connection->plot->conf_channelcolor)==notconfigured){
		//~ for(j=0;j<connection->plot->plotcount;j++){
			//~ channelPtr=g_list_nth_data(connection->plot->channellist,j);
			//~ channelPtr->ycolor = g_strdup("red");
		//~ }
	//~ }
//~ }

//~ /*
	//~ hmi_GtkPlotCanvas_read_matplc_conf()
	//~ Read configuration from matplc.conf
//~ */
//~ void hmi_GtkPlotCanvas_read_matplc_conf(connection_t *connection){
	//~ char *retval;
	//~ char *widgetname;
	//~ int i,j,k;
	//~ int nbofrows=0;
	//~ int nbofcols=0;
	//~ int ret;
	//~ f32 f32_tmp;
	//~ i32 i32_tmp;
	//~ channel_t *channel;
	//~ channel_t *channelPtr;

	//~ const char * symboltype_tab[] =
	//~ {									
		//~ "none",	"square", "circle",	"up_triangle", "down_triangle", "right_triangle", "left_triangle",	
		//~ "diamond", "plus", "cross",	"star",	"dot","impulse",	
		//~ "-"
	//~ };
	//~ const char * symbolstyle_tab[] =
	//~ {									
		//~ "empty", "filled", "opaque",
		//~ "-"
	//~ };
	//~ const char * linestyle_tab[] =
	//~ {									
		//~ "none",	"solid", "dotted",	"dashed", "dot_dash", "dot_dot_dash", "dot_dash_dash",	
		//~ "-"
	//~ };
	//~ const char * pointconnect_tab[] =
	//~ {									
		//~ "none",	"straight", "spline", "hv_step", "vh_step", "middle_step",	
		//~ "-"
	//~ };

//~ #define DEBUG_READ_MATPLC_CONF
//~ #undef DEBUG_READ_MATPLC_CONF


	//~ #ifdef DEBUG_READ_MATPLC_CONF
		//~ printf("hmi_GtkPlotCanvas_read_matplc_conf()\n");
	//~ #endif


	//~ widgetname=gtk_widget_get_name(connection->widget);

	//~ if( widgetname[0] == '_' ){
		//~ widgetname++;

		//~ nbofrows=conffile_get_table_rows(widgetname);
		//~ for(i=0;i<nbofrows;i++){
			//~ nbofcols=conffile_get_table_rowlen(widgetname, i);
			//~ /* detect number of curves */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"channel")==0){
				//~ connection->plot->plotcount=nbofcols-1;
				//~ for(j=0;j<connection->plot->plotcount;j++){
					//~ channel = (channel_t *)g_malloc(sizeof(channel_t));
					//~ channel->dataset=NULL;
					//~ channel->px=NULL;
					//~ channel->py=NULL;
					//~ channel->ylegend=NULL;
					//~ channel->ycolor=NULL;

					//~ channel->symboltype=GTK_PLOT_SYMBOL_STAR;
					//~ channel->symbolstyle=GTK_PLOT_SYMBOL_OPAQUE;
					//~ channel->symbolsize=DEFAULT_SYMBOLSIZE;
					//~ channel->linestyle=GTK_PLOT_LINE_SOLID;
					//~ channel->linewidth=DEFAULT_LINEWIDTH;
					//~ channel->pointconnect=GTK_PLOT_CONNECT_STRAIGHT;

					//~ connection->plot->channellist=g_list_append(connection->plot->channellist,channel);
				//~ }
			//~ }
		//~ }
		
		//~ for(i=0;i<nbofrows;i++){
			//~ nbofcols=conffile_get_table_rowlen(widgetname, i);
			//~ if(nbofcols==0){
				//~ printf("Warning: Empty Line detected\n");
				//~ continue;
			//~ }else{
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("Configuring SECTION [%s] (Number of Values=%d)\n",conffile_get_table(widgetname,i,0),nbofcols-1);
				//~ #endif
			//~ }

			//~ /* [SECTION: title] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"title")==0){
				//~ connection->plot->conf_title		= configured;
				//~ /* Title */
				//~ retval=conffile_get_table(widgetname,i,1);
				//~ connection->plot->title = (char *)g_malloc(strlen(retval));
				//~ strcpy(connection->plot->title,retval);
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->title = %s\n",connection->plot->title);		
				//~ #endif

				//~ /* x_title */
				//~ retval=conffile_get_table(widgetname,i,2);
				//~ connection->plot->x_title = (char *)g_malloc(strlen(retval));
				//~ strcpy(connection->plot->x_title,retval);
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->title = %s\n",connection->plot->x_title);		
				//~ #endif

				//~ /* y_title */
				//~ retval=conffile_get_table(widgetname,i,3);
				//~ connection->plot->y_title = (char *)g_malloc(strlen(retval));
				//~ strcpy(connection->plot->y_title,retval);
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->title = %s\n",connection->plot->y_title);		
				//~ #endif
			//~ }

			//~ /* [SECTION: timer] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"timer")==0){
				//~ connection->plot->conf_timer		= configured;
				//~ /* sampletime */
				//~ ret=conffile_get_table_f32(widgetname,i,1,&(f32_tmp), 1E-12, 1E12, DEFAULT_SAMPLETIME);
				//~ if(ret!=0){
					//~ connection->plot->sampletime = DEFAULT_SAMPLETIME;
				//~ }else{
					//~ connection->plot->sampletime = f32_tmp;
				//~ }
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->sampletime = %f\n",connection->plot->sampletime);		
				//~ #endif
			//~ }

			//~ /* [SECTION: plotctrl] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"plotctrl")==0){
				//~ connection->plot->conf_plotctrl	= configured;
				//~ /* startplot */
				//~ retval=conffile_get_table(widgetname,i,1);
				//~ if(retval==NULL){
					//~ printf("ERROR:Startplot not set in plotctrl (matplc.conf)\n");
					//~ exit(1);
				//~ }
				//~ connection->plot->startplot=get_pt(retval);
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->startplot made\n");		
				//~ #endif

				//~ /* stopplot */
				//~ retval=conffile_get_table(widgetname,i,2);
				//~ if(retval==NULL){
					//~ printf("ERROR:Stopplot not set in plotctrl (matplc.conf)\n");
					//~ exit(1);
				//~ }
				//~ connection->plot->stopplot=get_pt(retval);
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->stopplot made\n");		
				//~ #endif

				//~ /* resetplot */
				//~ retval=conffile_get_table(widgetname,i,3);
				//~ if(retval==NULL){
					//~ printf("ERROR:Resetplot not set in plotctrl (matplc.conf)\n");
					//~ exit(1);
				//~ }
				//~ connection->plot->resetplot=get_pt(retval);
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->resetplot made\n");		
				//~ #endif
			//~ }

			//~ /* [SECTION: channel] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"channel")==0){
				//~ connection->plot->conf_channel		= configured;
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tNumber of Curves: %d\n",connection->plot->plotcount);
				//~ #endif
				//~ /* yval */
				//~ for(j=0;j<connection->plot->plotcount;j++){
					//~ channelPtr=g_list_nth_data(connection->plot->channellist,j);
					//~ retval=conffile_get_table(widgetname,i,j+1);
					//~ channelPtr->ylegend = g_strdup(retval);
					//~ channelPtr->yval=get_pt(retval);
					//~ #ifdef DEBUG_READ_MATPLC_CONF
						//~ printf("\tconnection->plot->yval[%d] made\n",j+1);		
						//~ printf("\tconnection->plot->ylegend[%d] = %s\n",j+1,channelPtr->ylegend);  		
					//~ #endif
				//~ }
			//~ }

			//~ /* [SECTION: mode] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"mode")==0){
				//~ connection->plot->conf_mode		= configured;
				//~ /* plotmode */
				//~ retval=conffile_get_table(widgetname,i,1);
				//~ if(g_strcasecmp(retval,"timer")==0){
					//~ connection->plot->plotmode = timermode;
					//~ #ifdef DEBUG_READ_MATPLC_CONF
						//~ printf("\tconnection->plot->mode = timer\n");		
					//~ #endif
				//~ }else if (g_strcasecmp(retval,"plotxy")==0) {
					//~ connection->plot->plotmode = plotxymode;
					//~ #ifdef DEBUG_READ_MATPLC_CONF
						//~ printf("\tconnection->plot->mode = plotxy\n");		
					//~ #endif
				//~ }else{
					//~ printf("ERROR: PlotMode not set\n");		
					//~ exit(1);
				//~ }
			//~ }

			//~ /* [SECTION: plotx] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"plotx")==0){
				//~ connection->plot->conf_plotx		= configured;
				//~ /* xval */
				//~ nbofcols=conffile_get_table_rowlen(widgetname, i);
				//~ for(j=0;j<connection->plot->plotcount;j++){
					//~ channelPtr=g_list_nth_data(connection->plot->channellist,j);
					//~ if( j < (nbofcols-1) ){
						//~ channelPtr->xval=get_pt(conffile_get_table(widgetname,i,j+1));
					//~ }else{
						//~ channelPtr->xval=get_pt(conffile_get_table(widgetname,i,1));
					//~ }
					//~ #ifdef DEBUG_READ_MATPLC_CONF
						//~ printf("\tconnection->plot->xval[%d] made\n",j+1);		
					//~ #endif
				//~ }
			//~ }

			//~ /* [SECTION: channelcolor] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"channelcolor")==0){
				//~ connection->plot->conf_channelcolor= configured;
				//~ /* ycolor */
				//~ for(j=0;j<connection->plot->plotcount;j++){
					//~ channelPtr=g_list_nth_data(connection->plot->channellist,j);
					//~ retval=conffile_get_table(widgetname,i,j+1);
					//~ if(retval==NULL) retval=(char *)"red";
					//~ channelPtr->ycolor = g_strdup(retval);
					//~ #ifdef DEBUG_READ_MATPLC_CONF
						//~ printf("\tconnection->plot->ycolor[%d] = %s\n",j+1,channelPtr->ycolor);	
					//~ #endif
				//~ }
			//~ }

			//~ /* [SECTION: axis] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"axis")==0){
				//~ connection->plot->conf_axis		= configured;
				//~ /* xmin */
				//~ ret=conffile_get_table_f32(widgetname,i,1,&(f32_tmp), -1E12, 1E12, DEFAULT_XMIN);
				//~ if(ret!=0){
					//~ connection->plot->xmin = DEFAULT_XMIN;
				//~ }else{
					//~ connection->plot->xmin = f32_tmp;
				//~ }
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->xmin = %f\n",connection->plot->xmin);		
				//~ #endif

				//~ /* xmax */
				//~ ret=conffile_get_table_f32(widgetname,i,2,&(f32_tmp), -1E12, 1E12, DEFAULT_XMAX);
				//~ if(ret!=0){
					//~ connection->plot->xmax = DEFAULT_XMAX;
				//~ }else{
					//~ connection->plot->xmax = f32_tmp;
				//~ }
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->xmax = %f\n",connection->plot->xmax);		
				//~ #endif

				//~ /* ymin */
				//~ ret=conffile_get_table_f32(widgetname,i,3,&(f32_tmp), -1E12, 1E12, DEFAULT_YMIN);
				//~ if(ret!=0){
					//~ connection->plot->ymin = DEFAULT_YMIN;
				//~ }else{
					//~ connection->plot->ymin = f32_tmp;
				//~ }
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->ymin = %f\n",connection->plot->ymin);		
				//~ #endif

				//~ /* ymax */
				//~ ret=conffile_get_table_f32(widgetname,i,4,&(f32_tmp), -1E12, 1E12, DEFAULT_YMAX);
				//~ if(ret!=0){
					//~ connection->plot->ymax = DEFAULT_YMAX;
				//~ }else{
					//~ connection->plot->ymax = f32_tmp;
				//~ }
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->ymax = %f\n",connection->plot->ymax);		
				//~ #endif
			//~ }
			
			//~ /* [SECTION: color] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"color")==0){
				//~ connection->plot->conf_color		= configured;
				//~ /* plot_bg_color */
				//~ retval=conffile_get_table(widgetname,i,2);
				//~ if(retval!=NULL) {
					//~ connection->plot->plot_bg_color = (char *)g_malloc(strlen(retval));
					//~ strcpy(connection->plot->plot_bg_color,retval);
				//~ }
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->plot_bg_color = %s\n",connection->plot->plot_bg_color);		
				//~ #endif

				//~ /* legend_bg_color */
				//~ retval=conffile_get_table(widgetname,i,3);
				//~ if(retval!=NULL) {
					//~ connection->plot->legend_bg_color = (char *)g_malloc(strlen(retval));
					//~ strcpy(connection->plot->legend_bg_color,retval);
				//~ }
				//~ #ifdef DEBUG_READ_MATPLC_CONF
					//~ printf("\tconnection->plot->legend_bg_color = %s\n",connection->plot->legend_bg_color);		
				//~ #endif

			//~ }

			//~ /* [SECTION: symboltype] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"symboltype")==0){
				//~ connection->plot->conf_symboltype= configured;
				//~ for(j=0;j<connection->plot->plotcount;j++){
					//~ channelPtr=g_list_nth_data(connection->plot->channellist,j);
					//~ channelPtr->symboltype  = GTK_PLOT_SYMBOL_STAR;
					//~ retval=conffile_get_table(widgetname,i,j+1);
					//~ if(retval!=NULL){
						//~ k=0;
						//~ while( symboltype_tab[k][0] != '-' ){
							//~ if((g_strcasecmp(retval,symboltype_tab[k]))==0){
								//~ channelPtr->symboltype=k;
								//~ #ifdef DEBUG_READ_MATPLC_CONF
									//~ printf("\tconnection->plot->symboltype[%d] = %d\n",j+1,channelPtr->symboltype);		
								//~ #endif
								//~ break;
							//~ }
							//~ k++;
						//~ }
					//~ }
				//~ }
			//~ }

			//~ /* [SECTION: symbolstyle] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"symbolstyle")==0){
				//~ connection->plot->conf_symbolstyle= configured;
				//~ for(j=0;j<connection->plot->plotcount;j++){
					//~ channelPtr=g_list_nth_data(connection->plot->channellist,j);
					//~ channelPtr->symbolstyle  = GTK_PLOT_SYMBOL_OPAQUE;
					//~ retval=conffile_get_table(widgetname,i,j+1);
					//~ if(retval!=NULL){
						//~ k=0;
						//~ while( symbolstyle_tab[k][0] != '-' ){
							//~ if((g_strcasecmp(retval,symbolstyle_tab[k]))==0){
								//~ channelPtr->symbolstyle=k;
								//~ #ifdef DEBUG_READ_MATPLC_CONF
									//~ printf("\tconnection->plot->symbolstyle[%d] = %d\n",j+1,channelPtr->symbolstyle);		
								//~ #endif
								//~ break;
							//~ }
							//~ k++;
						//~ }
					//~ }
				//~ }
			//~ }

			//~ /* [SECTION: linestyle] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"linestyle")==0){
				//~ connection->plot->conf_linestyle= configured;
				//~ for(j=0;j<connection->plot->plotcount;j++){
					//~ channelPtr=g_list_nth_data(connection->plot->channellist,j);
					//~ channelPtr->linestyle  = GTK_PLOT_LINE_SOLID;
					//~ retval=conffile_get_table(widgetname,i,j+1);
					//~ if(retval!=NULL){
						//~ k=0;
						//~ while( linestyle_tab[k][0] != '-' ){
							//~ if((g_strcasecmp(retval,linestyle_tab[k]))==0){
								//~ channelPtr->linestyle=k;
								//~ #ifdef DEBUG_READ_MATPLC_CONF
									//~ printf("\tconnection->plot->linestyle[%d] = %d\n",j+1,channelPtr->linestyle);		
								//~ #endif
								//~ break;
							//~ }
							//~ k++;
						//~ }
					//~ }
				//~ }
			//~ }

			//~ /* [SECTION: pointconnect] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"pointconnect")==0){
				//~ connection->plot->conf_pointconnect= configured;
				//~ for(j=0;j<connection->plot->plotcount;j++){
					//~ channelPtr=g_list_nth_data(connection->plot->channellist,j);
					//~ channelPtr->pointconnect  = GTK_PLOT_CONNECT_STRAIGHT;
					//~ retval=conffile_get_table(widgetname,i,j+1);
					//~ if(retval!=NULL){
						//~ k=0;
						//~ while( pointconnect_tab[k][0] != '-' ){
							//~ if((g_strcasecmp(retval,pointconnect_tab[k]))==0){
								//~ channelPtr->pointconnect=k;
								//~ #ifdef DEBUG_READ_MATPLC_CONF
									//~ printf("\tconnection->plot->pointconnect[%d] = %d\n",j+1,channelPtr->pointconnect);		
								//~ #endif
								//~ break;
							//~ }
							//~ k++;
						//~ }
					//~ }
				//~ }
			//~ }

			//~ /* [SECTION: linewidth] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"linewidth")==0){
				//~ connection->plot->conf_linewidth= configured;
				//~ for(j=0;j<connection->plot->plotcount;j++){
					//~ channelPtr=g_list_nth_data(connection->plot->channellist,j);
					//~ ret=conffile_get_table_i32(widgetname,i,j+1,&(i32_tmp), 1, 100, DEFAULT_LINEWIDTH);
					//~ if(ret!=0){
						//~ channelPtr->linewidth = DEFAULT_LINEWIDTH;
					//~ }else{
						//~ channelPtr->linewidth = i32_tmp;
					//~ }
					//~ #ifdef DEBUG_READ_MATPLC_CONF
						//~ printf("\tconnection->plot->linewidth[%d] = %d\n",j+1,channelPtr->linewidth);		
					//~ #endif
				//~ }
			//~ }

			//~ /* [SECTION: symbolsize] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"symbolsize")==0){
				//~ connection->plot->conf_symbolsize= configured;
				//~ for(j=0;j<connection->plot->plotcount;j++){
					//~ channelPtr=g_list_nth_data(connection->plot->channellist,j);
					//~ ret=conffile_get_table_i32(widgetname,i,j+1,&(i32_tmp), 1, 100, DEFAULT_SYMBOLSIZE);
					//~ if(ret!=0){
						//~ channelPtr->symbolsize = DEFAULT_SYMBOLSIZE;
					//~ }else{
						//~ channelPtr->symbolsize = i32_tmp;
					//~ }
					//~ #ifdef DEBUG_READ_MATPLC_CONF
						//~ printf("\tconnection->plot->symbolsize[%d] = %d\n",j+1,channelPtr->symbolsize);		
					//~ #endif
				//~ }
			//~ }
		//~ }	
	//~ }
//~ }


//~ /*
	//~ hmi_GtkPlotCanvas_widget()
	//~ Update function for GtkPlotCanvas
	//~ Two modes: configuration through matplc.conf
		//~ (1) timermode
		//~ (2) plotxymode
//~ */
//~ void
//~ hmi_GtkPlotCanvas_widget(connection_t *connection){
	//~ GtkPlot *gtkplot=NULL;
	//~ gdouble x=0, y=0;
	//~ gdouble xmin, xmax;
	//~ gdouble xnewmin, xnewmax;
	//~ int i,j;
	//~ int newpoints=0;
	//~ int startvalue=0;
	//~ int stopvalue=0;
	//~ gdouble *oldpxPtr;
	//~ gdouble *oldpyPtr;
	//~ t_boolean newpoint_timer=false;
	//~ t_boolean newpoint_xy=false;
	//~ channel_t *channelPtr;


//~ #define DEBUG_GTKPLOTCANVAS_WIDGET
//~ #undef DEBUG_GTKPLOTCANVAS_WIDGET


	//~ /*	Detect plotstart
		//~ in timer-mode -> start timer
	//~ */
	//~ if((connection->plot->plotstart==false)&&(plc_get(connection->plot->startplot))){
		//~ connection->plot->plotstart=TRUE;
		//~ if(connection->plot->plotmode==timermode) plc_timer_enable(connection->plot->timer);
		//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET
			//~ printf("Plotstart=yes\n");
		//~ #endif
	//~ }
	
	//~ /*	Detect plotstop
		//~ in timer-mode -> stop timer
	//~ */
	//~ if((connection->plot->plotstart==TRUE)&&(plc_get(connection->plot->stopplot))){
		//~ if(connection->plot->plotmode==timermode){
			//~ plc_timer_disable(connection->plot->timer);
		//~ }
		//~ connection->plot->plotstart=false;
		//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET
			//~ printf("Plotstart=false\n");
		//~ #endif
	//~ }
	
	//~ /*	Detect resetplot
		//~ --> clear points, set new range, etc.
		//~ in timer-mode -> restart timer
	//~ */
	//~ if(plc_get(connection->plot->resetplot)){
		//~ if(connection->plot->plotmode==timermode){
			//~ plc_timer_clear(connection->plot->timer);
			//~ connection->plot->timer_wait=(connection->plot->sampletime);
			//~ plc_timer_disable(connection->plot->timer);
		//~ }
		//~ connection->plot->plotstart=false;
		//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET
			//~ printf("Resetting Plot: Plotstart=false\n");
		//~ #endif

		//~ /* Clear all points */
		//~ for(i=0;i<connection->plot->plotcount;i++){
			//~ channelPtr=g_list_nth_data(connection->plot->channellist,i);
				
		//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET
			//~ printf("Clearing Points: Plotcount=%d\n",connection->plot->plotcount);
		//~ #endif
			
			//~ channelPtr->px = (gdouble *)g_realloc(channelPtr->px, (1)*sizeof(gdouble));
			//~ channelPtr->py = (gdouble *)g_realloc(channelPtr->py, (1)*sizeof(gdouble));

			//~ /* void gtk_plot_data_set_x (GtkPlotData *data,gdouble *x); */
			//~ gtk_plot_data_set_x(channelPtr->dataset, channelPtr->px); 

			//~ /* void gtk_plot_data_set_y (GtkPlotData *data,gdouble *y); */
			//~ gtk_plot_data_set_y(channelPtr->dataset, channelPtr->py); 

			//~ /* void gtk_plot_data_set_numpoints (GtkPlotData *data,gint num_points);  */
			//~ gtk_plot_data_set_numpoints(channelPtr->dataset, 0); 
		//~ }

		//~ gtkplot = GTK_PLOT(connection->plot->plotfield);
		//~ gtk_plot_set_xrange(gtkplot, connection->plot->xmin , connection->plot->xmax);
		//~ gtk_plot_canvas_paint(GTK_PLOT_CANVAS(connection->widget));
		//~ gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(connection->widget));
	//~ }
	

	//~ /* Timer-Mode */
	//~ if ( (connection->plot->plotmode==timermode) && (plc_timer_done(connection->plot->timer, connection->plot->timer_wait)) ){
		//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET
			//~ printf("Timer-Mode: Getting new Point\n");
		//~ #endif
		//~ newpoint_timer=true;
	//~ }else{
		//~ newpoint_timer=false;
	//~ }

	//~ /*	xyplot-Mode */
	//~ if (connection->plot->plotmode==plotxymode){
		//~ newpoint_xy=false;
		//~ for(i=0;i<connection->plot->plotcount;i++){
			//~ channelPtr=g_list_nth_data(connection->plot->channellist,i);

			//~ if (connection->plot->plotstart==true){
				//~ x=plc_get_f32(channelPtr->xval);
				//~ y=plc_get_f32(channelPtr->yval);

				//~ if(channelPtr->dataset->num_points>0){
					//~ if( (fabs( (channelPtr->px[(channelPtr->dataset->num_points)-1]) - x )) > 1.0E-8 ){
						//~ newpoint_xy=true;
						//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET
							//~ printf("XY-Mode: Getting new x Point for channel %s\n",channelPtr->ylegend);
						//~ #endif
						//~ break;
					//~ }else if( (fabs( (channelPtr->py[(channelPtr->dataset->num_points)-1]) - y )) > 1.0E-8 ){
						//~ newpoint_xy=true;
						//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET
							//~ printf("XY-Mode: Getting new y Point for channel %s\n",channelPtr->ylegend);
						//~ #endif
						//~ break;
					//~ }
				//~ }else{
					//~ newpoint_xy=true;
					//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET
						//~ printf("XY-Mode: Getting first Point for channel %s\n",channelPtr->ylegend);
					//~ #endif
					//~ break;
				//~ }
			//~ }
		//~ }
	//~ }


	//~ if( newpoint_timer || newpoint_xy ) {
		//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET
			//~ printf("Plotting new Point\n");
		//~ #endif
		//~ /* #define GTK_PLOT(obj)        GTK_CHECK_CAST (obj, gtk_plot_get_type (), GtkPlot)
		   //~ #define GTK_CHECK_CAST(tobj, cast_type, cast) ((cast*) gtk_type_check_object_cast ((GtkTypeObject*) (tobj), (cast_type))) */
		//~ gtkplot = GTK_PLOT(connection->plot->plotfield);


		//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET
			//~ printf("Plotcount = %d\n",connection->plot->plotcount);
		//~ #endif

		//~ /* void gtk_plot_get_xrange (GtkPlot *plot,gdouble *xmin, gdouble *xmax); */
		//~ gtk_plot_get_xrange(gtkplot, &xmin , &xmax);

		//~ xnewmin=xmin;
		//~ xnewmax=xmax;
		//~ for(i=0;i<connection->plot->plotcount;i++){
			//~ channelPtr=g_list_nth_data(connection->plot->channellist,i);
			
			//~ channelPtr->px = (gdouble *)g_realloc(channelPtr->px, (channelPtr->dataset->num_points+1)*sizeof(gdouble));
			//~ channelPtr->py = (gdouble *)g_realloc(channelPtr->py, (channelPtr->dataset->num_points+1)*sizeof(gdouble));

			//~ y=plc_get_f32(channelPtr->yval);
			
			//~ if(newpoint_timer){
				//~ x = connection->plot->timer_wait;
				//~ xnewmin=x;
				//~ xnewmax=x;
			//~ }else if(newpoint_xy){
				//~ x=plc_get_f32(channelPtr->xval);
				//~ if(x>xnewmax) xnewmax=x;
				//~ if(x<xnewmin) xnewmin=x;
				
			//~ }

			//~ channelPtr->px[channelPtr->dataset->num_points] = x;
			//~ channelPtr->py[channelPtr->dataset->num_points] = y;

			//~ /* void gtk_plot_data_set_x (GtkPlotData *data,gdouble *x); */
			//~ gtk_plot_data_set_x(channelPtr->dataset, channelPtr->px); 

			//~ /* void gtk_plot_data_set_y (GtkPlotData *data,gdouble *y); */
			//~ gtk_plot_data_set_y(channelPtr->dataset, channelPtr->py); 

			//~ /* void gtk_plot_data_set_numpoints (GtkPlotData *data,gint num_points);  */
			//~ gtk_plot_data_set_numpoints(channelPtr->dataset, channelPtr->dataset->num_points+1); 


		//~ }


		//~ if( (xnewmax > xmax) || (xnewmin < xmin) ){

			//~ channelPtr=g_list_nth_data(connection->plot->channellist,0);
			//~ startvalue=0;
			//~ stopvalue=(channelPtr->dataset->num_points);
			//~ newpoints=(channelPtr->dataset->num_points);

			//~ if( xnewmax > xmax ){
				//~ xmax=xnewmax+0.9*(connection->plot->xmax-connection->plot->xmin);
				//~ xmin=xnewmax-0.1*(connection->plot->xmax-connection->plot->xmin);

				//~ /* get startvalue for next plot */
				//~ for(j=startvalue;j<stopvalue;j++){
					//~ if( (channelPtr->px[j]) >= xmin ){
						//~ newpoints=(channelPtr->dataset->num_points)-j;
						//~ startvalue=j;
						//~ break;
					//~ }
				//~ }

			//~ }else{
				//~ xmax=xnewmin+0.1*(connection->plot->xmax-connection->plot->xmin);
				//~ xmin=xnewmin-0.9*(connection->plot->xmax-connection->plot->xmin);;

				//~ /* get startvalue for next plot */
				//~ for(j=startvalue;j<stopvalue;j++){
					//~ if( (channelPtr->px[j]) <= xmax ){
						//~ newpoints=(channelPtr->dataset->num_points)-j;
						//~ startvalue=j;
						//~ break;
					//~ }
				//~ }

			//~ }

			//~ for(i=0;i<connection->plot->plotcount;i++){

				//~ channelPtr=g_list_nth_data(connection->plot->channellist,i);

				//~ oldpxPtr=channelPtr->px;
				//~ channelPtr->px = (gdouble *) g_memdup(&(channelPtr->px[startvalue]), newpoints*sizeof(gdouble));
				//~ g_free(oldpxPtr);
				//~ oldpxPtr=NULL;
				
				//~ oldpyPtr=channelPtr->py;
				//~ channelPtr->py = (gdouble *) g_memdup(&(channelPtr->py[startvalue]), newpoints*sizeof(gdouble));
				//~ g_free(oldpyPtr);
				//~ oldpyPtr=NULL;

				//~ /* void gtk_plot_data_set_x (GtkPlotData *data,gdouble *x); */
				//~ gtk_plot_data_set_x(channelPtr->dataset, channelPtr->px); 

				//~ /* void gtk_plot_data_set_y (GtkPlotData *data,gdouble *y); */
				//~ gtk_plot_data_set_y(channelPtr->dataset, channelPtr->py); 

				//~ gtk_plot_data_set_numpoints(channelPtr->dataset, newpoints); 
			//~ }

			//~ /* void gtk_plot_set_range (GtkPlot *plot,gdouble xmin, gdouble xmax,gdouble ymin, gdouble ymax);  */
			//~ gtk_plot_set_xrange(gtkplot, xmin , xmax);

			//~ /* void gtk_plot_canvas_paint (GtkPlotCanvas *canvas); 
				//~ Paint the canvas(update the changes) */
			//~ gtk_plot_canvas_paint(GTK_PLOT_CANVAS(connection->widget));

			//~ /* void gtk_plot_canvas_refresh (GtkPlotCanvas *canvas); 
				//~ Refresh the canvas */
			//~ gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(connection->widget));
		//~ }else {

			//~ for(i=0;i<connection->plot->plotcount;i++){
				//~ channelPtr=g_list_nth_data(connection->plot->channellist,i);
				//~ /* void gtk_plot_data_draw_points (GtkPlotData *data,gint n); */
				//~ gtk_plot_data_draw_points(channelPtr->dataset, 1);
			//~ }
			
			//~ /* void gtk_plot_refresh (GtkPlot *plot,GdkRectangle *area); 
				//~ ------------------------------------------------------------------------------------------ 
				//~ Refresh a certain area of the plot->
				//~ plot -> a GtkPlot widget
				//~ area -> area to be refreshed */
			//~ gtk_plot_refresh(gtkplot, NULL);
		//~ }

		//~ if(newpoint_timer) connection->plot->timer_wait+=(connection->plot->sampletime);
	//~ }
//~ }


//~ /*
	//~ hmi_GtkPlotCanvas_widget_init()
	//~ Initialize widget
//~ */
//~ void
//~ hmi_GtkPlotCanvas_widget_init(connection_t *connection){
	//~ GdkColor color;
	//~ channel_t *channelPtr;
	//~ int i;
	
//~ #define DEBUG_GTKPLOTCANVAS_WIDGET_INIT
//~ #undef DEBUG_GTKPLOTCANVAS_WIDGET_INIT

	//~ #ifdef DEBUG_GTKPLOTCANVAS_WIDGET_INIT
		//~ printf("hmi_GtkPlotCanvas_widget_init()\n");
	//~ #endif

	//~ hmi_GtkPlotCanvas_set_defaults(connection);
	//~ hmi_GtkPlotCanvas_check_sections(connection);
	//~ hmi_GtkPlotCanvas_read_matplc_conf(connection);
	//~ hmi_GtkPlotCanvas_check_dependence(connection);

	//~ /* Init timer */
	//~ plc_timer_start(connection->plot->timer);
	//~ plc_timer_disable(connection->plot->timer);

	//~ /* Create new plotfield */
	//~ connection->plot->plotfield = (GtkWidget *)g_malloc(sizeof(GtkWidget *));

	//~ /* GtkWidget* gtk_plot_new_with_size (GdkDrawable *drawable,gdouble width, gdouble height); 
		//~ ------------------------------------------------------------------------------------------ 
		//~ Create a new GtkPlot widget with specified dimensions
		//~ drawable -> A drawable Gdk widget
		//~ width,height -> Width,height of the new GtkPlot widget in percentage of widget size
		//~ Returns a new GtkPlot widget */
	//~ connection->plot->plotfield = gtk_plot_new_with_size(NULL,connection->plot->size_plot_x,connection->plot->size_plot_y);

	//~ gdk_color_parse(connection->plot->plot_bg_color, &color);
	//~ gdk_color_alloc(gtk_widget_get_colormap(connection->plot->plotfield), &color);

	//~ /* void gtk_plot_set_background (GtkPlot *plot,const GdkColor *background); */
	//~ gtk_plot_set_background(GTK_PLOT(connection->plot->plotfield), &color);

	//~ gdk_color_parse(connection->plot->legend_bg_color, &color);
	//~ gdk_color_alloc(gtk_widget_get_colormap(connection->widget), &color);
	//~ /* void gtk_plot_legends_set_attributes (GtkPlot *plot,const gchar *font,gint height,
											 //~ const GdkColor *foreground,const GdkColor *background); 
	//~ font -> font name; height -> height of the font; foreground,background -> colors of the text */
	//~ gtk_plot_legends_set_attributes(GTK_PLOT(connection->plot->plotfield),NULL, 0,NULL,&color);

	//~ /* void gtk_plot_set_range (GtkPlot *plot,gdouble xmin, gdouble xmax,gdouble ymin, gdouble ymax);  */
	//~ gtk_plot_set_range(GTK_PLOT(connection->plot->plotfield), connection->plot->xmin ,connection->plot->xmax, connection->plot->ymin, connection->plot->ymax);

	//~ /* void gtk_plot_axis_set_ticks (GtkPlot *plot,GtkPlotOrientation orientation,gdouble major_step,gint nminor);
		//~ ------------------------------------------------------------------------------------------ 
		//~ typedef enum{GTK_PLOT_AXIS_X,GTK_PLOT_AXIS_Y,GTK_PLOT_AXIS_Z} GtkPlotOrientation; */
	//~ gtk_plot_axis_set_ticks(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_X,
		//~ (connection->plot->xmax-connection->plot->xmin)*AXIS_TICKS_MAJOR,
		//~ AXIS_TICKS_MINOR);
	//~ gtk_plot_axis_set_ticks(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_Y,
		//~ (connection->plot->ymax-connection->plot->ymin)*AXIS_TICKS_MAJOR,
		//~ AXIS_TICKS_MINOR);

	//~ /* void gtk_plot_axis_set_labels_numbers (GtkPlot *plot,GtkPlotAxisPos axis,gint style,gint precision);  
		//~ ------------------------------------------------------------------------------------------ 
		//~ typedef enum{GTK_PLOT_AXIS_LEFT,GTK_PLOT_AXIS_RIGHT,GTK_PLOT_AXIS_TOP,GTK_PLOT_AXIS_BOTTOM} GtkPlotAxisPos;
		//~ typedef enum{GTK_PLOT_LABEL_FLOAT,GTK_PLOT_LABEL_EXP,GTK_PLOT_LABEL_POW} GtkPlotLabelStyle; */
	//~ i=(int)(ceil(log10(1.0/((connection->plot->xmax-connection->plot->xmin)*AXIS_TICKS_MAJOR)))); 
	//~ if(i<0)i=0;
	//~ gtk_plot_axis_set_labels_numbers(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_TOP,GTK_PLOT_LABEL_FLOAT,i);
	//~ gtk_plot_axis_set_labels_numbers(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_BOTTOM, GTK_PLOT_LABEL_FLOAT,i);

	//~ i=(int)(ceil(log10(1.0/((connection->plot->ymax-connection->plot->ymin)*AXIS_TICKS_MAJOR)))); 
	//~ if(i<0)i=0;
	//~ gtk_plot_axis_set_labels_numbers(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_LEFT,GTK_PLOT_LABEL_FLOAT,i);
	//~ gtk_plot_axis_set_labels_numbers(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_RIGHT, GTK_PLOT_LABEL_FLOAT,i);

	//~ /* void gtk_plot_axis_set_visible (GtkPlot *plot,GtkPlotAxisPos axis,gboolean visible);  */
	//~ gtk_plot_axis_set_visible(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_TOP, TRUE);
	//~ gtk_plot_axis_set_visible(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_RIGHT, TRUE);

	//~ /* void gtk_plot_grids_set_visible (GtkPlot *plot,gboolean vmajor,gboolean vminor,gboolean hmajor,gboolean hminor);  */
	//~ gtk_plot_grids_set_visible(GTK_PLOT(connection->plot->plotfield), TRUE, TRUE,TRUE, TRUE );

	//~ /* void gtk_plot_canvas_add_plot (GtkPlotCanvas *plot_canvas,GtkPlot *plot,gdouble x, gdouble y);
		//~ ------------------------------------------------------------------------------------------ 
		//~ x,y -> the coordinates of the plot in percentage of widget size */
	//~ gtk_plot_canvas_add_plot(GTK_PLOT_CANVAS(connection->widget), GTK_PLOT(connection->plot->plotfield),connection->plot->pos_draw_x,connection->plot->pos_draw_y);

	//~ /* void gtk_plot_axis_hide_title (GtkPlot *plot,GtkPlotAxisPos axis); */
	//~ gtk_plot_axis_hide_title(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_TOP);
	//~ gtk_plot_axis_hide_title(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_RIGHT);

	//~ /* void gtk_plot_axis_set_title (GtkPlot *plot,GtkPlotAxisPos axis, const gchar *title); */
	//~ gtk_plot_axis_set_title(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_LEFT, connection->plot->y_title);
	//~ gtk_plot_axis_set_title(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_BOTTOM, connection->plot->x_title);
	//~ /* void gtk_plot_axis_move_title (GtkPlot *plot, GtkPlotAxisPos axis, gint angle, gdouble x, gdouble y); */
	//~ gtk_plot_axis_move_title(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_LEFT,   90, 0.03, 0.5);
	//~ gtk_plot_axis_move_title(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_AXIS_BOTTOM, 0,  0.5 , 0.98);

	//~ /* void gtk_plot_set_legends_border (GtkPlot *plot,GtkPlotBorderStyle border,gint shadow_width);
		//~ ------------------------------------------------------------------------------------------ 
		//~ typedef enum{GTK_PLOT_BORDER_NONE,GTK_PLOT_BORDER_LINE,GTK_PLOT_BORDER_SHADOW,} GtkPlotBorderStyle; */
	//~ gtk_plot_set_legends_border(GTK_PLOT(connection->plot->plotfield), GTK_PLOT_BORDER_SHADOW, 3);

	//~ /* void gtk_plot_legends_move (GtkPlot *plot,gdouble x, gdouble y); 
		//~ ------------------------------------------------------------------------------------------ 
		//~ x,y -> new coordinates of the legend */
	//~ gtk_plot_legends_move(GTK_PLOT(connection->plot->plotfield), .70, .05);

	//~ gtk_widget_show(connection->plot->plotfield);

	//~ /* GtkPlotCanvasChild *
		//~ gtk_plot_canvas_put_text (GtkPlotCanvas *canvas,gdouble x,gdouble y,const gchar *font,
        	//~ gint height,gint angle,const GdkColor *fg,const GdkColor *bg,gboolean transparent,
			//~ GtkJustification justification,const gchar *text);
		//~ ------------------------------------------------------------------------------------------ 
		//~ Insert text in the canvas
		//~ canvas -> a GtkPlotCanvas widget
		//~ x,y -> coordinates where text is inserted (in percentage of the widget)
		//~ font -> text font
		//~ height -> font height
		//~ angle -> font angle
		//~ fg,bg -> foregound(text),background color
		//~ transparent -> TRUE or FALSE
		//~ justification -> GTK_JUSTIFY_LEFT, RIGHT, CENTER
		//~ text -> text string */
	//~ gtk_plot_canvas_put_text(GTK_PLOT_CANVAS(connection->widget), .45, .05,  
		//~ "Times-BoldItalic", 20, 0, NULL, NULL, TRUE, GTK_JUSTIFY_CENTER,connection->plot->title);

	//~ for(i=0;i<connection->plot->plotcount;i++) {
		//~ channelPtr=g_list_nth_data(connection->plot->channellist,i);

		//~ channelPtr->dataset = GTK_PLOT_DATA(gtk_plot_data_new());

		//~ /* void gtk_plot_add_data (GtkPlot *plot,GtkPlotData *data); 
			//~ ------------------------------------------------------------------------------------------ 
			//~ Add data to the plot widget: plot -> A GtkPlot widget; data -> data pointer */
		//~ gtk_plot_add_data(GTK_PLOT(connection->plot->plotfield), channelPtr->dataset);

		//~ gtk_widget_show(GTK_WIDGET(channelPtr->dataset));

		//~ gdk_color_parse(channelPtr->ycolor, &color);
		//~ gdk_color_alloc(gdk_colormap_get_system(), &color);

		//~ /* void gtk_plot_data_set_legend (GtkPlotData *dataset,const gchar *legend); */
		//~ gtk_plot_data_set_legend(channelPtr->dataset, channelPtr->ylegend);

		//~ /* void gtk_plot_data_set_symbol (GtkPlotData *data,GtkPlotSymbolType type,
			//~ GtkPlotSymbolStyle style,gint size,gfloat line_width,const GdkColor *color,
			//~ const GdkColor *border_color);
			//~ ----------------------------------------------------------------------------------
			//~ typedef enum{	GTK_PLOT_SYMBOL_NONE,GTK_PLOT_SYMBOL_SQUARE,GTK_PLOT_SYMBOL_CIRCLE,
							//~ GTK_PLOT_SYMBOL_UP_TRIANGLE,GTK_PLOT_SYMBOL_DOWN_TRIANGLE,
							//~ GTK_PLOT_SYMBOL_RIGHT_TRIANGLE,GTK_PLOT_SYMBOL_LEFT_TRIANGLE,
							//~ GTK_PLOT_SYMBOL_DIAMOND,GTK_PLOT_SYMBOL_PLUS,GTK_PLOT_SYMBOL_CROSS,
							//~ GTK_PLOT_SYMBOL_STAR,GTK_PLOT_SYMBOL_DOT,GTK_PLOT_SYMBOL_IMPULSE,
				    	//~ } GtkPlotSymbolType;
			//~ typedef enum{	GTK_PLOT_SYMBOL_EMPTY,GTK_PLOT_SYMBOL_FILLED,GTK_PLOT_SYMBOL_OPAQUE	
				    	//~ } GtkPlotSymbolStyle;

		//~ */
		//~ gtk_plot_data_set_symbol(channelPtr->dataset,
                        		 //~ channelPtr->symboltype,
                        		 //~ channelPtr->symbolstyle,
                        		 //~ channelPtr->symbolsize,
								 //~ channelPtr->linewidth,
								 //~ &color,
								 //~ &color);

		//~ /* void gtk_plot_data_set_line_attributes (GtkPlotData *data,
                                    	   //~ GtkPlotLineStyle style,
                                    	   //~ gfloat width,
                                    	   //~ const GdkColor *color);
			//~ ----------------------------------------------------------------------------------
			//~ typedef enum{	GTK_PLOT_LINE_NONE,	GTK_PLOT_LINE_SOLID,GTK_PLOT_LINE_DOTTED,
							//~ GTK_PLOT_LINE_DASHED,GTK_PLOT_LINE_DOT_DASH,GTK_PLOT_LINE_DOT_DOT_DASH,
							//~ GTK_PLOT_LINE_DOT_DASH_DASH   
						//~ } GtkPlotLineStyle; 	*/
		//~ gtk_plot_data_set_line_attributes(channelPtr->dataset,
											//~ channelPtr->linestyle,
											//~ channelPtr->linewidth,
											//~ &color);

		//~ /* void gtk_plot_data_set_connector (GtkPlotData *data,GtkPlotConnector connector);
			//~ ----------------------------------------------------------------------------------
			//~ typedef enum{	GTK_PLOT_CONNECT_NONE,GTK_PLOT_CONNECT_STRAIGHT,GTK_PLOT_CONNECT_SPLINE,
							//~ GTK_PLOT_CONNECT_HV_STEP,GTK_PLOT_CONNECT_VH_STEP,GTK_PLOT_CONNECT_MIDDLE_STEP  
						//~ } GtkPlotConnector;  */
		//~ gtk_plot_data_set_connector (channelPtr->dataset,channelPtr->pointconnect);

	//~ }	
	//~ /* Paint the canvas(update the changes) */
	//~ gtk_plot_canvas_paint(GTK_PLOT_CANVAS(connection->widget));

	//~ /* Refresh the canvas */
	//~ gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(connection->widget));

	//~ gtk_plot_clip_data(GTK_PLOT(connection->plot->plotfield), TRUE);


//~ }


//~ GtkWidget *
//~ hmi_GtkPlotCanvas(gchar *widgetname){

	//~ GtkWidget *canvas;
	//~ GdkColor color;
	//~ gint widget_height=DEFAULT_WIDGET_HEIGHT;
	//~ gint widget_width=DEFAULT_WIDGET_WIDTH;
	//~ char *bgcolor=(char*)"light blue";

	//~ int i;
	//~ int nbofrows=0;
	//~ int nbofcols=0;
	//~ int ret;
	//~ f32 f32_tmp;

//~ #define DEBUG_GTKPLOTCANVAS
//~ #undef DEBUG_GTKPLOTCANVAS

	//~ #ifdef DEBUG_GTKPLOTCANVAS
		//~ printf("hmi_GtkPlotCanvas()\n");
	//~ #endif

	//~ if( widgetname[0] == '_' ){
		//~ widgetname++;

		//~ nbofrows=conffile_get_table_rows(widgetname);

		//~ for(i=0;i<nbofrows;i++){
			//~ nbofcols=conffile_get_table_rowlen(widgetname, i);
			//~ if(nbofcols==0){
				//~ printf("Warning: Empty Line detected\n");
				//~ continue;
			//~ }
			
			//~ /* [SECTION: widget] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"widget")==0){
				//~ /* xsize */
				//~ ret=conffile_get_table_f32(widgetname,i,1,&(f32_tmp), 100, 10000, DEFAULT_WIDGET_WIDTH);
				//~ if(ret!=0){
					//~ widget_width = DEFAULT_WIDGET_WIDTH;
				//~ }else{
					//~ widget_width = f32_tmp;
				//~ }
				//~ #ifdef DEBUG_GTKPLOTCANVAS
					//~ printf("\twidget_width = %d\n",widget_width);		
				//~ #endif

				//~ /* ysize */
				//~ ret=conffile_get_table_f32(widgetname,i,2,&(f32_tmp), 100, 10000, DEFAULT_WIDGET_HEIGHT);
				//~ if(ret!=0){
					//~ widget_height = DEFAULT_WIDGET_HEIGHT;
				//~ }else{
					//~ widget_height = f32_tmp;
				//~ }
				//~ #ifdef DEBUG_GTKPLOTCANVAS
					//~ printf("\twidget_height = %d\n",widget_height);		
				//~ #endif
			//~ }

			//~ /* [SECTION: color] */
			//~ if(g_strcasecmp(conffile_get_table(widgetname,i,0),"color")==0){
				//~ /* Title */
				//~ bgcolor=conffile_get_table(widgetname,i,1);
				//~ if(bgcolor==NULL) bgcolor=(char *)"light blue";
				//~ #ifdef DEBUG_GTKPLOTCANVAS
					//~ printf("\tbgcolor = %s\n",bgcolor);		
				//~ #endif
			//~ }
		//~ }	
	//~ }


	//~ /* GtkWidget* gtk_plot_canvas_new (gint width, gint height,gdouble magnification); 
		//~ ----------------------------------------------------------------------------------
		//~ Create a new GtkPlotCanvas widget.
		//~ width,height -> width, height of the new canvas
		//~ magnification -> magnification of the canvas
		//~ Returns the canvas widget
	//~ */
	//~ canvas = gtk_plot_canvas_new(widget_width, widget_height, 1.);

	//~ /*	#define GTK_PLOT_CANVAS_UNSET_FLAGS(canvas, flags)  (GTK_PLOT_CANVAS_FLAGS(canvas) &= ~(flags))
		//~ #define GTK_PLOT_CANVAS_FLAGS(canvas)	  			(GTK_PLOT_CANVAS(canvas)->flags)
		//~ #define GTK_PLOT_CANVAS(obj)        				GTK_CHECK_CAST (obj, gtk_plot_canvas_get_type (), GtkPlotCanvas)
		//~ #define GTK_CHECK_CAST(tobj, cast_type, cast) ((cast*) gtk_type_check_object_cast ((GtkTypeObject*) (tobj), (cast_type)))
	//~ */
	//~ GTK_PLOT_CANVAS_UNSET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_DND_FLAGS);

	//~ gdk_color_parse(bgcolor, &color);
	//~ gdk_color_alloc(gtk_widget_get_colormap(canvas), &color);

	//~ /* void gtk_plot_canvas_set_background (GtkPlotCanvas *canvas,const GdkColor *background); */
	//~ gtk_plot_canvas_set_background(GTK_PLOT_CANVAS(canvas), &color);

	//~ gtk_widget_show(canvas);

	//~ return(canvas);
//~ }
//~ #endif


 /*
	 hmi_GtkImage_read_matplc_conf()
	 Read configuration from matplc.conf
	
	 [hmi_gtk]
	 GtkImage widgetname image1 ... imagen
	
 */
void hmi_GtkImage_read_matplc_conf(void)
{ 
	int nbofrows=0;
	int nbofcols=0;
	int iCol,iRow,iWtab,iPtab,found;
	char tablename[]="GtkImage";
	char *widgetpattern;
	char *sImage=NULL;
	char *image_name;
	FILE *fImage;
	
#ifdef DEBUG_hmi_GtkImage_read_matplc_conf
	printf("hmi_GtkImage_read_matplc_conf()\n");
#endif

	hmi_GtkImage.nbImageTab=0;
	hmi_GtkImage.ImageTab=NULL;

	nbofrows=conffile_get_table_rows(tablename);
	//printf("GtkImage:: nbofrows %d\n",nbofrows);
	
	for(iRow=0;iRow<nbofrows;iRow++){
		nbofcols=conffile_get_table_rowlen(tablename, iRow);

		if(nbofcols<2){
			printf("Warning: Incomplete Line detected\n");
			continue;
		}
		
		/* widgetpattern */
		widgetpattern=conffile_get_table(tablename,iRow,0);
		//printf("GtkImage:: Widgetname=%s nbofcols=%d\n",
		//	widgetpattern,nbofcols-1);

		/* pruefen ob der Eintrag schon vorhanden ist */
		found=FALSE;
		for (iWtab=0;iWtab<hmi_GtkImage.nbImageTab;iWtab++) {
			//printf("pruefen iWtab=%d ... ",iWtab);
			if (strcmp(hmi_GtkImage.ImageTab[iWtab].widgetpattern,widgetpattern)==0) {
				found=TRUE;
				free(widgetpattern);
				//printf("found\n");
				break;
			}
			//printf("\n");
		}

		if (!found) {
			hmi_GtkImage.nbImageTab++;
			hmi_GtkImage.ImageTab=(hmi_GtkImage_tab_t*)g_realloc(hmi_GtkImage.ImageTab,
				(hmi_GtkImage.nbImageTab*sizeof(hmi_GtkImage_tab_t)));
				
			//printf("g_realloc: nbImageTab=%d iWtab=%d\n",
			//	hmi_GtkImage.nbImageTab,iWtab);
			
			hmi_GtkImage.ImageTab[iWtab].nbImage=0;
			hmi_GtkImage.ImageTab[iWtab].widgetpattern=widgetpattern;
			hmi_GtkImage.ImageTab[iWtab].Image=NULL;
		}
		
		//printf("nbImage=%d\n",
		//	hmi_GtkImage.ImageTab[iWtab].nbImage
		//	);

		iPtab=hmi_GtkImage.ImageTab[iWtab].nbImage;
		hmi_GtkImage.ImageTab[iWtab].nbImage+=(nbofcols-1);
		hmi_GtkImage.ImageTab[iWtab].Image=
			(char**)g_realloc( hmi_GtkImage.ImageTab[iWtab].Image,
			hmi_GtkImage.ImageTab[iWtab].nbImage*sizeof(char*) );

		//printf("iPtab=%d nbImage=%d\n",
		//	iPtab,
		//	hmi_GtkImage.ImageTab[iWtab].nbImage
		//	);
		
		for (iCol=1;iCol<nbofcols;iCol++) {
			hmi_GtkImage.ImageTab[iWtab].Image[iPtab]=NULL;
			sImage=conffile_get_table(tablename,iRow,iCol);
			image_name = g_strdup_printf("%s.xpm",sImage);
			if ((fImage=fopen(image_name,"r"))!=NULL) {
				fclose(fImage);
				//printf("iCol=%d Image=%s\n",iCol,sImage);
				hmi_GtkImage.ImageTab[iWtab].Image[iPtab]=sImage;
				iPtab++;
			} else {
				printf("ERROR: Image %s not available!!\n",image_name);
				hmi_GtkImage.ImageTab[iWtab].nbImage--;
			}
			g_free(image_name);
		}		
		//printf("\n");
	}

#ifdef DEBUG_hmi_GtkImage_read_matplc_conf
	for (iWtab=0;iWtab<hmi_GtkImage.nbImageTab;iWtab++) {
		printf("%d: nbImage=%d wpat=%s Image=",
			iWtab,
			hmi_GtkImage.ImageTab[iWtab].nbImage,
			hmi_GtkImage.ImageTab[iWtab].widgetpattern);
		
		for (iPtab=0; iPtab<hmi_GtkImage.ImageTab[iWtab].nbImage;iPtab++) {
			printf(" %s",hmi_GtkImage.ImageTab[iWtab].Image[iPtab]);
		}
		printf("\n");	
	}
#endif		
}



/*
	hmi_GtkLabel_read_matplc_conf()
	Read configuration from matplc.conf
	
	[hmi_gtk]
	GtkLabel widgetname Formatstring
	
*/
void hmi_GtkLabel_read_matplc_conf(void)
{
	int nbofrows=0;
	int nbofcols=0;
	int iRow,iWtab,found;
	char tablename[]="GtkLabel";
	char *widgetpattern;
	
#ifdef DEBUG_hmi_GtkLabel_read_matplc_conf
	printf("hmi_GtkLabel_read_matplc_conf()\n");
#endif

	hmi_GtkLabel.nbLabelTab=0;
	hmi_GtkLabel.LabelTab=NULL;

	nbofrows=conffile_get_table_rows(tablename);
	//printf("GtkLabel:: nbofrows %d\n",nbofrows);
	
	for(iRow=0;iRow<nbofrows;iRow++){
		nbofcols=conffile_get_table_rowlen(tablename, iRow);

		if(nbofcols<2){
			printf("Warning: Incomplete Line detected\n");
			continue;
		}
		
		/* widgetpattern */
		widgetpattern=conffile_get_table(tablename,iRow,0);
		//printf("GtkLabel:: Widgetname=%s nbofcols=%d\n",widgetpattern,nbofcols-1);

		/* pruefen ob der Eintrag schon vorhanden ist */
		found=FALSE;
		for (iWtab=0;iWtab<hmi_GtkLabel.nbLabelTab;iWtab++) {
			//printf("pruefen iWtab=%d ... ",iWtab);
			if (strcmp(hmi_GtkLabel.LabelTab[iWtab].widgetpattern,widgetpattern)==0) {
				found=TRUE;
				free(widgetpattern); widgetpattern=NULL;
				free(hmi_GtkLabel.LabelTab[iWtab].formatstring);
				hmi_GtkLabel.LabelTab[iWtab].formatstring=NULL;
				//printf("found\n");
				break;
			}
			//printf("\n");
		}

		if (!found) {
			hmi_GtkLabel.nbLabelTab++;
			hmi_GtkLabel.LabelTab=(hmi_GtkLabel_tab_t*)g_realloc(hmi_GtkLabel.LabelTab,
				(hmi_GtkLabel.nbLabelTab*sizeof(hmi_GtkLabel_tab_t)));
				
			//printf("g_realloc: nbLabelTab=%d iWtab=%d\n",
			//	hmi_GtkLabel.nbLabelTab,iWtab);
			
			hmi_GtkLabel.LabelTab[iWtab].widgetpattern=widgetpattern;
			hmi_GtkLabel.LabelTab[iWtab].formatstring=NULL;
		}
		
		hmi_GtkLabel.LabelTab[iWtab].formatstring=conffile_get_table(tablename,iRow,1);

		//printf("\n");
	}

#ifdef DEBUG_hmi_GtkLabel_read_matplc_conf
	for (iWtab=0;iWtab<hmi_GtkLabel.nbLabelTab;iWtab++) {
		printf("%d: wpat=%s Formatstring=%s (%08lx)\n",
			iWtab,
			hmi_GtkLabel.LabelTab[iWtab].widgetpattern,
			hmi_GtkLabel.LabelTab[iWtab].formatstring,
			(unsigned long)hmi_GtkLabel.LabelTab[iWtab].formatstring);
		
	}
#endif		
}

/* Initialize the optionmenu widget with init value from plc
   This Function is called only once by parse connection,
   when window->first_scan=1 */
void
option_menu_widget_init(connection_t *connection){
	int nbofrows=0;
	int nbofcols=0;
	int iRow,iCol;
	gchar *tablename;
	GtkWidget *menu_item;
	GtkMenuShell *menu_shell;
	//int index;
	optionmenu_t *om;
	GList *glist;
	plc_pt_t pt;
	
	menu_shell=GTK_MENU_SHELL(GTK_MENU(GTK_OPTION_MENU(connection->widget)->menu));
	//printf("-->g_list_length=%d\n",g_list_length(menu_shell->children));

	tablename=(gchar *)gtk_widget_get_name(connection->widget);
	if(tablename[0]=='_') tablename++;
	
	nbofrows=conffile_get_table_rows(tablename);

	/* OptionMenuTab mit Dummy-Element (index=0) starten */
	om = (optionmenu_t *)g_malloc(sizeof(optionmenu_t));
	if(om==NULL) printf("(Z %d) Could not allocate memory for structure optionmenu_t",__LINE__);
	om->pv = 0;
	om->pt = plc_pt_null();
	connection->OptionMenuTab = g_list_append(connection->OptionMenuTab, om);

	/* Tabelle in matplc.conf durcharbeiten */
	for(iRow=0;iRow<nbofrows;iRow++){
		nbofcols=conffile_get_table_rowlen(tablename, iRow);
		if(nbofcols<2){
			printf("option_menu_widget_init: Warning: Incomplete Line detected\n");
			continue;
		}

		pt=get_pt(conffile_get_table(tablename,iRow,0));
		plc_set(pt,0);

		/* Tabellenzeile abarbeiten */
		for(iCol=1;iCol<nbofcols;iCol++){
			om = (optionmenu_t *)g_malloc(sizeof(optionmenu_t));
			if(om==NULL) printf("(Z %d) Could not allocate memory for structure optionmenu_t",__LINE__);
			
			menu_item=gtk_menu_item_new_with_label (conffile_get_table(tablename,iRow,iCol));
			gtk_menu_shell_append(menu_shell,menu_item);

			om->pv = iCol;
			om->pt = pt;

			connection->OptionMenuTab = g_list_append(connection->OptionMenuTab, om);
		}
	}
	
	gtk_signal_connect (GTK_OBJECT (GTK_OPTION_MENU (connection->widget)->menu),
                      "selection-done", GTK_SIGNAL_FUNC (update_optionmenu),
                      connection);

	gtk_option_menu_set_history(GTK_OPTION_MENU(connection->widget),1);
	gtk_widget_show_all(connection->widget);

	/* Set Point for the first item (which is now selected) */
	glist=g_list_nth(connection->OptionMenuTab,1);
	om=(optionmenu_t*)glist->data;
	plc_set(om->pt,om->pv);
	connection->variable->value=1;

}

//#ifdef GTKSOCKET
//#define DEBUG_GTKSOCKET
//#undef DEBUG_GTKSOCKET

//~ /*
	//~ Init function for GtkSocket
	//~ Set point with window-ID
//~ */
//~ void
//~ socket_widget_init(connection_t *connection){
	//~ gtk_widget_realize(connection->widget);
	//~ plc_set(connection->pt,(u32)(GDK_WINDOW_XWINDOW(connection->widget->window)));
	//~ #ifdef DEBUG_GTKSOCKET
		//~ printf("socket_widget_init: ID=%d\n",
			//~ (guint32)(GDK_WINDOW_XWINDOW(connection->widget->window)));fflush(stdout);
	//~ #endif
//~ }

//~ /*
	//~ Create socket window in creation function
//~ */
//~ GtkWidget *
//~ hmi_GtkSocket(gchar *widgetname){
	//~ GtkWidget *socket;

	//~ #ifdef DEBUG_GTKSOCKET
		//~ printf("hmi_GtkSocket(): Name=%s\n",widgetname);fflush(stdout);
	//~ #endif

 	//~ socket = gtk_socket_new();
	//~ gtk_widget_show_all(socket);

	//~ return(socket);
//~ }
//~ #endif


/* Parse the windows data structure and update the values in the connections */
void
parse_connection(gpointer a_connection, gpointer data){
  connection_t *connection = a_connection;
  window_node_t *window = data;
  guint type;

  /*
     If the widget is not there do nothing
	 (Gtk_Socket will be destroyed, when the plugged
	 window was destroyed)
   */
  if(!(GTK_IS_WIDGET(connection->widget)))return;

	
  /* An obvious opportunity for speed optimisation is to calculate the type
   * values one time and store them in variables 
   */
  // type = GTK_OBJECT(connection->widget)->klass->type; // Port to GTK2
  type = GTK_CLASS_TYPE(GTK_WIDGET_GET_CLASS(connection->widget));
  if(type == gtk_type_from_name("GtkLabel")){
    label_widget(connection);
  }
  else if(type == gtk_type_from_name("GtkProgressBar")){
    progress_widget(connection);
  }
  else if(type == gtk_type_from_name("GtkImage")){
    /* Force first draw of the widget using the current state */
    if(window->first_scan){
      connection->variable->value = !plc_get(connection->pt);
    }
    gtk_image_widget(connection);
  }
  else{
    /* From here enter the data entry widgets */
    if(window->first_scan){
      if(connection->variable){
	connection->variable->value = plc_get(connection->pt);
      }
    }
    if(type == gtk_type_from_name("GtkToggleButton")){
      toggle_button_widget(connection);
    }
    else if(type == gtk_type_from_name("GtkRadioButton")){
      toggle_button_widget(connection);
    }
    else if(type == gtk_type_from_name("GtkCheckButton")){
      toggle_button_widget(connection);
    }
    else if(type == gtk_type_from_name("GtkButton")){
      button_widget(connection);
    }
    else if(type == gtk_type_from_name("GtkEntry")){
	  if(window->first_scan) { 
      	entry_widget_init(connection);
	  }
      entry_widget(connection);
    }
    else if(type == gtk_type_from_name("GtkHScale")){
	  if(window->first_scan) { 
      	scale_widget_init(connection);
	  }
      scale_widget(connection);
    }
    else if(type == gtk_type_from_name("GtkVScale")){
	  if(window->first_scan) { 
      	scale_widget_init(connection);
	  }
      scale_widget(connection);
    }
    else if(type == gtk_type_from_name("GtkSpinButton")){
	  if(window->first_scan) { 
      	spin_button_widget_init(connection);
	  }
      spin_button_widget(connection);
    }
    else if(type == gtk_type_from_name("GtkOptionMenu")){
	  if(window->first_scan) {
      	option_menu_widget_init(connection);
	  }
    }
//~ #ifdef GTKSOCKET
    //~ else if(type == gtk_type_from_name("GtkSocket")){
	  //~ if(window->first_scan) {
      	//~ socket_widget_init(connection);
	  //~ }
    //~ }
//~ #endif
//~ #ifdef GTKEXTRA
    //~ else if(type == gtk_type_from_name("GtkPlotCanvas")){
	  //~ if(window->first_scan) { 
      	//~ hmi_GtkPlotCanvas_widget_init(connection);
	  //~ }
	  //~ hmi_GtkPlotCanvas_widget(connection);
    //~ }
//~ #endif
  }
}

void
parse_window(gpointer window, gpointer data){
  window_node_t *window_node = window;

  g_list_foreach (window_node->connections, (GFunc)parse_connection, window);
  window_node->first_scan = 0;
}

/* This function is called when gtk is idle (free of events) */
gboolean 
idle(gpointer data)
{
  plc_scan_beg();
  plc_update();

  g_list_foreach (windows, (GFunc)parse_window, NULL);
  
  plc_update();
  plc_scan_end();

  return TRUE;
}

/* add the variable name to the variables hash table for input fields */
variable_t
*add_variable(char *name, data_type_t type){
  variable_t *variable, *exist;

#define DEBUG_ADD_VARIABLE
#undef DEBUG_ADD_VARIABLE

  variable = g_malloc0(sizeof(variable_t));
  variable->type = type;
  exist = g_hash_table_lookup(variables, name);
  if(exist){
    /* Already created check that the type matches */
    if(exist->type != type){
      printf("Type inconsistency in variable %s\n", name);
    }
    return exist;
  }
  else{
	#ifdef DEBUG_ADD_VARIABLE
  		printf("Adding %s to hash table\n",name);
	#endif
    g_hash_table_insert(variables, name, variable); 
  }
  return variable;
}

/* init window data to comunicate with plc points */
void
init_widget_data(gpointer widget, gpointer user_data)
{
  window_node_t *main_window = user_data;
  connection_t *connection;
  char *widget_name;
  char *s;
  gchar **split_str;
  gchar *ImageName;
  int i;
  gboolean notype;
  guint type;

#define DEBUG_INIT_WIDGET_DATA
#undef DEBUG_INIT_WIDGET_DATA

#ifdef DEBUG_INIT_WIDGET_DATA
	printf("init_widget_data()\n");
#endif
	if(widget){
		widget_name = g_strdup(glade_get_widget_name(GTK_WIDGET(widget)));
		//sprintf("init_widget_data() %s \n",widget_name);

		/* Interim code to automaticaly adjust GtkImage to the widget size  
		 * Glade and libglade are currently not handling GtkImage image scaling 
		 */
		// type = GTK_OBJECT(widget)->klass->type; // Port to GTK2
  		type = GTK_CLASS_TYPE(GTK_WIDGET_GET_CLASS(widget));
		if(type == gtk_type_from_name("GtkImage")){
			if(widget_name[0] == '.'){
				split_str = g_strsplit(widget_name+1, "_", 2);
				if(split_str[0]){
					load_gtk_image(GTK_WIDGET(widget), split_str[0]);
				}
				g_free(split_str);
			}
			/* Load a transparent image to avoid painting a large 
			* non scaled image when using Image as display.
			*/
			else if(widget_name[0] == '_'){
				ImageName = g_strdup("Transparent.xpm");
				load_gtk_image(GTK_WIDGET(widget), ImageName);
				g_free(ImageName);
			}
		}

	#ifdef DEBUG_INIT_WIDGET_DATA
		printf("Name: %s\n", widget_name);
	#endif
    /* printf("Name: %s\n", widget_name); */
    /* if the name starts with '_' then create a connection */
    if(widget_name[0] == '_'){
      /* format for point name widgets
       *   _pointname[.id][.type][.parameter1[.parameter2]]
       * The parameters is used for example in boolean variables 
       *    for the text for on and off states
       */
      split_str = g_strsplit(widget_name+1, ".", 5);
      i = 0;
      while(split_str[i]){
	#ifdef DEBUG_INIT_WIDGET_DATA
	 	printf("%s \n", split_str[i]);
	#endif
        i++;
      }
      if(i>0){
	/* create a connection to a plc point for this widget */
	connection = (connection_t *)g_malloc(sizeof(connection_t));
	connection->data_type = bool_dt;
	connection->on = NULL;
	connection->off = NULL;
	connection->widget = widget;
	connection->pt = get_pt(split_str[0]);
	connection->ImageTab = NULL;
	connection->LabelTab = NULL;
//#ifdef GTKEXTRA
//	connection->plot = NULL;
//#endif
	connection->OptionMenuTab = NULL;

	if(i>2){
	  notype = 0;
	  s = split_str[2];
	  if (strcmp(s,  f32_TYPE) == 0)
	    connection->data_type =  f32_dt;
	  else if (strcmp(s,  i32_TYPE) == 0)
	    connection->data_type =  i32_dt;
	  else if (strcmp(s,  u32_TYPE) == 0)
	    connection->data_type =  u32_dt;
	  else if (strcmp(s,  i16_TYPE) == 0)
	    connection->data_type =  i16_dt;
	  else if (strcmp(s,  u16_TYPE) == 0)
	    connection->data_type =  u16_dt;
	  else if (strcmp(s,   i8_TYPE) == 0)
	    connection->data_type =   i8_dt;
	  else if (strcmp(s,   u8_TYPE) == 0)
	    connection->data_type =   u8_dt;
	  else if (strcmp(s, bool_TYPE) == 0)
	    connection->data_type = bool_dt;
	  else notype = 1;
	  if(notype){
	    connection->on = g_strdup(split_str[2]);
	    if(i>2){
	      connection->off = g_strdup(split_str[3]);
	    }
	  }
	  else{
	    if(i>2){
	      connection->on = g_strdup(split_str[3]);
	      if(i>3){
		connection->off = g_strdup(split_str[4]);
	      }
	    }
	  }
	}
	
	/* for GtkImage find the corresponding ImageTab Entry */
    if(type == gtk_type_from_name("GtkImage")){
		for (i=0;i<hmi_GtkImage.nbImageTab;i++) {
			if (fnmatch(hmi_GtkImage.ImageTab[i].widgetpattern,split_str[0],0)==0){

#ifdef DEBUG_INIT_WIDGET_DATA
				printf("init_widget_data(): ImageTab[%d]=%s\n",
					i,hmi_GtkImage.ImageTab[i].widgetpattern);
#endif
				connection->ImageTab=&hmi_GtkImage.ImageTab[i];
			}
		}
	}

	/* for GtkLabel find the corresponding LabelTab Entry */
    if(type == gtk_type_from_name("GtkLabel")){
		for (i=0;i<hmi_GtkLabel.nbLabelTab;i++) {
			if (fnmatch(hmi_GtkLabel.LabelTab[i].widgetpattern,split_str[0],0)==0){

#ifdef DEBUG_INIT_WIDGET_DATA
				printf("init_widget_data(): LabelTab[%d]=%s\n",
					i,hmi_GtkLabel.LabelTab[i].widgetpattern);
#endif
				connection->LabelTab=&hmi_GtkLabel.LabelTab[i];
			}
		}
	}

//#ifdef GTKEXTRA
//	if(type == gtk_type_from_name("GtkPlotCanvas")){
//		connection->plot = (plot_t *)g_malloc(sizeof(plot_t));
//		connection->plot->channellist=NULL;
//	i}
//#endif

	
	/* add the widget name to the variable table */
	connection->variable = add_variable(split_str[0], connection->data_type);
	main_window->connections = g_list_append(main_window->connections, connection);
	g_free(split_str);
      }
    }
    g_free(widget_name);
  }
}

/* Data entry widgets update functions */
void
toggle_button_widget_update(GtkWidget *w, variable_t *variable){
  /* printf("ToggleButton %d\n", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))); */
  if(variable->type == bool_dt){
    variable->value = (u32)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)); 
  }
}

void
button_widget_update(GtkWidget *w, variable_t *variable){
  if(variable->type == bool_dt){
	  //MYDEBUG
	  printf("Variable Value");
    variable->value = 1; 
  }
}

void 
entry_widget_update(GtkWidget *w, variable_t *variable){
  u32 u32_tmp;
  union {
    u32 u;
    f32 f;
  } f32_tmp;
  char *str_buff;

  str_buff = gtk_editable_get_chars (GTK_EDITABLE(w), 0, -1);
  if(variable->type == bool_dt){
    /* No digital type allowed for this widget */
  }
  else{
    switch (variable->type) {
    case f32_dt:     
      f32_tmp.f = atof(str_buff);
      variable->value =  f32_tmp.u;
      break;
    case  i8_dt: 
    case  u8_dt: 
    case i16_dt: 
    case u16_dt: 
    case i32_dt: 
    case u32_dt:
      /* Check if this does not have any type conversion implications */
      u32_tmp = (u32)atoi(str_buff);
      variable->value = u32_tmp;
      break;
    default : break;
    }; /* switch() */
  }
  g_free(str_buff);
}

void
scale_widget_update(GtkWidget *w, variable_t *variable){
	u32 u32_tmp;
	u16 u16_tmp;
	u8   u8_tmp;
	i32 i32_tmp;
	i16 i16_tmp;
	i8   i8_tmp;
	union {u32 u; f32 f;} f32_tmp;
	GtkAdjustment *adjustment;

	adjustment=gtk_range_get_adjustment(GTK_RANGE(w));
	u32_tmp = adjustment->value;
  
	switch (variable->type) {
	case f32_dt:
    	f32_tmp.f=adjustment->value;
		variable->value =  f32_tmp.u;
		//printf("scale_widget_update.f32: value=%f\n",f32_tmp);
		break;
	case u32_dt:
		variable->value =  u32_tmp;
		//printf("scale_widget_update.u32: value=%d\n",u32_tmp);
		break;
	case u16_dt:
    	u16_tmp=u32_tmp;
	u32_tmp=u16_tmp;
		variable->value =  u32_tmp;
		//printf("scale_widget_update.u16: value=%d\n",u16_tmp);
		break;
	case u8_dt:
    	u8_tmp=u32_tmp;
		variable->value =  *((u32 *)&u8_tmp);
		//printf("scale_widget_update.u8: value=%d\n",u8_tmp);
		break;
	case i32_dt:
    	i32_tmp=u32_tmp;
		variable->value =  *((u32 *)&i32_tmp);
		//printf("scale_widget_update.i32: value=%d\n",i32_tmp);
		break;
	case i16_dt:
    	i16_tmp=u32_tmp;
        i32_tmp=i16_tmp;
		variable->value =  *((u32 *)&i32_tmp);
		//printf("scale_widget_update.i16: value=%d\n",i16_tmp);
		break;
	case i8_dt:
    	i8_tmp=u32_tmp;
		variable->value =  *((u32 *)&i8_tmp);
		//printf("scale_widget_update.i8: value=%d\n",i8_tmp);
		break;
	default:
		break;
	}
}

void
spin_button_widget_update(GtkWidget *w, variable_t *variable){
	u32 u32_tmp;
	u16 u16_tmp;
	u8   u8_tmp;
	i32 i32_tmp;
	i16 i16_tmp;
	i8   i8_tmp;
	union {u32 u; f32 f;} tmp;
#define f32_tmp (tmp.f)

	f32_tmp = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(w));
	  
	switch (variable->type) {
	case f32_dt:
		variable->value =  tmp.u;
		//printf("spin_button_widget_update.f32: value=%f\n",f32_tmp);
		break;
	case u32_dt:
    	u32_tmp=f32_tmp;
		variable->value =  u32_tmp;
		//printf("spin_button_widget_update.u32: value=%d\n",u32_tmp);
		break;
	case u16_dt:
    	u16_tmp=f32_tmp;
	u32_tmp=u16_tmp;
		variable->value =  u32_tmp;
		//printf("spin_button_widget_update.u16: value=%d\n",u16_tmp);
		break;
	case u8_dt:
    	u8_tmp=f32_tmp;
		variable->value =  *((u32 *)&u8_tmp);
		//printf("spin_button_widget_update.u8: value=%d\n",u8_tmp);
		break;
	case i32_dt:
    	i32_tmp=f32_tmp;
		variable->value =  *((u32 *)&i32_tmp);
		//printf("spin_button_widget_update.i32: value=%d\n",i32_tmp);
		break;
	case i16_dt:
    	i16_tmp=f32_tmp;
	i32_tmp=i16_tmp;
		variable->value =  i32_tmp;
		//printf("spin_button_widget_update.i16: value=%d\n",i16_tmp);
		break;
	case i8_dt:
    	i8_tmp=f32_tmp;
		variable->value =  *((u32 *)&i8_tmp);
		//printf("spin_button_widget_update.i8: value=%d\n",i8_tmp);
		break;
	default:
		break;
	}
#undef f32_tmp
}



/* Function to update an entry value */
void
update_value(GtkWidget *w, gpointer data){
  variable_t *variable;
  data_type_t type;
  char *name;
  char **point_name;

#define DEBUG_UPDATE_VALUE
//#undef DEBUG_UPDATE_VALUE

#ifdef DEBUG_UPDATE_VALUE
	printf("update_value()\n");
#endif

  name = (char *)glade_get_widget_name(GTK_WIDGET(w));
  if(name[0] == '_'){
    point_name = g_strsplit(name+1, ".", 2);
    /* get variable from hash table */
    variable = g_hash_table_lookup(variables, point_name[0]);
	#ifdef DEBUG_UPDATE_VALUE
    	printf("Point Name: %s\n", point_name[0]);
	#endif

    /* An obvious opportunity for speed optimisation is to calculate the type
     * values one time and store them in variables 
     */
    // type = GTK_OBJECT(w)->klass->type; // Port to GTK2
    type = GTK_CLASS_TYPE(GTK_WIDGET_GET_CLASS(w));
    if(type == gtk_type_from_name("GtkToggleButton")){
      toggle_button_widget_update(w, variable);
    }
    else if(type == gtk_type_from_name("GtkRadioButton")){
      toggle_button_widget_update(w, variable);
    }
    else if(type == gtk_type_from_name("GtkCheckButton")){
      toggle_button_widget_update(w, variable);
    }
    else if(type == gtk_type_from_name("GtkButton")){
      button_widget_update(w, variable);
    }
    else if(type == gtk_type_from_name("GtkEntry")){
      entry_widget_update(w, variable);
    }
    else if(type == gtk_type_from_name("GtkHScale")){
      scale_widget_update(w, variable);
    }
    else if(type == gtk_type_from_name("GtkVScale")){
      scale_widget_update(w, variable);
    }
    else if(type == gtk_type_from_name("GtkSpinButton")){
      spin_button_widget_update(w, variable);
	}
  }
}

/* Function to update an entry value */
void
update_optionmenu(GtkWidget *w, gpointer data){
  int index;
  GtkMenuShell *menushell;
  optionmenu_t *om;
  GList *glist;

#define DEBUG_UPDATE_OPTIONMENU
#undef DEBUG_UPDATE_OPTIONMENU

#ifdef DEBUG_UPDATE_OPTIONMENU
	printf("update_optionmenu()\n");
#endif

	menushell=GTK_MENU_SHELL(w);
	index = g_list_index(menushell->children,gtk_menu_get_active(GTK_MENU(w)));

	/* Reset last selected Point */
	glist=g_list_nth(((connection_t *)data)->OptionMenuTab,((connection_t *)data)->variable->value);
	if (glist!=NULL) {
		om=(optionmenu_t*)glist->data;
		plc_set(om->pt,0);
	}	

	/* Get Value from GList */
	glist=g_list_nth(((connection_t *)data)->OptionMenuTab,index);
	if (glist!=NULL) {
		om=(optionmenu_t*)glist->data;
		plc_set(om->pt,om->pv);
		((connection_t *)data)->variable->value=index;
	}
}

/* Function to reset an entry value mainly for implementing push buttons*/
void
reset_value(GtkWidget *w, gpointer data){
  variable_t *variable;
  data_type_t type;
  char *name;
  char **point_name;

#define DEBUG_RESET_VALUE
#undef DEBUG_RESET_VALUE

#ifdef DEBUG_RESET_VALUE
	printf("reset_value()\n");
#endif

  name = (char *)glade_get_widget_name(GTK_WIDGET(w));
  if(name[0] == '_'){
    point_name = g_strsplit(name+1, ".", 2);
    /* get variable from hash table */
    variable = g_hash_table_lookup(variables, point_name[0]);
    // type = GTK_OBJECT(w)->klass->type; // Port GTK2
    type = GTK_CLASS_TYPE(GTK_WIDGET_GET_CLASS(w));
    if(type == gtk_type_from_name("GtkButton")){
      if(variable->type == bool_dt){
	variable->value = 0; 
      }
    }
  }
}

/* Generate data tree for a window */
void
new_window(GladeXML *xml, char *name, GtkWidget *widget){
  window_node_t *window;
  GList *widgetList;

#define DEBUG_NEW_WINDOW
#undef DEBUG_NEW_WINDOW

#ifdef DEBUG_NEW_WINDOW
	printf("new_window() %s \n",name);
#endif

  window = (window_node_t *)g_malloc(sizeof(window_node_t));
  /* Never forget to assign NULL to a before starting to popullate a GList */
  window->connections = NULL;
  window->first_scan = 1;
  window->name = name;
  window->widget = widget;

  windows = g_list_append(windows, window);

  widgetList = glade_xml_get_widget_prefix(xml, "");

  #ifdef DEBUG_NEW_WINDOW
	printf("Length : %d\n", g_list_length(widgetList));
  #endif

  g_list_foreach (widgetList, (GFunc)init_widget_data, window);
}

/* Run the about dialog */
void
on_about1_activate(GtkWidget *w)
{
  /* GladeXML *xml; */
  //const gchar *authors[] = {
  //  "Juan Carlos Orozco",
  //  NULL
  //};

  //~ gtk_widget_show (gnome_about_new ("HMI Interpreter", VERSION,
				    //~ "Copyright 2001 Juan Carlos Orozco",
				    //~ ("Credits:  "
				     //~ "MatPLC:"
				     //~ "  Jiri Baum, Mario de Sousa - creators of gmm; "
				     //~ "  Court Wollet - project leader; "
				     //~ "Glade:"
				     //~ "  Damon Chaplin, Martijn van Beers; "),
				    //~ (const gchar **) authors,
				    //~ NULL,
				    //~ NULL,
				    //~ NULL));
}

/* Function to compare the window name in the windows DList */
gint
comp_window_name(gconstpointer window1, gconstpointer name1){

#define DEBUG_COMP_WINDOW_NAME
#undef DEBUG_COMP_WINDOW_NAME


  window_node_t *window = (window_node_t *)window1;
  char *name = (char *)name1;

  #ifdef DEBUG_COMP_WINDOW_NAME
	  printf("comp_window_name() \n");
  #endif

  return strcmp(window->name, name);
}



/* Function to erase the variables in each connection */
void
free_connection(gpointer connection1, gpointer userdata){
  connection_t *connection = (connection_t *)connection1;
  variable_t *exist;
#ifdef GTKEXTRA
  channel_t *channelPtr;
#endif
  optionmenu_t *om;
  GList *glist;


#define DEBUG_FREE_CONNECTION
#undef DEBUG_FREE_CONNECTION


#ifdef GTKEXTRA
  int i;
#endif

#ifdef DEBUG_FREE_CONNECTION
	printf("free_connection(%s)  ***** START *****\n",(connection->widget->name)+1);
#endif

	if(connection->OptionMenuTab){
		glist=g_list_first(connection->OptionMenuTab);
		while (glist!=NULL) {
			om=(optionmenu_t*)glist->data;
			if (om!=NULL) {
				g_free(om);
				glist->data=NULL;
			}
			glist=g_list_next(glist);
		}
	
		g_list_free(connection->OptionMenuTab);
		connection->OptionMenuTab=NULL;
	}


	if(connection->variable){
		exist = g_hash_table_lookup(variables, (connection->widget->name)+1);
		if(exist){
 			#ifdef DEBUG_FREE_CONNECTION
 				printf("\tRemoving variable %s from hash table\n",(connection->widget->name)+1);
			#endif
    		g_hash_table_remove(variables, (connection->widget->name)+1); 
 			g_free(connection->variable);
 			connection->variable=NULL;
		}

	}
	if(connection->on){
		#ifdef DEBUG_FREE_CONNECTION
			printf("\tDeleting on\n");fflush(stdout);
		#endif
		g_free(connection->on);
		connection->on=NULL;
	}
	if(connection->off){
		#ifdef DEBUG_FREE_CONNECTION
			printf("\tDeleting off\n");fflush(stdout);
		#endif
		g_free(connection->off);
		connection->off=NULL;
	}

	if(connection->widget){
		#ifdef DEBUG_FREE_CONNECTION
			printf("\tSetting widget to Nullpointer\n");fflush(stdout);
		#endif
		connection->widget=NULL;
	}

//~ #ifdef GTKEXTRA

	//~ if(connection->plot){
		//~ for(i=0;i<connection->plot->plotcount;i++){
			//~ channelPtr=((GList *)g_list_last(connection->plot->channellist))->data;
			//~ if(channelPtr->dataset){
				//~ #ifdef DEBUG_FREE_CONNECTION
					//~ printf("\tChannel %s: Deleting plot->dataset[%d]\n",channelPtr->dataset,i);fflush(stdout);
				//~ #endif
				//~ gtk_plot_remove_data (GTK_PLOT(connection->plot->plotfield), channelPtr->dataset); 
				//~ gtk_widget_destroy(GTK_WIDGET(channelPtr->dataset));
				//~ g_free(channelPtr->dataset);
				//~ channelPtr->dataset=NULL;
			//~ }

			//~ if(channelPtr->px){
				//~ #ifdef DEBUG_FREE_CONNECTION
					//~ printf("\tChannel %s: Deleting plot->px[%d]\n",channelPtr->px,i);fflush(stdout);
				//~ #endif
				//~ g_free(channelPtr->px);
				//~ channelPtr->px=NULL;
			//~ }
			//~ if(channelPtr->py){
				//~ #ifdef DEBUG_FREE_CONNECTION
					//~ printf("\tChannel %s: Deleting plot->py[%d]\n",channelPtr->py,i);fflush(stdout);
				//~ #endif
				//~ g_free(channelPtr->py);
				//~ channelPtr->py=NULL;
			//~ }
			//~ if(channelPtr->ycolor){
				//~ #ifdef DEBUG_FREE_CONNECTION
					//~ printf("\tChannel %s: Deleting plot->ycolor[%d]\n",channelPtr->ycolor,i);fflush(stdout);
				//~ #endif
				//~ g_free(channelPtr->ycolor);
				//~ channelPtr->ycolor=NULL;
			//~ }
			//~ if(channelPtr->ylegend){
				//~ #ifdef DEBUG_FREE_CONNECTION
					//~ printf("\tChannel %s: Deleting plot->ylegend[%d]\n",channelPtr->ylegend,i);fflush(stdout);
				//~ #endif
				//~ g_free(channelPtr->ylegend);
				//~ channelPtr->ylegend=NULL;
			//~ }
			//~ connection->plot->channellist=g_list_remove(connection->plot->channellist,channelPtr);
		//~ }

		//~ if(connection->plot->channellist){
			//~ #ifdef DEBUG_FREE_CONNECTION
				//~ printf("\tDeleting channellist\n");fflush(stdout);
			//~ #endif
			//~ g_list_free(connection->plot->channellist);
		//~ }

		//~ if(connection->plot->channellist){
			//~ #ifdef DEBUG_FREE_CONNECTION
				//~ printf("\tDeleting plot->channellist\n");fflush(stdout);
			//~ #endif
			//~ g_free(connection->plot->channellist);
		//~ }

		//~ if(connection->plot->plotfield){
			//~ #ifdef DEBUG_FREE_CONNECTION
				//~ printf("\tDestroying plot->plotfield\n");fflush(stdout);
			//~ #endif
			//~ gtk_widget_destroy(connection->plot->plotfield);
			//~ //g_free(connection->plot->plotfield);
			//~ connection->plot->plotfield=NULL;
		//~ }


		//~ if(connection->plot->title){
			//~ #ifdef DEBUG_FREE_CONNECTION
				//~ printf("\tDeleting plot->title\n");fflush(stdout);
			//~ #endif
			//~ g_free(connection->plot->title);
		//~ }
		//~ if(connection->plot->x_title){
			//~ #ifdef DEBUG_FREE_CONNECTION
				//~ printf("\tDeleting plot->x_title\n");fflush(stdout);
			//~ #endif
			//~ g_free(connection->plot->x_title);
		//~ }
		//~ if(connection->plot->y_title){
			//~ #ifdef DEBUG_FREE_CONNECTION
				//~ printf("\tDeleting plot->y_title\n");fflush(stdout);
			//~ #endif
			//~ g_free(connection->plot->y_title);
		//~ }
		//~ if(connection->plot->plot_bg_color){
			//~ #ifdef DEBUG_FREE_CONNECTION
				//~ printf("\tDeleting plot->plot_bg_color\n");fflush(stdout);
			//~ #endif
			//~ g_free(connection->plot->plot_bg_color);
		//~ }
		//~ if(connection->plot->legend_bg_color){
			//~ #ifdef DEBUG_FREE_CONNECTION
				//~ printf("\tDeleting plot->legend_bg_color\n");fflush(stdout);
			//~ #endif
			//~ g_free(connection->plot->legend_bg_color);
		//~ }

	//~ g_free(connection->plot);
	//~ connection->plot=NULL;
	//~ }
//~ #endif

	g_free(connection);
	connection=NULL;

#ifdef DEBUG_FREE_CONNECTION
	printf("free_connection()  ***** END *****\n");
#endif

}

/* Exit window (== destroy_handler)
 * This handler only release the focus.
 */
void
exit_window(GtkWidget *w, gpointer data)
{
  if(w){
	// gtk_grab_remove(w);
	gtk_widget_hide(w);
  }
}

void
destroy(GtkWidget *w, gpointer data)
{
	gtk_main_quit();
}

/* handler for "delete_events" (from window-manager, e.g. X-Button)
 * This handler free the structures contained in the node
 * and finally destroy the window. 
 * The destroy-handler then calls exit_window().
 */
gboolean 
delete_event(GtkWidget *w, GdkEvent *event, gpointer data)
{
  GList *found_it;
  // window_node_t *free_this_window;
  char *name;

#define DEBUG_DELETE_EVENT
#undef DEBUG_DELETE_EVENT

#ifdef DEBUG_DELETE_EVENT
	printf("delete event()\n");
#endif


  if(w){
    name = (char *)glade_get_widget_name(GTK_WIDGET(w));
    #ifdef DEBUG_DELETE_EVENT
        printf("delete event....%s\n",name);
    #endif
    //gtk_grab_remove(w);

    found_it =  g_list_find_custom(windows, name,(GCompareFunc)comp_window_name);
    if(found_it){
        #ifdef DEBUG_DELETE_EVENT
            printf("we found it\n");
        #endif
	// Hide the window instead of destroying it.
        gtk_widget_hide(w);

        //windows = g_list_remove_link(windows, found_it);
        //free_this_window = (window_node_t *)(found_it->data);

        //g_free(free_this_window->name);
        //g_list_foreach(free_this_window->connections, (GFunc)free_connection, NULL);
        //g_list_free(free_this_window->connections);
    }
    //gtk_widget_destroy(w);
  }
  return TRUE;
}


/* quit-Handler only for window "app1"
 *
 * Window "app1" must connect the signal delete_event with quit_handler.
 * In matplc.conf you have to create the point "quit_app1" and assign it to
 * plcshutdown.
 *
 * The quit-handler sets the point quit_app1 and free the structures
 * contained in the node.
 */
void
quit_handler(GtkWidget *w, gpointer data){
  char *name;
  plc_pt_t pt_quitapp1;
  GList *found_it;
  window_node_t *free_this_window;

#define DEBUG_QUIT_HANDLER
#undef DEBUG_QUIT_HANDLER

  #ifdef DEBUG_QUIT_HANDLER
	printf("quit_handler()\n");
  #endif

  pt_quitapp1 = get_pt("quit_app1");	

  #ifdef DEBUG_QUIT_HANDLER
  	printf(" --> setting quit_app1\n");
  #endif

  plc_set(pt_quitapp1,1);
  plc_update();
	
	
  if(w){
    name = (char *)glade_get_widget_name(GTK_WIDGET(w));
	#ifdef DEBUG_QUIT_HANDLER
	  printf("quit_handler()....%s\n",name);
	#endif
	gtk_grab_remove(w);
	
    found_it =  g_list_find_custom(windows, name,(GCompareFunc)comp_window_name);
    if(found_it){
	  #ifdef DEBUG_QUIT_HANDLER
	  	printf("we found it\n");
 	  #endif
      windows = g_list_remove_link(windows, found_it);
      free_this_window = (window_node_t *)(found_it->data);

      g_free(free_this_window->name);
      g_list_foreach(free_this_window->connections, (GFunc)free_connection, NULL);
      g_list_free(free_this_window->connections);

    }
	gtk_widget_destroy(w);
  }  
}


/* Run a new window dialog using the name in the data field */
void
run_window(GtkWidget *w, gpointer data)
{
  GladeXML *xml;
  GtkWidget *window;
  gchar *name;
  GList *found_it;
  window_node_t *wn;
  GtkWidget *widget;

#define DEBUG_RUN_WINDOW
#undef DEBUG_RUN_WINDOW

  #ifdef DEBUG_RUN_WINDOW
	printf("run_window()\n");
  #endif

  /* Search for if this window already exists */
  found_it = g_list_find_custom(windows, data,(GCompareFunc)comp_window_name);
  if(found_it){
    wn = found_it->data;
    widget = wn->widget;
    gtk_widget_show(widget);
  }
  else{
    /* We will use the name string for the window name in the window_node 
     * structure, so we will free it when the window is closed.
     */
    name = g_strdup((char *)data);
	#ifdef DEBUG_RUN_WINDOW
    	printf("Data %s\n", name);
	#endif
    /* load the about1 dialog */
    xml = glade_xml_new("hmi_gtk2.glade", name, NULL);
    /* in case we can't load the interface, bail */
    if(!xml) {
      g_warning("We could not load the interface!");
      return;
    }
    /* Set the event to dispose the window from the DList when the 
   * window is closed 
   */
    window = glade_xml_get_widget(xml, name);

    /* set the parent of the dialog to be the main window */
    /* gnome_dialog_set_parent(GNOME_DIALOG(dialog), GTK_WINDOW(app)); */

    if(window){

      new_window(xml, name, window);
  
      // Try hiding the window instead of destroying it. To avoid segfault.
      g_signal_connect(G_OBJECT(window), "delete_event",
			G_CALLBACK(delete_event), NULL);

      /* unref the xml file as it's not needed anymore */
      /* gtk_object_unref(GTK_OBJECT(xml)); */

      // GTK2 Complains about this way of destroying the xml data.
      //g_signal_connect_data(G_OBJECT(window), "destroy",
      //			      G_CALLBACK(exit_window),
      //			      xml, (GClosureNotify)g_object_unref,
      //			      FALSE);

      g_signal_connect(G_OBJECT(window), "destroy",
      			G_CALLBACK(exit_window), NULL);
      
      /* autoconnect any signals */
      glade_xml_signal_autoconnect(xml);
    }
  }
}

/* Fuctions to call 10 predefined windows from the menus */
void run_window1(GtkWidget *parent){
  char str[] = "window1";
  run_window(parent, str);
}

void run_window2(GtkWidget *parent){
  char str[] = "window2";
  run_window(parent, str);
}

void run_window3(GtkWidget *parent){
  char str[] = "window3";
  run_window(parent, str);
}

void run_window4(GtkWidget *parent){
  char str[] = "window4";
  run_window(parent, str);
}

void run_window5(GtkWidget *parent){
  char str[] = "window5";
  run_window(parent, str);
}

void run_window6(GtkWidget *parent){
  char str[] = "window6";
  run_window(parent, str);
}

void run_window7(GtkWidget *parent){
  char str[] = "window7";
  run_window(parent, str);
}

void run_window8(GtkWidget *parent){
  char str[] = "window8";
  run_window(parent, str);
}

void run_window9(GtkWidget *parent){
  char str[] = "window9";
  run_window(parent, str);
}

void run_window10(GtkWidget *parent){
  char str[] = "window10";
  run_window(parent, str);
}


/* Main program: initialize the plc, create the main window
 * and connect the gtk signals
 */
int
main(int argc, char *argv[])
{
  GladeXML *xml;
  GtkWidget *window;
  const char *module_name = "hmi_gtk2";
  char *name;

#define DEBUG_MAIN
//#undef DEBUG_MAIN

  #ifdef DEBUG_MAIN
	printf("main()\n");
  #endif

  /* We will use the name string for the window name in the window_node 
   * structure, so we will free it when the window is closed.
   */
  name = g_strdup("window1");

  if (plc_init(module_name, argc, argv) < 0) {
    printf("Error initializing PLC\n");
    return -1;
  }

  /* initialize the variables hash table */
  variables = g_hash_table_new ((GHashFunc)g_str_hash, (GCompareFunc)g_str_equal);

  /* initialize gnome */
  //gnome_init("hmi_gtk2", VERSION, argc, argv);
  gtk_init(&argc, &argv);

  
  /* initialize glade for gnome */
  //glade_gnome_init();

  /* Read from matplc.conf */
  //hmi_GtkImage_read_matplc_conf();

  /* Read from matplc.conf */
  hmi_GtkLabel_read_matplc_conf();

  /* load the main window (which is named app1) */
  /* run_window(NULL, "app1"); */

  #ifdef DEBUG_MAIN
	printf("before xml load\n");
  #endif  

  xml = glade_xml_new("hmi_gtk2.glade", name, NULL);
	//xml = glade_xml_new("hmi_gtk2.glade", NULL, NULL);
  /* in case we can't load the interface, bail */
  if(!xml) {
    g_warning("We could not load the interface!");
    return -1;
  }
	
  #ifdef DEBUG_MAIN
	printf("xml loaded\n");
  #endif  

  app = glade_xml_get_widget(xml, name);

  /* Set the event to dispose the window from the DList when the 
   * window is closed 
   */
  window = glade_xml_get_widget(xml, name);

  windows = NULL;
  new_window(xml, name, window);
  
  // GTK2 Complains about this way of destroying the xml data.
  //g_signal_connect_data(G_OBJECT(window), "destroy",
  //			G_CALLBACK(exit_window), xml, 
  //	 		(GClosureNotify)g_object_unref,
  //			FALSE);

  g_signal_connect(G_OBJECT(window), "destroy",
  		  G_CALLBACK(destroy), NULL);
  
  /* autoconnect any signals */
  glade_xml_signal_autoconnect(xml);

  /* run the main loop */
  if(!g_idle_add((GSourceFunc)idle, NULL)){
    printf("Error cannot add idle function\n");
    return -1;
  }
  /* printf("Before gtk_main()\n"); */
  gtk_main();
  /* printf("After gtk_main()\n"); */

  return 0;
}
