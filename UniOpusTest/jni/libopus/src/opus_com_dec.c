#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef OPUS_BUILD
#error "OPUS_BUILD _MUST_ be defined to build Opus and you probably want a decent config.h, see README for more details."
#endif

#include <stdarg.h>
#include "celt.h"
#include "opus.h"
#include "entdec.h"
#include "modes.h"
#include "API.h"
#include "stack_alloc.h"
#include "float_cast.h"
#include "opus_private.h"
#include "os_support.h"
#include "structs.h"
#include "define.h"
#include "mathops.h"

static int parse_size(const unsigned char *data, opus_int32 len, short *size)
{
   if (len<1)
   {
      *size = -1;
      return -1;
   } else if (data[0]<252)
   {
      *size = data[0];
      return 1;
   } else if (len<2)
   {
      *size = -1;
      return -1;
   } else {
      *size = 4*data[1] + data[0];
      return 2;
   }
}

static int opus_packet_parse_impl(const unsigned char *data, opus_int32 len,
      int self_delimited, unsigned char *out_toc,
      const unsigned char *frames[48], short size[48], int *payload_offset)
{
   int i, bytes;
   int count;
   int cbr;
   unsigned char ch, toc;
   int framesize;
   int last_size;
   const unsigned char *data0 = data;

   if (size==NULL)
      return OPUS_BAD_ARG;

   framesize = opus_packet_get_samples_per_frame(data, 48000);

   cbr = 0;
   toc = *data++;
   len--;
   last_size = len;
   switch (toc&0x3)
   {
   /* One frame */
   case 0:
      count=1;
      break;
   /* Two CBR frames */
   case 1:
      count=2;
      cbr = 1;
      if (!self_delimited)
      {
         if (len&0x1)
            return OPUS_INVALID_PACKET;
         size[0] = last_size = len/2;
      }
      break;
   /* Two VBR frames */
   case 2:
      count = 2;
      bytes = parse_size(data, len, size);
      len -= bytes;
      if (size[0]<0 || size[0] > len)
         return OPUS_INVALID_PACKET;
      data += bytes;
      last_size = len-size[0];
      break;
   /* Multiple CBR/VBR frames (from 0 to 120 ms) */
   default: /*case 3:*/
      if (len<1)
         return OPUS_INVALID_PACKET;
      /* Number of frames encoded in bits 0 to 5 */
      ch = *data++;
      count = ch&0x3F;
      if (count <= 0 || framesize*count > 5760)
         return OPUS_INVALID_PACKET;
      len--;
      /* Padding flag is bit 6 */
      if (ch&0x40)
      {
         int padding=0;
         int p;
         do {
            if (len<=0)
               return OPUS_INVALID_PACKET;
            p = *data++;
            len--;
            padding += p==255 ? 254: p;
         } while (p==255);
         len -= padding;
      }
      if (len<0)
         return OPUS_INVALID_PACKET;
      /* VBR flag is bit 7 */
      cbr = !(ch&0x80);
      if (!cbr)
      {
         /* VBR case */
         last_size = len;
         for (i=0;i<count-1;i++)
         {
            bytes = parse_size(data, len, size+i);
            len -= bytes;
            if (size[i]<0 || size[i] > len)
               return OPUS_INVALID_PACKET;
            data += bytes;
            last_size -= bytes+size[i];
         }
         if (last_size<0)
            return OPUS_INVALID_PACKET;
      } else if (!self_delimited)
      {
         /* CBR case */
         last_size = len/count;
         if (last_size*count!=len)
            return OPUS_INVALID_PACKET;
         for (i=0;i<count-1;i++)
            size[i] = last_size;
      }
      break;
   }
   /* Self-delimited framing has an extra size for the last frame. */
   if (self_delimited)
   {
      bytes = parse_size(data, len, size+count-1);
      len -= bytes;
      if (size[count-1]<0 || size[count-1] > len)
         return OPUS_INVALID_PACKET;
      data += bytes;
      /* For CBR packets, apply the size to all the frames. */
      if (cbr)
      {
         if (size[count-1]*count > len)
            return OPUS_INVALID_PACKET;
         for (i=0;i<count-1;i++)
            size[i] = size[count-1];
      } else if(size[count-1] > last_size)
         return OPUS_INVALID_PACKET;
   } else
   {
      /* Because it's not encoded explicitly, it's possible the size of the
         last packet (or all the packets, for the CBR case) is larger than
         1275. Reject them here.*/
      if (last_size > 1275)
         return OPUS_INVALID_PACKET;
      size[count-1] = last_size;
   }

   if (frames)
   {
      for (i=0;i<count;i++)
      {
         frames[i] = data;
         data += size[i];
      }
   }

   if (out_toc)
      *out_toc = toc;

   if (payload_offset)
      *payload_offset = data-data0;

   return count;
}

int opus_packet_parse(const unsigned char *data, opus_int32 len,
      unsigned char *out_toc, const unsigned char *frames[48],
      short size[48], int *payload_offset)
{
   return opus_packet_parse_impl(data, len, 0, out_toc,
                                 frames, size, payload_offset);
}

int opus_packet_get_samples_per_frame(const unsigned char *data,
      opus_int32 Fs)
{
   int audiosize;
   if (data[0]&0x80)
   {
      audiosize = ((data[0]>>3)&0x3);
      audiosize = (Fs<<audiosize)/400;
   } else if ((data[0]&0x60) == 0x60)
   {
      audiosize = (data[0]&0x08) ? Fs/50 : Fs/100;
   } else {
      audiosize = ((data[0]>>3)&0x3);
      if (audiosize == 3)
         audiosize = Fs*60/1000;
      else
         audiosize = (Fs<<audiosize)/100;
   }
   return audiosize;
}

int opus_packet_get_nb_channels(const unsigned char *data)
{
   return (data[0]&0x4) ? 2 : 1;
}

int opus_packet_get_nb_frames(const unsigned char packet[], opus_int32 len)
{
   int count;
   if (len<1)
      return OPUS_BAD_ARG;
   count = packet[0]&0x3;
   if (count==0)
      return 1;
   else if (count!=3)
      return 2;
   else if (len<2)
      return OPUS_INVALID_PACKET;
   else
      return packet[1]&0x3F;
}