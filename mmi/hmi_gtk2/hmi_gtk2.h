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
 * TODO
 *
 * Add new widgets:
 *   Add more widgets for analog or digital input and output. (Button, Scroller, etc)
 *   An image widget that will display one of two images depending on the digital value.
 *   Gauge widget.
 *
 * Write errors to the plc log.
 *
 * When trying to open an already oppened window send it to the foreground
 *
 * Pre-assign the widget types to variables to optimize execution time.
 */

/*
 * Functions that need to be edited or created to add support for a new widget:
 *
 *   x_widget - Where x is the type of the new widget. Update the value
 *             from the plc or variable to the widget.
 *   parse_connection - Add a case for the new type and call xWidget
 * 
 * In case the widget is for data entry also edit:
 *
 *   x_widget_update - Add the variable update from the new entered value.
 *   updateValue - Add a case for the new type and call xWidgetUpdate
 *
 * Note: the new widget should be supported by glade and libglade, otherwise
 * more changes should be made (to glade, libglade).
 */

/* DEBUGING */
#define DEBUG_hmi_GtkLabel_read_matplc_conf
#undef DEBUG_hmi_GtkLabel_read_matplc_conf

#define DEBUG_init_widget_data
#undef DEBUG_init_widget_data

#define DEBUG_hmi_GtkImage_read_matplc_conf
#undef DEBUG_hmi_GtkImage_read_matplc_conf

/* define version ourselves, really this should be defined by the build
   process if we use autoconf/automake */
#define VERSION "2.1"

/* The strings in the config file used to define what format the data is in */
#define  f32_TYPE "f32"
#define  i32_TYPE "i32"
#define  u32_TYPE "u32"
#define  i16_TYPE "i16"
#define  u16_TYPE "u16"
#define   i8_TYPE "i8"
#define   u8_TYPE "u8"
#define bool_TYPE "bool"

#define BUFF_SIZE 128

typedef enum {bool_dt, i32_dt, u32_dt, i16_dt, u16_dt, i8_dt, u8_dt, f32_dt} data_type_t;

	typedef enum {false=0,true=1} t_boolean ;

/* data type to store GtkImage config data */
typedef struct{
	int	nbImage;
	char *widgetpattern;
	char **Image;
} hmi_GtkImage_tab_t;

typedef struct{
	int	nbImageTab;
	hmi_GtkImage_tab_t *ImageTab;
} hmi_GtkImage_t;
	
/* data type to store GtkLabel config data */
typedef struct{
	char *widgetpattern;
	char *formatstring;
} hmi_GtkLabel_tab_t;

typedef struct{
	int nbLabelTab;
	hmi_GtkLabel_tab_t *LabelTab;
} hmi_GtkLabel_t;

/* data type to store variables */
typedef struct{
  data_type_t type;
  u32 value;
} variable_t;

/* data required to relate a widget with a plc point to form a connection */
typedef struct {
  GtkWidget *widget;
  plc_pt_t pt;
  variable_t *variable; /* Only data entry fields will use this field */
  data_type_t data_type;
  
  /* Used by ImageWidget */
  char *on;								/* for ON state */
  char *off;							/* for OFF state */
  hmi_GtkImage_tab_t *ImageTab;		/* reference to image table */
  hmi_GtkLabel_tab_t *LabelTab;		/* reference to label table */
  GList *OptionMenuTab;					/* reference to the OptionMenu table */ 
} connection_t;

/* data required to relate a window with a list of connections to the plc */
/* Added a pointer to the window widget */
typedef struct {
  gboolean first_scan;
  char *name;
  GList *connections;
  GtkWidget *widget;
} window_node_t;

/* this structure is used to search a window in a DList */
typedef struct{
  gboolean found_it;
  char *name;
} search_window_t;

typedef struct{
	int			pv;
	plc_pt_t 	pt;
}optionmenu_t;


/* The window list pointer */
GList *windows;

/* The variable list to allow several input widgets for the same PLC point 
 * this table will point to u32 values which will represent the values to
 * update the PLC
 */
