/*
 * (c) 2000 Jiri Baum
 *          Mario de Sousa
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


#ifndef __SHMEM_UTIL_H
#define __SHMEM_UTIL_H


/* creates shared memory with random key; returns NULL on error */
/* shmbytes: size in bytes of the shared memory to create... */
void * create_shmem_rand(int *shmkey, unsigned int shmbytes);

/* creates shared memory with specified key; returns NULL on error */
/* shmbytes: size in bytes of the shared memory to create... */
void * create_shmem(int shmkey, unsigned int shmbytes);

int delete_shmem(int shmkey);
void *attach_shmem(int shmkey,  unsigned int shmbytes); /* returns NULL on error */
int detach_shmem(void *shmptr, unsigned int shmbytes);

#endif /* __SHMEM_UTIL_H */
