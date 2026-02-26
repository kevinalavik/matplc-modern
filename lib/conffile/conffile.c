/*
 * (c) 2000  Mario de Sousa
 *           Jiri Baum
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
 * Configuration File Reader - access library implementation
 *
 * This file implements the routines in conffile.h
 *
 * These routines are used to read and parse the data in
 * the matplc.conf file
 */

/* $Revision: 1.20 $ */


/* use GNU extensions - in particular the getline() function */
/* The GNU extensions are not available under QNX, so we provide the
 * source code the the extensions we are interested in 
 * under the lib/gnu directory.
 * In that case, we must include their definitions...
 */
#if defined(PLC_HOSTOS_QNX)
#include <gnu/gnu.h>
#else
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>  /* required for index() */
#include <ctype.h>
#include <wordexp.h>

#include "conffile.h"
#include <log/log.h>
#include <misc/ds_util.h>
#include <misc/string_util.h>



static const char COMMENT_CHAR = '#';

/*
 * The way the code works, a null is also effectively a comment character.
 * This may be fixed or broken in the future.
 */


static const int debug = 0;
/* #define TESTMAIN */


static const char * conffile_module_name_ = NULL;


/*
 * "variables" (ie, settings; they are not actually allowed to vary)
 *
 * Each entry is a char*.
 */
static dict*vars = NULL;

static inline int vars_new(void) {
  if (vars != NULL)
    return 0;    /* already initialised...*/
  if ((vars = dict_new()) == NULL)
    return -1;
  return 0;
}

static inline int vars_delete(void) {
  int i;

  if (vars == NULL)
    return 0;    /* already deleted... */
  for (i = 0; i < dict_len(vars); i++) {
    free(dict_key(vars, i));
    free(dict_value(vars, i));
  }
  dict_delete(vars);
  vars = NULL;
  return 0;
}




/*
 * "records" (settings which need a list of things; the prototypical
 * example being the "PLC:point" setting: there is any number of points,
 * and each has several fields).
 *
 * Each entry is a list*, each entry of that is a list*, and each entry of
 * that is a char*.
 */
static dict*recs = NULL;

static inline int recs_new(void) {
  if (recs != NULL)
    return 0;    /* already initialised...*/
  if ((recs = dict_new()) == NULL)
    return -1;
  return 0;
}

static inline int recs_delete(void) {
  int i,j,k;

  if (recs == NULL)
    return 0;    /* already deleted... */

  for (i = 0; i < dict_len(recs); i++) {
    list *table = dict_value(recs, i);
    for (j = 0; j < list_len(table); j++) {
      list *rec = list_item(table, j);
      for (k = 0; k < list_len(rec); k++) {
        free(list_item(rec, k));
      }
      list_delete(rec);
    }
    list_delete(table);
    free(dict_key(recs, i));
  }
  dict_delete(recs);
  recs = NULL;
  return 0;
}




/*
 * list of files that have been or are being read - used to break *include
 * cycles and avoid re-parsing a file more than once.
 *
 * Each entry is a const char*.
 */
static dict*files = NULL;

static inline int files_new(void) {
  if (files != NULL)
    return 0;    /* already initialised...*/
  if ((files = dict_new()) == NULL)
    return -1;
  return 0;
}

static inline int files_delete(void) {
  int i;

  if (files == NULL)
    return 0;    /* already deleted... */
  for (i = 0; i < dict_len(files); i++) {
    free(dict_key(files, i));
    /*
     NOTE: the following line must not be included!
           both the key and the value are one and the same pointer!
    free(dict_value(files, i));
    */
  }
  dict_delete(files);
  files = NULL;
  return 0;
}



/* skip whitespace (tabs, spaces and line-trailing comments) */
inline static void skipspace(char **work)
{
  *work += strspn(*work, " \t\f\v\r\n");
  if (**work == COMMENT_CHAR)
    *work += strlen(*work);
}

/* can this character be part of a word? */
inline static int iswordchar(const char c)
{
  return isalnum(c) || (c == '_') || (c & 0x80);
}

