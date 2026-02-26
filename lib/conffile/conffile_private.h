#ifndef conffile_private
#define conffile_private

#include "../misc/ds_util.h"

/* 
 * Load a particular config file and return it; does not change the config
 * as it is in memory (if any). This function is for the use of the config
 * editor only, so its interface and actions can change at its convenience.
 * For instance, it'll probably end up having line number tracking, include
 * file suppression, etc. If anyone else uses it, be sure to note here and
 * in the .c file.
 */
void conffile_load_file(const char *filename, dict ** _vars, dict ** _recs,
			dict ** _files);

#endif /* conffile_private */
