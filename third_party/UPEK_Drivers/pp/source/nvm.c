/**
 * @file nvm.c
 * I/O functions to read/write NVM records
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#include <stddef.h>
#include "perfectprintint.h"
#include "touchip.h"
#include "tci.h"
#include "nvm.h"
#include "st_oem.h"
#include "crypt.h"
#include "ioreg.h"


//---------------------------- Local Functions ------------------------------
static int NVM_ReadRepairRecord(NVMRecordType TargetRecord, BYTE *Buffer, int *BufferLength);
static int NVM_FindRecord(NVMRecordType RecordType, int *Addr);
static int NVMemoryRead(DWORD NVM_Addr, DWORD NumBytes, BYTE *pucBuf);
static int NVMemoryReadPacket(DWORD NVM_Addr, WORD NumBytes, BYTE *DataBuff);
static int NVMemoryReadHuge(DWORD NVM_Addr, DWORD NumBytes, BYTE *pucBuf);
static int ReadNVMRecord(NVMRecordType RecordType, BYTE *Buffer, int BufferLength);


//----------------------------- Local Variables -----------------------------
/**
 * @defgroup nvmrepairrecordparams NVM Repair Record Parameters
 * @{
 */
#define MAX_PIX_RECORD_SIZE     (4+(2*MAXBADPIXELS)) ///< Max size of a pixel repair record in NVM
#define MAX_OMAP_RECORD_SIZE    (564)                ///< Max size of a pixel repair record in NVM
#define MAX_RECORD_SIZE         MAX_OMAP_RECORD_SIZE ///< Max size of a repair record in NVM
/**  @} */

//-------------------------- Function definitions ---------------------------

//---------------------------------------------------------------------------
/**
 *  @brief Read large amount of data from the Non-Volatile memory associated 
 *  with the TouchChip.
 *
 *  @param NVM_Addr Starting location in NVM
 *  @param NumBytes Number of bytes to read
 *  @param DataBuff Pointer to storage for data read.  Must be pre-allocated in memory
 *
 *  @return @ref STERR_OK upon success
 *  @return (other values) USB errors
 */
//---------------------------------------------------------------------------
static int NVMemoryReadHuge(DWORD NVM_Addr, DWORD NumBytes, BYTE *DataBuff)
{
    int retval;

    retval = OnSendCmdTableUSB(ST_TOUCH_GET_NV_HUGE, NVM_Addr, DataBuff, NumBytes);

    return Check_USB_CommunicationError(&retval);
}

//---------------------------------------------------------------------------
/**
 *  @brief Read from the Non-Volatile memory associated with the TouchChip.
 *
 *  Maximum value of bytes read is 58. The function works only on USB with 
 *  firmware 0x17 or later.
 *
 *  @param NVM_Addr Starting location in NVM
 *  @param NumBytes Number of bytes to read (max of 58)
 *  @param DataBuff Pointer to storage for data read.  Must be pre-allocated in memory
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_BADPARAMETER if the buffer is too large
 *  @return @ref STERR_NVM_READ if problem reading from NVM
 *  @return (other values) USB errors
 */
