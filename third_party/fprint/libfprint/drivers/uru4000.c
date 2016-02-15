/*
 * Digital Persona U.are.U 4000/4000B driver for libfprint
 * Copyright (C) 2007 Daniel Drake <dsd@gentoo.org>
 * Portions Copyright (c) 2010 DigitalPersona, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#define FP_COMPONENT "uru4000"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <openssl/aes.h>
#include <usb.h>

#include <fp_internal.h>
#include "uru4000.h"

#define EP_INTR			(1 | USB_ENDPOINT_IN)
#define EP_DATA			(2 | USB_ENDPOINT_IN)
#define USB_RQ			0x04
#define CTRL_IN			(USB_TYPE_VENDOR | USB_ENDPOINT_IN)
#define CTRL_OUT		(USB_TYPE_VENDOR | USB_ENDPOINT_OUT)
#define CTRL_TIMEOUT	5000
#define BULK_TIMEOUT	5000
#define DATABLK1_RQLEN	0x10000
#define DATABLK2_RQLEN	0xb340
#define DATABLK2_EXPECT	0xb1c0
#define CAPTURE_HDRLEN	64
#define IRQ_LENGTH		64
#define CR_LENGTH		16

enum {
	IRQDATA_SCANPWR_ON = 0x56aa,
	IRQDATA_FINGER_ON = 0x0101,
	IRQDATA_FINGER_OFF = 0x0200,
	IRQDATA_DEATH = 0x0800,
};

enum {
	REG_HWSTAT = 0x07,
	REG_MODE = 0x4e,
	FIRMWARE_START = 0x100,
	REG_RESPONSE = 0x2000,
	REG_CHALLENGE = 0x2010,
};

enum {
	MODE_INIT = 0x00,
	MODE_AWAIT_FINGER_ON = 0x10,
	MODE_AWAIT_FINGER_OFF = 0x12,
	MODE_CAPTURE = 0x20,
	MODE_SHUT_UP = 0x30,
	MODE_READY = 0x80,
};

enum {
	MS_KBD,
	MS_INTELLIMOUSE,
	MS_STANDALONE,
	MS_STANDALONE_V2,
	DP_URU4000,
	DP_URU4000B,
};

int isUru4000 = 0;

static const struct uru4k_dev_profile {
	const char *name;
	gboolean auth_cr;
} uru4k_dev_info[] = {
	[MS_KBD] = {
		.name = "Microsoft Keyboard with Fingerprint Reader",
		.auth_cr = FALSE,
	},
	[MS_INTELLIMOUSE] = {
		.name = "Microsoft Wireless IntelliMouse with Fingerprint Reader",
		.auth_cr = FALSE,
	},
	[MS_STANDALONE] = {
		.name = "Microsoft Fingerprint Reader",
		.auth_cr = FALSE,
	},
	[MS_STANDALONE_V2] = {
		.name = "Microsoft Fingerprint Reader v2",
		.auth_cr = TRUE,	
	},
	[DP_URU4000] = {
		.name = "Digital Persona U.are.U 4000",
		.auth_cr = FALSE,
	},
	[DP_URU4000B] = {
		.name = "Digital Persona U.are.U 4000B",
		.auth_cr = FALSE,
	},
};

/* As we don't know the encryption scheme, we have to disable encryption
 * by powering the device down and modifying the firmware. The location of
 * the encryption control byte changes based on device revision.
 *
 * We use a search approach to find it: we look at the 3 bytes of data starting
 * from these addresses, looking for a pattern "ff X7 41" (where X is dontcare)
 * When we find a pattern we know that the encryption byte is the X7 byte.
 */
static const uint16_t fwenc_offsets[] = {
	0x6cc, 0x510, 0x62d, 0x792, 0x7f4,
};

struct uru4k_dev {
	const struct uru4k_dev_profile *profile;
	uint8_t interface;
	AES_KEY aeskey;
};

/* For 2nd generation MS devices */
static const unsigned char crkey[] = {
	0x79, 0xac, 0x91, 0x79, 0x5c, 0xa1, 0x47, 0x8e,
	0x98, 0xe0, 0x0f, 0x3c, 0x59, 0x8f, 0x5f, 0x4b,
};

/*
 * The following portion supplied by DigitalPersona, Inc.
 */

void fill_skipped_lines (PDEV_HW_HEADER pHwHdr)
{
   unsigned char* pbyData = (unsigned char*)(pHwHdr+1);
   unsigned int delta = 0;          // total # of lines skipped
   unsigned int nSkippedLine = 0;   // line # at which the skipped lines begin
   unsigned int nLine = 0;          // line that are normal
   PFRAME_INFO pfi = NULL;
   
   // Find the skipped line
   for (unsigned int i = 0; i < FRAME_INFO_ARRAY_SIZE; i++)
   {
      if (nLine >= pHwHdr->ImageHeader.wHeight)
         break;
      
      pfi = &pHwHdr->ImageHeader.FrameInfo[i];
      if (0 == pfi->byNbOfLines) // check for 0-termination
         break;
      
      if (pfi->Flags.bit.Skip && !pfi->Flags.bit.NotClocked)      // line has skipped
      {
         nSkippedLine = nLine;
         delta += pfi->byNbOfLines;
         break;
      }
      
      nLine += pfi->byNbOfLines;
   }
   
   if (nSkippedLine)
   {
      unsigned int nLinesToMove = pHwHdr->ImageHeader.wHeight - nSkippedLine;
      
      g_memmove (
         pbyData + pHwHdr->ImageHeader.wWidth * (nSkippedLine+delta),  // dest  
         pbyData + pHwHdr->ImageHeader.wWidth * nSkippedLine,          // src
         pHwHdr->ImageHeader.wWidth * nLinesToMove
         );
      
      pHwHdr->ImageHeader.wHeight += delta;
      pfi->Flags.bit.Skip = 0;
   }
}

