/*
 * (c) 2002 Mario de Sousa
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


/* TODO:
 *       - clean up the code (access to nw_array_, etc...)
 */


#include <fcntl.h>      /* File control definitions */
#include <stdio.h>      /* Standard input/output */
#include <string.h>
#include <stdlib.h>
#include <termio.h>     /* POSIX terminal control definitions */
#include <sys/time.h>   /* Time structures for select() */
#include <unistd.h>     /* POSIX Symbolic Constants */
#include <assert.h>
#include <errno.h>      /* Error definitions */
#include <time.h>       /* clock_gettime()   */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  /* required for htons() and ntohs() */
#include <netinet/tcp.h> /* TCP level socket options */
#include <netinet/ip.h>  /* IP  level socket options */

#include <plc.h>           /* required for plc types... */
#include <misc/sin_util.h> /* internet socket utility functions... */

#include "mb_layer1.h"  /* The public interface this file implements... */
#include "mb_tcp_private.h"

/* #define DEBUG */       /* uncomment to see the data sent and received */


#define modbus_tcp_write           modbus_write
#define modbus_tcp_read            modbus_read
#define modbus_tcp_init            modbus_init
#define modbus_tcp_done            modbus_done
#define modbus_tcp_connect         modbus_connect
#define modbus_tcp_listen          modbus_listen
#define modbus_tcp_close           modbus_close
#define modbus_tcp_silence_init    modbus_silence_init
#define modbus_tcp_get_min_timeout modbus_get_min_timeout




/************************************/
/**                                **/
/** Include common code...         **/
/**                                **/
/************************************/

#include "mb_time_util.h"




/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****                Forward Declarations                  ****/
/****                    and Defaults                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


  /* A Node Descriptor metadata,
   *   Due to the fact that modbus TCP is connection oriented,
   *   and that if the client detects an error the connection
   *   must be shut down and re-established automatically,
   *   the modbus TCP layer needs to keep the address of the remote server.
   *
   * We do this by implementing a node descriptor table, in which each
   *   entry will have the remote address, and the file descriptor
   *   of the socket currently in use.
   *
   * We do not pass the file descriptor up to the next higher layer. We
   *   send them the node descriptor instead...
   */
#define MB_MASTER_NODE AF_INET  /* This MUST be set to AF_INET !! */
#define MB_LISTEN_NODE AF_AX25  /* actually any value will do, as long as it is != AF_INET */
#define MB_SLAVE_NODE  AF_X25   /* actually any value will do, as long as it is != AF_INET */
#define MB_FREE_NODE   AF_ROSE  /* actually any value will do, as long as it is != AF_INET */
typedef sa_family_t nd_type_t;

typedef struct {
    int    fd;                 /* socket descriptor == file descriptor */
                               /* NOTE:
                                *   Modbus TCP says that on error, we should close
                                *   a connection and retry with a new connection.
                                *   Since it takes time for a socket to close
                                *   a connection if the remote server is down,
                                *   we close the connection on the socket, close the
                                *   socket itself, and create a new one for the new
                                *   connection. There will be times when the node will
                                *   not have any valid socket, and it will have to
                                *   be created on the fly.
                                *   When the node does not have a valid socket,
                                *   fd will be set to -1
                                */
    struct sockaddr_in addr;   /* NOTE:
                                *   We re-use the addr.sin_family member to define the
                                *   state of the node descriptor.
                                *   If addr.sin_family == MB_MASTER_NODE
                                *      The node descriptor was initialised by the
                                *      modbus_connect() function.
                                *      The node descriptor is being used by a master
                                *      device, and the addr contains the address of the slave.
                                *      Remember that in this case fd may be >= 0 while
                                *      we have a valid connection, or it may be < 0 when
                                *      the connection needs to be reset.
                                *   If addr.sin_family == MB_LISTEN_NODE
                                *      The node descriptor was initialised by the
                                *      modbus_listen() function.
                                *      The node is merely used to accept() new connection
                                *      requests. The new slave connections will use another
                                *      node to transfer data.
                                *      In this case fd must be >= 0.
                                *      fd < 0 is an ilegal state and should never occur.
                                *   If addr.sin_family == MB_SLAVE_NODE
                                *      The node descriptor was initialised when a new
                                *      connection request arrived on a MB_LISTEN type node.
                                *      The node descriptor is being used by a slave device,
                                *      and is currently being used to connect to a master.
                                *      In this case fd must be >= 0.
                                *      fd < 0 is an ilegal state and should never occur.
                                *   If addr.sin_family == FREE_ND
                                *      The node descriptor is currently not being used.
                                *      In this case fd is set to -1, but is really irrelevant.
                                */
    int close_on_silence;      /* A flag used only by Master Nodes.
                                * When (close_on_silence > 0), then the connection to the
                                * slave device will be shut down whenever the
                                * modbus_tcp_silence_init() function is called.
                                * Remember that the connection will be automatically
                                * re-established the next time the user wishes to communicate
                                * with the same slave (using this same node descripto).
                                * If the user wishes to comply with the sugestion
                                * in the OpenModbus Spec, (s)he should set this flag
                                * if a silence interval longer than 1 second is expected.
                                */
} nd_entry_t;


