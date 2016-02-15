/**
 * @file badlines_new.h
 * @brief Constants and functions used to repair image based on sensor data.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef BADLINES_NEW_H_
#define BADLINES_NEW_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup pixelrepairparams Pixel Repair Parameters
 *
 * Constants used to repair missing pixels for sensors (TCS1C)
 * with ESD protections structures in place of some pixels.  These
 * were originally single pixels, but now they are entire columns.
 * @{
 */
#define MISSING_PIXEL_XSTART            47      ///< First missing column in TCS1 and TCS2
#define MISSING_PIXEL_YSTART            0
#define MISSING_PIXEL_XSTEP             52      ///< Delta between missing columns in TCS1 and TCS2
#define MISSING_PIXEL_YSTEP             1
#define MISSING_PIXEL_XSTOP_TCS1        203     ///< Last missing column in TCS1
#define MISSING_PIXEL_XSTOP_TCS2        151     ///< Last missing column in TCS2

#define MIN_CONSECUTIVE_BAD_PIXEL       15      ///< Minimum length of short line
#define MIN_BAD_PIXEL_ON_SHORT_LINE     25

#define NUM_OF_ROWS_IN_ONE_CHUNK        18
#define PIX_AVG_BOADER                  4
#define NUM_OF_IMAGES_FOR_AVG           4

#define BADPIXELDIFFTH                  20
#define BADPIXELTH                      30

#define TOP_PIXEL_AVERAGE_OFFSET        35
#define BOTTOM_PIXEL_AVERAGE_OFFSET     5
#define GAIN_OFFSET                     7
#define TOP_PIXEL_AVERAGE_CHARGE        140
#define BOTTOM_PIXEL_AVERAGE_CHARGE     124
#define GAIN_CHARGE                     7
/** @} */

/**
 * @defgroup shortlinerepairparams Short Line Repair Parameters
 * @brief Constants used to repair short lines for sensors (TCS1C).
 * @{
 */
#define MAX_COL_OFFSET                  12      ///< Maximum column offset for short lines
#define MAX_ROW_OFFSET                  10      ///< Maximum row offset for short lines
/** @} */

/**
 * @defgroup eimgradcalparams EIM Gradient Calibration Parameters
 * @{
 */
#define EIMGRAD_BUFFER_LEFT             2       ///< The column to start testing image saturation (EIM Gradient Calibration) - @ref EIMGradientCalibration
#define EIMGRAD_BUFFER_RIGHT            2       ///< The column to stop testing image saturation (EIM Gradient Calibration) - @ref EIMGradientCalibration
#define EIMGRAD_BUFFER_TOP              2       ///< The row to start testing image saturation (EIM Gradient Calibration) - @ref EIMGradientCalibration
#define EIMGRAD_BUFFER_BOTTOM           2       ///< The row to stop testing image saturation (EIM Gradient Calibration) - @ref EIMGradientCalibration
#define EIMGRAD_FRAMES_FOR_AVG          10      ///< The number of images to use to compute average image (EIM Gradient Calibration) - @ref EIMGradientCalibration
#define EIMGRAD_NUM_TOL_SAT_PIXELS      15      ///< The number of saturated pixels tolerated before considering image saturated (EIM Gradient Calibration) - @ref EIMGradientCalibration
#define EIMGRAD_SATPIX_MIN_THRESHOLD    253     ///< The minimum threshold value of a saturated pixel (EIM Gradient Calibration) - @ref EIMGradientCalibration
#define EIMGRAD_MAX_EIMOFFSET           62      ///< The maximum offset to consider for EIM offset (EIM Gradient Calibration) - @ref EIMGradientCalibration

#define EIMGRAD_STD_NUM_STRIPS          5       ///< The number of strip averages to compute when calculating the negative factor for the standard EIM row average gradient - @ref EIMGradientCalibration
#define EIMGRAD_STD_STRIP_OFFSET        2       ///< The top row and column offset from the edge of the sensor to the area to compute strip averages - @ref EIMGradientCalibration
#define EIMGRAD_STD_STRIP_WIDTH         8       ///< The width of a strip used to compute a strip average - @ref EIMGradientCalibration
#define EIMGRAD_STD_STRIP_HEIGHT        32      ///< The height of a strip used to compute a strip average - @ref EIMGradientCalibration
/** @} */

/**
 * @defgroup usbparams USB Parameters
 * @{
 */
#define PKT_SIZE                        58      ///< Size of repair packet sent to repair sensor - @ref USBCorrectCommand
/** @} */


//--------------------------- Function prototypes ---------------------------
extern void InitializeRepairGlobals();
extern int SensorRepair();
extern int RetrieveGainOffInfo(GainOffsetStruct *gain_offset);
extern int LoadGradientProfile(GradientProfile *gradient);
extern int LoadReaderProfile(ReaderProfile *rprofile, BOOL loadEIMGrad);
extern int CorrectRowsAfterBadRows(int *BadRow, unsigned char *Buffer, unsigned long BufferSize);
extern int RepairPixels(BadPix *table, unsigned char *imgin);

#endif  // #ifndef BADLINES_NEW_H_
