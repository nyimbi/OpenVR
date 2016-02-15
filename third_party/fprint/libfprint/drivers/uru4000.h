/*
 * Copyright (c) 2010 DigitalPersona, Inc.
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

#if !defined(_URU4000_INCLUDED_)
#define _URU4000_INCLUDED_

typedef struct _CORRECTION_COEFF 
{
   int   iXmin;
   int   iXmax;
   int   iYmin;
   int   iYmax;
   float fk2;
   float fScaleFactor;
   float r1[3];   /* r1 and r2 are the first and second column of the rotation matrix R */
   float r2[3];
   float T[3];    /* the translation vector */
   float f[2];    /* focal lengths */
   float t[2];    /* image coordinates of the intersection of the optical axis with the image plane */
} CORRECTION_COEFF, *PCORRECTION_COEFF;

typedef struct _NORMALIZE_IMG 
{
   float fGamma;       // = 1.65f
   int   iPerHIgnore;  // = 5;  iPercentHighIgnore
   int   iPerLIgnore;  // = 1;  iPercentby10LowIgnore
} NORMALIZE_IMG, *PNORMALIZE_IMG;

typedef struct _CENTERING_INFO {
   // factory position of the patch
   float deltaX;
   float deltaY;
} CENTERING_INFO, *PCENTERING_INFO;

//-----------------------------------------------
// Image size: 
//-----------------------------------------------
#define  DW_IN_ROWS         290
#define  DW_IN_COLS         367

#define  DW_OUT_ROWS        504  
#define  DW_OUT_COLS        456  

#define  DW_X_PATCH_ETALON  204
#define  DW_Y_PATCH_ETALON  137

//-----------------------------------------------
// Image format: 
//-----------------------------------------------
#define  DEV_DATA_TYPE           (FD_DATA_TYPE_2D_IMAGE)
#define  DEV_IMAGE_TYPE          (FD_IMAGE_GRAY_SCALE)
#define  DEV_IMAGE_OUT_COLS      (DW_OUT_COLS)
#define  DEV_IMAGE_OUT_ROWS      (DW_OUT_ROWS)

#define  DEV_IMAGE_X_RES         (500u)   
#define  DEV_IMAGE_Y_RES         (500u)  

#define  DEV_IMAGE_BPP           (8u)
#define  DEV_IMAGE_PADDING       (FD_IMAGE_RIGHT_PADDING)
#define  DEV_IMAGE_SIGNIFIC_BPP  (8)
#define  DEV_IMAGE_POLARITY      (FD_IMAGE_POSITIVE_POLARITY)
#define  DEV_IMAGE_COLOR_REP     (FD_IMAGE_NO_COLOR_REPRESENTATION)
#define  DEV_IMAGE_PLANE         (1u)

// image sizes output by the device
#define  DEV_IMAGE_WIDTH         (384u)
#define  DEV_IMAGE_HEIGHT        (290u)

// output width of the Trim operation; increase to show margin
#define  IMAGE_TRIMMED_WIDTH     (367u)

// imager: optical black margins
#define  IMAGER_OWHITE_WIDTH     (367u)

#define  MAX_PIXEL_VALUE          (0xFE)
#define  DEV_MIN_SATURATION_LEVEL (249)

#define  DEV_IMAGE_OWHITE_START  (11u)

#define  DEV_FAKE_STRIP_SIZE     (91)

#define  GAMMA                   (1.65f)
#define  PER_HIGH_IGNORE         (5)
#define  PER_LOW_IGNORE          (1)

#define BLACK           0
#define GRAYSCALES    256
#define DW_BACKGROUND (GRAYSCALES - 1)


#define DW_Y_CORR       1.047f     
#define DW_X_CORR       1.006f

/* reduction factor */
#define DW_SCALEFACTOR (55.95f*DW_Y_CORR)

/* range of coords for out image */
#define DW_xMin -124
#define DW_xMax 330
#define DW_yMin -43
#define DW_yMax 456

/* k2: coefficient of the 2nd order term in the radial distortion of the lens */
#define DW_k2 -0.17068028036701f

/* r1 is the first column of the rotation matrix R */
#define DW_r1 {0.999996967790909f, 0.00205820137943802f, -0.00135211540555979f} 

/* r2 is the second column of the rotation matrix R */
#define DW_r2 {-0.00204698747083614f, 0.389487102785644f, -0.921029644803f} 

/* T: the translation vector */
#define DW_T {-1.81041561965488f, -0.80600051095657f, 32.3533646594741f} 

/* f: focal lengths */
#define DW_f {1544.63849266742f*DW_Y_CORR/DW_X_CORR, 2474.4623498026f}

/* t: image coordinates of the intersection of the optical axis with the image plane */
#define DW_t {191.328302439083f, 121.006524591601f} 

//-----------------------------------------------
// Y-Magnification
//-----------------------------------------------