void subtract_optical_black (unsigned char* buff, unsigned int siz, unsigned int width)
{
   unsigned int nLines = siz / width;
   
   for (unsigned int j = 0; j < nLines; j++) {
      unsigned char* ps = buff + j*width;
      unsigned int u = 0;
      unsigned int n = 0, i;

      for (i = 0; i < DEV_IMAGE_OWHITE_START-1; i++) {
         u += ps[i]; 
         n++;
      }           
      if (n > 0) {
         u = (u + n/2) / n;
      }
      
      if (n > 0 && u > 0) {
         unsigned char oblack_avg = (unsigned char)(u);  
       
         for (i = DEV_IMAGE_OWHITE_START; i < (DEV_IMAGE_OWHITE_START+IMAGER_OWHITE_WIDTH); i++)  {
            if (ps[i] >= DEV_MIN_SATURATION_LEVEL) {
               // overflow: preserve o.white
               ps[i] = MAX_PIXEL_VALUE; // 0xFE;
            } else if (ps[i] > u) {
               ps[i] -= oblack_avg;
            } else {
               ps[i] = 0;     // optical black
            }
         }
         
         for (i = 0; i < DEV_IMAGE_OWHITE_START; i++) {
            ps[i] = oblack_avg;
         }
         
         for (i = DEV_IMAGE_OWHITE_START+IMAGER_OWHITE_WIDTH; i < DEV_IMAGE_WIDTH; i++)  {
            ps[i] = oblack_avg;
         }
      }
   } 
}

float my_fabs(float x)
{
   if (x>=0) return x;
   else return -x;
}

void normalize_img (
   unsigned int   inRows, 
   unsigned int   inCols, 
   unsigned char* inOutImg,
   NORMALIZE_IMG* NormImg
)
{
   long long int var = 0;
   int slope, mean = 0, std = 0;
   float halfPIbySqrt3;
   int hist_sum = 0;
   int histogram[GRAYSCALES]={0}, sum_histogram = 0;
   unsigned i, j, loc, count = 0, filler_high = DW_BACKGROUND, filler_low = BLACK, rowscols = inRows*inCols;
   float sx=0.0f, sy=0.0f, sxx=0.0f, sxy=0.0f;
   unsigned char gamma_map[GRAYSCALES]={0};
   int *max_rows =  (int *) g_alloca(inRows * sizeof(int));

   NormImg->fGamma = (NormImg->fGamma == 0) ? GAMMA : NormImg->fGamma;

   halfPIbySqrt3 = 3.14159f/NormImg->fGamma/1.7321f; /*(float)sqrt(3.0f)*/
   // This part of the code estimates the Light non-uniformity in the verticle direction 
   for (i=0; i<inRows; i++) 
   {
      max_rows[i] = 0;
      for (j=0; j<inCols; j++)
      {
         loc = j + i*inCols;
         if (inOutImg[loc] > max_rows[i]) max_rows[i] =  inOutImg[loc];
      }
   }
   for (i=0; i<inRows; i++)
   {
      sx += (float)i;
      sy += (float)max_rows[i];
      sxy += (float)i* max_rows[i];
      sxx += i*(float)i;
   }

   float b = 0;  // by default we don't change the image if del == 0. Slope would be null
   float del=(float)(inRows*sxx - sx*sx);
   if (del != 0) {
      b=(inRows*sxy- sx*sy)/del; //a=(sxx*sy - sx*sxy)/del; 
   }
   slope = (int)(b * inRows + 0.5);
   /////////////////////////////////////////////////////////////////////////////////////////////////////////


   // This part of the code removes the Light non-uniformity and also estimates mean and variance of image
   for(i=0; i<inRows; i++)
   {	
      int additive = slope*((int)inRows-(int)i)/((int)inRows);
      for(j=0; j<inCols; j++)
      {
         loc = j + i*inCols;
         int temp = inOutImg[loc] + additive;
         if (temp <= DW_BACKGROUND) inOutImg[loc] = (unsigned char)temp;
         else inOutImg[loc] = (unsigned char)DW_BACKGROUND;
         mean += inOutImg[loc];                
         var  += inOutImg[loc] * inOutImg[loc];
         count ++;
      }
   }
   if (count == 0 || count == 1) {
      // It should never happen. But if!!!!!!!
      // avoid division by zero
      count = 2;
   }
   mean = mean/count; 
   std  = (int)sqrt( (double) ((var/count - mean*mean)*count/(count-1))); // best unbiased estimate
   if (std == 0) std = 1; // avoid divide by 0 in next for loop
   /////////////////////////////////////////////////////////////////////////////////////////////////////////


   // This part of the code applies sigmoid function to the image intensity and builds a histogram
   for (i = 0;i < GRAYSCALES; i++)
   {
      float x = (float) ((float)((int)i - mean) * halfPIbySqrt3 / std / 4.1f);
      if (x <= -1.0f)
         gamma_map[i] = (unsigned char)BLACK;
      else if (x >= 1.0f)
         gamma_map[i] = (unsigned char)DW_BACKGROUND;
      else 
         gamma_map[i] = (unsigned char)(DW_BACKGROUND * (0.5 + x*(1-my_fabs(x)/2.0f))); 
   }
   for (i = 0; i < rowscols; i++)
   {
      inOutImg[i] = gamma_map[inOutImg[i]];
      histogram[inOutImg[i]]++;
   }
   for(i = 0; i < GRAYSCALES; i++)
   sum_histogram += histogram[i];
   /////////////////////////////////////////////////////////////////////////////////////////////////////////


   // This part of the code removes some noise in the background and streches intensity linearly to 0-255         
   for(int i_int = DW_BACKGROUND; i_int >= BLACK; i_int--)
   {
      hist_sum += histogram[i_int];
      if (sum_histogram != 0)
      {
         if (hist_sum*100/(sum_histogram) > NormImg->iPerHIgnore) // 5 percent; since background is "high" - more noise here to be removed
         {
            filler_high = i_int;
            break;
         }
      }
   }
   hist_sum = 0;
   for(i = BLACK; i <= DW_BACKGROUND; i++)
   {
      hist_sum += histogram[i];
      if (sum_histogram != 0)
      {
         if (hist_sum*1000/(sum_histogram) > NormImg->iPerLIgnore) // 0.1 percent; since forground is "low" - only saturation here  
         {
            filler_low = i;
            break;
         }
      }
   } 
   memset(gamma_map,0,GRAYSCALES);
   for(i = BLACK; i <= DW_BACKGROUND; i++)
   {
      if (i < filler_low)
         gamma_map[i] = (unsigned char)BLACK;
      else if (i > filler_high)
         gamma_map[i] = (unsigned char)(DW_BACKGROUND-1);  // image is 0-254; 255 is reserved
      else if ((filler_high-filler_low) != 0)
         gamma_map[i] = (unsigned char)(((int)i - (int)filler_low)*(DW_BACKGROUND-1)/(filler_high-filler_low));
   }

   for (i=0; i < rowscols; i++)
   {
      inOutImg[i] = gamma_map[inOutImg[i]];
   }
}

