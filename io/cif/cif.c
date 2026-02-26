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
 * this file implements the cif - Hilscher card drivers
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> /* required for bzero(...) */

#include <plc.h>
#include "../lib/misc/string_util.h"
#include "../lib/io/io_hw.h"

#include "cif.h"


/*

  *** NOTE ***

  The MatPLC philosofy requires that the user be able to
  map plc points that are scattered about the plcGlobal memory
  into consecutive memory locations of the cif device.

  We could do this using one of two methods:

  (A)
  We first gather all the data we will be writing to the cif
  device into our own (malloc'ed) memory location, and pass
  it all in one go to the cif device already in the correct order.

  (B)
  We could ask Hilscher to write a new ioctl that would allow us
  to do partial writes to the cif device's memory directly. This
  would allow us to use the cif device's memory itself to integrate
  the scatered plc points, and threfore do without this module's own
  malloc'ed memory.
  Note that the current exchangeIO() function, although allowing for
  partial data exchange, it always assumes that the data is complete
  and signals the card to start sending out messages over the network.
  We we really need for this solution are two independent functions, one
  for writing data to the process image, and another to signal the card
  to start processing the data.

  Solutions (A) and (B) are the typical memory vs speed tradeoff
  commonly seen in programs. (A) uses more memory, but is probably
  faster than (B) because unlike (B), (A) involves a single call into
  the kernel. This means a single copy_from_user(...) of the call
  parameters will be made in the device driver, and interrupt setting
  and resetting is done only once.

  We use solution (A) mostly because it is the only one currently
  supported by the device driver. If we ever decide we want solution
  (B), it shouldn't be too difficult to add the required ioctls to the
  device driver, especially considering the device driver source
  code is GPL'd. Actually, I've (Mario) already been digging into the
  device driver source code, and it seems it would be rather
  straight forward.

*/


/********************/
/* global variables */
/********************/
const char *IO_MODULE_DEFAULT_NAME = "cif";

static unsigned short BoardId_ = 0;

/* process image size in the DPM of the cif card */
/* This variable is first set not by getting it from the card itself
 * but from the MatPLC config file.
 * It is then used in io_hw_parse_io_addr() to issue warnings
 * when a point is mapped outside the process image.
 * At runtime (io_hw_init() ) it is updated with the real size of
 * the process image of the card being used, and a warning is
 * issued if this value differs from the one specified in the config file.
 */
static u32 process_image_size_ = 0;


/* time out used to access the cif card */
static unsigned long timeout_ = 0;

/* The maps and their parameters required for solution (A)
 * discussed above...
 *
 * We start off with some hocus-pocus...
 * Instead of malloc() memory the size of the process image to use
 * as out_map_ and in_map_ arrays, we figure out what is the first and
 * last byte of each process image that is being accessed, and malloc()
 * only the memory between those two limits.
 * There is a very high probabilbity that this will substantially
 * reduce the size of the array we write and read from the cif
 * card on each I/O exchange!
 *
 * We use the io_hw_parse_io_addr() function to keep the
 * map_ofs and map_sz updated with the correct values.
 * We start off with the offset set to any value,
 * and size set to 0.
 *
 * The memory for the maps is malloc() in the io_hw_init() function.
 */
static u8 *out_map_ = NULL;
static u8 * in_map_ = NULL;

static u16 out_map_ofs_ = 0;
static u16  in_map_ofs_ = 0;

static u16 out_map_sz_ = 0;
static u16  in_map_sz_ = 0;


/*
 * Array of bytes used to store which bits of the output
 * process image are already being written to. This is merely
 * used to generate warnings during config time.
 *
 * Memory is malloced in io_hw_parse_config(),
 * used by io_hw_parse_io_addr(),
 * and freed in io_hw_init()
 *
 * I know, not at all elegant! Sugestions are welcome...
 */
u8 *used_bits_ = NULL;


/*************************/
/* global variable types */
/*************************/