/*
 * functions to grab a syntactic element, advancing *work to point just
 * past it. Space for the element is allocated using malloc(). If *work
 * doesn't start with the requested type of element, returns NULL. Each of
 * these functions will skip trailing space, but not leading space. Escapes
 * for quotes and backslashes are handled, all other backslashes pass
 * through.
 *
 * Buglet: when there are escaped characters, the allocation will be
 * excessive (by the number of handled escapes).
 */

/* utility fn for the other get... functions */
inline static char *getspan_(char **start, char *end)
{
  char *res, *tmp;
  if (*start >= end)
    return NULL;
  tmp = res = malloc(end - *start + 1);
  for (; *start < end; (*start)++, tmp++) {
    if ((**start == '\\')
	&& ((*(*start + 1) == '\\') || (*(*start + 1) == '"'))) {
      (*start)++;
    }
    *tmp = **start;
  }
  *tmp = '\0';
  skipspace(start);
  return res;
}


/* get a quoted string */
static char*getquoted(char **work)
{
  char *tmp=(*work)+1;
  if (**work != '"')
    return NULL;
  while (*tmp!='"') {
    if (*tmp=='\\')
      tmp+=2;
    else if (*tmp=='\0')
      return NULL; /* unterminated string */
    else
      tmp++;
  }
  (*work)++; /* skip initial quote */
  if (tmp==*work) /* empty string is OK */
    tmp=strdup("");
  else
    tmp=getspan_(work,tmp);
  (*work)++; /* skip ending quote */
  skipspace(work);
  return tmp;
}


/* get a word, which is an identifier-like thing (unless it's quoted) */
static char *getword(char **work)
{
  char *tmp = *work;
  if (**work == '"')
    return getquoted(work);
  while (*tmp) {
    if (*tmp == '\\')
      tmp += 2;
    else if (iswordchar(*tmp))
      tmp++;
    else
      break;
  }
  return getspan_(work, tmp);
}


/* get a value, which is basically non-whitespace (or quoted) */
static char*getvalue(char **work)
{
  char *tmp=*work;
  if (**work == '"')
    return getquoted(work);
  while (*tmp) {
    if (*tmp=='\\')
      tmp+=2;
    else if (isgraph(*tmp) || (*tmp&0x80))
      tmp++;
    else
      break;
  }
  return getspan_(work,tmp);
}


static char* get_record(FILE*f)
/*
 * This function uses malloc to allocate memory for the record - calling
 * function must de-allocate memory with free(). Note: the space allocated
 * may be excessive if the line is preceded by longer comments.
 */
{
/* At the moment does not yet allow records to     */
/* span multiple lines by using the '\' character. */

  int res = 0;
  size_t rec_len = 0;
  char *record = NULL, *tmp;

  if (debug)
    printf("conffile_get_record(): ...\n");

  if (!f)
    return NULL;

  do {
    res = getline(&record, &rec_len, f);

    if (res < 0) {
      if (record)
	free(record);
      return NULL;
    }

    /* see if there's anything in there other than blanks and comments */
    tmp=record;
    skipspace(&tmp);
  } while (!*tmp);

  /*
   * unfortunately, we can't return the record without the blanks, because
   * then it wouldn't be any good for free()
   */
  return record;
}


/* util function for accessing recs */
static list *get_tableptr(const char *section, const char *name)
{
  char *fullname;
  list *table;

  if (debug)
    printf("get_tableptr(%s,%s): ...\n",section,name);

  if (!recs) {
    /*
    printf("Must call conffile_init() before conffile_get_table()\n");
    exit(1);
    */
    return NULL;
  }

  if (!section || !name)
    return NULL;

  fullname = strdup3(section, ":", name);
  table = dict_get(recs, fullname);
  free(fullname);
  return table;
}


/* Public access to the recs */
char *conffile_get_value_sec(const char *section, const char *name)
{
  char *fullname, *foundval;

  if (debug)
    printf("conffile_get_value_sec(%s, %s): ...\n", section, name);

  if (!vars) {
    /*
    printf("Must call conffile_init() before conffile_get_value()\n");
    exit(1);
    */
    return NULL;
  }

  if (!section || !name)
    return NULL;

  fullname=strdup3(section, ":", name);
  if (fullname == NULL) {
    /* Yikes!! we are running out of memory, even for a small malloc as this! */
    /* If we return a NULL, the program will assume the value was not found
     * in the config file, and will go happily on it's way. If it does not
     * require any more memory, the program will be executing but incorrectly
     * configured! If it does require more memory, the error will eventually
     * show up elsewhere.
     * Either way, we are probably better off just quiting here and now!
     */
    perror("out of memory!");
    exit(EXIT_FAILURE);
  }
  foundval=dict_get(vars, fullname);
  free(fullname);

  if (foundval) {
    return strdup(foundval);
  } else {
    return NULL;
  }
}