//---------------------------------------------------------------------------
static int NVMemoryReadPacket(DWORD NVM_Addr, WORD NumBytes, BYTE *DataBuff)
{
    int retval;
    int minimum_bytes;
    BYTE buffer64[64];  // default size of return packet is 64 bytes.

    // check max value of bytes
    if(NumBytes > 58)
    {
        return STERR_BADPARAMETER;
    }

    // don't allow single byte reads...causes firmware problem.
    minimum_bytes = max(NumBytes, 2);

    retval = OnSendCmdTableUSB((ST_TOUCH_GET_NV|0x80), NVM_Addr, buffer64, minimum_bytes);
    if(Check_USB_CommunicationError(&retval) != STERR_OK)
    {
        return retval;
    }
    if(buffer64[0] != ST_TOUCH_GET_NV)
    {
        // not the packet we expected!
        return STERR_NVM_READ;
    }

    memcpy(DataBuff, (buffer64+1), NumBytes*sizeof(BYTE));

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief Read from the Non-Volatile memory associated with the TouchChip.
 *
 *  Buffers larger than 58bytes are broken up. The function works only on USB with 
 *  firmware 0x17 or later.
 *
 *  @param NVM_Addr Starting location in NVM
 *  @param NumBytes Number of bytes to read
 *  @param pucBuf   Pointer to storage for data read.  Must be pre-allocated in memory
 * 
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_FIRMWAREREV if not supported by firmware
 *  @return (other values) USB errors
 */
//---------------------------------------------------------------------------
static int NVMemoryRead(DWORD NVM_Addr, DWORD NumBytes, BYTE *pucBuf)
{
    int retval = STERR_OK;
    int total;
    int tempAddress;
    int BytesToRead;
    int remaining;

    total = 0;
    tempAddress = NVM_Addr;

    while((DWORD)total < NumBytes)
    {
        remaining = (NumBytes - total);
        BytesToRead = ((remaining > 58)?(58):(remaining));

        retval = NVMemoryReadPacket((DWORD)tempAddress, (WORD)BytesToRead, pucBuf);
        if(retval != STERR_OK)
        {
            return(retval);
        }

        total += BytesToRead;
        pucBuf += BytesToRead;
        tempAddress += BytesToRead;
    }

    return(retval);
}

//---------------------------------------------------------------------------
/**
 *  @brief The function reads the repair info record in the NVM memory, and fills
 *  the arrays of bad pixels
 *
 *  @param psBP Bad pixel structure
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_INVALID_VALUE if the record exists but it's not valid 
 *  (returned by the function @ref NVM_ReadRepairRecord) or the number of pixels 
 *  is too large.
 *  @return (other values) USB internal function errors
 */
//---------------------------------------------------------------------------
int NVM_ReadPixRepairInfo(BadPix *psBP)
{
    BYTE RepairRec[MAX_PIX_RECORD_SIZE];
    BYTE *recP;
    int PagePixCnt;
    int i,iResult;
    int length=MAX_PIX_RECORD_SIZE;

    // read the record and check if it's valid
    iResult = NVM_ReadRepairRecord(PixRepInfo, RepairRec, &length);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    // load the pixels from page 1
    recP = &RepairRec[2];
    psBP->NumBadPixels = *recP++;
    if(psBP->NumBadPixels > MAXBADPIXELS)
    {
        return STERR_INVALID_VALUE;
    }

    PagePixCnt = RepairRec[2];
    i = 0;
    while(PagePixCnt-- > 0)
    {
        psBP->BadPixLst[i].xcor = (USHORT)*recP++;
        psBP->BadPixLst[i++].ycor = (USHORT)*recP++;
    }

    // load the pixels from page 2 (the last element is the total counter)
    (*recP > 0)?(PagePixCnt=(*recP++ - 1)):(PagePixCnt=*recP++);
    psBP->NumBadPixels += PagePixCnt;
    if(psBP->NumBadPixels > MAXBADPIXELS)
    {
        return STERR_INVALID_VALUE;
    }

    while(PagePixCnt-- > 0)
    {
        psBP->BadPixLst[i].xcor = (USHORT)*recP++;
        // all pixels in this page have ROW MSB == 1
        psBP->BadPixLst[i++].ycor = (USHORT)*recP++ | 0x0100;
    }

    // load the total counter (stored at the end of page 2)
    memcpy(&psBP->BadPixLst[MAXBADPIXELS-1].ycor, recP, 2);

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function reads the gain/offset info record in the NVM memory, 
 *  and fills the array.
 *
 *  @param GainOff Array of 4 bytes of gain/offset info
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_INVALID_VALUE if the record exists but it's not valid 
 *  (returned by the function @ref NVM_ReadRepairRecord)
 *  @return (other values) USB internal function errors
 */
//---------------------------------------------------------------------------
int NVM_ReadGainOffInfo(BYTE *GainOff)
{
    BYTE RepairRec[16];
    int i, iResult;
    int length=10;

    // read the record and check if it's valid
    iResult = NVM_ReadRepairRecord(GainOffsetInfo, RepairRec, &length);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    for(i=0; i<4; i++)
    {
        *GainOff++ = RepairRec[2+i];
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function reads the gradient info record in the NVM memory.
 *
 *  Only the fields "num_blocks" and "i_val" are filled by this function
 *
 *  @param p_gp  Gradient profile structure
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_INVALID_VALUE if the record exists but it's not valid (returned
 *  by the function @ref NVM_ReadRepairRecord)
 *  @return (other values) USB internal function errors
 */
//---------------------------------------------------------------------------
int NVM_ReadGradientInfo(PGradientProfile p_gp)
{
    BYTE RepairRec[MAX_RECORD_SIZE];
    int i, iResult;
    int length;

    length = (sizeof(p_gp->num_blocks) + sizeof(p_gp->i_val) + 2);

    // read the record and check if it's valid
    iResult = NVM_ReadRepairRecord(GradProfInfo, RepairRec, &length);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    // get the gradient info
    p_gp->num_blocks = RepairRec[2];
    for(i=0; i<p_gp->num_blocks; i++)
    {
        p_gp->i_val[i] = RepairRec[3+i];
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function reads the reader profile record from the NVM memory and 
 *  optionally reads the EIM gradient from the system or NVM memory.
 *
 *  @param p_rp Reader profile structure
 *  @param loadEIMGrad Should include EIM gradient when loading reader profile?
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_NOT_SUPPORTED if action is unsupported
 *  @return @ref STERR_INVALID_VALUE if the record exists but it's not valid
 *  @return @ref STERR_WIN_REG_FAIL if there is a Windows registry failure
 *  @return @ref STERR_FILE_IN_OUT_FAIL if there is a file I/O failure
 *  @return @ref STERR_SENSOR_NOT_CALIBRATED if the sensor needs to be calibrated and has not
 *  @return @ref STERR_BAD_GRADIENT_DATA if gradient data in NVM, Windows registry or 
 *  external file is missing/corrupt
 *  @return (other values) USB internal function errors
 */
//---------------------------------------------------------------------------
int NVM_ReadReaderProfile(PReaderProfile p_rp, BOOL loadEIMGrad)
{
    NVM_READER_PROFILE nvmrprof;
    int i, pos, iResult;
    unsigned char checksum;

    // locate the address of the reader profile structure
    iResult = NVM_FindRecord(ReaderProfInfo, &pos);
    if(iResult!=STERR_OK)
    {
        // if there is no valid structure, assume unknown standard reader
        p_rp->Version    = 0x0;
        p_rp->PPRevision = RELEASE;
        p_rp->PPRevision <<= 4;
        p_rp->PPRevision |= VERSION;
        p_rp->PPRevision <<= 4;
        p_rp->PPRevision |= VERSION_SUB;
        p_rp->PPRevision <<= 4;
        p_rp->PPRevision |= VERSION_SUB_SUB;
        p_rp->ReaderType = READER_TYPE_ST9;
        p_rp->SensorType = SENSOR_TYPE_C;
        p_rp->ReaderMode = READER_MODE_STD;
        return STERR_OK;
    }

    // read the reader profile
    iResult = NVMemoryRead((DWORD)pos, (DWORD)sizeof(NVM_READER_PROFILE), (BYTE*)&nvmrprof);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    // get the reader profile
    p_rp->Version = nvmrprof.Version;
    p_rp->PPRevision = nvmrprof.PPRevision;
    p_rp->ReaderType = nvmrprof.ReaderType;
    p_rp->SensorType = nvmrprof.SensorType;
    p_rp->ReaderMode = nvmrprof.ReaderMode;
    p_rp->EIMOffset = nvmrprof.EIMOffset;
    p_rp->EIMBaseOffset = nvmrprof.EIMBaseOffset;
    p_rp->EIMGradCheckSum = nvmrprof.EIMGradCheckSum;
    for(i=0; i<YMAX; i++)
    {
        p_rp->EIMGradRowAverage[i] = nvmrprof.EIMGradRowAverage[i];
    }
    p_rp->EIMGradLoc = nvmrprof.EIMGradLoc;

    // get the EIM gradient from the system or NVM memory
    if((p_rp->ReaderMode == READER_MODE_EIM) && loadEIMGrad)
    {
        // make sure sensor has been calibrated
        if(p_rp->EIMOffset == 0)
        {
            return STERR_SENSOR_NOT_CALIBRATED;
        }

        // for supported reader types, get EIM gradient from NVM and export it to the system
        if(p_rp->ReaderType == READER_TYPE_STM32)
        {
            {
                // get EIM gradient from NVM
                iResult = NVMemoryReadHuge((DWORD)p_rp->EIMGradLoc, (DWORD)(XMAX*YMAX), (BYTE*)p_rp->EIMGrad);
                if(iResult!=STERR_OK)
                {
                    return iResult;
                }
            }

            // check checksum
            checksum = 0;
            for(i=0; i<(XMAX*YMAX); i++)
            {
                checksum += p_rp->EIMGrad[i];
            }
            if(checksum != p_rp->EIMGradCheckSum)
            {
                return STERR_BAD_GRADIENT_DATA;
            }
        }
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function reads the repair info record in the NVM memory, and 
 *  fills the arrays of @a BadRow and @a BadCol.
 *
 *  @param BadRow Array of bad rows
 *  @param BadCol Array of bad columns
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_INVALID_VALUE if the record exists but it's not valid 
 *  (returned by the function @ref NVM_ReadRepairRecord)
 *  @return (other values) USB internal function errors
 */
//---------------------------------------------------------------------------
int NVM_ReadLineRepairInfo(int *BadRow, int *BadCol)
{
    BYTE RepairRec[MAX_RECORD_SIZE];
    BYTE *recP;
    int NCol,NRow;
    int i, iResult;
    int length = sizeof(RepairRec);

    // read the record and check if it's valid
    iResult = NVM_ReadRepairRecord(RepairInfo, RepairRec, &length);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    // load the bad columns
    NCol = RepairRec[2];
    recP = &RepairRec[3];
    for(i=0; i<NCol; i++)
    {
        BadCol[i+1] = (int)recP[i];
    }
    BadCol[0] = NCol;

    // load the bad rows (bad rows in RepairRec are saved into items of 2 byte length)
    NRow = RepairRec[3+NCol];
    recP = &RepairRec[4+NCol];
    for(i=1; i<=NRow; i++,recP+=2)
    {
        BadRow[i] = ((recP[0]<<8) + recP[1]);
    }
    BadRow[0] = NRow;

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief Internal function to read OEM data
 *
 *  Always reads 64 bytes.
 *
 *  @param data   Buffer to receive the data
 *  @param offset Selects which of the three fields to read
 *
 *  @return @ref STERR_OK upon success
 *  @return (other values) USB internal function errors
 */
//---------------------------------------------------------------------------
int NVM_ReadOEMInfo(BYTE *data, int offset)
{
    int pos;
    int iResult;

    // find the address of the record
    iResult = NVM_FindRecord(OEMInfo, &pos);
    if(iResult != STERR_OK)
    {
        return iResult;
    }

    // read the field of interest
    iResult = NVMemoryRead((DWORD)(pos+offset), (DWORD)64, data);
    return(iResult);
}

//---------------------------------------------------------------------------
/**
 *  @brief Initialize the global NVM directory structure with a copy 
 *  of the first 58 bytes of the NVM to facilitate lookup of NVM data 
 *  locations.
 *
 *  @param pnvmd Pointer to NVM structure storage for data read. Must 
 *               be pre-allocated in memory.
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_BADPARAMETER if the buffer is too large
 *  @return @ref STERR_NVM_READ if problem reading from NVM
 *  @return (other values) USB errors
 */
//---------------------------------------------------------------------------
int InitNVMDir(NVM_DIR *pnvmd)
{
    return NVMemoryReadPacket(0, sizeof(NVM_DIR), (BYTE*)pnvmd);
}

//---------------------------------------------------------------------------
/**
 *  @brief Override the global data that can be configured from the NVM. 
 *
 *  If the configuration record does not exist, keep default values.
 *
 *  @return @ref STERR_OK upon success
 *  @return (other values) USB errors
 */
//---------------------------------------------------------------------------
int OverrideGlobals(void)
{
    int ret;
    STOEMField stdat;
    PSystemData psd;
    BYTE temp[16];

    memset(temp, 0, 16);
    psd = &stdat.sdb.sd;

    // read in the OEM field that contains the configuration data
    ret = NVM_ReadOEMInfo((BYTE*)&stdat, 0);
    // if it is not present or not valid, keep default values
    if((ret==STERR_RECORD_NOT_FOUND) || (psd->Valid!=NVM_FORMAT_VALID))
    {
        return(STERR_OK);
    }
    // otherwise, if no error, set up the globals from the config data.
    else if(ret==STERR_OK)
    {
        // assume the compressed talbe is present if IgnoreSTab not 0
        if(psd->IgnoreSTab == 0)
        {
            ExpandSettingsTable(GPrv.TCSettingTable, &psd->STab);
        }

        // orientation
        if(psd->Orientation <= 3)
        {
            GPrv.Orientation = psd->Orientation;
        }

        // readout timing
        if(psd->ChargeWidth != 0)
        {
            GPrv.ChargeWidth = psd->ChargeWidth;
        }

        if(psd->PeriodWidth != 0)
        {
            GPrv.PeriodWidth = psd->PeriodWidth;
        }

        if(psd->ResetWidth != 0)
        {
            GPrv.ResetWidth = psd->ResetWidth;
        }

        if(psd->DigitalOffset != 0)
        {
            InitDigitalGainTable(psd->DigitalGain1, psd->DigitalGain2, psd->DigitalOffset, Glbv.Prv.D_gain);
        }

        if(psd->ArrayHeight != 0)
        {
             GlbWin->MaxHeight = (psd->ArrayHeight*2);
        }

        if(psd->ArrayWidth != 0)
        {
            GlbWin->MaxWidth = (psd->ArrayWidth*2);
        }

        if(psd->ConfigFlag != 0)
        {
            GPrv.ConfigFlag = psd->ConfigFlag;
        }

        if(psd->SensorVersion != 0)
        {
            GPub.SensorVersion = psd->SensorVersion;
        }

        return STERR_OK ;
    }
    // otherwise something went wrong...return the error
    else
    {
        return ret;
    }
}

//---------------------------------------------------------------------------
/**
 *  @brief The function reads the repair record in the NVM memory.
 *
 *  The @a BufferLength parameter returns the length of the record read.
 *
 *  @param TargetRecord Type of record (pixel, line, gain/offset) - @ref NVMRecordType
 *  @param Buffer       Repair record buffer.  Must be pre-allocated in memory.
 *  @param BufferLength Buffer/record length 
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_BADPARAMETER if any of the input parameters are invalid
 *  @return @ref STERR_INVALID_VALUE if the record exists but it's not valid
 *  @return @ref STERR_BUFFER_TOO_SMALL if the input buffer is not big enough
 *  @return (other values) Errors returned by internal functions
 */
//---------------------------------------------------------------------------
static int NVM_ReadRepairRecord(NVMRecordType TargetRecord, BYTE *Buffer, int *BufferLength)
{
    BYTE LocBuf[MAX_RECORD_SIZE];
    int iResult;
    //const int MaxReadLength=50;

    // allow only selected target records 
    if((TargetRecord != RepairInfo) &&
       (TargetRecord != PixRepInfo) &&
       (TargetRecord != GainOffsetInfo) &&
       (TargetRecord != GradProfInfo) &&
       (TargetRecord != LatentInfo))
    {
        return STERR_BADPARAMETER;
    }

    iResult = ReadNVMRecord(TargetRecord, LocBuf, *BufferLength);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    // check that the length of the record is less than the user buffer
    if((int)LocBuf[0] > *BufferLength)
    {
        return STERR_BUFFER_TOO_SMALL;
    }
    *BufferLength = (int)LocBuf[0];

    // check the validity of the record (second byte of the record)
    if(LocBuf[1] != VALIDITY_FLAG)
    {
        return STERR_INVALID_VALUE;
    }

    //copy the record into the user buffer
    memcpy(Buffer, LocBuf, *BufferLength);

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function looks for the first instance of a record in the NVM 
 *  and returns its position. 
 *
 *  It uses a global cached image of the NVM header
 *
 *  @param TargetRecord Type of the record to be read - @ref NVMRecordType
 *  @param Addr         Address of the record in NVM.  Must be pre-allocated in memory.
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_FIRMWAREREV if NVM format not valid
 *  @return @ref STERR_RECORD_NOT_FOUND if record not found
 */
//---------------------------------------------------------------------------
static int NVM_FindRecord(NVMRecordType TargetRecord, int *Addr)
{
    int NumRecs;
    PNVM_HEADER pnvmh;
    PNVM_REC_PTR_FIELD pnvmrpf;

    pnvmh = &GPrv.nvmdir.nvmh;

    // check if the header is valid
    if(pnvmh->ValidByte != NVM_FORMAT_VALID)
    {
        return STERR_FIRMWAREREV;
    }

    // total number of pointer fields
    NumRecs = pnvmh->NumRecs;
    // in case something is really whacked
    NumRecs = min(NumRecs, MAX_NVMRECS);

    pnvmrpf = &GPrv.nvmdir.rfp[0];

    // look for the "record pointer field" to get the starting address
    while((NumRecs > 0) && (pnvmrpf->RecType != TargetRecord))
    {
        NumRecs--;
        pnvmrpf++;  // point to next record pointer
    }

    // check if the record was found
    if(NumRecs == 0)
    {
        return STERR_RECORD_NOT_FOUND;
    }

    // we can now get the starting address (this is the starting byte of the record)
    if(pnvmrpf->RecType == ReaderProfInfo)
    {
        *Addr = ((pnvmrpf->RecLoc)<<3);
    }
    else
    {
        *Addr = ((pnvmrpf->RecLoc)<<2);
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function reads a record in the NVM memory.
 *
 *  If the record is > @a BufferLength, only @a BufferLength bytes are read. If the 
 *  record is smaller than @a BufferLength, extra bytes are read. (The first byte 
 *  of the record always tells the length.)
 *
 *  @param RecordType   Type of the record to be read - @ref NVMRecordType
 *  @param Buffer       User buffer.  Must be pre-allocated in memory.
 *  @param BufferLength Length of the buffer
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_BADPARAMETER if the input parameter @a RecordType is not valid
 *  @return @ref STERR_INVALID_VALUE if record exists but it's not valid
 *  @return @ref STERR_RECORD_NOT_FOUND	if record not found
 */
//---------------------------------------------------------------------------
static int ReadNVMRecord(NVMRecordType RecordType, BYTE *Buffer, int BufferLength)
{
    BYTE *bptr;
    int RecAddr;
    int iResult;
    int remaining;
    int BytesToRead;

    iResult = NVM_FindRecord(RecordType, &RecAddr);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    remaining = BufferLength;
    bptr = Buffer;

    do
    {
        BytesToRead = ((remaining > 58)?(58):(remaining));

        iResult = NVMemoryRead((DWORD)RecAddr, (DWORD)BytesToRead, bptr);
        if(iResult!=STERR_OK)
        {
            return iResult;
        }

        RecAddr += BytesToRead;
        bptr += BytesToRead;
        remaining -= BytesToRead;
    } while(remaining > 0);

    return STERR_OK;
}
