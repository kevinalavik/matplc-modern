/*
 * (c) 2000 Mario de Sousa
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
 * This file implements the functions in
 *   synch_setup.h
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "synch.h"
#include "synch_private.h"
#include "synch_setup.h"
#include "synch_sem.h"

#include <misc/mutex_util.h>
#include <misc/string_util.h>
#include <conffile/conffile.h>
#include <cmm/cmm.h>
#include <log/log.h>
#include <state/state.h>  /* required for plc_module_max_count(); */


static int debug = 0;

/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****          auxiliary functions that implement          ****/
/****               Generic Data Structures                ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/

/**********************************/
/*                                */
/* A simple variable-sized array  */
/*  of arbitary data types.       */
/*                                */
/**********************************/

/* interface that member data types must support */
#define list_member_i			\
  void (*member_free)(void *);		\
  int (*member_comp)(char *, void *);	\
  int (*member_print)(FILE *, void *);

/* virtual function table */
typedef struct {
  list_member_i;
} list_member_vf_table_t;


typedef struct {
  list_member_i;
  int count;
  void **data;
} list;

typedef list * list_t;

static inline list_t list_new(list_member_vf_table_t list_member_vf_table)
{
  list *lst = malloc(sizeof(list));

  if (lst == NULL)
    return NULL;

  lst->member_free  = list_member_vf_table.member_free;
  lst->member_comp  = list_member_vf_table.member_comp;
  lst->member_print = list_member_vf_table.member_print;
  lst->count = 0;
  lst->data = NULL;
  return lst;
}


static inline void list_free(list_t lst)
{
  int index;

  if (lst == NULL)
    return;

  for (index = 0; index < lst->count; index++)
    lst->member_free(lst->data[index]);

  free (lst);
}


static inline int list_append(list_t lst, void *item)
{
  void *tmp_ptr = realloc(lst->data, lst->count * sizeof(void *));
  if (tmp_ptr == NULL)
    return -1;

  lst->count++;
  lst->data = realloc(lst->data, lst->count * sizeof(void *));
  lst->data[lst->count - 1] = item;

  return 0;
}

static inline int list_len(list_t lst)
{
  return lst->count;
}

static inline void *list_item(list_t lst, int index)
{
  if ((index < 0) || (index >= lst->count))
    return NULL;
  return lst->data[index];
}


static int list_index(list_t lst, void *item)
{
  int index;

  for (index = 0; index < lst->count; index++)
    if (lst->member_comp(item, lst->data[index]) == 0)
      return index;

  return -1;
}


static int list_print(list_t lst, FILE *stream)
{
  int index;

  for (index = 0; index < lst->count; index++)
    lst->member_print(stream, lst->data[index]);

  return 0;
}


/**********************************/
/*                                */
/* Functions for string data type */
/*  to be used in the list        */
/*                                */
/**********************************/
 
static int string_member_print(FILE *stream, char *str)
{
    return fprintf(stream, "%s\n", str);
}

static list_member_vf_table_t string_vf_table = {
  			&free, 
  			(int (*)(char *, void *))&strcmp,
  			(int (*)(FILE *, void *))&string_member_print
                      };


/**********************************/
/*                                */
/* A simple matrix of integers.   */
/*                                */
/**********************************/

typedef struct {
  int columns;
  int lines;
  int *data;
} matrix;

typedef matrix * matrix_t;


static inline matrix_t matrix_new(int columns, int lines, int init_value)
{
  int index;
  matrix_t tmp_matrix = NULL; 

  if ((tmp_matrix = (matrix_t)malloc(sizeof(matrix))) == NULL)
    goto error_exit_0;

  if ((tmp_matrix->data = malloc(sizeof(int) * lines * columns)) == NULL)
    goto error_exit_1;

  tmp_matrix->lines = lines;
  tmp_matrix->columns = columns;

  for (index = 0; index < lines * columns; index++) {
    tmp_matrix->data[index] = init_value;
  }

  return tmp_matrix; 

error_exit_1:
  free(tmp_matrix);

error_exit_0:
  return NULL;
}

static inline void matrix_free(matrix_t m)
{
  if (m == NULL)
    return;

  free(m->data);
  free(m);
}


static inline int matrix_get(matrix_t matrix, int column, int line)
{
  return matrix->data[line * matrix->columns + column];
}


static inline int matrix_set(matrix_t matrix, int column, int line, int value)
{
  return (matrix->data[line * matrix->columns + column] = value);
}


static int matrix_print(matrix_t matrix, FILE *stream)
{
  int c_index, l_index;

  for (l_index = 0; l_index < matrix->lines; l_index++) {
    for (c_index = 0; c_index < matrix->columns; c_index++) {
      fprintf(stream, "%d  ", matrix_get(matrix, c_index, l_index));
    }
    fprintf(stream, "\n");
  }

  return 0;
}



/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****  Data Structure that completely defines a petri net  ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/

#define new(var_type) var_type ## _new(NULL)