static void nd_entry_init(nd_entry_t *nde) {
  nde->addr.sin_family = MB_FREE_NODE;
  nde->fd = -1; /* not currently connected... */
}




typedef struct {
      /* the array of node descriptors, and current size... */
    nd_entry_t *node;
    int        node_count;      /* total number of nodes in the node[] array */
    int        free_node_count; /*  number of free nodes in the node[] array */
      /* fd set with all the fd in the node array already set
       * This saves time in not having to re-initialize a fd_set every time
       * the modbus_read() function is called...
       */
    fd_set     all_fds;
      /* the highest fd in the fds set... This is used for the select() call */
    int        all_fd_high;
      /* fd set with all the fd belonging to MB_MASTER_NODE and MB_SLAVE_NODE
       * type nodes already set.
       * This saves time in not having to re-initialize a fd_set every time
       * the modbus_read() function is called...
       */
    fd_set     ms_fds;
      /* the highest fd in the fds set... This is used for the select() call */
    int        ms_fd_high;
} nd_table_t;


static int nd_table_init(nd_table_t *ndt, int nd_count) {
  int count;

  /* initialise the node table with default values... */
  ndt->node  = NULL;
  ndt->node_count = 0;
  ndt->free_node_count = 0;
  FD_ZERO(&(ndt->all_fds));
  ndt->all_fd_high = -1;
  FD_ZERO(&(ndt->ms_fds));
  ndt->ms_fd_high  = -1;

  /* initialise the node descriptor metadata array... */
  ndt->node = malloc(sizeof(nd_entry_t) * nd_count);
  if (ndt->node == NULL) {
    plc_log_errmsg(1, "Out of memory: error initializing node address buffer");
    return -1;
  }
  ndt->node_count = nd_count;
  ndt->free_node_count = nd_count;

    /* initialise the state of each node in the array... */
  for (count = 0; count < nd_count; count++) {
    nd_entry_init(&ndt->node[count]);
  } /* for() */

  return nd_count; /* number of succesfully created nodes! */
}



static int nd_table_get_free_node(nd_table_t *ndt, nd_type_t nd_type) {
  int count;

  /* check for free nodes... */
  if (ndt->free_node_count <= 0)
    /* no free nodes... */
    return -1;

  /* Decrement the free node counter...*/
  ndt->free_node_count--;

  /* search for a free node... */
  for (count = 0; count < ndt->node_count; count++) {
    if(ndt->node[count].addr.sin_family == MB_FREE_NODE) {
      /* found one!! */
      ndt->node[count].addr.sin_family = nd_type;
      return count;
    }
  } /* for() */

  /* Strange... We should have free nodes, but we didn't finda any! */
  /* Let's try to get into a consistent state, and return an error! */
  ndt->free_node_count = 0;
  return -1;
}



static void nd_table_close_node(nd_table_t *ndt, int nd) {

  if(ndt->node[nd].addr.sin_family == MB_FREE_NODE)
    /* Node already free... */
    return;

  /* Increment the free node counter...*/
  ndt->free_node_count++;
  /* Mark the node as being free. */
  ndt->node[nd].addr.sin_family = MB_FREE_NODE;

  return;
}



/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****                Global Library State                  ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/

 /* The receive buffer... */
 /* NOTE: The library supports multiple simultaneous connections,
  *       but will only read a message from one at a time.
  *       (i.e. once it starts receiving a message from one node, it
  *       will read that message to the end, even though another message
  *       may start arriving later on another node, but finish arriving
  *       earlier than the first. The second message will be ignored until
  *       the first has been completely received, or a timeout occurs...)
  *       This means we can re-use the same buffer for all the nodes.
  *       It doesn't seem to be worth the hassle and increased memory
  *       use to implement it properly with one buffer per node...
  */
static u8 *recv_buf_ = NULL;

 /* The node descriptor table... */
 /* NOTE: This variable is also used to check whether the library
  *       has been previously initialised.
  *       If == NULL, «> library not yet initialised...
  */
static nd_table_t *nd_table_ = NULL;



/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****              Local Utility functions...              ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a>b)?a:b)

/************************************/
/**                                **/
/**  Configure socket for Modbus   **/
/**                                **/
/************************************/