void inverse_projection (
   float Pin[2], 
   const CORRECTION_COEFF* pCorCoeff, 
   float P3d[3]
)
{
   float Pnorm[2];
   float b[2];
   float A[2][2];
   float d[2];
   float detA;
   float x, y;
   
   b[0] = (Pin[0] - pCorCoeff->t[0]) / (pCorCoeff->f[0]);
   b[1] = (Pin[1] - pCorCoeff->t[1]) / (pCorCoeff->f[1]);
   
   Pnorm[0] = b[0];
   Pnorm[1] = b[1];
   
   /* builds a system A*[x y]' = d, where (x,y) are the coords of Pout */
   A[0][0] = -1;
   A[0][1] = (Pnorm[0]*pCorCoeff->r2[2]);   
   A[1][0] = 0;
   A[1][1] = (Pnorm[1]*pCorCoeff->r2[2] - pCorCoeff->r2[1]);
   
   d[0] = (pCorCoeff->T[0] - Pnorm[0]*pCorCoeff->T[2]);
   d[1] = (pCorCoeff->T[1] - Pnorm[1]*pCorCoeff->T[2]);
   
   detA = A[0][0]*A[1][1] - A[0][1]*A[1][0];
   
   x = (A[1][1]*d[0] - A[0][1]*d[1]) / detA;
   y = (-A[1][0]*d[0] + A[0][0]*d[1]) / detA;
   
   P3d[0] = x;
   P3d[1] = y;
   P3d[2] = 0;
}