typedef struct petri_net_t {

/* public: */
/*petri_net_t (*new)(struct petri_net_t *this); */
  void (*delete)(struct petri_net_t *this);
  void (*print) (struct petri_net_t *this, FILE *stream);
  int  (*load_from_config)(struct petri_net_t *this);

  list_t    places;
  list_t    transitions;

    /* NOTE: A list_t cannot contain int's, so we use a one line matrix */
  matrix_t  init_tokens;

    /* NOTE: we do not use a single matrix for arcs both leaving
     *       and arriving at a transition (negative and positive
     *       weights) because we might have both of these arcs
     *       on the same transition, and it is not possible to
     *       have both positive and negative values in the same
     *       location of the matrix!
     *
     * We therefore use two matrixes, one for the arcs arriving
     * at the transitions (the in_arc_weights), and another for
     * the arcs departing from a transition (the out_arc_weights)
     *
     * Both these arcs will have 0 when no arc is present, and the
     * weight of the arc (always > 0) when an arc is present.
     */
    /* one column for each place, one line for each transition... */
    /* 0 -> no arc present...                                     */
  matrix_t  in_arc_weights;
  matrix_t  out_arc_weights;

    /* one column for each place, one line for each transition... */
    /* 0 -> no null arc                                           */
    /* 1 -> null arc from place to transition                     */
  matrix_t  null_arcs;

} petri_net_t;


/* forward declarations */
static inline void petri_net_t_delete(petri_net_t *this);
static        void petri_net_t_print (petri_net_t *this, FILE *stream);
static        int  petri_net_t_load_from_config(petri_net_t *this);



static inline petri_net_t petri_net_t_new(petri_net_t *this) {

  petri_net_t local_petri_net;

  if (this == NULL)
    this = &local_petri_net;

/*this->new = petri_net_t_new; */
  this->delete = petri_net_t_delete;
  this->print  = petri_net_t_print;
  this->load_from_config = petri_net_t_load_from_config;

  this->places          = NULL;
  this->transitions     = NULL;
  this->init_tokens     = NULL;
  this->in_arc_weights  = NULL;
  this->out_arc_weights = NULL;
  this->null_arcs       = NULL;

  return *this;
}


static inline void petri_net_t_delete(petri_net_t *this) {

  if (this == NULL)
    return;

  list_free(this->places);
  list_free(this->transitions);
  matrix_free(this->init_tokens);
  matrix_free(this->in_arc_weights);
  matrix_free(this->out_arc_weights);
  matrix_free(this->null_arcs);
}



/* forward declaration */
static int synch_get_simple_synch(petri_net_t *pnet);
static int synch_get_places(list_t *places);
static int synch_get_init_tokens(list_t places, matrix_t *init_tokens);
static int synch_get_transitions(list_t *trans);
static int synch_get_arcs(petri_net_t *pnet);

static int petri_net_t_load_from_config(petri_net_t *this)
{
  int num_sem, num_trans;

  if (synch_get_simple_synch(this) < 0)
    goto error_exit_0;

  if ((num_sem = list_len(this->places)) > 0)
    return num_sem;

  if ((num_sem = synch_get_places(&(this->places))) < 0)
    goto error_exit_0;

  if (num_sem > 0) {
    if (synch_get_init_tokens(this->places, &(this->init_tokens)) < 0)
      goto error_exit_1;

    if ((num_trans = synch_get_transitions(&(this->transitions))) < 0)
      goto error_exit_2;

    if (synch_get_arcs(this) < 0)
      goto error_exit_3;
  }

  return num_sem;

/*
error_exit_4:
  matrix_free(this->in_arc_weights);
  matrix_free(this->out_arc_weights);
  matrix_free(this->null_arcs);
*/
error_exit_3:
  list_free(this->transitions);

error_exit_2:
  matrix_free(this->init_tokens);

error_exit_1:
  list_free(this->places);

error_exit_0:
  return -1;
}



static void petri_net_t_print(petri_net_t *this, FILE *stream) {

  printf("\n\nplaces:\n");
  list_print(this->places, stream);
  printf("\n\ntransitions:\n");
  list_print(this->transitions, stream);
  printf("\n\nin_arc_weights:\n");
  matrix_print(this->in_arc_weights, stream);
  printf("\n\nout_arc_weights:\n");
  matrix_print(this->out_arc_weights, stream);
  printf("\n\nnull_arcs:\n");
  matrix_print(this->null_arcs, stream);
  printf("\n\ninit_tokens:\n");
  matrix_print(this->init_tokens, stream);
}



/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****        The main setup and shutdown functions.        ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/

/* forward declaration */
static int synch_load_confmap(petri_net_t *petri_net);


/* Setup all common resources required for the synch library.  */
/* In this case, it is to create the semaphore set, and insert */
/* the synchpt definitions in the cmm.                         */
/*
 * max_module_count -> the maximum number of simultaneously running modules...
 */
