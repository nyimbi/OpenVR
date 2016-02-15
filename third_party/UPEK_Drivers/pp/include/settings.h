/**
 * @file settings.h
 * Functions to manipulate settings table for the TouchChip
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef SETTINGS_H_
#define SETTINGS_H_


//-------------------------------- Structures -------------------------------
/// Setting data
typedef struct
{
    int iSignChrg;
    int iChrg;
    int iGain;
    int iOffset;
} setting;

/// Used to fill in full settings table
typedef struct _CompressedSettingsTable
{
    BYTE gain0;
    BYTE offset0;
    char charge0;       ///< Note that this could be negative!
    BYTE dat[(64/4)];   ///< Compressed charge steps
} CompressedSettingsTable, FAR *LPCompressedSettingsTable, *PCompressedSettingsTable;


//--------------------------- Function prototypes ---------------------------
extern void ExpandSettingsTable(setting *STab, CompressedSettingsTable *CSTab);
extern int DownloadSettingTable(setting *SettingTable, int NumSettings);
extern void InitSettingsTable(int SensorVersion);

#endif  // #ifndef SETTINGS_H_
