/*
 * (c) 2003 Jiri Baum
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <comedi.h>
#include "comedi_syntax.h"

// #define debug printf("%s - ", s); printf
#define debug(...)

#define DIGIT '0': case '1': case '2': case '3': case '4': case '5': \
              case '6': case '7': case '8': case '9'
const char * const devprefix = "/dev/comedi";
const char * const devdefault = "/dev/comedi0";

struct keywordpair {
  const char *s;
  int v;
} keywords[] = {
  {"AREF_GROUND", AREF_GROUND},
  {"GROUND", AREF_GROUND},
  {"GND", AREF_GROUND},
  {"AREF_COMMON", AREF_COMMON},
  {"COMMON", AREF_COMMON},
  {"COM", AREF_COMMON},
  {"CMN", AREF_COMMON},
  {"AREF_DIFF", AREF_DIFF},
  {"DIFF", AREF_DIFF},
  {"DIF", AREF_DIFF},
  {"AREF_OTHER", AREF_OTHER},
  {"OTHER", AREF_OTHER},
  {"OTH", AREF_OTHER},
  {NULL, 0}
};

#define skipspace (s+=strspn(s, " \t\n"))

#define parse_int(tgt) do { \
  skipspace; \
  tgt = strtol(s, &t, 0); \
  if (s==t) {debug("bad integer"); goto error;}; \
  s=t; \
} while(0)

/* 
 * The parse_comedi_addr function is based on the syntax diagram in
 * comedi_syntax.dot, and is structured as a FSM. The labels correspond to
 * the nodes of that diagram. Make sure to have the diagram handy,
 * otherwise the function won't make any sense :-)
 */
comedi_addr parse_comedi_addr(const char *s) {
  comedi_addr res;
  char *t = (char*)s;
  struct keywordpair *k;
  int n;
  memset(&res, 0, sizeof(res));

  skipspace;

  /* S */
  t = strchr(s,':');
  if (t) {
    /* device */
    switch(s[0]) {
      case DIGIT:
	debug("numbered device\n");
        res.device=malloc(strlen(devprefix)+(t-s)+1);
	if (!res.device) goto error;
	strcpy(res.device, devprefix);
	strncat(res.device, s, t-s);
	break;
      default:
	debug("filename device\n");
        res.device=malloc((t-s)+1);
	if (!res.device) goto error;
	strncpy(res.device, s, t-s);
	res.device[t-s]=0;
	break;
    }
    s=t+1; /* point it past the colon */
    skipspace;
  } else {
    debug("default device\n");
    res.device=malloc(strlen(devdefault)+1);
    if (!res.device) goto error;
    strcpy(res.device, devdefault);
  }
  debug("device is %s\n", res.device);
  goto typeL;

typeL:
  switch (toupper(s++[0])) {
    case 'N': res.type = 0; goto typeR;
    case 'U': res.type = 1; goto typeR;
    case 'X': res.type = 2; goto typeR;
    case 'Y': res.type = 3; goto typeR;
    case 'I': res.type = 4; goto typeR;
    case 'O': case 'Q': res.type = 5; goto typeR;
    default: goto error;
  }

typeR:
  debug("typeR\n");

  skipspace;
  switch (s[0]) {
    case DIGIT: goto subdevice;
    case '.': s++; /* dot */
	      goto channel;
    default: goto error;
  }

subdevice:
  debug("subdevice\n");
  parse_int(res.subdev);

  skipspace;
  switch (s[0]) {
    case '.': s++; /* dot */
	      goto channel;
    default: goto options; 
  }

channel:
  parse_int(res.chan);
  goto options;

options:
  debug("options\n");
  skipspace;
  switch (s++[0]) {
    case 0: return res; /* E */
    case '_': goto aref;
    case '/': goto range;
    default: goto error;
  }

aref:
  debug("aref\n");
  skipspace;
  switch (s[0]) {
    case DIGIT: parse_int(res.aref); goto options;
    default: /* parse keyword */
      n = strspn(s, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		    "abcdefghijklmnopqrstuvwxyz");
      debug("keyword, %d letters\n",n);
      for (k=keywords;k->s;k++) {
	if (!strncmp(s,k->s,n)) {
	  debug("found keyword, value %d\n", k->v);
	  res.aref=k->v;
	  s+=n;
	  goto options;
	}
      }
      goto error;
  }

range:
  debug("range\n");
  parse_int(res.range);
  goto options;

error:
  debug("parsing error\n");
  if (res.device) {
    free(res.device);
    res.device=0;
  }
  res.type = -1;
  return res;
}
