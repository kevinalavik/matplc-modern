#ifndef udp_common_h
#define udp_common_h

#include <plc.h>

#define SIMPLE_UDP_MAGIC 0xf4c6 /* magic value */

typedef struct {
  u16 magic;
  u16 serial; /* wraps around */
  u32 data[0];
} msg_t;

typedef struct {
  int count;
  plc_pt_t pt[0];
} pt_list_t;

/* retrieve the list of points from the named table */
pt_list_t *get_point_list(const char *tablename);

#endif /* udp_common_h */