static int configure_socket(int socket_id) {

  /* configure the socket */
    /* set the TCP no delay flag. */
  {int bool_opt = 1;
  if (setsockopt(socket_id, SOL_TCP, TCP_NODELAY,
                 (const void *)&bool_opt, sizeof(bool_opt))
      < 0)
    return -1;
  }

    /* set the IP low delay option. */
  {int priority_opt = IPTOS_LOWDELAY;
  if (setsockopt(socket_id, SOL_IP, IP_TOS,
                 (const void *)&priority_opt, sizeof(priority_opt))
      < 0)
    return -1;
  }

#if 0
    /* send buffer */
    /* NOTE: For slave devices, that may be receiving multiple
     *       requests before they have a chance to reply to the first,
     *       it probably is a good idea to have a larger receive buffer.
     *
     *       For the send buffer, it probably does not make sense to
     *       waste time asking for a smaller buffer, since the larger
     *       default buffer has already been allocated (the socket has already
     *       been created!)
     *
     *       We might just as well leave out the configuration of the socket
     *       buffer size...
     */
#define SOCK_BUF_SIZE 300 /* The size proposed in the Modbus TCP spec. */
  {int sock_buf_size;
  sock_buf_size = SOCK_BUF_SIZE;
  if (setsockopt(socket_id, SOL_SOCKET, SO_SNDBUF,
                 (const void *)&sock_buf_size, sizeof(sock_buf_size))
      < 0)
    return -1;
    /* recv buffer */
  sock_buf_size = SOCK_BUF_SIZE;
  if (setsockopt(socket_id, SOL_SOCKET, SO_RCVBUF,
             (const void *)&sock_buf_size, sizeof(sock_buf_size))
      < 0)
    return -1;
  }
#endif

  return 0;
}


/************************************/
/**                                **/
/** Connect socket to remote host  **/
/**                                **/
/************************************/

/* This function will create a new socket, and connect it to a remote host... */
static inline int open_connection(int nd) {
  int socket_id;

  if (nd_table_->node[nd].fd >= 0)
    /* nd already connected) */
    return nd_table_->node[nd].fd;

  if (nd_table_->node[nd].addr.sin_family != AF_INET)
    /* invalid remote address, or invalid nd */
    return -1;

  /* lets try to connect... */
    /* create the socket */
  if ((socket_id = socket(PF_INET, DEF_TYPE, 0 /* protocol_num */)) < 0)
    return -1;

  /* configure the socket */
  if (configure_socket(socket_id) < 0) {
    close(socket_id);
    return -1;
  };

    /* establish the connection to remote host */
  if (connect(socket_id,
      (struct sockaddr *)&(nd_table_->node[nd].addr),
      sizeof(nd_table_->node[nd].addr)) < 0) {
    close(socket_id);
    return -1;
  }

  nd_table_->node[nd].fd = socket_id;
  FD_SET(socket_id, &(nd_table_->all_fds));
  nd_table_->all_fd_high = max(nd_table_->all_fd_high, socket_id);
  FD_SET(socket_id, &(nd_table_->ms_fds));
  nd_table_->ms_fd_high = max(nd_table_->ms_fd_high, socket_id);

  return socket_id;
}


/* This function will accept a new connection request, and attribute it to a new node... */
static inline int accept_connection(int nd) {
  int socket_id, new_nd;

  if ((new_nd = nd_table_get_free_node(nd_table_, MB_SLAVE_NODE)) < 0)
    /* no available free nodes for the new connection... */
    return -1;

  /* lets accept new connection request... */
  if ((socket_id = accept(nd_table_->node[nd].fd, NULL, NULL)) < 0) {
    /* error establishing new connection... */
    nd_table_close_node(nd_table_, new_nd);  /* first free up the un-used node. */
    return -1;
  }

  /* configure the socket */
  if (configure_socket(socket_id) < 0) {
    nd_table_close_node(nd_table_, new_nd);  /* first free up the un-used node. */
    close(socket_id);
    return -1;
  };

  /* set up the node entry and update the fd sets */
  nd_table_->node[new_nd].fd = socket_id;
  FD_SET(socket_id, &(nd_table_->all_fds));
  nd_table_->all_fd_high = max(nd_table_->all_fd_high, socket_id);
  FD_SET(socket_id, &(nd_table_->ms_fds));
  nd_table_->ms_fd_high = max(nd_table_->ms_fd_high, socket_id);

  return new_nd;
}


static inline void close_connection(int nd) {
  if (nd_table_->node[nd].fd < 0)
    /* nd already disconnected */
    return;

  shutdown(nd_table_->node[nd].fd, SHUT_RDWR);
  close(nd_table_->node[nd].fd);
  FD_CLR(nd_table_->node[nd].fd, &nd_table_->all_fds);
  FD_CLR(nd_table_->node[nd].fd, &nd_table_->ms_fds);
  nd_table_->node[nd].fd = -1;
}



/************************************/
/**                                **/
/**     Data format conversion     **/
/**                                **/
/************************************/

/*
 * Functions to convert u16 variables
 * between network and host byte order
 *
 * NOTE: Modbus uses MSByte first, just like
 *       tcp/ip, so we use the htons() and
 *       ntoh() functions to guarantee
 *       code portability.
 */

static inline u16 mb_hton(u16 h_value) {
/*  return h_value; */
  return htons(h_value);
}