GHashTable *variables;

/* a global pointer to the main window */
static GtkWidget *app;

/* Global Config Data for GtkImage */
static hmi_GtkImage_t hmi_GtkImage;

/* Global Config Data for GtkLabel*/
static hmi_GtkLabel_t hmi_GtkLabel;

/* prototypes */
void do_nothing(GtkWidget *w);
void add_number(GtkWidget *w, gpointer data);
void run_window(GtkWidget *w, gpointer data);
void update_value(GtkWidget *w, gpointer data); 
void update_optionmenu(GtkWidget *w, gpointer data); 
plc_pt_t get_pt(const char *pt_name);
void label_digital_out(GtkWidget *w, gboolean state, char *on, char *off);
void load_gtk_image(GtkWidget *widget, char *name);
void label_widget(connection_t *connection);
void progress_widget(connection_t *connection);
void gtk_image_widget(connection_t *connection);
void toggle_button_widget(connection_t *connection);
void button_widget(connection_t *connection);
void entry_widget(connection_t *connection);
void entry_widget_init(connection_t *connection);
void scale_widget(connection_t *connection);
void scale_widget_init(connection_t *connection);
void spin_button_widget(connection_t *connection);
void spin_button_widget_init(connection_t *connection);

//#ifdef GTKEXTRA
//void hmi_GtkPlotCanvas_check_sections(connection_t *connection);
//void hmi_GtkPlotCanvas_set_defaults(connection_t *connection);
//void hmi_GtkPlotCanvas_check_dependence(connection_t *connection);
//void hmi_GtkPlotCanvas_read_matplc_conf(connection_t *connection);
//void hmi_GtkPlotCanvas_widget(connection_t *connection);
//void hmi_GtkPlotCanvas_widget_init(connection_t *connection);
//GtkWidget *hmi_GtkPlotCanvas(gchar *widgetname);
//#endif

void hmi_GtkImage_read_matplc_conf(void);
void hmi_GtkLabel_read_matplc_conf(void);
void option_menu_widget_init(connection_t *connection);

//#ifdef GTKSOCKET
//#define DEBUG_GTKSOCKET
//#undef DEBUG_GTKSOCKET
//void socket_widget_init(connection_t *connection);
//GtkWidget *hmi_GtkSocket(gchar *widgetname);
//#endif

void parse_connection(gpointer a_connection, gpointer data);
void parse_window(gpointer window, gpointer data);
gboolean idle(gpointer data);
variable_t *add_variable(char *name, data_type_t type);
void init_widget_data(gpointer widget, gpointer user_data);
void toggle_button_widget_update(GtkWidget *w, variable_t *variable);
void button_widget_update(GtkWidget *w, variable_t *variable);
void entry_widget_update(GtkWidget *w, variable_t *variable);
void scale_widget_update(GtkWidget *w, variable_t *variable);
void spin_button_widget_update(GtkWidget *w, variable_t *variable);
void update_value(GtkWidget *w, gpointer data);
void update_optionmenu(GtkWidget *w, gpointer data);
void reset_value(GtkWidget *w, gpointer data);
void new_window(GladeXML *xml, char *name, GtkWidget *widget);
void on_about1_activate(GtkWidget *w);
gint comp_window_name(gconstpointer window1, gconstpointer name1);
void exit_window(GtkWidget *w, gpointer data);
void destroy(GtkWidget *w, gpointer data);
gboolean delete_event(GtkWidget *w, GdkEvent *event, gpointer data);
void quit_handler(GtkWidget *w, gpointer data);
void run_window(GtkWidget *w, gpointer data);
void run_window1(GtkWidget *parent);
void run_window2(GtkWidget *parent);
void run_window3(GtkWidget *parent);
void run_window4(GtkWidget *parent);
void run_window5(GtkWidget *parent);
void run_window6(GtkWidget *parent);
void run_window7(GtkWidget *parent);
void run_window8(GtkWidget *parent);
void run_window9(GtkWidget *parent);
void run_window10(GtkWidget *parent);