void dewarp (
   int inRows, 
   int inCols, 
   const unsigned char* inImg, 
   int outRows, 
   int outCols,
   unsigned char* outImg,
   const CORRECTION_COEFF* pCorCoeff,
   const CENTERING_INFO* pCenterInfo
)
{
   int i, j, PinY; 

   float y, PtransfY, InvPtransfZ, PnormY;

   int         iColNb;
   float       yr20T0;
   float       yr21T1;
   float       yr22T2;
   int         iXmin;
   int         iXmax;
   int         iYmin;
   int         iYmax;
   float       fk2;           
   float       reductionFactor;
   float       r10;
   float       r11;
   float       r12;
   float       r20;
   float       r21;
   float       r22;
   float       T0;
   float       T1;
   float       T2;
   float       f0;
   float       f1;
   float       t0;
   float       t1;
   float       t0_5;
   float       t1_5;
   float       deltaX=0;
   float       deltaY=0;
   float       distDeltaX=0;
   float       distDeltaY=0;
   float       Pin[2] = {0, 0};
   float       P3d[3] = {0, 0, 0};
   
   // for speed improvement we use local variable instead of the structure 
   // Simpler compution based on the stack
   iXmin             = pCorCoeff->iXmin;
   iXmax             = pCorCoeff->iXmax;
   iYmin             = pCorCoeff->iYmin;
   iYmax             = pCorCoeff->iYmax;
   fk2               = pCorCoeff->fk2;
   reductionFactor   = 1 / (pCorCoeff->fScaleFactor / 700 * DEV_IMAGE_X_RES);
   r10               = pCorCoeff->r1[0];
   r11               = pCorCoeff->r1[1];
   r12               = pCorCoeff->r1[2];
   r20               = pCorCoeff->r2[0];
   r21               = pCorCoeff->r2[1];
   r22               = pCorCoeff->r2[2];
   T0                = pCorCoeff->T[0];
   T1                = pCorCoeff->T[1];
   T2                = pCorCoeff->T[2];
   f0                = pCorCoeff->f[0];
   f1                = pCorCoeff->f[1];
   t0                = pCorCoeff->t[0];
   t1                = pCorCoeff->t[1];
   t0_5              = t0 + 0.5f;
   t1_5              = t1 + 0.5f;
   
   //assert(inRows == DW_IN_ROWS && inCols = DW_IN_COLS);
   
   if (pCenterInfo == NULL) 
   {
      deltaX = 0;
      deltaY = 0;
	}
	else 
   {
      deltaX = pCenterInfo->deltaX;
      deltaY = pCenterInfo->deltaY;
   }
   
   Pin[0] = -deltaX;
   Pin[1] = -deltaY;
   inverse_projection(Pin, pCorCoeff, P3d);
   
   distDeltaX = P3d[0]/reductionFactor;
   distDeltaY = P3d[1]/reductionFactor;
         
   float xt = distDeltaX * reductionFactor;
   const int P = 16;
   
   for(i=0; i<outRows; i++)
   {
      y = (i + distDeltaY) * reductionFactor;
      
      iColNb = i*outCols;
      yr20T0 = T0;
      yr21T1 = y*r21 + T1;
      yr22T2 = y*r22 + T2;
      
      PtransfY = yr21T1;
      InvPtransfZ = 1 / (yr22T2);
      PnormY = PtransfY * InvPtransfZ;
      PinY = (int) (PnormY*f1 + t1_5 + deltaY);
      if (PinY >= inRows)
      {
         memset((outImg+i*outCols),DW_BACKGROUND,(outRows-i)*outCols);
         break;
      }
      
      float InvPtransfZf0 = InvPtransfZ * f0;
      int xReductionFactor = (int) ((reductionFactor * InvPtransfZf0) * (1<<P));
      int x = (int) (((xt + yr20T0)*InvPtransfZf0 + t0 + deltaX) * (1<<P));
      int PinYinCols = PinY*inCols;
      
      for(j=0; j<outCols; j++)
      {
         int x0 = x;
         int x1 = x0 >> P;
         int x2 = x1 + 1;
         x0 &= (1<<P)-1;
         
         if (x2 >= inCols)
         {
            memset((outImg+iColNb+j),DW_BACKGROUND,(outCols-j));
            break;
         }
         else
         {
            if((PinY >= 0) && (x1 >= 0))
            {
               outImg[iColNb + j] = (((1<<P) - x0) * inImg[PinYinCols + x1] + 
               x0  * inImg[PinYinCols + x2]) >> P;
            }
            else
            {
               outImg[iColNb + j] = DW_BACKGROUND;
            }
         }
         x += xReductionFactor;
      }
   }   
}

void correct_image(
   unsigned char* input_img_hdr,
   unsigned char* out_buff, unsigned int out_siz, unsigned int out_width, unsigned int out_height
)
{
   PDEV_HW_HEADER pHwHdr = (PDEV_HW_HEADER) input_img_hdr;
   unsigned char* raw_data = (unsigned char*) (pHwHdr + 1);
   unsigned int data_size = 0;
   unsigned int nLines = 0;

   fill_skipped_lines (pHwHdr);    
         
   data_size = pHwHdr->ImageHeader.wWidth * pHwHdr->ImageHeader.wHeight;

   subtract_optical_black (raw_data, data_size, pHwHdr->ImageHeader.wWidth);
   
   for (unsigned int i = 0; i < data_size; i++) {
	raw_data[i] ^= 0xff;
   }	      

   nLines = pHwHdr->ImageHeader.wHeight;
   for (unsigned int j = 0; j < nLines; j++) {
      	g_memmove (
            raw_data + j*IMAGE_TRIMMED_WIDTH, 
            raw_data + j*pHwHdr->ImageHeader.wWidth+ DEV_IMAGE_OWHITE_START,
            IMAGE_TRIMMED_WIDTH
         );
   }

   CENTERING_INFO cei = {0.0, 0.0};
   cei.deltaX = (float)((signed)pHwHdr->ImageHeader.byPatchCenterX - (signed)DW_X_PATCH_ETALON);
   cei.deltaY = (float)((signed)pHwHdr->ImageHeader.byPatchCenterY - (signed)DW_Y_PATCH_ETALON);
   NORMALIZE_IMG normImg = {GAMMA, PER_HIGH_IGNORE, PER_LOW_IGNORE }; 
   CORRECTION_COEFF correctionCoeff = {DW_xMin, DW_xMax, DW_yMin, DW_yMax, DW_k2, DW_SCALEFACTOR, DW_r1, DW_r2, DW_T, DW_f, DW_t };
   
   normalize_img (DW_IN_ROWS, DW_IN_COLS, raw_data, &normImg);
   dewarp (DW_IN_ROWS,  DW_IN_COLS,  raw_data,
           DW_OUT_ROWS, DW_OUT_COLS, out_buff,
           &correctionCoeff, &cei);
}