static inline u16 mb_ntoh(u16 m_value) {
/*  return m_value; */
  return ntohs(m_value);
}

static inline u8 msb(u16 value) {
/*  return Most Significant Byte of value; */
  return (value >> 8) & 0xFF;
}

static inline u8 lsb(u16 value) {
/*  return Least Significant Byte of value; */
  return value & 0xFF;
}

#define u16_v(char_ptr)  (*((u16 *)(&(char_ptr))))


/************************************/
/**                                **/
/**   Build/Check a frame header   **/
/**                                **/
/************************************/

/* A modbus TCP frame header has 6 bytes...
 *   header[0-1] -> transaction id
 *   header[2-3] -> must be 0
 *   header[4-5] -> frame data length (must be <= 255)
 */
#if TCP_HEADER_LENGTH < 6
#error This code assumes a header size of 6 bytes, but TCP_HEADER_LENGTH < 6
#endif

static inline void build_header(u8 *header,
                                u16 transaction_id,
                                u16 byte_count)
{
  u16_v(header[0]) = mb_hton(transaction_id);
  header[2] = 0;
  header[3] = 0;
  u16_v(header[4]) = mb_hton(byte_count);
}


static inline int check_header(u8  *header,
                               u16 *transaction_id,
                               u16 *byte_count)
{
  if ((header[2] != 0) || (header[3] != 0))
    return -1;

  *transaction_id = mb_ntoh(*(u16 *)(header + 0));
  *byte_count     = mb_ntoh(*(u16 *)(header + 4));

  if (*byte_count > MAX_L2_FRAME_LENGTH)
    return -1;

  return 0;
}





/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****              Sending of Modbus TCP Frames            ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/

int modbus_tcp_write(int nd,  /* node descriptor */
                     u8 *data,
                     size_t data_length,
                     u16 transaction_id)
{
#define data_vector_size 2

  /* NOTE: The following variables do not have to be static for the code to work
   *       correctly. It simply does not make sense to re-initialize these
   *       structures with the same values every time the function is called.
   */
  static u8            header[TCP_HEADER_LENGTH];
  static struct iovec  data_vector[data_vector_size] = {
                         {(void *)header, TCP_HEADER_LENGTH},
                         {NULL, 0}};
  static struct msghdr msg = {NULL, 0, data_vector, data_vector_size, NULL, 0, 0};
  int res, bytes_sent;

#ifdef DEBUG
  printf("modbus_tcp_write(): called...  nd=%d\n", nd);
#endif

  if ((nd >= nd_table_->node_count) || (nd < 0))
    /* invalid node descriptor... */
    return -1;

  /*************************
  * prepare the header...  *
  *************************/
  build_header(header, transaction_id, data_length);
#ifdef DEBUG
/* Print the hex value of each character that is about to be
 * sent over the bus.
 */
  { int i;
    printf("modbus_tcp_write(): sending data...\n");
    for(i = 0; i < TCP_HEADER_LENGTH; i++)
      printf("[0x%2X]", header[i]);
    for(i = 0; i < data_length; i++)
      printf("[0x%2X]", data[i]);
    printf("\n");
  }
#endif

  /******************************************
   * do we need to re-establish connection? *
   ******************************************/
  if (open_connection(nd) < 0) {
#ifdef DEBUG
    printf("modbus_tcp_write(): could not establish connection...\n");
#endif
    return -1;
  }

  /**********************
   * write to output... *
   **********************/
  /* We are optimising for the most likely case, and in doing that
   * we are making the least likely case have worse behaviour!
   * Read on for an explanation...
   *
   * - The optimised behaviour for the most likely case:
   * We have set the NO_DELAY flag on the socket, so the IP datagram
   * is not delayed and is therefore sent as soon as any data is written to
   * the socket.
   * In order to send the whole message in a single IP datagram, we have to
   * write both the the header and the data with a single call to write()
   * In order to not to have to copy the data around just to add the
   * message header, we use sendmsg() instead of write()!
   *
   * - The worse behaviour for the least likely case:
   * If for some reason only part of the data is sent with the first call to
   * write(), a datagram is sent right away, and the subsequent data will
   * be sent in another datagram. :-(
   */
  data_vector[data_vector_size - 1].iov_base = data;
  data_vector[data_vector_size - 1].iov_len  = data_length;
  data_vector[                   0].iov_base = header;
  data_vector[                   0].iov_len  = TCP_HEADER_LENGTH;
  bytes_sent = 0;
  while (1) {
     /* Please see the comment just above the main loop!! */
    res = sendmsg(nd_table_->node[nd].fd, &msg, 0);
    if (res < 0) {
      if ((errno != EAGAIN ) && (errno != EINTR )) {
        /* error sending message... */
        close_connection(nd);
        return -1;
      } else {
        continue;
      }
    } else {
      /* res >= 0 */
      bytes_sent += res;
      if (bytes_sent >= data_length + TCP_HEADER_LENGTH) {
        /* query succesfully sent! */
#ifdef DEBUG
        printf("modbus_tcp_write(): sent %d bytes\n", bytes_sent);
#endif
        return data_length;
      }

      /* adjust the data_vector... */
      if (res < data_vector[0].iov_len) {
	u8* tmp = data_vector[0].iov_base;
	tmp += res; 
        data_vector[0].iov_len -= res;
        data_vector[0].iov_base = tmp;
      } else {
	u8* tmp = data_vector[1].iov_base;
	tmp += res; 
        res -= data_vector[0].iov_len;
        data_vector[0].iov_len  = 0;
        data_vector[1].iov_len -= res;
        data_vector[1].iov_base = tmp;
      }
    }
  } /* while (1) */

  /* humour the compiler... */
  return -1;
}