/* our interpretation of the generic io_addr_t */
typedef struct {
	  u16 offset;
	  u8  mask[5];
	  u8  mode;     /* (mode & 0x0F)      -> bit_shift  */
                        /* (mode & 0xF0) >> 4 -> numb_bytes */
        } io_addr_cif_t;



/*********************************/
/* the hardware access functions */
/*********************************/

/*
   NOTE:
     I am copying the u32 byte by byte because I am not sure
     what would happen if I tried to write the u32 value onto
     an un-aligned 4 byte sequence, for e.g., writing the u32
     value onto bytes out_map_[1] .. out_map_[4].
     Note that the first byte of out_map_ is out_map_[0].
     out_map_[1] is hte second byte of the array, and probably
     is located at an odd numbered byte address!
 */

#define io_addr_cif ((io_addr_cif_t *)io_addr)

int io_hw_read(io_addr_t *io_addr, u32 *value)
{
#define io_byte     ((u8 *)value)
  u16 abs_ofs = io_addr_cif->offset - in_map_ofs_;
  u8 byte_4 = 0;

  switch (io_addr_cif->mode & 0xF0) {
    case(0x50):     byte_4  = (in_map_[abs_ofs+4] & io_addr_cif->mask[4]);
    case(0x40):  io_byte[3] = (in_map_[abs_ofs+3] & io_addr_cif->mask[3]);
    case(0x30):  io_byte[2] = (in_map_[abs_ofs+2] & io_addr_cif->mask[2]);
    case(0x20):  io_byte[1] = (in_map_[abs_ofs+1] & io_addr_cif->mask[1]);
    case(0x10):  io_byte[0] = (in_map_[abs_ofs  ] & io_addr_cif->mask[0]);
                 break;
    default:     return -1;
  } /* switch() */

    /* NOTE: *value is unsigned, so the C standard guarantees we
             will get 0's shifted into the high bits.
             For unsigned types, the value shifted into the high
             bits is 0 for positive values, and compiler specific
             for negative values!
             WE WANT 0's !!!
    */
  *value >>= io_addr_cif->mode & 0x0F;
  io_byte[3] |= byte_4 << (8 - (io_addr_cif->mode & 0x0F));

  return 0;
#undef io_byte
}


int io_hw_write(io_addr_t *io_addr, u32 value)
{
#define io_byte     ((u8 *)&value)
  u16 abs_ofs = io_addr_cif->offset - out_map_ofs_;

  if ((io_addr_cif->mode & 0xF0) == 0x50)
   out_map_[abs_ofs+4] =
            (out_map_[abs_ofs+4] & ~io_addr_cif->mask[4]) |
            ((io_byte[3] >> (8 - (io_addr_cif->mode & 0x0F))) & io_addr_cif->mask[4]);

  value <<= io_addr_cif->mode & 0x0F;

  switch (io_addr_cif->mode & 0xF0) {
    case(0x50):
    case(0x40):  out_map_[abs_ofs+3] =
                      (out_map_[abs_ofs+3] & ~io_addr_cif->mask[3]) |
                      (io_byte[3] & io_addr_cif->mask[3]);
    case(0x30):  out_map_[abs_ofs+2] =
                      (out_map_[abs_ofs+2] & ~io_addr_cif->mask[2]) |
                      (io_byte[2] & io_addr_cif->mask[2]);
    case(0x20):  out_map_[abs_ofs+1] =
                      (out_map_[abs_ofs+1] & ~io_addr_cif->mask[1]) |
                      (io_byte[1] & io_addr_cif->mask[1]);
    case(0x10):  out_map_[abs_ofs  ] =
                      (out_map_[abs_ofs  ] & ~io_addr_cif->mask[0]) |
                      (io_byte[0] & io_addr_cif->mask[0]);
                 break;
    default:     return -1;
  } /* switch() */

  return 0;
#undef io_byte
}


