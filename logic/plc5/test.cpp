//
//    embedded.cpp - This is program for embedded applications.
//
//    Copyright (C) 2000 by Hugh Jack <jackh@gvsu.edu>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//
// Last Modified: October 5, 2000
//

#include <plc.h>
#include <stdlib.h>

class A_t {
 private:
   int _a;
 public:
   A_t(void) {};
   A_t(int a) {_a = a;};
   ~A_t(void) {};

   void print(void) {printf("%d\n", _a);}
};



main(int argc, char **argv){
 	A_t	*A;

//	if (plc_init("plc5_interpreter", argc, argv) < 0) {
//
	if (plc_done() < 0) {
          printf("Error initializing the plc.\n");
          return EXIT_FAILURE;
        }
//

	A = new A_t(2);
	A->print();
	delete A;

	return EXIT_SUCCESS;
}