/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****              Receiving Modbus TCP Frames             ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


/* A helper function to modbus_tcp_read()
 *
 * WARNING: The semantics of this function are not what you would expect!
 *
 *          if (data_already_available != 0)
 *          It assumes that select() has already been called before
 *          this function got called, and we are therefore guaranteed
 *          to have at least one byte to read off the socket (the fd).
 *
 *          if (data_already_available == 0)
 *          it starts off by calling select()!
 *
 *
 * NOTE: Ususal select semantics for (a: end_time == NULL) and
 *       (b: *end_time == 0) also apply.
 *
 *       (a) Indefinite timeout
 *       (b) Try once, and and quit if no data available.
 */
/* RETURNS: number of bytes read
 *          -1 read error!
 *          -2 timeout
 */
static int read_bytes(int fd,
                      u8 *data,
                      int max_data_count,
                      const struct timespec *end_time,
                      int data_already_available)
{
  fd_set rfds;
  int res, data_count;

  data_count = 0;
  while (data_count < max_data_count) {
    /*============================*
     * wait for data availability *
     *============================*/
    if (data_already_available == 0) {
      int sel_res;
      FD_ZERO(&rfds);
      FD_SET(fd, &rfds);
      sel_res = my_select(fd + 1, &rfds, end_time);
      if (sel_res < 0)
        return -1;
      if (sel_res == 0)
        /* timeout! */
        return -2;
    }

    /*============================*
     * read the available data... *
     *============================*/
    res = read(fd, data + data_count, max_data_count - data_count);
    if (res == 0) {
      /* We are guaranteed to have data to read off the fd since we called
       * select(), but read() returned 0 bytes.
       * This means that the remote process has closed down the connection,
       * so we return 0.
       */
      return 0;
    }

    if (res < 0) {
      if (errno != EINTR)
        return -1;
      else
        res = 0;
    }
#ifdef DEBUG
    {/* display the hex code of each character received */
      int i;
      for (i=0; i < res; i++)
        printf("<0x%2X>", *(data + data_count + i));
    }
#endif
    data_count += res;
    data_already_available = 0;
  } /* while ()*/

  /* data read succesfully... */
  return data_count;
}




/************************************/
/**                                **/
/**    Read a Modbus TCP frame     **/
/**                                **/
/************************************/

/* The public function that reads a valid modbus frame.
 * The frame is read from:
 *   -  the node descriptor nd, if nd >= 0
 *   -  any valid and initialised node descriptor, if nd = -1
 *      In this case, the node where the data is eventually read from
 *      is returned in *nd.
 *
 * The send_data and send_length parameters are ignored...
 *
 * return value: The length (in bytes) of the valid frame,
 *               -1 on error
 *
 * NOTE: Ususal select semantics for (a: recv_timeout == NULL) and
 *       (b: *recv_timeout == 0) also apply.
 *
 *       (a) Indefinite timeout
 *       (b) Try once, and and quit if no data available.
 */

/* NOTES:
 *  - We re-use the recv_buf_ to load the frame header, so we have to make
 *    sure that the buffer is large enough to take it...
 */
 /* RETURNS: number of bytes read
  *          -1 on read from file/node error
  *          -2 on timeout
  */
#if RECV_BUFFER_SIZE < TCP_HEADER_LENGTH
#error The receive buffer is smaller than the frame header length.
#endif