int synch_setup(const char *module_name, int max_module_count)
{
  int synch_semkey;
  plcsynchsem_t synch_semid = PLCSYNCHSEM_FAILED;
  int index, synch_place_count, free_place_count, total_place_count;
  synch_t *synch_shm = NULL;
  petri_net_t petri_net = new(petri_net_t);

  if (debug)
    printf("synch_setup(): module_name=%s, max_module_count=%d\n",
           module_name, max_module_count);

  if (max_module_count < 0)
    goto error_exit_0;

  if (petri_net.load_from_config(&petri_net) < 0)
    goto error_exit_0;

  /* determine the relevant 'constants'... */
    /* we use one semaphore for each place in the synch petri net,
     * one semaphore for each running module (used later by the state library to
     * control the module RUN/STOP mode), and an extra sempahore for the
     * MatPLC global RUN/mode state.
     */
  synch_place_count = list_len(petri_net.places);
  free_place_count  = max_module_count + 1;
  total_place_count = synch_place_count + free_place_count;

/*
    printf("\n\n\n");
    petri_net.print(&petri_net, stdout);
*/

  /* parse the config file for parameters */
  if (synch_parse_conf(&synch_semkey,
                       NULL, NULL /* we are not interested in the scan synchps */
                      )
      < 0)
    goto error_exit_1;


  /* create configuration shared memory block */
  synch_shm = cmm_block_alloc(SYNCH_T_BLOCK_TYPE,
                              SYNCH_T_BLOCK_NAME,
                              sizeof_synch_t(total_place_count));

  if (synch_shm == NULL) {
    plc_log_errmsg(9,"synch_setup(): could not alloc config memory block.\n");
    goto error_exit_1;
  }
  /* create the place alloc mutex, used to control access to the
   * place_use_array[]...
   */
  if (plcmutex_create(&(synch_shm->place_alloc_mutexid)) < 0)
    goto error_exit_2;

  /* create the synchronisation sempahores... */
  if (total_place_count <= 0) {
    synch_semid = PLCSYNCHSEM_FAILED;
    synch_semkey = -1;
  } else {
    /* create an array with initial number of tokens for each petri net place... */
    int *init_values = (int *)malloc(total_place_count*sizeof(int));
    if (NULL == init_values)
      goto error_exit_3;

    for (index = 0; index < total_place_count; index++)
      init_values[index]=0;
    for (index = 0; index < synch_place_count; index++)
      init_values[index] = matrix_get(petri_net.init_tokens, index, 0);

    /* now really create the synch petri net semaphore set... */
    { int res;
      if (synch_semkey == 0)
        res = plcsynchsem_createrand(&synch_semkey, total_place_count, init_values);
      else
        res = plcsynchsem_create(synch_semkey, total_place_count, init_values);

      if (res < 0)
        goto error_exit_3;
    }
  }

  /* initialize the conf mem area */
  synch_shm->magic              = SYNCH_MAGIC;
  synch_shm->place_count        = total_place_count;
  synch_shm->synch_semkey       = synch_semkey;

  /* reset the place_use_array... */
  for (index = 0; index < total_place_count; index++)
    places_in_use(synch_shm)[index] = 0;
  for (index = 0; index < synch_place_count; index++)
    places_in_use(synch_shm)[index] = 1;

  /* Load the confmap with the synchpts definitions */
  if (synch_load_confmap(&petri_net) < 0) {
    plc_log_errmsg(9, "synch_setup(): error loading confmap...\n");
    goto error_exit_4;
  }

  /* free the memory allocated for the petri_net... */
  petri_net.delete(&petri_net);

  /* initialize access to the synch library */
  synch_init(module_name);

  /* "the synch library is now open for business" */
  return 0;

error_exit_4:
  if (synch_semkey >= 0)
    plcsynchsem_delete(synch_semkey);
error_exit_3:
  plcmutex_destroy(&(synch_shm->place_alloc_mutexid));
error_exit_2:
  cmm_block_free(synch_shm);
error_exit_1:
  petri_net.delete(&petri_net);
error_exit_0:
  return -1;
}


/* delete all resources used by the synch library */
int synch_shutdown(void)
{
  int res;
  synch_t *synch_shm;
  u32 size;

  /* get hold of configuration shared memory block */
  synch_shm = cmm_block_find(SYNCH_T_BLOCK_TYPE,
                             SYNCH_T_BLOCK_NAME,
                             &size);

  if (synch_shm == NULL)
    return -1;

  /* If the following condition is false, we canot safely
   * access the synch_shm->place_count, etc.. variables!!
   */
  if (size < sizeof(synch_t)) {
    cmm_block_free(synch_shm);
    return -1;
  }

  /* assume success... */
  res = 0;

  if (size != sizeof_synch_t(synch_shm->place_count))
    res = -1;

  /* delete the synch semaphore set.. */
  if (synch_shm->synch_semkey >= 0)
    if (plcsynchsem_delete(synch_shm->synch_semkey) < 0)
      res = -1;

  /* delete the place_alloc mutex... */
  if (plcmutex_destroy(&(synch_shm->place_alloc_mutexid)) < 0)
    res = -1;

  /* free the cmm block used by the synch lib */
  if (cmm_block_free(synch_shm) < 0)
    res = -1;

  return res;
}


/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****        Load the cmm with the transitions             ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/

/* store one transition config in config memory area */
static int synch_config_put_transition(const char *name,
                                       int transition_index,
                                       u16 place_count,
                                       matrix_t in_arc_weights,
                                       matrix_t out_arc_weights,
                                       matrix_t null_arcs)
{
  int sem_index, block_size;
  plc_transition_t transition;

  block_size = sizeof_transition_t(place_count);

  transition = (plc_transition_t)cmm_block_alloc(SYNCH_TRANSITION_BLOCK_TYPE,
                                                 name, block_size);

  if (transition == NULL) {
    plc_log_errmsg(1, "Not enough memory in the confmap to store transition %s.",
                   name);
    goto error_exit_0;
  }

  transition_priv(transition)->place_count = place_count;

  for (sem_index = 0; sem_index < place_count; sem_index++) {
    if (matrix_get(in_arc_weights, sem_index, transition_index) > 0)
      transition_wait_values(transition)[sem_index] =
                 matrix_get(in_arc_weights, sem_index, transition_index);
    else
      /* NOTE: a transition uses (-1) when no arc is present! */
      transition_wait_values(transition)[sem_index] = -1;
  } /* for(;;) */

  for (sem_index = 0; sem_index < place_count; sem_index++) {
    if (matrix_get(out_arc_weights, sem_index, transition_index) > 0)
      transition_signal_values(transition)[sem_index] =
                 matrix_get(out_arc_weights, sem_index, transition_index);
    else
      /* NOTE: a transition uses (-1) when no arc is present! */
      transition_signal_values(transition)[sem_index] = -1;
  } /* for(;;) */

  for (sem_index = 0; sem_index < place_count; sem_index++) {
    if (matrix_get(null_arcs, sem_index, transition_index) != 0) {
      if (transition_wait_values(transition)[sem_index] == -1) {
        transition_wait_values(transition)[sem_index] = 0;
      } else {
        /* we cannot insert a null arc when we already have another
         * non null arc from the same place to the same transition...
         */
        goto error_exit_1;
      }
    }
  } /* for(;;) */

  transition_priv(transition)->magic = TRANSITION_MAGIC;

  return 0;

error_exit_1:
  cmm_block_free(transition);
error_exit_0:
  return -1;
}