char *conffile_get_table_sec(const char *section, const char *name,
			     int r, int c)
{
  list *table, *row;
  char *cell;

  table = get_tableptr(section, name);
  if (!table)
    return NULL;
  row = list_item(table, r);
  if (!row)
    return NULL;
  cell = list_item(row, c);
  if (!cell)
    return NULL;
  if (debug)
    printf("get_table_sec(%s,%s,%d,%d)->%s\n", section, name, r, c, cell);
  return strdup(cell);
}


int conffile_get_table_rows_sec(const char *section, const char *name)
{
  list *table;

  table = get_tableptr(section, name);
  if (!table)
    return 0;
  return list_len(table);
}


int conffile_get_table_rowlen_sec(const char *section, const char *name,
				  int r)
{
  list *table, *row;

  table = get_tableptr(section, name);
  if (!table)
    return 0;
  row = list_item(table, r);
  if (!row)
    return 0;
  if (debug)
    printf("get_table_rowlen_sec(%s,%s,%d) -> %d\n",
	   section, name, r, list_len(row));
  return list_len(row);
}


/* here come all the implicit-section versions */
char *conffile_get_value(const char *name)
{
  return conffile_get_value_sec(conffile_module_name_, name);
}

char *conffile_get_table(const char *name, int row, int col)
{
  return conffile_get_table_sec(conffile_module_name_, name, row, col);
}

int conffile_get_table_rows(const char *name)
{
  return conffile_get_table_rows_sec(conffile_module_name_, name);
}

int conffile_get_table_rowlen(const char *name, int row)
{
  return conffile_get_table_rowlen_sec(conffile_module_name_, name, row);
}


/* getting number values */
#define __conffile_get_value_type(type, mod_name, name,                    \
                                  val, min_v, max_v, def_v)                \
{                                                                          \
 char *tmp_str;                                                            \
 int  ret_val;                                                             \
                                                                           \
 *(val) = def_v;                                                           \
                                                                           \
 if ((tmp_str = conffile_get_value_sec ((mod_name), (name)))               \
     == NULL)                                                              \
   return 0;                                                               \
                                                                           \
 ret_val  = string_str_to_ ## type (tmp_str, (val), (min_v), (max_v));     \
 free (tmp_str);                                                           \
 return ret_val;                                                           \
}


int conffile_get_value_i32(const char *name,
                           i32 *val, i32 min_val, i32 max_val, i32 def_val)
{__conffile_get_value_type(i32,conffile_module_name_,name,
                           val,min_val,max_val,def_val);}

int conffile_get_value_u32(const char *name,
                           u32 *val, u32 min_val, u32 max_val, u32 def_val)
{__conffile_get_value_type(u32,conffile_module_name_,name,
                           val,min_val,max_val,def_val);}

int conffile_get_value_f32(const char *name,
                           f32 *val, f32 min_val, f32 max_val, f32 def_val)
{__conffile_get_value_type(f32,conffile_module_name_,name,
                           val,min_val,max_val,def_val);}

int conffile_get_value_d  (const char *name,
                           double *val, double min_val, double max_val, double def_val)
{__conffile_get_value_type(d ,conffile_module_name_,name,
                           val,min_val,max_val,def_val);}



int conffile_get_value_sec_i32(const char *module_name, const char *name,
                               i32 *val, i32 min_val, i32 max_val, i32 def_val)
{__conffile_get_value_type(i32,module_name,name,
                           val,min_val,max_val,def_val);}

int conffile_get_value_sec_u32(const char *module_name, const char *name,
                               u32 *val, u32 min_val, u32 max_val, u32 def_val)
{__conffile_get_value_type(u32,module_name,name,
                           val,min_val,max_val,def_val);}