int modbus_tcp_read(int *nd,                /* node descriptor */
                    u8 **recv_data_ptr,
                    u16 *transaction_id,
                    const u8 *send_data,   /* ignored ! */
                    int send_length,       /* ignored ! */
                    const struct timespec *recv_timeout) {

  struct timespec cur_time, end_time, *ts_ptr;
  u8 *local_recv_data_ptr;
  u16 local_transaction_id = 0;
  u16 frame_length;
  int orig_nd, fd, fd_high, data_already_available;

#ifdef DEBUG
  printf("modbus_tcp_read(): called...  nd=%d\n", *nd);
#endif

  if (nd == NULL)
    return -1;

  if (*nd >= nd_table_->node_count)
    /* invalid *nd                      */
    /* remember that *nd < 0 is valid!! */
    return -1;

  if (recv_data_ptr == NULL)
    recv_data_ptr = &local_recv_data_ptr;
  if (transaction_id == NULL)
    transaction_id = &local_transaction_id;

  /* We will potentially call read() multiple times to read in a single frame.
   * We therefore determine the absolute time_out, and use this as a parameter
   * for each call to read_bytes() instead of using a relative timeout.
   *
   * NOTE: see also the timeout related comment in the read_bytes() function!
   *
   * NOTE: clock_gettime() is rather expensive, between 7000 and 7500 clock
   *       cycles (measured with rdtsc on an Intel Pentium)
   *       gettimeofday() is half as expensive (3000 to 3500 clock cycles),
   *       but is not POSIX compliant... :-(
   *       Nevertheless this is peanuts (20 us on a 350 MHz cpu) compared to
   *       the timescales required to read a modbus frame over a serial bus
   *       (aprox. 10 ms for a 10 byte frame on a 9600 baud bus!)
   */
  /* get the current time... */
  if (recv_timeout == NULL) {
    ts_ptr = NULL;
  } else {
    ts_ptr = &end_time;
    if ((recv_timeout->tv_sec == 0) && (recv_timeout->tv_nsec == 0)) {
      end_time = *recv_timeout;
    } else {
      if (clock_gettime(CLOCK_REALTIME, &cur_time) < 0)
        return -1;
      end_time = timespec_add(cur_time, *recv_timeout);
    }
  }

  /* We will loop forever...
   * We jump out of the loop and return from the function as soon as:
   *  - we receive a valid modbus message;
   *    OR
   *  - we time out.
   */
  orig_nd = *nd;
  while(1) {
    *nd = orig_nd;

    /* If we must read off a single node... */
    if (*nd >= 0)
      /* but the node does not have a valid fd */
      if ((nd_table_->node[*nd].addr.sin_family == MB_FREE_NODE) ||
          (nd_table_->node[*nd].fd < 0))
        /* then we return an error... */
        return -1;

    /* We want to call the read_bytes() function only after we are sure there
     * is data waiting to be read on the socket, so we call select here.
     *
     * We only call select() here for the case when (*nd >= 0),
     * since for the other case select() will be called later on...
     */
    data_already_available = 1;
    if (*nd >= 0)
      data_already_available = 0;

    /* if *nd < 0, we will be:
     *  - reading off any valid node descriptor!!
     *  - and accepting new connection requests, if we have free nodes available!!
     */
    while (*nd < 0) {
      int nd_count;
      fd_set rfds;

      if (nd_table_->free_node_count > 0) {
        /* We have free nodes, so we will also listen on nodes
         * that will accept new connections requests...
         */
        rfds = nd_table_->all_fds;
        fd_high = nd_table_->all_fd_high;
      } else {
        /* We do not have free nodes, so we will only listen on nodes
         * that do not accept new connections requests...
         */
        rfds = nd_table_->ms_fds;
        fd_high = nd_table_->ms_fd_high;
      }

      {int sel_res = my_select(fd_high + 1, &rfds, ts_ptr);
        if (sel_res < 0)
          return -1;
        if (sel_res == 0)
	 /* timeout! */
          return -2;
      }

      /* figure out which nd is ready to be read... */
      for (nd_count = 0; nd_count < nd_table_->node_count; nd_count++) {
        if (nd_table_->node[nd_count].addr.sin_family != MB_FREE_NODE) {
          if (FD_ISSET(nd_table_->node[nd_count].fd, &rfds)) {
            /* Found the node descriptor... */
            if (nd_table_->node[nd_count].addr.sin_family == MB_LISTEN_NODE) {
              /* We must accept a new connection...
               * No need to check for errors.
               * If one occurs, there is nothing we can do...
               */
              *nd = accept_connection(nd_count);
#ifdef DEBUG
              printf("modbus_tcp_read(): new connection request on nd=%d\n", nd_count);
              printf("modbus_tcp_read(): new connection accepted on nd=%d", *nd);
#endif
              *nd = -1; /* this will keep us in the while loop */
            } else {
              /* We will read a frame off this nd */
              *nd = nd_count; /* this will get us out of the while loop later on. */
            }
            /* we have found the node descriptor, so let's jump out of the for(;;) loop */
            break;
          }
        }
      } /* for(;;) */
    } /* while (*nd < 0) */

    /* Just a consistency check... */
    if (*nd < 0)
      /* If the code is correct, this should never occur... */
      return -1;

#ifdef DEBUG
    printf("modbus_tcp_read(): reading off nd=%d\n", *nd);
#endif
    /*=========================*
     * read a Modbus TCP frame *
     *=========================*/
    /* assume error... */
    fd = nd_table_->node[*nd].fd;
    *recv_data_ptr = NULL;

    /*-------------*
     * read header *
     *-------------*/
    if (read_bytes(fd, recv_buf_, TCP_HEADER_LENGTH, ts_ptr, data_already_available)
        == TCP_HEADER_LENGTH) {
      /* let's check for header consistency... */
      if (check_header(recv_buf_, transaction_id, &frame_length) < 0) {
#ifdef DEBUG
        printf("modbus_tcp_read(): frame with non valid header...\n");
#endif
        /* We let the code fall through to the error handler, that will
         * close the connection!
         * This is exactly what the modbus TCP protocol specifies!
         */
      } else {
        /*-----------*
         * read data *
         *-----------*/
        data_already_available = 0;
        if (read_bytes(fd, recv_buf_, frame_length, ts_ptr, data_already_available)
            == frame_length) {
          /* frame received succesfully... */
#ifdef DEBUG
          printf("\n");
#endif
          *recv_data_ptr = recv_buf_;
          return frame_length;
        }
      }
    }

    /* We had an error reading the frame...
     * We handle it by closing the connection, as specified by
     * the modbus TCP protocol!
     *
     * NOTE: The error may have been a timeout.
     *       In that case we let the execution loop once again
     *       in the while(1) loop. My_select() will be called again
     *       and the timeout detected. The timeout error code (-2)
     *       will be returned correctly!
     */

#ifdef DEBUG
    printf("modbus_tcp_read(): error reading frame. Closing connection...\n");
#endif
    /* We close the socket... */
    close_connection(*nd);

    /* If it is a slave, we free the node... */
    if(nd_table_->node[*nd].addr.sin_family == MB_SLAVE_NODE)
        nd_table_close_node(nd_table_, *nd);

    /* We try to read another frame... */
  } /* while(1) */

  /* humour the compiler... */
  return -1;
}






