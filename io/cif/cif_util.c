/*
 * (c) 2001 Mario de Sousa
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




/*
  cif_util.c

*/

#include <stdlib.h>
#include <string.h>

#include <plc.h>

/* we define this here so the inline functions in cif_util.i don't get included */
#define __CIF_UTIL_C
#include "cif_util.h"

static int debug = 0;


/*
 * Get the type-checker to help us catch bugs; unfortunately then we can't
 * combine bits (easily), but we aren't doing that now, and when need it,
 * we can easily just go back to using u16 as the type for status_
 */
typedef struct {u16 s;} status_t;

static const status_t cif_fully_initialized      = {0x0001};



/* the current status of the cif library (how far did init get?) */
static status_t cif_status_ = {0};

/* access functions for the status_ variable */
inline int cif_get_status(const status_t stat_bit) {
  return (cif_status_.s & stat_bit.s) != 0;
}

inline void cif_set_status(const status_t stat_bit) {
  cif_status_.s |= stat_bit.s;
}

inline void cif_rst_status(const status_t stat_bit) {
  cif_status_.s &= (~stat_bit.s);
}



#define __assert_cif_initialized(ret_val)                                   \
        { if (!cif_get_status(cif_fully_initialized))                       \
            {if (debug)                                                     \
               printf ("CIF Library not initialized at %s:%d.\n", \
                      __FILE__, __LINE__);                                  \
             return ret_val;                                                \
            };                                                              \
        };

#define __assert_cif_not_initialized(ret_val)                         \
        { if (cif_get_status(cif_fully_initialized))                  \
            return ret_val;                                           \
        };




  /* the Id of the Board the user will be using */
  /* Must first call cif_init(BoardId)
   * before calling any other funtion
   */
 /* NOTE: cannot be static. It is used in the cif_util.i file */
unsigned short cif_util_BoardId_;





#define chararray_to_str(c_ptr) __chararray_to_str(c_ptr, sizeof(c_ptr))

static inline char *__chararray_to_str(char *chararray, int size) {
  char *buf;

  if ((buf = (char *)malloc(size + 1)) == NULL)
    return NULL;

  strncpy(buf, chararray, size);
  buf[size] = '\0';

  return buf;
}


/* We use these bcd to char functions because the values are
   returned by the device driver in inverse byte order, so
   we can't simply do a
   printf("%.8X", value);

   we could have used
  #define __b(ofs,val)  ((val >> (8*ofs)) & 0xFF)
  printf("%2.X%2.X%2.X%2.X",
         __b(0,value),__b(0,value),
         __b(0,value),__b(0,value))

  I prefer to use these more generic bcd_to_xxx functions
  because they do not assume a set size of the unsigned long
  variable type.
*/


static inline char bcd_to_char(unsigned short tmp_us) {
  return (tmp_us < 10)?tmp_us + '0':tmp_us - 10 + 'A';
}

static char *__bcd_to_str(unsigned long tmp_ul, int skip_leading_zeros) {
  char *buf;
  int i, j;

  if ((buf = (char *)malloc(2*sizeof(unsigned long) + 1)) == NULL)
    return NULL;

  for (i = 0, j = 0; i < sizeof(unsigned long); i++) {
    buf[j] = bcd_to_char(((tmp_ul >> (i*8 + 4)) &0x0F));
    if ((buf[j] != '0') || (skip_leading_zeros == 0))
      {j++; skip_leading_zeros = 0;}
    buf[j] = bcd_to_char(((tmp_ul >> (i*8    )) &0x0F));
    if ((buf[j] != '0') || (skip_leading_zeros == 0))
      {j++; skip_leading_zeros = 0;}  }

  buf[j] = '\0';

  return buf;
}


static inline char *bcd_to_str(unsigned long tmp_ul) {
  return __bcd_to_str(tmp_ul, 0);
}