int conffile_get_value_sec_f32(const char *module_name, const char *name,
                               f32 *val, f32 min_val, f32 max_val, f32 def_val)
{__conffile_get_value_type(f32,module_name,name,
                           val,min_val,max_val,def_val);}

int conffile_get_value_sec_d  (const char *module_name, const char *name,
                               double *val,    double min_val,
                               double max_val, double def_val)
{__conffile_get_value_type(d ,module_name,name,
                           val,min_val,max_val,def_val);}



#define __conffile_get_table_type(type,                                    \
                                  mod_name, name, row, col,                \
                                  val, min_v, max_v, def_v)                \
{                                                                          \
 char *tmp_str;                                                            \
 int  ret_val;                                                             \
                                                                           \
 *(val) = def_v;                                                           \
                                                                           \
 if ((tmp_str = conffile_get_table_sec ((mod_name), (name), (row), (col))) \
     == NULL)                                                              \
   return 0;                                                               \
                                                                           \
 ret_val  = string_str_to_ ## type (tmp_str, (val), (min_v), (max_v));     \
 free (tmp_str);                                                           \
 return ret_val;                                                           \
}


int conffile_get_table_i32(const char *name, int row, int col,
                           i32 *val, i32 min_val, i32 max_val, i32 def_val)
{__conffile_get_table_type(i32,conffile_module_name_,name,row,col,
                           val,min_val,max_val,def_val);}

int conffile_get_table_u32(const char *name, int row, int col,
                           u32 *val, u32 min_val, u32 max_val, u32 def_val)
{__conffile_get_table_type(u32,conffile_module_name_,name,row,col,
                           val,min_val,max_val,def_val);}

int conffile_get_table_f32(const char *name, int row, int col,
                           f32 *val, f32 min_val, f32 max_val, f32 def_val)
{__conffile_get_table_type(f32,conffile_module_name_,name,row,col,
                           val,min_val,max_val,def_val);}

int conffile_get_table_sec_i32(const char *module_name, const char *name,
                               int row, int col,
                               i32 *val, i32 min_val, i32 max_val, i32 def_val)
{__conffile_get_table_type(i32,module_name,name,row,col,
                           val,min_val,max_val,def_val);}

int conffile_get_table_sec_u32(const char *module_name, const char *name,
                               int row, int col,
                               u32 *val, u32 min_val, u32 max_val, u32 def_val)
{__conffile_get_table_type(u32,module_name,name,row,col,
                           val,min_val,max_val,def_val);}

int conffile_get_table_sec_f32(const char *module_name, const char *name,
                               int row, int col,
                               f32 *val, f32 min_val, f32 max_val, f32 def_val)
{__conffile_get_table_type(f32,module_name,name,row,col,
                           val,min_val,max_val,def_val);}



void conffile_parse_i32(const char *section, const char *name,
                        i32 *var, i32 min, i32 max, i32 def,
                        int use_default)
{
  int res = 0;

  if (!use_default)
    def = *var;

  if (section == NULL)
    res = conffile_get_value_i32(name, var, min, max, def);
  else
    res = conffile_get_value_sec_i32(section, name, var, min, max, def);

  if ((res < 0) && (use_default)) {
    plc_log_wrnmsg(1, "cannot understand %s = %s,"
                   " or value out of bounds [%d...%d]. Using default %d",
                   name, conffile_get_value(name), min, max, *var);
  }

  if ((res < 0) && (!use_default)) {
    plc_log_wrnmsg(1, "cannot understand %s = %s,"
                   " or value out of bounds [%d...%d].",
                   name, conffile_get_value(name), min, max);
  }
}


char *conffile_var_by_index(int number){
/* added by Hugh Jack to be able to list global variables */
	if((number >= 0) && (number < dict_len(vars))){
		return dict_key(vars, number);
	}

	return NULL;	/* number outside range */
}