/*
 * End of portion supplied by DigitalPersona, Inc.
 */

/*
 * HWSTAT
 *
 * This register has caused me a lot of headaches. It pretty much defines
 * code flow, and if you don't get it right, the pretty lights don't come on.
 * I think the situation is somewhat complicated by the fact that writing it
 * doesn't affect the read results in the way you'd expect -- but then again
 * it does have some obvious effects. Here's what we know
 *
 * BIT 7: LOW POWER MODE
 * When this bit is set, the device is partially turned off or something. Some
 * things, like firmware upload, need to be done in this state. But generally
 * we want to clear this bit during late initialization, which can sometimes
 * be tricky.
 *
 * BIT 2: SOMETHING WENT WRONG
 * Not sure about this, but see the init function, as when we detect it,
 * we reboot the device. Well, we mess with hwstat until this evil bit gets
 * cleared.
 *
 * BIT 1: IRQ PENDING
 * Just had a brainwave. This bit is set when the device is trying to deliver
 * and interrupt to the host. Maybe?
 */

static int get_hwstat(struct fp_img_dev *dev, unsigned char *data)
{
	int r;

	/* The windows driver uses a request of 0x0c here. We use 0x04 to be
	 * consistent with every other command we know about. */
	r = usb_control_msg(dev->udev, CTRL_IN, USB_RQ, REG_HWSTAT, 0,
		data, 1, CTRL_TIMEOUT);
	if (r < 0) {
		fp_err("error %d", r);
		return r;
	} else if (r < 1) {
		fp_err("read too short (%d)", r);
		return -EIO;
	}

	fp_dbg("val=%02x", *data);
	return 0;
}

static int set_hwstat(struct fp_img_dev *dev, unsigned char data)
{
	int r;
	fp_dbg("val=%02x", data);

	r = usb_control_msg(dev->udev, CTRL_OUT, USB_RQ, REG_HWSTAT, 0,
		&data, 1, CTRL_TIMEOUT);
	if (r < 0) {
		fp_err("error %d", r);
		return r;
	} else if (r < 1) {
		fp_err("read too short (%d)", r);
		return -EIO;
	}

	return 0;
}

static int set_mode(struct fp_img_dev *dev, unsigned char mode)
{
	int r;

	fp_dbg("%02x", mode);
	r = usb_control_msg(dev->udev, CTRL_OUT, USB_RQ, REG_MODE, 0, &mode, 1,
		CTRL_TIMEOUT);
	if (r < 0) {
		fp_err("error %d", r);
		return r;
	} else if (r < 1) {
		fp_err("write too short (%d)", r);
		return -EIO;
	}

	return 0;
}

static int read_challenge(struct fp_img_dev *dev, unsigned char *data)
{
	int r;

	/* The windows driver uses a request of 0x0c here. We use 0x04 to be
	 * consistent with every other command we know about. */
	r = usb_control_msg(dev->udev, CTRL_IN, USB_RQ, REG_CHALLENGE, 0,
		data, CR_LENGTH, CTRL_TIMEOUT);
	if (r < 0) {
		fp_err("error %d", r);
		return r;
	} else if (r < CR_LENGTH) {
		fp_err("read too short (%d)", r);
		return -EIO;
	}

	return 0;
}

static int write_response(struct fp_img_dev *dev, unsigned char *data)
{
	int r;

	r = usb_control_msg(dev->udev, CTRL_OUT, USB_RQ, REG_RESPONSE, 0, data,
		CR_LENGTH, CTRL_TIMEOUT);
	if (r < 0) {
		fp_err("error %d", r);
		return r;
	} else if (r < 1) {
		fp_err("write too short (%d)", r);
		return -EIO;
	}

	return 0;
}

/*
 * 2nd generation MS devices added an AES-based challenge/response
 * authentication scheme, where the device challenges the authenticity of the
 * driver.
 */
static int auth_cr(struct fp_img_dev *dev)
{
	struct uru4k_dev *urudev = dev->priv;
	unsigned char challenge[CR_LENGTH];
	unsigned char response[CR_LENGTH];
	int r;

	fp_dbg("");

	r = read_challenge(dev, challenge);
	if (r < 0) {
		fp_err("error %d reading challenge", r);
		return r;
	}

	AES_encrypt(challenge, response, &urudev->aeskey);

	r = write_response(dev, response);
	if (r < 0)
		fp_err("error %d writing response", r);

	return r;
}