/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****        Initialising and Shutting Down Library        ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


/******************************/
/**                          **/
/**   Load Default Values    **/
/**                          **/
/******************************/

static void set_defaults(const char **service) {
  /* Set the default values, if required... */
  if (*service == NULL)
    *service = DEF_SERVICE;
}


/******************************/
/**                          **/
/**    Initialise Library    **/
/**                          **/
/******************************/
/* returns the number of nodes succesfully initialised...
 * returns -1 on error.
 */
int modbus_tcp_init(int nd_count,
                    optimization_t opt /* ignored... */,
                    int *extra_bytes) {
#ifdef DEBUG
  printf("modbus_tcp_init(): called...\n");
  printf("creating %d nodes:\n", nd_count);
#endif

  if (nd_table_ != NULL)
    /* library already previously initialised! */
    return -1;

  if (nd_count <= 0)
    /* invalid node count... */
    goto error_exit_0;

    /* set the extra_bytes value... */
    /* Please see note before the modbus_rtu_write() function for a
     * better understanding of this extremely ugly hack... This will be
     * in the mb_rtu.c file!!
     *
     * The number of extra bytes that must be allocated to the data buffer
     * before calling modbus_tcp_write()
     */
  if (extra_bytes != NULL)
    *extra_bytes = 0;

  /* initialise recv buffer */
  recv_buf_ = malloc(sizeof(u8) * RECV_BUFFER_SIZE);
  if (recv_buf_ == NULL) {
    plc_log_errmsg(1, "Out of memory: error initializing receive buffer");
    goto error_exit_0;
  }

  /* create the node table structure... */
  nd_table_ = malloc(sizeof(nd_table_t));
  if (nd_table_ == NULL) {
    plc_log_errmsg(1, "Out of memory: error initializing node table");
    goto error_exit_1;
  }

  /* initialise the node table... */
  if (nd_table_init(nd_table_, nd_count) < 0)
    goto error_exit_2;

#ifdef DEBUG
  printf("modbus_tcp_init(): %d node(s) opened succesfully\n", nd_count);
#endif
  return nd_count; /* number of succesfully created nodes! */

/*
error_exit_3:
  free(nd_table_->node);
  nd_table_->node = NULL;
  nd_table_->count = 0;
*/
error_exit_2:
  free(nd_table_); nd_table_ = NULL;
error_exit_1:
  free(recv_buf_); recv_buf_ = NULL;
error_exit_0:
  return -1;
}






/******************************/
/**                          **/
/**    Open a Master Node    **/
/**                          **/
/******************************/