/* conffile_dump() is mostly for debugging */
void conffile_dump(void)
{
  int i,j,k;

  printf("Config read from:\n");
  for(i=0; i<dict_len(files); i++) {
    printf("%s\n", dict_key(files, i));
  }

  printf("\nVariables:\n");
  for(i=0; i<dict_len(vars); i++) {
    printf("`%s' is `%s'\n", dict_key(vars, i), (char*)dict_value(vars, i));
  }

  printf("\nTables:\n");
  for(i=0; i<dict_len(recs); i++) {
    list *table=dict_value(recs, i);
    printf("`%s':\n", dict_key(recs, i));
    for (j=0; j<list_len(table); j++) {
      list *rec=list_item(table, j);
      printf(" ");
      for(k=0; k<list_len(rec); k++) {
	printf(" `%s'", (char*)list_item(rec, k));
      }
      printf("\n");
    }
  }

  printf("\n===\n");

}


static int parse_file(char *filename);
static void parse_line(char *line, char **section);

/*
 * this is just a stub routine for now; eventually it should print the
 * filename and line number and (if relevant) the include-traceback.
 */
static const char *cur_filename = "(unknown file)";
static int parse_error(const char *msg)
{
  printf("Problem understanding the config %s: %s\n", cur_filename, msg);
  if (debug) conffile_dump();
  return -1;
}

static inline void must_end(char *line)
{
  if (line[0])
    parse_error("Unexpected extra stuff at end of line");
}


static void parse_var(char *name, char *line, char *section)
{
  char *fullname, *oldval, *value = getvalue(&line);
  must_end(line);

  if (!section)
    parse_error("Setting outside a section!");

  fullname = strdup3(section, ":", name);

  oldval = dict_get(vars, fullname);

  if (oldval) {
    if (strcmp(oldval, value))
      parse_error("Two different settings for the same variable");
    else {
      /* same value - that's OK */
      free(fullname);
      free(value);
    }
  } else {
    dict_add(vars, fullname, value);
  }
  free(name);
}

static void parse_rec(char *name, char *line, const char *section)
{
  char *fullname;
  list *rec, *table;

  if (!section) {
    parse_error("Record outside a section!");
    section = "(no section)";
  }

  if (debug)
    printf("parse_rec(%s,...,%s)\n", name, section);

  fullname = strdup3(section, ":", name);

  /* get the record out of the line */
  rec = list_new();
  while (line[0]) {
    char *field = getvalue(&line);
    if (!field) {
      parse_error("Invalid field format in a table");
      break;
    }
    list_append(rec, field);
  }

  if (debug)
    printf("got %d fields\n", list_len(rec));

  /* check if that table already exists */
  table = dict_get(recs, fullname);
  if (table) {
    /* exists, add to it */
    list_append(table, rec);
    /* we will not be needing fullname anymore */
    free(fullname);
    if (debug)
      printf("table exists, added to it, now has %d rows\n", list_len(table));
  } else {
    /* doesn't exist, create it */
    table=list_new();
    list_append(table, rec);
    dict_add(recs,fullname,table);
    /* dict_add() will keep a reference to fullname, so we do not free it */
    /* it will later be free by recs_delete() */
    if (debug)
      printf("created table\n");
  }

  free(name);
}

static void parse_tempsect(char *sect, char *line) {
  parse_line(line, &sect);
  free(sect);
}

static void parse_section(char *line, char **section) {
  if (*section)
    free(*section);
  *section=getword(&line);
  if (line[0] != ']')
    parse_error("expected `]' after section name");
  if (index(*section,':'))
    parse_error("found `:' in section name (sections may not have colons)");
  line++;
  skipspace(&line);
  must_end(line);
}

static void parse_directive(char *line)
{
  int res;
  char *keyword;
  skipspace(&line);

  keyword = getword(&line);

  if (!strcmp(keyword, "include")) {
    wordexp_t fnames;
    int i;
    char *filename = getvalue(&line);
    must_end(line);

    res = wordexp(filename, &fnames, WRDE_SHOWERR|WRDE_NOCMD);
    switch (res) {
      case 0: 
        /* Expansion succesfull. Let's continue... */
        for(i=0;i<fnames.we_wordc;i++)
          parse_file(strdup(fnames.we_wordv[i]));
          /* must use strdup, because parse_file keeps a pointer to the name */
        free(filename);
        wordfree(&fnames);
        break; 
      case -1: 
        /* QNX should be reporting error ENOSYS, since wordexp() is not suported.
         * But it doesn't, even though the help/manual pages says it should!
         * We therefore disable the test condition for now...
         * Hopefully it won't break too many things under Linux!
         */
        /*if (errno == ENOSYS)*/
          /* wordexp() not supported. We will have to do without. Let's continue... */
          parse_file(strdup(filename));
          /* do not free the string, because parse_file keeps a pointer to the name */
          /* free(filename); */
          break;
        /* else Fall through to error... */  
      default:  
        parse_error("shell expansion failed for *include");
    } /* switch() */  
  } else {
    parse_error("Unknown directive");
  }

  free(keyword);
}

