/**
 * @file reg_map.h
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef REG_MAP_H_
#define REG_MAP_H_


//-------------------------------- Constants --------------------------------
/**
 * @defgroup revfregisters TouchCHip Revision F Registers
 * DEFINE Register vs Variables for TouchCHip Revision F
 * @{
 */
#define SIGNCHARGE          0
#define CHARGE              1
#define IRES1               2
#define PBIAS               3
#define PCASCVAL            4
#define GPIOIN              5
#define INTDT               6
#define IBIAS               7
#define OFFSET              8
#define GAIN                9
#define NRSTAS              10
#define FREQAS              11
#define SHDWOSC             12
#define FREQCNTRL           13
#define CKGBLEN             14
#define SELBUF              15
#define NCORERST            16
#define COLREPCK            17
#define ROWREPCK            18
#define TESTROW             19
#define TESTCOL             20
#define MUXSEL              21
#define SELIN               22
#define SELOUT              23
#define SWRESET             24
#define INTEXTEN            25
#define GOINTEN             26
#define TESTFIFO            27
#define CHKCLK              28
#define TESTDATA            29
#define SUPPLYOFF           30
#define DETPD               31
#define D2APD               32
#define LSD                 33
#define VENDORCODE0         34
#define PAGE0               35
#define STARTCOL            36
#define STARTCOLH           360
#define STARTCOL8           37
#define STOPCOL             38
#define STOPCOL8            39
#define INCCOL              40
#define STARTROW            41
#define STARTROW8           42
#define STOPROW             43
#define STOPROW8            44
#define INCROW              45
#define RESETWIDTH          46
#define CHARGEWIDTH         47
#define PERIODWIDTH         48
#define SYNCRES             49
#define EXTDTINTSEL         50
#define SELEXTINT           51
#define SELPOWERONINT       52
#define CLOCKOUTCNTR        53
#define RESETLAG            54
#define VENDORCODE1         55
#define PAGE1               56
#define CHENABLE            57
#define CHARGESTEP          58
#define AUTOREGACTIVE       59
#define KEEPRAMPVALUE       60
#define AUTOREGCONT         61
#define AUTOREGENABLE       62
#define OFFSETSTEP          63
#define AUTOREGSTOP         64
#define SETUPSELECTED       65
#define GPIOOUT             66
#define GPIOCNT             67
#define CHECKLINE           68
#define ROWCHINC0           69
#define ROWCHINC1           70
#define ROWCHINC2           71
#define ROWCHINC3           72
#define ROWCHINC4           73
#define ROWCHINC5           74
#define ROWCHINC6           75
#define ROWCHINC7           76
#define VENDORCODE2         77
#define PAGE2               78
#define PIXREPEN1           79  ///< old name
#define COLPIXREPDIS        79  ///< new name
#define PIXREPEN2           80  ///< old name
#define ROWPIXREPDIS        80  ///< new name
#define SNGLPIX             81
#define FREERUN             82
#define INSERTHEADERS       83
#define COMPRESS            84
#define NRESINT             85
#define INTEN               86
#define CONTINUOUS          87
#define INTMODE             88
#define ENABLBURST          90
#define COUNTTH             91
#define COMPTHH             92
#define COMPTHL             93
#define TOTBADROWS          94
#define TOTBADCOLS          95
#define DONEONCE            96
#define DONE                97
#define READOUTDONE         98
#define VALIDAVERAGE        99
#define AVERAGE             100
#define NINT                101
#define PIXELRESULT         102
#define RESTART             103
#define START               104
#define KEEPGOING           105
#define STAYON              106
#define PDTIME              107
#define INTTIME             108
#define PUTIME              109
#define VENDORCODE3         110
#define PAGE3               111
#define PIXDATA             112
#define BINCODE             113
#define DISABLEUSER         114
#define DIGITLCORR          115
#define DISABLCLEARPIX      116
#define GO                  117
#define STOP                118
#define SKIPGRAYFRAME       119
#define CHECKLINE5          120
#define GRIDPULLUP          121
#define SETTINGS            122
//#define CAL_OFFSET          123 // Obsolete
#define MISC_CNTL           123
#define XFER_SPEED          124
#define FW_REV              125 ///< Read Only
#define TURBO0a6            126
#define TURBO0a5            127
#define TURBO0e6            128
#define TURBO0e5            129
#define TURBO0e4            130
#define TURBO0e3            131
#define TC_IDD              132 /**< @brief When Read: proportional to the elactrical current used by the sensor.
                                            When Written: sets latchup current threshold */
#define TC_CUST_ID          133 ///< Read only
#define TC_CUST_VER         134 ///< Read only
//#define FW_GPIO             135 // obsolete name
#define APP_REG             135 ///< new name...application register
#define USB_BUSADDR         136 /**< @brief USB bus address of the device...a number from 0 to 127
                                            guaranteed to be unique for each system, even with N TouchChip attached */
#define TC_PP_VERSION       137 ///< Perfect Print Release+Version number
#define NV_REPAIR           138 ///< (w/o) Force ST9 to read the repair record in NVM and deploy it to the sensor
/*>>>TBD obsolete */ #define LICENSE_CHECK       139 // (r/o) Force ST9 to check license id and silicon id: match=1, no match=0
#define SC_DET_REP          140 ///< (w/o) Cause ST9 to detect shorted columns and deploy repairs to the sensor
/** @} */


//--------------------------- Function prototypes ---------------------------
void MapRegsF(int Regnum, int *indexreg, int *numbit, int *shift);

#endif  // #ifndef REG_MAP_H_