/* store every transition configuration in the configuration memory (cmm) */
static int synch_load_confmap(petri_net_t *petri_net /* list_t transitions */)
{
  int index, transitions_loaded;
  int place_count, transition_count;
  char *transition_name;
/*  synchpt_list_member_t transition_list_member; */

  transitions_loaded = 0;

  /* start by loading the definition of the null transition */
  if (synch_config_put_transition(SYNCH_NULL_TRANSITION_NAME,
                                  0, 0, NULL, NULL, NULL)
      < 0) {
    goto error_exit_0;
  }

  place_count = list_len(petri_net->places);
  transition_count = list_len(petri_net->transitions);

  for (index = 0; index < transition_count; index++) {

    if ((transition_name = list_item(petri_net->transitions, index)) == NULL) {
      plc_log_errmsg(1, "Internal program error. This error should not have "
                     "occured. An inconsistency was found in internal data.");
      goto error_exit_0;
    }

    if (synch_config_put_transition(transition_name, index, place_count,
                                    petri_net->in_arc_weights,
                                    petri_net->out_arc_weights,
                                    petri_net->null_arcs)
        < 0) {
      plc_log_errmsg(9, "Error storing transition %s in plc config memory area.",
                     transition_name);
      continue; /* just carry on so all synchpts with error get logged */
    } else {
      transitions_loaded++;
      plc_log_trcmsg(9, "Success storing transition %s in plc cofig memory area.",
                     transition_name);
    }
  } /* for */

  if (transitions_loaded != list_len(petri_net->transitions))
    goto error_exit_0;

  return 0;

error_exit_0:
  return -1;
}


/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****   Parse the complex synchronisation configuration    ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


/* parse the config file for defined places... */
static int synch_get_places(list_t *places)
{
  int index, num_rows, numb_places;
  list_t local_places;
  char *tmp_name;

  numb_places = 0;
  if (places == NULL) places = &local_places;

  if ((*places = list_new(string_vf_table)) == NULL)
    goto error_exit_0;

  num_rows = conffile_get_table_rows_sec(SYNCH_PLACES_TABLE_SECTION,
                                         SYNCH_PLACES_TABLE_NAME);

  for (index = 0; index < num_rows; index++) {
    tmp_name = conffile_get_table_sec(SYNCH_PLACES_TABLE_SECTION,
                                      SYNCH_PLACES_TABLE_NAME,
                                      index, 0);
    if (tmp_name != NULL) {
      if (list_append(*places, tmp_name) < 0)
        goto error_exit_1;
    }
  } /* for */

normal_exit:
  numb_places = list_len(*places);

  if (places == &local_places) {
    /* we must free the memory locally */
    list_free(*places);
    *places = NULL;
    }

  return numb_places;

error_exit_1:
  numb_places = list_len(*places);

error_exit_0:
  plc_log_errmsg(1,"Not enough memory to read all the places. "
                   "Only read %d places",
                   numb_places);
  goto normal_exit;
}


/* parse the config file for the initial number of tokens in each place... */
static int synch_get_init_tokens(list_t places, matrix_t *init_tokens)
{
  int index, num_rows, place_index;
  matrix_t local_init_tokens;
  char *tmp_name;
  u32 tmp_init_tokens;

  if (places == NULL)
    return -1;

  if (init_tokens == NULL) init_tokens = &local_init_tokens;

  if ((*init_tokens = matrix_new(list_len(places), /* number of columns */
                                 1, /* number of lines */
                                 0  /* initial value   */ ))
       == NULL)
    goto error_exit_0;


  num_rows = conffile_get_table_rows_sec(SYNCH_PLACES_TABLE_SECTION,
                                         SYNCH_PLACES_TABLE_NAME);

  for (index = 0; index < num_rows; index++) {
    tmp_name = conffile_get_table_sec(SYNCH_PLACES_TABLE_SECTION,
                                      SYNCH_PLACES_TABLE_NAME,
                                      index, 0);
    conffile_get_table_sec_u32(SYNCH_PLACES_TABLE_SECTION,
                               SYNCH_PLACES_TABLE_NAME,
                               index, 1,
                               &tmp_init_tokens,
                               SYNCH_PLACES_MIN_TOKEN,
                               SYNCH_PLACES_MAX_TOKEN,
                               SYNCH_PLACES_DEF_TOKEN);

    if (tmp_name != NULL) {
      if ((place_index = list_index(places, tmp_name)) < 0) {
        plc_log_errmsg(1, "This error should not have occured. "
                          "Cannot find place %s in temporary list of places to"
                          " set it's initial token count to %d.",
                          tmp_name, tmp_init_tokens);
      } else {
      if (matrix_set(*init_tokens, place_index, 0, tmp_init_tokens)
          != tmp_init_tokens)
        plc_log_errmsg(1, "This error should not have occured. "
                          "Could not set initial number of tokens in place %s "
                          "to its correct value %d. It is now set to %d.",
                          tmp_name, tmp_init_tokens,
                          matrix_get(*init_tokens, place_index, 0));
      }
    free(tmp_name);
    }
  } /* for */

  if (init_tokens == &local_init_tokens) {
    /* we must free the memory locally */
    matrix_free(*init_tokens);
    *init_tokens = NULL;
    }

  return 0;

error_exit_0:
  plc_log_errmsg(1,"Not enough memory to read the initial number of tokens "
                   "in each place.");
  return -1;
}