int io_hw_read_end(void) {
 return cif_read_in(in_map_ofs_,             /* recv offset */
                    in_map_sz_,              /* recv size   */
                    in_map_,                 /* recv buffer */
                    timeout_);               /* ulTimeout   */
}


int io_hw_write_end(void) {
  return cif_write_out(out_map_ofs_,            /* send offset */
                       out_map_sz_,             /* send size   */
                       out_map_,                /* send buffer */
                       timeout_);               /* ulTimeout   */
}


int io_hw_parse_config(void) {
  u32 tmp_u32;

  /* This is the first function of the io_hw library that gets called,
   * so it is probably the best place to make any consistency checks
   * between this and the generic io library.
   *
   * make sure our io_addr_cif_t struct fits inside an io_addr_t struct
   */
  if (sizeof(io_addr_t) < sizeof(io_addr_cif_t)) {
    plc_log_errmsg(1, "This I/O module has been compiled with an incompatible io library.");
    plc_log_errmsg(2, "sizeof(io_addr_t) defined in io.h differs from "
                      "sizeof(io_addr_cif_t) defined in cif.c.");
    return -1;
  }

  /* now lets really read the config... */

  /* get the BoardID this module will use */
  if (conffile_get_value_u32(CIF_BOARDID_NAME,
	                     &tmp_u32,
			     CIF_BOARDID_MIN,
			     CIF_BOARDID_MAX,
			     CIF_BOARDID_DEF)
      < 0) {
    char *tmp_str = conffile_get_value(CIF_BOARDID_NAME);
    plc_log_wrnmsg(1, "Invalid cif board id %s. "
                        "Using default %d.",
                     tmp_str, tmp_u32);
    free(tmp_str);
  }
  BoardId_ = tmp_u32;

  /* get the timeout this module will use */
  if (conffile_get_value_u32(CIF_TIMEOUT_NAME,
	                     &tmp_u32,
			     CIF_TIMEOUT_MIN,
			     CIF_TIMEOUT_MAX,
			     CIF_TIMEOUT_DEF)
      < 0) {
    char *tmp_str = conffile_get_value(CIF_BOARDID_NAME);
    plc_log_wrnmsg(1, "Invalid timeout %s. "
                        "Using default %d.",
                     tmp_str, tmp_u32);
    free(tmp_str);
  }
  timeout_ = tmp_u32;

  /* Get the DPM size of the card that will be used. Value is in kBytes */
  /* This will later be confirmed at runtime.        */
  if (conffile_get_value_u32(CIF_MAX_DPM_SIZE_NAME,
	                     &tmp_u32,
			     CIF_MAX_DPM_SIZE_MIN,
			     CIF_MAX_DPM_SIZE_MAX,
			     CIF_MAX_DPM_SIZE_DEF)
      < 0) {
    char *tmp_str = conffile_get_value(CIF_BOARDID_NAME);
    tmp_u32 = CIF_MAX_DPM_SIZE_DEF;
    plc_log_wrnmsg(1, "Invalid DPM size %s. "
                        "Using default %d.",
                     tmp_str, CIF_MAX_DPM_SIZE_DEF);
    free(tmp_str);
  }
  /* determine the size of the card's In and Out process images */
  process_image_size_ = ((tmp_u32 * 1024) - 1024) / 2;
  if ((used_bits_ = malloc(process_image_size_)) == NULL)
    return -1;
  bzero(used_bits_, process_image_size_);

  return 0;
}



/* parse the io address */
/* accepted syntax:
 *   <ofs>[.<bit>]
 *
 *   <ofs> : offset into the in/out Process Image
 *   <bit> : defaults to 0
 */