typedef struct _Y_MAG_INFO 
{
   int   iPatchX;
   int   iPatchY;
   int  bConfidence;
   int  bUseComputedYmag;
} Y_MAG_INFO, *PY_MAG_INFO;

typedef struct _YMAG_DATA
{
   int   iPatchX;
   int   iPatchY;
   int   iGhostY;
   float fYmagFactor;
   unsigned int dwTime;
   int  bConfidence;   // if true, this data is valid
} YMAG_DATA, *PYMAG_DATA;

typedef struct _YmagRect 
{
   unsigned int x0,y0,x1,y1;
} YmagRect, *PYmagRect;

typedef struct YmagSingleT
{
   // Measure(dwHistThreshold) must lie within (dwNominal +- dwRange) * 0.001
   unsigned int dwHistThreshold;  // in units of 0.1%
   unsigned int dwNominal;        // in units of 0.001 pixels
   unsigned int dwRange;          // in units of 0.001 pixels    
} YmagSingleT, *PYmagSingleT;

typedef struct _YmagTestSet
{
   YmagSingleT ystPatchY, ystPatchX, ystGhostY;
} YmagTestSet, *PYmagTestSet;

typedef struct _YMAG_PARAMS
{
   unsigned int    dwNominalDistance;      // in units of .001 pixels
   unsigned int    dwCorrectionMultiplier; // in units of .001
   unsigned int    dwMinDistance;          // in pixels
   unsigned int    dwMaxDistance;          // in pixels
   unsigned int    dwPatchToEdgeMinDist;   // in pixels
   unsigned int    dwMaxAmbientVariance;
   YmagRect PatchSearchRect;
   YmagRect GhostSearchRect;
   YmagTestSet High;
   YmagTestSet Low;
   YmagTestSet Nominal;
   unsigned int    dwPPD;                  // in pixels, patch to patch distance
} YMAG_PARAMS, *PYMAG_PARAMS;

typedef struct _FRAME_FLAGS_BIT 
{
   unsigned char     Skip:          1;
   unsigned char     Scramble:      1;
   unsigned char     NotClocked:    1;
   unsigned char     unused3_6:     4;
   unsigned char     NewFrame:      1;
}  __attribute__((packed)) FRAME_FLAGS_BIT;

typedef union _FRAME_FLAGS_CTRL 
{
   unsigned char        byte;
   FRAME_FLAGS_BIT bit;
} __attribute__((packed)) FRAME_FLAGS_CTRL;

typedef struct _FRAME_INFO 
{
   FRAME_FLAGS_CTRL  Flags;
   unsigned char              byNbOfLines;
} __attribute__((packed)) FRAME_INFO, * PFRAME_INFO;


#define FRAME_INFO_ARRAY_SIZE 0x0F

typedef struct _URU4KBASE_HW_HEADER 
{
   union {
      unsigned char  RawBytes[0x40];

      // type        name                       dec   hex
      struct {
         unsigned char        byHeaderSize;     //        0    0x00
         unsigned char        byDataOffset;     //        1    0x01, = IMAGE_HEADER_SIZE
         unsigned short        wWidth;           //        2    0x02, = 384
         unsigned short        wHeight;          //        4    0x04, = 289
         unsigned char        byFrameNumber;    //        6    0x06
         unsigned char        byLEDs;           //        7    0x07
         unsigned char        byPatchCenterX;   //        8    0x08
         unsigned char        byPatchCenterY;   //        9    0x09
         unsigned char        byTypeOfFrame;    //       10    0x0A

         struct {
            unsigned char     byAutoGain;       // 0     11    0x0B
            unsigned char     byOffsetLevel;    // 1     12    0x0C
            unsigned short    wShutterSpeed;    // 2     13    0x0D
            unsigned char     byFixedGain;      // 4     15    0x0F
         } __attribute__((packed)) Imager;

         // The table of FRAME_INFO's is terminated by a { ,0} element
         FRAME_INFO  FrameInfo[FRAME_INFO_ARRAY_SIZE];   //   16    0x10

// Extras
         unsigned char        byWhitePixCount;  //       46    0x2E
         unsigned char        byCalibFlags;     //       47    0x2F
         unsigned char        byPatchWidth;     //       48    0x30
         unsigned char        byPatchHeight;    //       49    0x31
         unsigned char        byDimMainRatio32; //       50    0x32
         unsigned char        byPatchMainRatio; //       51    0x33  // reserved; do not use
         unsigned char        byReserved34;     //       52    0x34
         unsigned char        byFdBkgDim;       //       53    0x35
         unsigned short        wDefaultShutterSpeed; //  54    0x36

//       unsigned char        byReserved38[8];  //       56    0x38
      } __attribute__((packed)) ImageHeader;
   };
} __attribute__((packed)) DEV_HW_HEADER, *PDEV_HW_HEADER;


#endif