/* parse the config file for defined transitions... */
static int synch_get_transitions(list_t *trans)
{
  int num_rows, numb_trans, index;
  list_t local_trans;
  char *tmp_name;

  numb_trans = 0;
  if (trans == NULL) trans = &local_trans;

  if ((*trans = list_new(string_vf_table)) == NULL)
    goto error_exit_0;

  num_rows = conffile_get_table_rows_sec(SYNCH_TRANS_TABLE_SECTION,
                                         SYNCH_TRANS_TABLE_NAME);

  for (index = 0; index < num_rows; index++) {
    tmp_name = conffile_get_table_sec(SYNCH_TRANS_TABLE_SECTION,
                                      SYNCH_TRANS_TABLE_NAME,
                                      index, 0);

    if (tmp_name != NULL) {
      if (list_append(*trans, tmp_name) < 0)
        goto error_exit_1;
    }
  } /* for */

normal_exit:
  numb_trans = list_len(*trans);

  if (trans == &local_trans) {
    /* we must free the memory locally */
    list_free(*trans);
  }

  return numb_trans;

error_exit_1:
  numb_trans = list_len(*trans);

error_exit_0:
  plc_log_errmsg(1, "Not enough memory to read all transitions. "
                    "Read %d transitions.",
                    numb_trans);
  goto normal_exit;
}


/* parse the arrow string in an arc definition */
/*  accepted syntax:
 *     [-][(][<weight>][)][-]>
 *
 *  possible correct examples:
 *   ----->
 *   >
 *   -()->
 *   ()>
 *   (>
 *   )>
 *   -7->
 *   8>
 *   (9)>
 *   (9>
 *   9)>
 *   etc...
 */
static int synch_parse_arc_weight(char *arc)
{
  char *tmp_arc, *numb_str, *end_str;
  int  numb_str_len, weight = 0;

  if ((tmp_arc = strdup(arc)) == NULL) {
    plc_log_errmsg(1, "Not enough memory to parse arc.");
    return -1;
  }

  numb_str = tmp_arc + strspn(tmp_arc, "-");
  if (numb_str[0] == '(')
    numb_str += 1;
  numb_str_len = strcspn(numb_str, ")->");
  end_str = numb_str + numb_str_len;
  if (end_str[0] == ')')
    end_str += 1;
  end_str += strspn(end_str, "-");

  if (strcmp(">", end_str) != 0) {
    plc_log_errmsg(1, "Invalid arc weight syntax %s.", arc);
    goto error_exit_0;
  }

  if (numb_str_len == 0)
    return SYNCH_ARCS_DEF_WEIGHT;

  *(numb_str + numb_str_len) = '\0';

  if (string_str_to_int(numb_str, &weight,
                        SYNCH_ARCS_MIN_WEIGHT,
                        SYNCH_ARCS_MAX_WEIGHT)
      < 0) {
    plc_log_errmsg(1, "Invalid arc weight syntax %s, "
                      "or value out of range (%d .. %d).",
                   arc, SYNCH_ARCS_MIN_WEIGHT, SYNCH_ARCS_MAX_WEIGHT);
    goto error_exit_0;
  }

  free(tmp_arc);
  return weight;

error_exit_0:
  free(tmp_arc);
  return -1;
}


/* parse all the arcs defined in the config file */
/*
 * accepted syntax
 *
 *  {place | transition} <arrow> {transition | place}
 *
 *  where <arrow> should follow the syntax defined by the
 *   parse_arc_weight() function
 *
 *  and if arc starts at a place, it must end at a transition,
 *   or if arc starts at a transition, it must end at a place.
 *
 */
