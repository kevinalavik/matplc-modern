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

//#include "serial.h"
//#include "network.h"
//#include "das08.h"
#include "global.h"

#ifndef __IO
#define __IO

	#define		_DEFAULT_SOCKET		1234
	#define		_SYNC_SOCKET		1235

	// Inputs and Outputs
	#define	_KEYBOARD		700
	#define	_PAR_PORT		701
	#define	_ETH_PLC5		702  // PLC-5 drivers can be attached here
	#define _SCREEN			703
	#define _COM1			704
	#define _COM2			705
	#define _DAS08			706
	#define _NETWORK_SOCK	707
	#define	_COMMAND		708

	#define	_D_PORT_A		800
	#define	_D_PORT_B		801
	#define	_D_PORT_C		802
	#define	_D_PORT_D		803
	#define	_A_IN_0			810
	#define	_A_IN_1			811
	#define	_A_IN_2			812
	#define	_A_IN_3			813
	#define	_A_IN_4			814
	#define	_A_IN_5			815
	#define	_A_IN_6			816
	#define	_A_IN_7			817
	#define	_A_OUT_0		820
	#define	_A_OUT_1		821


	class io {
		private:
			#define		MAXIMUM_IO_LIST	20
			#define		_IO_UNDEFINED			1200
			#define		_IO_NETWORK				1201
			#define		_IO_SERIAL				1202
			#define		_IO_DAS08				1203
			#define		_IO_STREAM				1204
			#define		_IO_ASYNC				1205
			#define		_IO_SYNC				1206
			struct{		int			type;
/*
						serial_io	*com;
						network		*net;
						das08		*daq;
*/
						int /*FILE*/ file;
			} io_list[MAXIMUM_IO_LIST];
			int			io_count;

		public:
						io();
						~io();
			int			remove(int);
			int			add(int, int, int, int, char*);
			#define		_IO_READ				1302
			#define		_IO_READ_INIT			1303
			#define		_IO_READ_END			1304
			#define		_IO_WRITE				1305
			#define		_IO_WRITE_INIT			1306
			#define		_IO_WRITE_END			1307
			#define		_IO_CONNECTION_WAIT		1308
			#define		_IO_CONNECTION_READ		1309
			#define		_IO_CONNECTION_WRITE	1310
			#define		_IO_CONNECTION_END		1311
			#define		_IO_IS_CONNECTED		1312
			#define		_IO_DECODE				1313
			#define		_IO_IS_WAITING			1314

			int			command(int, int, int, int, int, char*, double, int*, char*, int, double*);
	};

#endif