int cif_init(unsigned short BoardId) {

 __assert_cif_not_initialized(-1);

 /* check if device driver is loaded */
 if (DevOpenDriver() != DRV_NO_ERROR) {
   plc_log_errmsg(1, "Could not access cif device driver. Is it loaded?");
   return -1;
   }

 /* Get driver version and installed communication boards */
 {
 BOARD_INFO tBoardInfo;
   if (DevGetBoardInfo(&tBoardInfo) != DRV_NO_ERROR) {
     plc_log_errmsg(1, "Could not get configuration info "
                       "from cif device driver. ");
     return -1;
   } else {
     int i;
     plc_log_trcmsg(1, "CIF Device Driver: %s", tBoardInfo.abDriverVersion);
     for (i = 0; i < MAX_DEV_BOARDS; i++)
       if (tBoardInfo.tBoard[i].usAvailable == 1) {
         if (tBoardInfo.tBoard[i].usIrq != 0)
           plc_log_trcmsg(1, "Found Communication Board %d: IO=0x%X, IRQ=%d",
                          tBoardInfo.tBoard[i].usBoard,
                          tBoardInfo.tBoard[i].ulPhysicalAddress,
                          tBoardInfo.tBoard[i].usIrq
                         );
         else
           plc_log_trcmsg(1, "Found Communication Board %d: IO=0x%X, polling mode",
                          tBoardInfo.tBoard[i].usBoard,
                          tBoardInfo.tBoard[i].ulPhysicalAddress
                         );
     }
   } /* if() else */
 }

 plc_log_trcmsg(1, "Using communication board %d.", BoardId);

 /* initialise the communication board */
 if (DevInitBoard(BoardId) != DRV_NO_ERROR) {
   plc_log_errmsg(1, "Could not initialize communication board %d.", BoardId);
   return -1;
   }

 /* Get Info out of the communication board */
 {
   VERSIONINFO tVersionInfo;
   char *tmp_str1, *tmp_str2;
   char *tmp_str3, *tmp_str4;
   char *tmp_str5, *tmp_str6;
   if (DevGetInfo(BoardId, GET_VERSION_INFO,
                  sizeof(tVersionInfo), &tVersionInfo)
       != DRV_NO_ERROR) {
     plc_log_wrnmsg(2, "Could not obtain communication board %d version info.", BoardId);
   } else {
     plc_log_trcmsg(2, "Communication Board %d: Date %s, "
                       "Serial Num. %s, OS %s-%s-%s, OEM %s",
                    BoardId,
                    tmp_str1=bcd_to_str(tVersionInfo.ulDate),
                    tmp_str2=bcd_to_str(tVersionInfo.ulSerialNo),
                    tmp_str3=chararray_to_str(tVersionInfo.abPcOsName0),
                    tmp_str4=chararray_to_str(tVersionInfo.abPcOsName1),
                    tmp_str5=chararray_to_str(tVersionInfo.abPcOsName2),
                    tmp_str6=chararray_to_str(tVersionInfo.abOemIdentifier)
                   );
     free(tmp_str1); free(tmp_str2);
     free(tmp_str3); free(tmp_str4);
     free(tmp_str5); free(tmp_str6);
   }
 }
 {
   FIRMWAREINFO tFirmwareInfo;
   char *tmp_str1, *tmp_str2;
   if (DevGetInfo(BoardId, GET_FIRMWARE_INFO,
                  sizeof(tFirmwareInfo), &tFirmwareInfo)
       != DRV_NO_ERROR) {
     plc_log_wrnmsg(2, "Could not obtain communication board %d firmware info.", BoardId);
   } else {
     plc_log_trcmsg(2, "Communication Board %d: Firmware-> Name %s, Version %s",
                    BoardId,
                    tmp_str1=chararray_to_str(tFirmwareInfo.abFirmwareName),
                    tmp_str2=chararray_to_str(tFirmwareInfo.abFirmwareVersion)
                   );
     free(tmp_str1);
     free(tmp_str2);
   }
 }
 {
   RCSINFO tRcsInfo;
   if (DevGetInfo(BoardId, GET_RCS_INFO,
                  sizeof(tRcsInfo), &tRcsInfo)
       != DRV_NO_ERROR) {
     plc_log_wrnmsg(2, "Could not obtain communication board %d RCS info.", BoardId);
   } else {
     plc_log_trcmsg(2, "Communication Board %d: RCS-> Version %d, Errors %d, "
                       "Segment Free Counter %d, Device Base Address %d, Driver Type 0x%x "
                       "HostWatchdog 0x%.2X, DeviceWatchdog 0x%.2X",
                    BoardId,
                    tRcsInfo.usRcsVersion,
                    tRcsInfo.bRcsError,
                    tRcsInfo.bSegmentCount,
                    tRcsInfo.bDeviceAdress,
                    tRcsInfo.bDriverType,
                    tRcsInfo.bHostWatchDog,
                    tRcsInfo.bDevWatchDog
                   );
   }
 }
 {
   DEVINFO tDevInfo;
   char *tmp_str;
   if (DevGetInfo(BoardId, GET_DEV_INFO,
                  sizeof(tDevInfo), &tDevInfo)
       != DRV_NO_ERROR) {
     plc_log_wrnmsg(2, "Could not obtain communication board %d Device info.", BoardId);
   } else {
     plc_log_trcmsg(2, "Communication Board %d: DPM Size %d (x 1024) bytes, Dev. Type %d, "
                       "Dev. Model %d, Dev. Id. %s",
                    BoardId,
                    tDevInfo.bDpmSize,
                    tDevInfo.bDevType,
                    tDevInfo.bDevModel,
                    tmp_str=chararray_to_str(tDevInfo.abDevIdentifier)
                   );
     free(tmp_str);
   }
 }
 {
   IOINFO tIoInfo;
   if (DevGetInfo(BoardId, GET_IO_INFO,
                  sizeof(tIoInfo), &tIoInfo)
       != DRV_NO_ERROR) {
     plc_log_wrnmsg(2, "Could not obtain communication board %d I/O info.", BoardId);
   } else {
     plc_log_trcmsg(2, "Communication Board %d: COM bit %d, I/O Exchange Mode %d, "
                       "I/O Exchange Counter %d",
                    BoardId,
                    tIoInfo.bComBit,
                    tIoInfo.bIOExchangeMode,
                    tIoInfo.ulIOExchangeCnt
                   );
   }
 }
 {
   TASKINFO tTaskInfo;
   char *tmp_str;
   int i;
   if (DevGetInfo(BoardId, GET_TASK_INFO,
                  sizeof(tTaskInfo), &tTaskInfo)
       != DRV_NO_ERROR) {
     plc_log_wrnmsg(3, "Could not obtain communication board %d task info.", BoardId);
   } else {
     for (i = 0; i < 7; i++) {
       plc_log_trcmsg(3, "Communication Board %d: Task %d-> Name %s, Version %d, State 0x%.2X",
                      BoardId, i+1,
                      tmp_str=chararray_to_str(tTaskInfo.tTaskInfo[i].abTaskName),
                      tTaskInfo.tTaskInfo[i].usTaskVersion,
                      tTaskInfo.tTaskInfo[i].bTaskCondition
                     );
       free(tmp_str);
     } /* for(;;) */
   }
 }
 {
   DRIVERINFO tDriverInfo;
   if (DevGetInfo(BoardId, GET_DRIVER_INFO,
                  sizeof(tDriverInfo), &tDriverInfo)
       != DRV_NO_ERROR) {
     plc_log_wrnmsg(3, "Could not obtain communication board %d Driver state.", BoardId);
   } else {
     plc_log_trcmsg(3, "Communication Board %d: Driver State-> "
                       "OpenCnt %d, CloseCnt %d, ReadCnt %d, WriteCnt %d, IRQcnt %d, "
                       "InitMsgFlg 0x%.2X, ReadMsgFlg 0x%.2X, WriteMsgFlg 0x%.2X, "
                       "LastFunction 0x%.2X, "
                       "WriteState 0x%.2X, ReadState 0x%.2X, HostFlags 0x%.2X, "
                       "DevFlags 0x%.2X, ExIOflag 0x%.2X, ExIOcnt %d",
                    BoardId,
                    tDriverInfo.ulOpenCnt,
                    /* tDriverInfo.ulCloseCnt, */
                    tDriverInfo.CloseCnt, /* typo in cifuser.h */
                    tDriverInfo.ulReadCnt,
                    tDriverInfo.ulWriteCnt,
                    tDriverInfo.ulIRQCnt,
                    tDriverInfo.bInitMsgFlag,
                    tDriverInfo.bReadMsgFlag,
                    tDriverInfo.bWriteMsgFlag,
                    tDriverInfo.bLastFunction,
                    tDriverInfo.bWriteState,
                    tDriverInfo.bReadState,
                    tDriverInfo.bHostFlags,
                    tDriverInfo.bMyDevFlags,
                    tDriverInfo.bExIOFlag,
                    tDriverInfo.ulExIOCnt
                   );
   }
 }

 /* signal the board an (our) application is ready to use it */
 if (DevSetHostState(BoardId, HOST_READY, 0L) != DRV_NO_ERROR) {
   plc_log_errmsg(1, "Could not change communication board state to READY.");
   return -1;
   }

 cif_util_BoardId_ = BoardId;

 cif_set_status(cif_fully_initialized);

 return 0;
}


int cif_done(void) {
 int res = 0;

 __assert_cif_initialized(0);

 /* clear the application ready state */
 if (DevSetHostState(cif_util_BoardId_, HOST_NOT_READY, 0L) != DRV_NO_ERROR) {
   plc_log_errmsg(1, "Could not change communication board state to NOT_READY.");
   res = -1;
   }

 /* close the link to the communication board */
 if (DevExitBoard(cif_util_BoardId_) != DRV_NO_ERROR) {
   plc_log_errmsg(1, "Could not shutdown communication board.");
   res = -1;
   }

 /* close connection to the device driver */
 if (DevCloseDriver() != DRV_NO_ERROR) {
   plc_log_errmsg(1, "Could not close connection to cif device driver.");
   res = -1;
   }

 cif_rst_status(cif_fully_initialized);

 return res;
}




int cif_DPM_size(void) {

  DEVINFO tDevInfo;

  __assert_cif_initialized(-1);

  if (DevGetInfo(cif_util_BoardId_, GET_DEV_INFO, sizeof(tDevInfo), &tDevInfo)
      != DRV_NO_ERROR)

    return -1;
  else
    return tDevInfo.bDpmSize;
}