/*
static int synch_get_arcs(list_t places, list_t trans,
                          matrix_t *arc_weights_w, matrix_t *null_arcs_w)
*/
static int synch_get_arcs(petri_net_t *pnet)
{
  int numb_arcs, num_rows, index, place_count, trans_count;
  int res, weight;
  char *tmp_name1;
  char *tmp_name2;
  char *tmp_name3;
  int place1_index, place3_index, trans1_index, trans3_index;

  numb_arcs = 0;
  if (pnet == NULL)
    return -1;

  place_count = list_len(pnet->places);
  trans_count = list_len(pnet->transitions);

  /* Create the matrixes... */
  if ((pnet->in_arc_weights =
        matrix_new(place_count,  /* number of columns */
                   trans_count,  /* number of lines   */
                   0))           /* initial value     */
       == NULL) {
    plc_log_errmsg(9, "Error creating in_arc_weights matrix.");
    goto error_exit_0;
  }

  if ((pnet->out_arc_weights =
        matrix_new(place_count,  /* number of columns */
                   trans_count,  /* number of lines   */
                   0))           /* initial value     */
       == NULL) {
    plc_log_errmsg(9, "Error creating out_arc_weights matrix.");
    goto error_exit_1;
  }

  if ((pnet->null_arcs =
        matrix_new(place_count,  /* number of columns */
                   trans_count,  /* number of lines   */
                   0))           /* initial value     */
       == NULL) {
    plc_log_errmsg(9, "Error creating null_arcs matrix.");
    goto error_exit_2;
  }

  num_rows = conffile_get_table_rows_sec(SYNCH_ARCS_TABLE_SECTION,
                                         SYNCH_ARCS_TABLE_NAME);

  for (index = 0; index < num_rows; index++) {
    tmp_name1 = conffile_get_table_sec(SYNCH_ARCS_TABLE_SECTION,
                                       SYNCH_ARCS_TABLE_NAME,
                                       index, 0);
    tmp_name2 = conffile_get_table_sec(SYNCH_ARCS_TABLE_SECTION,
                                       SYNCH_ARCS_TABLE_NAME,
                                       index, 1);
    tmp_name3 = conffile_get_table_sec(SYNCH_ARCS_TABLE_SECTION,
                                       SYNCH_ARCS_TABLE_NAME,
                                       index, 2);

    if ((tmp_name1 == NULL) || (tmp_name2 == NULL) || (tmp_name3 == NULL)) {
      plc_log_errmsg(1, "Arc number %d has missing parameters. "
                        "Skipping this arc.",
                     index);
      goto skip_arc;
    }

    place1_index = list_index(pnet->places, tmp_name1);
    place3_index = list_index(pnet->places, tmp_name3);
    trans1_index = list_index(pnet->transitions, tmp_name1);
    trans3_index = list_index(pnet->transitions, tmp_name3);

    if ((place1_index >= 0) && (trans1_index >= 0)) {
      plc_log_errmsg(1, "%s refers to both a place and transition "
                        "in arc number %d. Skipping this arc.",
                     tmp_name1, index);
      goto skip_arc;
    }

    if ((place3_index >= 0) && (trans3_index >= 0)) {
      plc_log_errmsg(1, "%s refers to both a place and transition "
                        "in arc number %d. Skipping this arc.",
                     tmp_name3, index);
      goto skip_arc;
    }

    if ((place1_index < 0) && (trans1_index < 0)) {
      plc_log_errmsg(1, "%s is neither a place nor a transition "
                        "in arc number %d. Skipping this arc.",
                     tmp_name1, index);
      goto skip_arc;
    }

    if ((place3_index < 0) && (trans3_index < 0)) {
      plc_log_errmsg(1, "%s is neither a place nor a transition "
                        "in arc number %d. Skipping this arc.",
                     tmp_name3, index);
      goto skip_arc;
    }

    if ((place1_index >= 0) && (place3_index >= 0)) {
      plc_log_errmsg(1, "Arc number %d starts and ends at a place "
                        "(%s, %s). Skipping this arc.",
                     index, tmp_name1, tmp_name3);
      goto skip_arc;
    }

    if ((trans1_index >= 0) && (trans3_index >= 0)) {
      plc_log_errmsg(1, "Arc number %d starts and ends at a transition "
                        "(%s, %s). Skipping this arc.",
                     index, tmp_name1, tmp_name3);
      goto skip_arc;
    }

    /* parse the arc weight */
    weight = synch_parse_arc_weight(tmp_name2);

    if (weight < 0) {
      plc_log_errmsg(1, "Arc number %d has an invalid weight %s. ",
                     "Skipping this arc.",
                     index, tmp_name2);
      goto skip_arc;
    }

     /* if we have a null arc from a transition to a place... */
    if ((weight == 0) && ((trans3_index < 0) || (place1_index < 0))) {
      plc_log_errmsg(1, "Arc number %d has an invalid weight %s. ",
                     "Skipping this arc.",
                     index, tmp_name2);
      goto skip_arc;
    }

    res = 0;

    if (weight > 0) {
      if (trans1_index >= 0)
        res = (matrix_set(pnet->out_arc_weights, place3_index, trans1_index, weight)
              == weight)?1:0;

      if (trans3_index >= 0)
        res = (matrix_set(pnet->in_arc_weights, place1_index, trans3_index, weight)
              == weight)?1:0;
    } else {
      /* it must be a null arc... */
      if ((trans3_index >= 0) && (weight == 0))
        res = (matrix_set(pnet->null_arcs, place1_index, trans3_index, 1)
              == 1)?1:0;
    }
    if (res == 0)
      plc_log_errmsg(1, "This error should not have occured. "
                        "Could not set initial weight of arc %d "
                        "to its correct value %d.",
                        index, weight);

    numb_arcs++;

skip_arc:
    free(tmp_name1);
    free(tmp_name2);
    free(tmp_name3);
  } /* for */

normal_exit:
  return numb_arcs;

/*
error_exit_3:
  matrix_free(pnet->null_arcs);
*/
error_exit_2:
  matrix_free(pnet->out_arc_weights);
error_exit_1:
  matrix_free(pnet->in_arc_weights);

error_exit_0:
  plc_log_errmsg(1, "Not enough memory to read all arcs. "
                    "Read %d arcs.",
                    numb_arcs);
  goto normal_exit;
}



/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****    Parse the simple synchronisation configuration    ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


/* Small auxiliary function to the synch_get_simple_synch() function... */
/* Append the element to the list, if not already present...
 */
