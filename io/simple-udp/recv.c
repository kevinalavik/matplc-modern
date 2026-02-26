/* fpont 12/99 */
/* pont.net    */
/* udpServer.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>		/* close() */
#include <string.h>		/* memset() */

#include "common.h"

#define window 100

int main(int argc, char *argv[])
{
  int sd, rc, n, i;
  socklen_t cliLen;
  struct sockaddr_in cliAddr, servAddr;
  pt_list_t *points=NULL;
  msg_t *msg=NULL;
  int msg_size=0;
  u16 serial;
  i32 last_serial=3*u16_MAX;
  u32 port;

  plc_init("simple_udp_recv", argc, argv);

  if (conffile_get_value_u32("port", &port, u16_MIN, u16_MAX, 29600)!=0) {
    printf("Couldn't get port number from config\n");
    exit(1);
  }

  /* socket creation */
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sd < 0) {
    printf("%s: cannot open socket \n", argv[0]);
    exit(1);
  }

  /* bind local server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(port);
  rc = bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if (rc < 0) {
    printf("%s: cannot bind port number %d \n",
	   argv[0], port);
    exit(1);
  }

  /* get the list of points to be sent and initialize the buffer */
  points = get_point_list("recv");
  if (!points) {
    printf("Didn't get point list (out of memory?)\n");
    exit(1);
  }
  msg_size = sizeof(msg_t) + points->count*sizeof(u32);
  msg = (msg_t*)malloc(msg_size);
  if (!msg) {
    printf("Can't allocate message buffer.\n");
    exit(1);
  }

  printf("%s: waiting for data on port UDP %u\n",
	 argv[0], port);

  /* server infinite loop */
  while (1) {
    plc_scan_beg();
    /* plc_update(); */

    /* receive message */
    cliLen = sizeof(cliAddr);
    n = recvfrom(sd, msg, msg_size, 0,
		 (struct sockaddr *) &cliAddr, &cliLen);

    if (n < 0) {
      printf("Cannot receive data.\n");
    } else if (n!=msg_size) {
      printf("Datagram too short (truncated or misconfigured), discarding.\n");
    } else if (msg->magic!=htons(SIMPLE_UDP_MAGIC)) {
      printf("Bad magic on received datagram, discarding.\n");
    } else {
      serial=ntohs(msg->serial);

      if (((serial>last_serial) && (serial-last_serial<u16_MAX-window)) ||
          ((serial<last_serial) && (last_serial-serial>window))) {
	for (i = 0; i < points->count; i++)
	  plc_set(points->pt[i], ntohl(msg->data[i]));
	last_serial = serial;
      } else {
	printf("Datagram received out of order, discarding.\n");
      }
    }

    plc_update();
    plc_scan_end();
  }				/* end of server infinite loop */

  return 0;
}
