/**
 * @file tci.h
 * Legacy Version 2.0 Function supported by Rev. E sensor
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef TCI_H__
#define TCI_H__


//-------------------------------- Constants --------------------------------
/**
 * @defgroup stauthentifyparams Authentification Parameters
 * Definition for Signature ID in STAuthentify
 * @{
 */
#define MAX_ALGO_ID             16
#define PASS_LENGTH             16
#define LIC_NUM_SIZE            17
/** @} */

/**
 * @defgroup stauthentifyusers Authorized User IDs
 * Authorized users for STAuthentify
 * @{
 */
#define SAGEM_ID                'A'
#define PROTECTOR_ID            'D'
#define PERFECTPRINT_ID         'P'
/** @} */

/**
 * @defgroup tciddthresholds TC IDD Thresholds
 * @{
 */
#define TC_IDD_INCREASE         5
#define TC_IDD_THRESH_100MA     205
#define TC_IDD_THRESH_20MA      246

#define TC_IDD_INCREASE_EIM     7
#define TC_IDD_THRESH_100MA_EIM 198
#define TC_IDD_THRESH_20MA_EIM  244
/** @} */


//--------------------------- Function prototypes ---------------------------
extern void InitGlobalVar();
extern int InitializeUSB();
extern void SaveWindowSettings(WindowSettings *window_settings);
extern int RestoreWindowSettings(WindowSettings *window_settings);
extern int SetWindow(int X0, int Y0, int X, int Y, int IncX, int IncY);
extern int SetWindowFullSize();
extern void SaveSetting(int *settings);
extern int RestoreSetting(int *settings);
extern int Reset();
extern int TCI_CheckFirmware();
extern int TCI_CheckSensorVersion();
extern int TCI_GetIDD(BYTE *IDD_val);
extern int TCI_SetIDDLimit(BYTE IDD_val);
extern int Check_ESD_CommunicationError();
extern int Check_USB_CommunicationError(int *retval);
extern void InitConfigFlag(BYTE SensorVersion, BYTE *ConfigFlag);
extern void InitAuthentify();
extern int CheckAlgoId(char *algoId, char *passId);

#endif  // #ifndef TCI_H__