static int list_append_if_missing(list_t lst,
                                  char *item,
                                  const char *item_suffix,
                                  const char *item_desc)
{
  char *dup_item;

  /* create new item to be inserted in the list... */
  if (item_suffix != NULL)
    dup_item = strdup2(item, item_suffix);
  else
    dup_item = strdup(item);

  if (dup_item == NULL) {
    plc_log_errmsg(9, "Not enough memory to duplicate %s name %s. ",
                   item_desc, item);
    return -1;
  }

  /* check if item already in the list... */
  if (list_index(lst, dup_item) >= 0) {
    free(dup_item);
    return 0;
  }

  /* insert the item in the list... */
  if (list_append(lst, dup_item) < 0) {
    plc_log_errmsg(9, "Error appending %s %s to list of %ss. ",
                   item_desc, item, item_desc);
    free(dup_item);
    return -1;
  }

  /* NOTE: do not free(dup_item). We keep a pointer to it in the list!!!! */
  return 0;
}


/* Small auxiliary function to the synch_get_simple_synch() function... */
/* Search for an element in the list...
 */
static int list_index2(list_t lst,
                       char *item,
                       const char *item_suffix)
{
  char *dup_item;
  int index;

  /* create new item... */
  if (item_suffix != NULL)
    dup_item = strdup2(item, item_suffix);
  else
    dup_item = strdup(item);

  if (dup_item == NULL) {
    plc_log_errmsg(9, "Not enough memory to duplicate %s. ", item);
    return -1;
  }

  /* search for item in the list... */
  index = list_index(lst, dup_item);

  free(dup_item);

  return index;
}


/*
 * Parse the simplified module synchronisation syntax and
 * populate the petri net
 */
