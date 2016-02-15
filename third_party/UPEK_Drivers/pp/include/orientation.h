/**
 * @file orientation.h
 * Functions to do the orientation compensation
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef _PP_ORIENTATION_H_
#define _PP_ORIENTATION_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup orientpositions Orientation Positions
 * @{
 */
#define ORIENT_ZERO         0   ///< Sensor in standard position
#define ORIENT_LEFT_90      1   ///< Sensor rotated 90 degrees counter-clockwise
#define ORIENT_LEFT_180     2   ///< Sensor rotated 180 degrees
#define ORIENT_LEFT_270     3   ///< Sensor rotated 270 degrees counter-clockwise
/** @} */


//-------------------------------- Structures -------------------------------
/// Structure for physical and logical window setting
typedef struct
{
    int X0;     ///< Start x (offset)
    int Y0;     ///< Start y (offset)
    int X;      ///< x
    int Y;      ///< y
    int IncX;   ///< Delta x
    int IncY;   ///< Delta y
} PhyLogWinSetting;


//--------------------------- Function prototypes ---------------------------
extern int MapPhysicalToLogicalWindow(WindowSettings *p_LogWin, WindowSettings *p_PhysWin);
extern int MapLogicalToPhysicalWindow(PhyLogWinSetting *p_PhysWin, PhyLogWinSetting *p_LogWin);

extern int MapPhysicalToLogicalImage(unsigned char *logImg, unsigned char *physImg, int BufferSize);
extern int MapLogicalToPhysicalImage(unsigned char *physImg, unsigned char *logImg, int BufferSize);

extern int MapPhysicalToLogicalSensorData(SensorDescr *p_LogSensorDat, SensorDescr *p_PhySensorDat);

extern int MapPhysicalToLogicalRepairData(RepairDataStruct *p_LogRepairDat, RepairDataStruct *p_PhyRepairDat);

extern int MapPhysicalToLogicalEIMGradientData(BYTE *p_LogEIMGradientDat, BYTE *p_PhyEIMGradientDat, BYTE *p_LogEIMRowAverageDat, BYTE *p_PhyEIMRowAverageDat);

#endif  // #ifndef _PP_ORIENTATION_H_