int io_hw_parse_io_addr(io_addr_t *io_addr,
		 	const char *addr_str,
			dir_t dir,
			int pt_len)
{
  const u8 mask_all_one = ~0;/* make sure we have the correct length of ones */
  u8 bit_shift, num_bytes;
  u32   tmp_u32;
  char *tmp_str;
  const char *tmp_cstr;
  io_addr_cif_t *io_addr_ptr = (io_addr_cif_t *)io_addr;
  u16 *map_ofs, *map_sz;

  if (strlen(addr_str) <= 0)
    return -1;

  /* read the <bit>, if present */
  tmp_u32 = CIF_IOADDR_DEF_BIT;
  tmp_cstr = addr_str + strcspn(addr_str, ".");
  if (*tmp_cstr == '.')
    if (string_str_to_u32(tmp_cstr + 1, &tmp_u32, 0, 7) < 0) {
      plc_log_wrnmsg(1, "IO address %s does not conform to the <ofs>[.<bit>] "
                        "synntax. Could not determine the value of <bit>.", addr_str);
      return -1;
    }
  io_addr_ptr->mode = 0x00;
      /* the bit shift */
  bit_shift=tmp_u32;
  io_addr_ptr->mode |=   bit_shift & 0x0F;
      /* the number of bytes */
  num_bytes = ((1 + (pt_len - 1 + bit_shift)/8) & 0x0F);
  io_addr_ptr->mode |= num_bytes << 4;
  io_addr_ptr->mask[4] = 0x00;

      /* the masks */
        /* 1st byte */
  io_addr_ptr->mask[0] = (~(mask_all_one << pt_len)) << bit_shift;
        /* middle bytes */
  memset((void *)&(io_addr_ptr->mask[1]), mask_all_one, num_bytes - 1);
        /* last byte */
  io_addr_ptr->mask[num_bytes-1] = ~(mask_all_one << ((pt_len + bit_shift) % 8)) ;
  if ((pt_len + bit_shift) % 8 == 0)
    io_addr_ptr->mask[num_bytes-1] = mask_all_one;

  /* read the offset <ofs> */
  tmp_str = strdup(addr_str);
  *(tmp_str + strcspn(addr_str, ".")) = '\0';
  tmp_u32 = 0;
  if (string_str_to_u32(tmp_str, &tmp_u32, 0, process_image_size_ -1) < 0) {
    free(tmp_str);
    plc_log_wrnmsg(1, "IO address %s has invalid offset. "
                      "It should be in the range [0 .. %d]. "
                      "Have you correctly configured the card's DPM size (DPMsize=xx)?",
                   addr_str, process_image_size_ -1);
    return -1;
  }
  free(tmp_str);
  io_addr_ptr->offset = tmp_u32;

  /* check if point doesn't overflow outside the process image */
  if (io_addr_ptr->offset + ((pt_len - 1 + bit_shift) / 8)
       > process_image_size_ -1) {
    plc_log_wrnmsg(1, "PLC point has more bits (%d) than can fit in the "
                      "process image (bytes 0..%d) starting at byte.bit %d.%d.",
                   pt_len, process_image_size_ - 1,
                   io_addr_ptr->offset, bit_shift);
    /* in this case, we do not ignore it, but return an error */
    return -1;
  }

  /* if an output, check if it doesn't overlay any previously mapped points... */
  if (dir == dir_out) {
    int i, res;

    for (res = 0, i = 0; i < num_bytes; i++)
      res |= io_addr_ptr->mask[i] & used_bits_[io_addr_ptr->offset + i];
    if (res != 0) {
      plc_log_wrnmsg(1, "PLC point will be overwriting output bits "
                        "already being written to by another plc point."
                    );
    }
    /* reserve the points we are using for future checks... */
    for (i = 0; i < num_bytes; i++)
      used_bits_[io_addr_ptr->offset + i] |= io_addr_ptr->mask[i];
  }

  /* update the xx_first and xx_last variables... */
  if (dir == dir_out) {
    map_ofs = &out_map_ofs_;
    map_sz  = &out_map_sz_;
  } else
  if (dir == dir_in) {
    map_ofs = &in_map_ofs_;
    map_sz  = &in_map_sz_;
  } else
  /* strange error... Should never occur... */
    return -1;

  if (*map_sz == 0) {
    *map_sz = num_bytes;
    *map_ofs = io_addr_ptr->offset;
  }
  if (io_addr_ptr->offset < *map_ofs) {
    /* resize downward... */
    *map_sz += *map_ofs - io_addr_ptr->offset;
    *map_ofs = io_addr_ptr->offset;
  }
  if (io_addr_ptr->offset + num_bytes > *map_ofs + *map_sz) {
    /* resize upward... */
    *map_sz = (io_addr_ptr->offset + num_bytes) - *map_ofs;
  }

  return 0;
}