static int synch_get_simple_synch(petri_net_t *pnet) {
  int index, num_rows, numb_places, tmp_weight, place_index, trans_index;
  char *tmp_name1, *tmp_name2, *tmp_name3;

  if (pnet == NULL)
    return -1;

  numb_places = 0 ;
  if ((pnet->places = list_new(string_vf_table)) == NULL)
    goto error_exit_0;

  if ((pnet->transitions =
        list_new(string_vf_table)) == NULL)
    goto error_exit_1;

  num_rows = conffile_get_table_rows_sec(SYNCH_SIMPLE_TABLE_SECTION,
                                         SYNCH_SIMPLE_TABLE_NAME);

  /* read in all the places and transitions... */
  for (index = 0; index < num_rows; index++) {
    int error = 0;

    tmp_name1 = conffile_get_table_sec(SYNCH_SIMPLE_TABLE_SECTION,
                                       SYNCH_SIMPLE_TABLE_NAME,
                                       index, 0);
    tmp_name2 = conffile_get_table_sec(SYNCH_SIMPLE_TABLE_SECTION,
                                       SYNCH_SIMPLE_TABLE_NAME,
                                       index, 1);
    tmp_name3 = conffile_get_table_sec(SYNCH_SIMPLE_TABLE_SECTION,
                                       SYNCH_SIMPLE_TABLE_NAME,
                                       index, 2);

    if ((tmp_name1 == NULL) || (tmp_name2 == NULL) || (tmp_name3 == NULL)) {
      plc_log_errmsg(1, "Synch arc number %d has invalid parameters. "
                        "Skipping this arc.",
                     index);
      goto loop1_continue;
    }

    /* first check if the arrow is correclty formed */
    if ((tmp_weight = synch_parse_arc_weight(tmp_name2)) <= 0) {
      if (tmp_weight == 0)
        plc_log_errmsg(1, "Synch arc number %d (%s) "
                          "has an invalid weight %d. Skipping this arc.",
                       index, tmp_name2, tmp_weight);
      else
        plc_log_errmsg(1, "Synch arc number %d (%s) has a syntax error. "
                          "Skipping this arc.",
                       index, tmp_name2);
      goto loop1_continue;
    }

    /* add modules to transition and places lists */
      /* but first check if not already in the list */
    error = 1; /* assume error */
    if (list_append_if_missing(pnet->places, tmp_name1, NULL, "place") < 0)
      goto loop1_continue;
    if (list_append_if_missing(pnet->places, tmp_name3, NULL, "place") < 0)
      goto loop1_continue;
    if (list_append_if_missing(pnet->transitions, tmp_name1,
                               SYNCH_TRANSITION_BEG_SUFFIX, "transition") < 0)
      goto loop1_continue;
    if (list_append_if_missing(pnet->transitions, tmp_name1,
                               SYNCH_TRANSITION_END_SUFFIX, "transition") < 0)
      goto loop1_continue;
    if (list_append_if_missing(pnet->transitions, tmp_name3,
                               SYNCH_TRANSITION_BEG_SUFFIX, "transition") < 0)
      goto loop1_continue;
    if (list_append_if_missing(pnet->transitions, tmp_name3,
                               SYNCH_TRANSITION_END_SUFFIX, "transition") < 0)
      goto loop1_continue;

    /* we got through without any errors... */
    error = 0;

  loop1_continue:
    free(tmp_name1);
    free(tmp_name2);
    free(tmp_name3);
    if (error != 0)
      goto error_exit_2;
  } /* for */

  /* Now create the matrixes... */
  if ((pnet->in_arc_weights =
        matrix_new(list_len(pnet->places),       /* number of columns */
                   list_len(pnet->transitions),  /* number of lines   */
                   0))                           /* initial value     */
       == NULL) {
    plc_log_errmsg(9, "Error creating in_arc_weights matrix.");
    goto error_exit_2;
  }

  if ((pnet->out_arc_weights =
        matrix_new(list_len(pnet->places),       /* number of columns */
                   list_len(pnet->transitions),  /* number of lines   */
                   0))                           /* initial value     */
       == NULL) {
    plc_log_errmsg(9, "Error creating out_arc_weights matrix.");
    goto error_exit_3;
  }

  if ((pnet->null_arcs =
        matrix_new(list_len(pnet->places),       /* number of columns */
                   list_len(pnet->transitions),  /* number of lines   */
                   0))                           /* initial value     */
       == NULL) {
    plc_log_errmsg(9, "Error creating out_arc_weights matrix.");
    goto error_exit_4;
  }

  /* Now populate the out_arc_weights matrix...
   * with the transition -> place arcs...
   */
  for (index = 0; index < num_rows; index++) {
    int error = 0;

    tmp_name1 = conffile_get_table_sec(SYNCH_SIMPLE_TABLE_SECTION,
                                       SYNCH_SIMPLE_TABLE_NAME,
                                       index, 0);
    tmp_name2 = conffile_get_table_sec(SYNCH_SIMPLE_TABLE_SECTION,
                                       SYNCH_SIMPLE_TABLE_NAME,
                                       index, 1);
    tmp_name3 = conffile_get_table_sec(SYNCH_SIMPLE_TABLE_SECTION,
                                       SYNCH_SIMPLE_TABLE_NAME,
                                       index, 2);

    if ((tmp_name1 == NULL) || (tmp_name2 == NULL) || (tmp_name3 == NULL)) {
      goto loop2_continue;
    }

    /* first check if the arrow is correclty formed */
    if ((tmp_weight = synch_parse_arc_weight(tmp_name2)) <= 0) {
      /* arrow is incorrectly formed. We will ignore this arrow.
       * Note that the user has already been warned this arrow is
       * being ignored when we firt looped through all the arrows and
       * inserted the places and transitions in the petri net.
       *
       * Therefore, here we simply ignore the error...
       */
      goto loop2_continue;
    }

    /* add weights to the matrixes... */
    error = 1; /* assume error! */
    if ((place_index = list_index(pnet->places, tmp_name3)) < 0)
      goto loop2_continue; /* ugly error that should not have occured... */
    if ((trans_index = list_index2(pnet->transitions, tmp_name1,
                                   SYNCH_TRANSITION_END_SUFFIX)) < 0)
      goto loop2_continue; /* ugly error that should not have occured... */
    if (matrix_set(pnet->out_arc_weights, place_index, trans_index, tmp_weight) != tmp_weight)
      goto loop2_continue; /* ugly error that should not have occured... */
    /* got through without any errors... */
    error = 0;

  loop2_continue:
    free(tmp_name1);
    free(tmp_name2);
    free(tmp_name3);
    if (error != 0)
      goto error_exit_4;
  } /* for(;;) */

  /* Now populate the in_arc_weights matrix...
   * with the place -> transition arcs...
   */
  numb_places = list_len(pnet->places);
  for (index = 0; index < numb_places; index++) {
    char *place_name;
    int error = 0;

    if ((place_name = list_item(pnet->places, index)) == NULL)
      goto loop3_continue;

    error = 1; /* assume error... */
    if ((trans_index = list_index2(pnet->transitions, place_name,
                                   SYNCH_TRANSITION_BEG_SUFFIX)) < 0)
      goto loop3_continue; /* ugly error that should not have occured... */
    if (matrix_set(pnet->in_arc_weights, /*place_*/index, trans_index, 1) != 1)
      goto loop3_continue; /* ugly error that should not have occured... */

    /* we got through without any errors... */
    error = 0;

  loop3_continue:
    if (error != 0)
      goto error_exit_4;
  } /* for(;;) */

  /* Now create the init_tokens matrix... */
  if ((pnet->init_tokens =
        matrix_new(list_len(pnet->places),       /* number of columns */
                   1,                            /* number of lines   */
                   0))                           /* initial value     */
       == NULL)
    goto error_exit_5;

  /* Now populate the init_tokens matrix... */
  num_rows = conffile_get_table_rows_sec(SYNCH_SIMPLESTART_TABLE_SECTION,
                                         SYNCH_SIMPLESTART_TABLE_NAME);

  for (index = 0; index < num_rows; index++) {
    int error = 0;

    tmp_name1 = conffile_get_table_sec(SYNCH_SIMPLESTART_TABLE_SECTION,
                                       SYNCH_SIMPLESTART_TABLE_NAME,
                                       index, 0);

    if (tmp_name1 == NULL) {
      continue;
    }

    /* add init values to the matrix... */
    error = 1; /* assume error... */
    if ((place_index = list_index(pnet->places, tmp_name1)) < 0)
      goto loop4_continue; /* module does not exist in synch table... */

    if (matrix_set(pnet->init_tokens, place_index, 0, 1) != 1)
      goto loop4_continue; /* ugly error that should not have occured... */

    /* we got through without any errors... */
    error = 0;

  loop4_continue:
    free(tmp_name1);
    if(error != 0)
      goto error_exit_6;
  } /* for(;;) */

/*
normal_exit:
*/
  numb_places = list_len(pnet->places);
  return numb_places;

error_exit_6:
  matrix_free(pnet->init_tokens);
error_exit_5:
  matrix_free(pnet->null_arcs);
error_exit_4:
  matrix_free(pnet->out_arc_weights);
error_exit_3:
  matrix_free(pnet->in_arc_weights);
error_exit_2:
  list_free(pnet->transitions);
error_exit_1:
  numb_places = list_len(pnet->places);
  list_free(pnet->places);
error_exit_0:
  plc_log_errmsg(1,
                 "Error while parsing the module synchronisation parameters.");
  return -1;
}