static int get_irq(struct fp_img_dev *dev, unsigned char *buf, int timeout)
{
	uint16_t type;
	int r;
	int infinite_timeout = 0;

	if (timeout == 0) {
		infinite_timeout = 1;
		timeout = 2000;
	}

	/* Darwin and Linux behave inconsistently with regard to infinite timeouts.
	 * Linux accepts a timeout value of 0 as infinite timeout, whereas darwin
	 * returns -ETIMEDOUT immediately when a 0 timeout is used. We use a
	 * looping hack until libusb is fixed.
	 * See http://thread.gmane.org/gmane.comp.lib.libusb.devel.general/1315 */

retry:
	r = usb_interrupt_read(dev->udev, EP_INTR, buf, IRQ_LENGTH, timeout);
	//if (r == -ETIMEDOUT && infinite_timeout)
//		goto retry;

	if (r == -ETIMEDOUT && infinite_timeout) {
		if(isUru4000==1){
			
			r=usb_reset(dev->udev);
			if(r<0){
				fp_dbg("USB reset error");
				return r;
			}

		}
		
		goto retry;
	}



	if (r < 0) {
		if (r != -ETIMEDOUT)
			fp_err("interrupt read failed, error %d", r);
		return r;
	} else if (r < IRQ_LENGTH) {
		fp_err("received %d byte IRQ!?", r);
		return -EIO;
	}

	type = GUINT16_FROM_BE(*((uint16_t *) buf));
	fp_dbg("irq type %04x", type);

	/* The 0800 interrupt seems to indicate imminent failure (0 bytes transfer)
	 * of the next scan. I think I've stopped it from coming up, not sure
	 * though! */
	if (type == IRQDATA_DEATH)
		fp_warn("oh no! got the interrupt OF DEATH! expect things to go bad");

	return 0;
}

enum get_irq_status {
	GET_IRQ_SUCCESS = 0,
	GET_IRQ_OVERFLOW = 1,
};

static int get_irq_with_type(struct fp_img_dev *dev,
	uint16_t irqtype, int timeout)
{
	uint16_t hdr;
	int discarded = 0;
	unsigned char irqbuf[IRQ_LENGTH];

	fp_dbg("type=%04x", irqtype);

	do {
		int r = get_irq(dev, irqbuf, timeout);
		if (r < 0)
			return r;

		hdr = GUINT16_FROM_BE(*((uint16_t *) irqbuf));
		if (hdr == irqtype)
			break;
		discarded++;
	} while (discarded < 3);

	if (discarded > 0)
		fp_dbg("discarded %d interrupts", discarded);

	if (hdr == irqtype) {
		return GET_IRQ_SUCCESS;
	} else {
		/* I've seen several cases where we're waiting for the 56aa powerup
		 * interrupt, but instead we just get three 0200 interrupts and then
		 * nothing. My theory is that the device can only queue 3 interrupts,
		 * or something. So, if we discard 3, ask the caller to retry whatever
		 * it was doing. */
		fp_dbg("possible IRQ overflow detected!");
		return GET_IRQ_OVERFLOW;
	}
}

static int await_finger_on(struct fp_img_dev *dev)
{
	int r;

retry:
	r = set_mode(dev, MODE_AWAIT_FINGER_ON);
	if (r < 0)
		return r;

	r = get_irq_with_type(dev, IRQDATA_FINGER_ON, 0);
	if (r == GET_IRQ_OVERFLOW)
		goto retry;
	else
		return r;
}

static int await_finger_off(struct fp_img_dev *dev)
{
	int r;

retry:
	r = set_mode(dev, MODE_AWAIT_FINGER_OFF);
	if (r < 0)
		return r;
	
	r = get_irq_with_type(dev, IRQDATA_FINGER_OFF, 0);
	if (r == GET_IRQ_OVERFLOW)
		goto retry;
	else
		return r;
}

static int capture(struct fp_img_dev *dev, gboolean unconditional,
	struct fp_img **ret)
{
	int r;

	uint16_t bcdDevice = usb_device(dev->udev)->descriptor.bcdDevice;
	int bCorr = 0;
	struct fp_img *img = NULL;
	size_t image_size = DATABLK1_RQLEN + DATABLK2_EXPECT - CAPTURE_HDRLEN;

	struct fp_img* corrected_img = NULL;
	size_t corrected_img_size = DEV_IMAGE_OUT_COLS * DEV_IMAGE_OUT_ROWS;

	fp_dbg("Hardware rev=%x\n", bcdDevice);
	if ((bcdDevice & 0xfff) == 0x123) bCorr = 1;

	int hdr_skip = CAPTURE_HDRLEN;

	r = set_mode(dev, MODE_CAPTURE);
	if (r < 0)
		return r;

	/* The image is split up into 2 blocks over 2 USB transactions, which are
	 * joined contiguously. The image is prepended by a 64 byte header which
	 * we completely ignore.
	 *
	 * We mimic the windows driver behaviour by requesting 0xb340 bytes in the
	 * 2nd request, but we only expect 0xb1c0 in response. However, our buffers
	 * must be set up on the offchance that we receive as much data as we
	 * asked for. */

	img = fpi_img_new(DATABLK1_RQLEN + DATABLK2_RQLEN);

	r = usb_bulk_read(dev->udev, EP_DATA, img->data, DATABLK1_RQLEN,
		BULK_TIMEOUT);
	if (r < 0) {
		fp_err("part 1 capture failed, error %d", r);
		goto err;
	} else if (r < DATABLK1_RQLEN) {
		fp_err("part 1 capture too short (%d)", r);
		r = -EIO;
		goto err;
	}