int io_hw_init(void)
{
  int card_DPM_size;

  /* We start off by free() the used_bits_ array */
  free(used_bits_);

  /* Now allocate the in/out process image maps... */
  if (out_map_sz_ > 0)
    if ((out_map_ = (u8 *)malloc(out_map_sz_)) == NULL) {
      plc_log_errmsg(1, "Out of memory.");
      io_hw_done();
      return -1;
    }

  if (in_map_sz_ > 0)
   if ((in_map_ = (u8 *)malloc( in_map_sz_)) == NULL) {
      plc_log_errmsg(1, "Out of memory.");
      io_hw_done();
      return -1;
   }

  /* OK. Now lets try accessing the cif card... */
  if (cif_init(BoardId_) < 0) {
    plc_log_errmsg(1, "Error connecting to cif card.");
    io_hw_done();
    return -1;
  }

  /* Initialise the output map with the current output status */
  if (cif_read_out(out_map_ofs_,            /* send offset */
                   out_map_sz_,             /* send size   */
                   out_map_)                /* send buffer */
      < 0) {
    plc_log_errmsg(1, "Error connecting to cif card.");
    io_hw_done();
    return -1;
  }

  /* Initialise the in_map_ with 0.
   * This is important for points that may fall outside
   * the card's available process image size.
   * Please read further comments in this same function...
   */
  memset(in_map_, 0, in_map_sz_);

  /* Now verify if the card we are acessing really does have the
     DPM with the size the user said it has...
   */
  if ((card_DPM_size = cif_DPM_size()) < 0)
    plc_log_wrnmsg(1, "Could not obtain card's DPM size.");
  else
    if ((((card_DPM_size * 1024) - 1024) / 2) != process_image_size_)
      plc_log_wrnmsg(1, "Card's DPM size (%d) is different to the size specified "
                        "in the configuration file (%d).",
                     card_DPM_size, 1 + (process_image_size_ * 2)/1024);

  /* Now verify if we will be trying to access bytes outside the
     card's available process image
   */
  process_image_size_ = ((card_DPM_size * 1024) - 1024) / 2;
  if (process_image_size_ < out_map_ofs_ + out_map_sz_) {
    plc_log_wrnmsg(1, "Some point(s) have been configured to be written to offsets "
                      "larger than the card's available process image size, "
                      "and will therefore be ignored.");
     /* Although the out_map_ includes memory for the points above
      * the process image size, and these points will be written to this
      * map by the io_hw_write(), they will not be sent to the cif
      * card in io_hw_write_end() function.
      */
    out_map_sz_ = process_image_size_ - out_map_ofs_;
  }

  if (process_image_size_ <  in_map_ofs_ +  in_map_sz_) {
    plc_log_wrnmsg(1, "Some point(s) have been configured to be read from offsets "
                      "larger than the card's available process image size, "
                      "and will therefore always be set to 0.");
     /* Although the in_map_ includes memory for the points above
      * the process image size, and these points will be read from this
      * map by the io_hw_read(), they will not be updated with the correct
      * input values in io_hw_read_end().
      * It is important that the in_map_ gets initially set to 0
      * so these points will always get read with the value 0.
      */
    in_map_sz_ = process_image_size_ - in_map_ofs_;
  }

  return 0;
}


int io_hw_done(void) {
  free(out_map_); out_map_ = NULL;
  free( in_map_);  in_map_ = NULL;
  return cif_done();
}