int modbus_tcp_connect(node_addr_t node_addr) {
  int node_descriptor;
  struct sockaddr_in tmp_addr;

#ifdef DEBUG
  printf("modbus_tcp_connect(): called...\n");
  printf("        %s:%s\n",
         node_addr.addr.tcp.host,
         node_addr.addr.tcp.service);
#endif

  /* Check for valid address family */
  if (node_addr.naf != naf_tcp)
    /* wrong address type... */
    return -1;

  /* set the default values... */
  set_defaults(&(node_addr.addr.tcp.service));

  /* Check the parameters we were passed... */
  if(sin_init_addr(&tmp_addr,
                   node_addr.addr.tcp.host,
                   node_addr.addr.tcp.service,
                   DEF_PROTOCOL)
       < 0) {
    /*
    plc_log_wrnmsg(1, "Error parsing/resolving address %s:%s",
                   node_addr.addr.tcp.host,
                   node_addr.addr.tcp.service);
    */
    return -1;
  }

  /* find a free node descriptor */
    /* NOTE: The following code line works beacuse MB_MASTER_NODE is set to AF_INET! */
#if MB_MASTER_NODE != AF_INET
#error The code only works if MB_MASTER_NODE == AF_INET, which they are not!
#endif
  if ((node_descriptor = nd_table_get_free_node(nd_table_, MB_MASTER_NODE)) < 0)
    /* if no free nodes to initialize, then we are finished... */
    return -1;

  nd_table_->node[node_descriptor].addr = tmp_addr;
  nd_table_->node[node_descriptor].fd   = -1; /* not currently connected... */
  nd_table_->node[node_descriptor].close_on_silence = node_addr.addr.tcp.close_on_silence;

  if (nd_table_->node[node_descriptor].close_on_silence < 0)
    nd_table_->node[node_descriptor].close_on_silence = DEF_CLOSE_ON_SILENCE;

#ifdef DEBUG
  printf("modbus_tcp_connect(): returning nd=%d\n", node_descriptor);
#endif
  return node_descriptor;
}




/******************************/
/**                          **/
/**    Open a Slave Node     **/
/**                          **/
/******************************/

int modbus_tcp_listen(node_addr_t node_addr) {
  int fd, nd;

#ifdef DEBUG
  printf("modbus_tcp_listen(): called...\n");
  printf("        %s:%s\n",
         node_addr.addr.tcp.host,
         node_addr.addr.tcp.service);
#endif

  /* Check for valid address family */
  if (node_addr.naf != naf_tcp)
    /* wrong address type... */
    goto error_exit_0;

  /* set the default values... */
  set_defaults(&(node_addr.addr.tcp.service));

  /* create a socket and bind it to the appropriate port... */
  fd = bind_socket(node_addr.addr.tcp.host,
                   node_addr.addr.tcp.service,
                   DEF_PROTOCOL);
  if (fd < 0)
    goto error_exit_0;
  if (listen(fd, DEF_MAX_PENDING_CONNECTION_REQUESTS) < 0)
    goto error_exit_0;

  /* find a free node descriptor */
  if ((nd = nd_table_get_free_node(nd_table_, MB_LISTEN_NODE)) < 0) {
    /* if no free nodes to initialize, then we are finished... */
    goto error_exit_1;
  }

  /* nd_table_->node[nd].addr = tmp_addr; */ /* does not apply for MB_LISTEN_NODE */
  nd_table_->node[nd].fd = fd; /* not currently connected... */

  /* update the fd sets...*/
  FD_SET(fd, &(nd_table_->all_fds));
  nd_table_->all_fd_high = max(nd_table_->all_fd_high, fd);

#ifdef DEBUG
  printf("modbus_tcp_listen(): returning nd=%d\n", nd);
#endif
  return nd;

error_exit_1:
  close(fd);
error_exit_0:
  return -1;
}



/******************************/
/**                          **/
/**       Close a node       **/
/**                          **/
/******************************/

int modbus_tcp_close(int nd) {
#ifdef DEBUG
  printf("modbus_tcp_close(): called... nd=%d\n", nd);
#endif

  if ((nd < 0) || (nd >= nd_table_->node_count))
    /* invalid nd */
    return -1;

  if (nd_table_->node[nd].addr.sin_family == MB_FREE_NODE)
    /* already free node */
    return 0;

  close_connection(nd);

  nd_table_->node[nd].addr.sin_family = MB_FREE_NODE;
  nd_table_->free_node_count++;

  return 0;
}



/**********************************/
/**                              **/
/**  Close all open connections  **/
/**                              **/
/**********************************/

int modbus_tcp_silence_init(void) {
  int nd;

#ifdef DEBUG
  printf("modbus_tcp_silence_init(): called...\n");
#endif

  /* close all master connections that remain open... */
  for (nd = 0; nd < nd_table_->node_count; nd++)
    if (nd_table_->node[nd].addr.sin_family == MB_MASTER_NODE)
      if (nd_table_->node[nd].close_on_silence > 0)
        /* node is is being used for a master device,
         * and wishes to be closed...   ...so we close it!
         */
         close_connection(nd);

  return 0;
}



/******************************/
/**                          **/
/**   Shutdown the Library   **/
/**                          **/
/******************************/

int modbus_tcp_done(void) {
  int i;

    /* close all the connections... */
  for (i = 0; i < nd_table_->node_count; i++)
    modbus_tcp_close(i);

  /* Free memory... */
  free(nd_table_->node);
  free(recv_buf_); recv_buf_ = NULL;
  free(nd_table_); nd_table_ = NULL;

  return 0;
}




double modbus_tcp_get_min_timeout(int baud,
                                  int parity,
                                  int data_bits,
                                  int stop_bits) {
  return 0;
}