	r = usb_bulk_read(dev->udev, EP_DATA, img->data + DATABLK1_RQLEN,
		DATABLK2_RQLEN, BULK_TIMEOUT);
	if (r < 0) {
		fp_err("part 2 capture failed, error %d", r);
		goto err;
	} else if (r != DATABLK2_EXPECT) {
		if (r == DATABLK2_EXPECT - CAPTURE_HDRLEN) {
			/* this is rather odd, but it happens sometimes with my MS
			 * keyboard */
			fp_dbg("got image with no header!");
			hdr_skip = 0;
		} else {
			fp_err("unexpected part 2 capture size (%d)", r);
			r = -EIO;
			goto err;
		}
	}

	if (!bCorr) {
		/* remove header and shrink allocation */
		g_memmove(img->data, img->data + hdr_skip, image_size);
		img = fpi_img_resize(img, image_size);
		img->flags = FP_IMG_V_FLIPPED | FP_IMG_H_FLIPPED | FP_IMG_COLORS_INVERTED;
		*ret = img;
	}
	else {
		corrected_img = fpi_img_new(corrected_img_size);
		correct_image(img->data, corrected_img->data, corrected_img_size, DEV_IMAGE_OUT_COLS,  DEV_IMAGE_OUT_ROWS);
		*ret = corrected_img;
		fp_img_free(img);		
		uru4000_driver.img_height = DEV_IMAGE_OUT_ROWS;
		uru4000_driver.img_width = DEV_IMAGE_OUT_COLS;
	}

	return 0;
err:
	fp_img_free(img);
	return r;
}

static int fix_firmware(struct fp_img_dev *dev)
{
	uint16_t enc_addr = 0;
	unsigned char fwdata[3];
	unsigned char val, new;
	int r;
	int i;

	for (i = 0; i < G_N_ELEMENTS(fwenc_offsets); i++) {
		uint16_t try_addr = fwenc_offsets[i];

		fp_dbg("looking for encryption byte at %x", try_addr);
		r = usb_control_msg(dev->udev, 0xc0, 0x0c, try_addr, 0, fwdata, 3,
			CTRL_TIMEOUT);
		if (r < 0)
			return r;
		if (r < 3)
			return -EPROTO;

		fp_dbg("data: %02x %02x %02x", fwdata[0], fwdata[1], fwdata[2]);
		if (fwdata[0] == 0xff && (fwdata[1] & 0x0f) == 0x07
				&& fwdata[2] == 0x41) {
			fp_dbg("spotted the encryption byte!");
			enc_addr = try_addr;
			break;
		} else {
			fp_dbg("that's not the encryption byte :(");
		}
	}

	if (!enc_addr) {
		fp_err("could not find encryption byte");
		return -ENODEV;
	}

	val = fwdata[1];
	enc_addr++;
	fp_dbg("encryption byte at %x reads %02x", enc_addr, val);
	new = val & 0xef;
	if (new == val || ((enc_addr-1)==0x6cc)) {
		fp_dbg("encryption already disabled");
		return 0;
	}

	fp_dbg("fixing encryption byte to %02x", new);
	return usb_control_msg(dev->udev, 0x40, 0x04, enc_addr, 0, &new, 1,
		CTRL_TIMEOUT);
}

static int do_init(struct fp_img_dev *dev)
{
	unsigned char status;
	unsigned char tmp;
	struct uru4k_dev *urudev = dev->priv;
	gboolean need_auth_cr = urudev->profile->auth_cr;
	int timeouts = 0;
	int i;
	int r;

retry:
	r = get_hwstat(dev, &status);
	if (r < 0)
		return r;

	/* After closing an app and setting hwstat to 0x80, my ms keyboard
	 * gets in a confused state and returns hwstat 0x85. On next app run,
	 * we don't get the 56aa interrupt. This is the best way I've found to
	 * fix it: mess around with hwstat until it starts returning more
	 * recognisable values. This doesn't happen on my other devices:
	 * uru4000, uru4000b, ms fp rdr v2 
	 * The windows driver copes with this OK, but then again it uploads
	 * firmware right after reading the 0x85 hwstat, allowing some time
	 * to pass before it attempts to tweak hwstat again... */
	if ((status & 0x84) == 0x84) {
		fp_dbg("rebooting device power");
		r = set_hwstat(dev, status & 0xf);
		if (r < 0)
			return r;

		for (i = 0; i < 100; i++) {
			r = get_hwstat(dev, &status);
			if (r < 0)
				return r;
			if (status & 0x1)
				break;
			usleep(10000);
		}
		if ((status & 0x1) == 0) {
			fp_err("could not reboot device power");
			return -EIO;
		}
	}
	
	if ((status & 0x80) == 0) {
		status |= 0x80;
		r = set_hwstat(dev, status);
		if (r < 0)
			return r;
	}


	r = fix_firmware(dev);
	if (r < 0)
		return r;

	/* Power up device and wait for interrupt notification */
	/* The combination of both modifying firmware *and* doing C-R auth on
	 * my ms fp v2 device causes us not to get to get the 56aa interrupt and
	 * for the hwstat write not to take effect. We loop a few times,
	 * authenticating each time, until the device wakes up. */
	for (i = 0; i < 100; i++) { /* max 1 sec */
		r = set_hwstat(dev, status & 0xf);
		if (r < 0)
			return r;

		r = get_hwstat(dev, &tmp);
		if (r < 0)
			return r;

		if ((tmp & 0x80) == 0)
			break;

		usleep(10000);

		if (need_auth_cr) {
			r = auth_cr(dev);
			if (r < 0)
				return r;
		}
	}

	if (tmp & 0x80) {
		fp_err("could not power up device");
		return -EIO;
	}

	r = get_irq_with_type(dev, IRQDATA_SCANPWR_ON, 300);
	if (r == GET_IRQ_OVERFLOW) {
		goto retry;
	} else if (r == -ETIMEDOUT) {
		timeouts++;
		if (timeouts <= 3) {
			fp_dbg("scan power up timeout, retrying...");
			goto retry;
		} else {
			fp_err("could not power up scanner after 3 attempts");
		}
	}
	return r;
}