static void parse_line(char *line, char **section)
{
  skipspace(&line);

  if (debug)
    printf("About to parse line %s", line);

  if (iswordchar(line[0])) {
    char *word = getword(&line);
    if (iswordchar(line[0]) || (line[0]=='"')) {
      parse_rec(word, line, *section);   /* this funct. will free(word) */
    } else {
      char symbol = line++[0];
      skipspace(&line);
      switch (symbol) {
      case '=':
	parse_var(word, line, *section); /* this funct. will free(word) */
	break;
      case ':':
	parse_tempsect(word, line);      /* this funct. will free(word) */
	break;
      default:
        free(word);
	parse_error("unknown character after first word");
      }
    }
  } else {
    switch (line[0]) {
    case '[':
      parse_section(line + 1, section);
      break;
    case '*':
      parse_directive(&(line[1]));
      break;
    default:
      parse_error("line starts with unknown character");
    }
  }
}

static int parse_file(char *filename)
{
  FILE *f;
  char *line;
  char *section = NULL;
  const char *old_curfilename = cur_filename;

  cur_filename = filename;

  if (debug)
    printf("parse_file(%s)\n",filename);

  if (dict_get(files, filename)) {
    return 0;
  }
  dict_add(files, filename, filename);

  f = fopen(filename, "r");

  if (f == NULL) {
    parse_error("could not open file");
    return -1;
  }

  do {
    line = get_record(f);
    if (line) {
      parse_line(line, &section);
      free(line);
    }
  } while (line);
  free(section);

  if (fclose(f))
    perror("closing config file");

  cur_filename = old_curfilename;

  return 0;
}

static inline int parse_config(const char *conffile)
{
  char *filename;

  /* We cannot pass a const char * to parse_file()
   * since it will keep a reference to this pointer
   * which will later on be free()d in files_delete()
   *
   * So we create our own char * copy to pass to parse_config()
   */

  filename = strdup(conffile);
  if (filename == NULL)
    goto error_exit_0;

  if ((vars = dict_new()) == NULL)
    goto error_exit_1;

  if (recs_new() < 0)
    goto error_exit_2;

  if (files_new() < 0)
    goto error_exit_3;

  if (parse_file(filename) < 0)
    goto error_exit_4;

  return 0;

error_exit_4:
  files_delete();
error_exit_3:
  recs_delete();
error_exit_2:
  vars_delete();
error_exit_1:
  /* we do NOT free(filename) since parse_file() keeps a reference to it! */
error_exit_0:
  return -1;
}

int conffile_init(const char *conffile, const char *mod_name)
{
  if (vars) {
    printf("conffile_init() called again!\n");
  }

  conffile_module_name_ = mod_name;

  if (parse_config(conffile) < 0)
    return -1;

  return 0;
}

int conffile_done(void)
{
  files_delete();
  recs_delete();
  vars_delete();

  conffile_module_name_ = NULL;

  return 0;
}

/*
 *  This function is for the use of the config editor only, so its
 *  interface and actions can change at its convenience. If anyone else
 *  uses it, be sure to note here and in the .h file.
 */

void conffile_load_file(const char *filename, dict ** _vars, dict ** _recs,
			dict ** _files)
{
  dict *oldvars, *oldrecs, *oldfiles;

  oldvars = vars; oldrecs = recs; oldfiles = files;
  vars = recs = files = NULL;

  parse_config(filename);

  *_vars=vars; *_recs=recs; *_files=files;

  vars = oldvars; recs = oldrecs; files = oldfiles;
}

#ifdef TESTMAIN

int main(void)
{
  parse_config(DEF_CONFIG_FILE);
  conffile_dump();

  return 0;
}

#endif
