#include <stdio.h>
#include "comedi_syntax.h"

char *types[] = {
  "raw analog input", 
  "raw analog output", 
  "physical analog input", 
  "physical analog output", 
  "digital input", 
  "digital output"
};

int main(int argc, char **argv) {
  comedi_addr a;

  if (!argv[1]) {
    printf("Usage: %s address\n", argv[0]);
    return 2;
  }
  a = parse_comedi_addr(argv[1]);

  if (a.device) {
    printf("%s %s %i %i %i %i\n", a.device, types[a.type], a.subdev, a.chan,
	a.range, a.aref);
    return 0;
  } else {
    printf("couldn't parse\n");
    return 1;
  }
}
