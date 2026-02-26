/* fpont 12/99 */
/* pont.net    */
/* udpClient.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>		/* memset() */
#include <sys/time.h>		/* select() */

#include "common.h"

#define MAX_MSG 100


int main(int argc, char *argv[])
{

  int sd, rc, i;
  struct sockaddr_in cliAddr, remoteServAddr;
  struct hostent *h;
  const char *recv;
  pt_list_t *points=NULL;
  msg_t *msg=NULL;
  int msg_size=0;
  u16 serial;
  u32 port;

  plc_init("simple_udp_send", argc, argv);

  /* get server IP address (no check if input is IP address or DNS name */
  recv=conffile_get_value("to_host");
  if (!recv) 
    recv = "localhost";
  h = gethostbyname(recv);
  if (h == NULL) {
    printf("%s: unknown host '%s' \n", argv[0], recv);
    exit(1);
  }
  if (conffile_get_value_u32("to_port", &port, u16_MIN, u16_MAX, 29600)!=0) {
    printf("Couldn't get port number from config\n");
    exit(1);
  }

  printf("%s: sending data to '%s' (IP : %s) port %d\n", argv[0], h->h_name,
	 inet_ntoa(*(struct in_addr *) h->h_addr_list[0]), port);

  remoteServAddr.sin_family = h->h_addrtype;
  memcpy((char *) &remoteServAddr.sin_addr.s_addr,
	 h->h_addr_list[0], h->h_length);
  remoteServAddr.sin_port = htons(port);

  /* socket creation */
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sd < 0) {
    printf("%s: cannot open socket \n", argv[0]);
    exit(1);
  }

  /* bind any port */
  cliAddr.sin_family = AF_INET;
  cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  cliAddr.sin_port = htons(0);

  rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
  if (rc < 0) {
    printf("%s: cannot bind port\n", argv[0]);
    exit(1);
  }

  /* get the list of points to be sent and initialize the buffer */
  points = get_point_list("send");
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

  msg->magic = htons(SIMPLE_UDP_MAGIC);
  serial = 0;

  while (1) {
    plc_scan_beg();
    plc_update();

    msg->serial = htons(serial);

    for(i=0;i<points->count;i++) {
      msg->data[i]=htonl(plc_get(points->pt[i]));
    }

    /* send data */
    rc = sendto(sd, msg, msg_size, 0,
		(struct sockaddr *) &remoteServAddr,
		sizeof(remoteServAddr));

    if (rc < 0)
      perror("Cannot send data");

    if (serial!=u16_MAX)
      serial++;
    else
      serial=0;

    /* plc_update(); */
    plc_scan_end();
  }

}