static int dev_init(struct fp_img_dev *dev, unsigned long driver_data)
{
	struct usb_config_descriptor *config;
	struct usb_interface *iface = NULL;
	struct usb_interface_descriptor *iface_desc;
	struct usb_endpoint_descriptor *ep;
	struct uru4k_dev *urudev;
	int i;
	int r;

	/* Find fingerprint interface */
	config = usb_device(dev->udev)->config;
	for (i = 0; i < config->bNumInterfaces; i++) {
		struct usb_interface *cur_iface = &config->interface[i];

		if (cur_iface->num_altsetting < 1)
			continue;

		iface_desc = &cur_iface->altsetting[0];
		if (iface_desc->bInterfaceClass == 255
				&& iface_desc->bInterfaceSubClass == 255 
				&& iface_desc->bInterfaceProtocol == 255) {
			iface = cur_iface;
			break;
		}
	}

	if (iface == NULL) {
		fp_err("could not find interface");
		return -ENODEV;
	}

	/* Find/check endpoints */

	if (iface_desc->bNumEndpoints != 2) {
		fp_err("found %d endpoints!?", iface_desc->bNumEndpoints);
		return -ENODEV;
	}

	ep = &iface_desc->endpoint[0];
	if (ep->bEndpointAddress != EP_INTR
			|| (ep->bmAttributes & USB_ENDPOINT_TYPE_MASK) !=
				USB_ENDPOINT_TYPE_INTERRUPT) {
		fp_err("unrecognised interrupt endpoint");
		return -ENODEV;
	}

	ep = &iface_desc->endpoint[1];
	if (ep->bEndpointAddress != EP_DATA
			|| (ep->bmAttributes & USB_ENDPOINT_TYPE_MASK) !=
				USB_ENDPOINT_TYPE_BULK) {
		fp_err("unrecognised bulk endpoint");
		return -ENODEV;
	}

	/* Device looks like a supported reader */

	r = usb_claim_interface(dev->udev, iface_desc->bInterfaceNumber);
	if (r < 0) {
		fp_err("interface claim failed");
		return r;
	}

	urudev = g_malloc0(sizeof(*urudev));
	
	if(driver_data==4){
		isUru4000 = 1;
	}
	
	
	urudev->profile = &uru4k_dev_info[driver_data];
	urudev->interface = iface_desc->bInterfaceNumber;
	AES_set_encrypt_key(crkey, 128, &urudev->aeskey);
	dev->priv = urudev;

	r = do_init(dev);
	if (r < 0)
		goto err;

	return 0;
err:
	usb_release_interface(dev->udev, iface_desc->bInterfaceNumber);
	g_free(urudev);
	return r;
}

static void dev_exit(struct fp_img_dev *dev)
{
	struct uru4k_dev *urudev = dev->priv;

	set_mode(dev, MODE_INIT);
	set_hwstat(dev, 0x80);
	usb_release_interface(dev->udev, urudev->interface);
	g_free(urudev);
}

static const struct usb_id id_table[] = {
	/* ms kbd with fp rdr */
	{ .vendor = 0x045e, .product = 0x00bb, .driver_data = MS_KBD },

	/* ms intellimouse with fp rdr */
	{ .vendor = 0x045e, .product = 0x00bc, .driver_data = MS_INTELLIMOUSE },

	/* ms fp rdr (standalone) */
	{ .vendor = 0x045e, .product = 0x00bd, .driver_data = MS_STANDALONE },

	/* ms fp rdr (standalone) v2 */
	{ .vendor = 0x045e, .product = 0x00ca, .driver_data = MS_STANDALONE_V2 },

	/* dp uru4000 (standalone) */
	{ .vendor = 0x05ba, .product = 0x0007, .driver_data = DP_URU4000 },

	/* dp uru4000b (standalone) */
	{ .vendor = 0x05ba, .product = 0x000a, .driver_data = DP_URU4000B },

	/* terminating entry */
	{ 0, 0, 0, },
};

struct fp_img_driver uru4000_driver = {
	.driver = {
		.id = 2,
		.name = FP_COMPONENT,
		.full_name = "Digital Persona U.are.U 4000/4000B",
		.id_table = id_table,
	},
	.flags = FP_IMGDRV_SUPPORTS_UNCONDITIONAL_CAPTURE,
	.img_height = 289,
	.img_width = 384, 

	.init = dev_init,
	.exit = dev_exit,
	.await_finger_on = await_finger_on,
	.await_finger_off = await_finger_off,
	.capture = capture,
};

