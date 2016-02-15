/**
 * @file ioreg.c
 * I/O functions to read/write TC registers and NVM records
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#include "perfectprintint.h"
#include "touchip.h"
#include "tci.h"
#include "ioreg.h"


//--------------------------- Local Functions -------------------------------
static int ReadPhysicalUSBRegister(int PhysRegNum, int *DataR, int forceSensorRead);
static int WritePhysicalUSBRegister(int PhysRegNum, int DataR, int forceSensorWrite);
static int FlushPendingWrites();
static int USB_Read_Register_Table(int Num_Regs, BYTE *reg_table);
static void GetByteData(int WholeData, int LogData, int numbit, int shift, BYTE *Bytedata);
static int GetIndexReg(int PhysRegNum);


//----------------------------- Local Variables -----------------------------
/// Table of the TC physical registers. Initialized with the default values. 
struct RegTableElem
{
    BYTE regnum;
    BYTE value;
    BOOL voltl;
} RegTable[] = {{0x00, 0x9E, 0},
                {0x01, 0xBF, 0},
                {0x02, 0x85, 0},
                {0x04, 0x85, 0},
                {0x05, 0x02, 1},
                {0x06, 0x05, 0},
                {0x07, 0x26, 0},
                {0x08, 0x25, 0},
                {0x09, 0x4D, 0},
                {0x0A, 0x10, 0},
                {0x0B, 0x10, 0},
                {0x0C, 0x00, 0},
                {0x0D, 0x00, 1},
                {0x0E, 0x00, 0},
                {0x0F, 0x94, 1},
                {0x10, 0x00, 0},
                {0x11, 0x00, 0},
                {0x12, 0xFF, 0},
                {0x13, 0x00, 0},
                {0x14, 0x01, 0},
                {0x15, 0x00, 0},
                {0x16, 0x00, 0},
                {0x17, 0x67, 0},
                {0x18, 0x01, 0},
                {0x19, 0x01, 0},
                {0x1A, 0x09, 0},
                {0x1B, 0x09, 0},
                {0x1C, 0x0F, 0},
                {0x1D, 0x00, 0},
                {0x1F, 0x94, 1},
                {0x20, 0x13, 0},
                {0x21, 0x00, 0},
                {0x22, 0x00, 0},
                {0x23, 0x00, 0},
                {0x24, 0x1D, 0},
                {0x27, 0x01, 0},
                {0x28, 0x02, 0},
                {0x29, 0x03, 0},
                {0x2A, 0x04, 0},
                {0x2B, 0x05, 0},
                {0x2C, 0x06, 0},
                {0x2D, 0x00, 0},
                {0x2E, 0x00, 0},
                {0x2F, 0x94, 1},
                {0x30, 0x00, 0},
                {0x31, 0x00, 0},
                {0x32, 0x00, 0},
                {0x33, 0x00, 0},
                {0x34, 0x00, 0},
                {0x35, 0x60, 0},
                {0x36, 0x62, 1},
                {0x37, 0x00, 0},
                {0x38, 0x7E, 0},
                {0x39, 0x69, 0},
                {0x3A, 0x4D, 0},
                {0x3D, 0x31, 0},
                {0x3F, 0x94, 1},
                {0xFF, 0x00, 1}};
#define IsVolatile(idx)         RegTable[idx].voltl
#define SetRegData(idx,data)    RegTable[idx].value=data
#define GetRegData(idx)         RegTable[idx].value

static struct RegWriteStruct
{
    BYTE regnum;
    BYTE data;
} RegWriteList[MAX_WRITING_BLOCKS];

const int MaxNumReg = (sizeof(RegTable) / sizeof(struct RegTableElem));
static int ListIdx = 0;


//-------------------------- Function definitions ---------------------------

//---------------------------------------------------------------------------
/**
 *  @brief The function returns the value of the logical register.
 *
 *  The function reads the internal register table instead of the actual register 
 *  (see also @ref ReadPhysicalUSBRegister). The calling function can force reading the 
 *  actual TC register setting the parameter @a forceSensorRead = @c Force.
 *
 *  @param RegNum           Register number to be read (logical)
 *  @param Data             Data read from the register.  Must be pre-allocated in memory
 *  @param forceSensorRead  Force reading from the actual register - @ref ReadWriteOpt
 *
 *  @see ReadPhysicalUSBRegister
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 *  @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 *  @return @ref STERR_BAD_DEVICE if no device has been defined/open
 *  @return (other values) Errors from functions called internally
 */
//---------------------------------------------------------------------------
int ReadLogicalRegister(int RegNum, int *Data, int forceSensorRead)
{
    int iResult=0;
    int DataR=0;
    int i=0, PhysRegNum=0, numbit=0, shift=0;
    int OneMul=0x0000;

    // check if an error has already occurred
    if(Glbv.Prv.isCommunicationError)
    {
        return STERR_SENSOR_COMMUNICATION;
    }

    // translate logical register to physical register location
    MapRegsF(RegNum, &PhysRegNum, &numbit, &shift);

    // have now mapped logical to physical,
    // and derived all quantities needed to extract partial data from whole data

    switch(Glbv.Pub.DeviceType)
    {
    case ST_NO_DEVICE:
        iResult = STERR_NODEVICE;
        return iResult;

    case ST_USB:
        // force reading if it's volatile 
        iResult = ReadPhysicalUSBRegister(PhysRegNum, &DataR, forceSensorRead);
        if(iResult!=STERR_OK)
        {
            return iResult;
        }
        break;

    default:
        return STERR_BAD_DEVICE;
    }

    // extract the logical portion of the whole data read
    // first, build an hexadecimal number with numbit ones
    for(i=0; i<numbit; i++)
    {
        OneMul = ((OneMul << 1) | 0x0001);
    }

    // shifting the zeros at the right place
    for(i=0; i<shift; i++)
    {
        OneMul = ((OneMul << 1) & 0xfffe);
    }

    *Data = ((DataR & OneMul) >> shift);

    return iResult;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function update the logical register with the data given in input.
 *
 *  The function updates the internal register table instead of the actual register 
 *  (see @ref WritePhysicalUSBRegister). The calling function can force writing the 
 *  actual TC register setting the parameter @a forceSensorWrite = @c Force.
 *
 *  @param RegNum            Register number to be written (logical)
 *  @param Data              Data to be written
 *  @param forceSensorWrite  Force the writing to the actual register - @ref ReadWriteOpt
 *
 *  @see WritePhysicalUSBRegister
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_SENSOR_COMMUNICATION if error with communicating with sensor
 *  @return @ref STERR_NODEVICE if the TouchChip has not been initialized
 *  @return @ref STERR_BAD_DEVICE if no device has been defined/open
 *  @return (other values) Errors from functions called internally
 */
//---------------------------------------------------------------------------
int WriteLogicalRegister(int RegNum, int Data, int forceSensorWrite)
{
    int iResult=0;
    int PhysRegNum, numbit=0, shift=0;
    int WholeData=0x0;
    //int i=0, ZeroMul=0xffff, DataS=0;
    BYTE bytedata;

    // check if an error has already occurred
    if(Glbv.Prv.isCommunicationError)
    {
        return STERR_SENSOR_COMMUNICATION;
    }

    switch(Glbv.Pub.DeviceType)
    {
    case ST_NO_DEVICE:
        iResult = STERR_NODEVICE;
        return iResult;

    case ST_USB:

        // translate logical register to physical register location
        MapRegsF(RegNum, &PhysRegNum, &numbit, &shift);
        iResult = ReadPhysicalUSBRegister(PhysRegNum, &WholeData, forceSensorWrite);
        if(iResult!=STERR_OK)
        {
            return iResult;
        }

        // build the byte value to write
        GetByteData(WholeData, Data, numbit, shift, &bytedata);
        iResult = WritePhysicalUSBRegister(PhysRegNum, (int)bytedata, forceSensorWrite);
        if(iResult!=STERR_OK)
        {
            return iResult;
        }

        break;

    default:
        return STERR_BAD_DEVICE;
    }

    return iResult;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function returns the register data value, after reading it either from
 *  the actual TC register or from the internal register table.
 *
 *  The calling function can force to read the actual TC register with the parameter 
 *  @a forceSensorRead = @c Force.
 *
 *  @param PhysRegNum       Register number to be read (physical)
 *  @param DataR            Data read from the register.  Must be pre-allocated in memory
 *  @param forceSensorRead  Force reading from the actual register - @ref ReadWriteOpt
 *
 *  @return @ref STERR_OK upon success
 *  @return (other values) Errors from functions called internally
 */
//---------------------------------------------------------------------------
static int ReadPhysicalUSBRegister(int PhysRegNum, int *DataR, int forceSensorRead)
{
    int index, iResult;
    int reg_data;

    // check if the register is volatile
    index = GetIndexReg(PhysRegNum);
    if(index < 0)
    {
        forceSensorRead = ForceSingleReg;
    }

    // check if it has to read the actual register
    if(forceSensorRead || IsVolatile(index))
    {
        // flush any pending writes
        iResult = FlushPendingWrites();
        if(iResult!=STERR_OK)
        {
            return iResult;
        }

        if(forceSensorRead == ForceSingleReg)
        {
            // read just the single register
            iResult = OnReadUSB(PhysRegNum, &reg_data);
            if(Check_USB_CommunicationError(&iResult) != STERR_OK)
            {
                return iResult;
            }

            if(index > 0)
            {
                SetRegData(index, reg_data);
            }

            *DataR = reg_data;
        }
        else
        {
            // read all the registers and update the internal table
            iResult = LoadRegTable();
            if(iResult!=STERR_OK)
            {
                return iResult;
            }

            // read from the table
            *DataR = GetRegData(index);
        }
    }
    else
    {
        // read from the table
        *DataR = GetRegData(index);
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function update the register data value, either in writing
 *  the actual TC register or the internal register table.
 *
 *  The calling function can force to write the actual TC register with the 
 *  parameter @a forceSensorWrite = @c Force.
 *
 *  @param PhysRegNum       Register number to write (physical)
 *  @param DataR            Data to be written in the register
 *  @param forceSensorWrite Force writing the actual register - @ref ReadWriteOpt
 *
 *  @return @ref STERR_OK upon success
 *  @return (other values) Errors from functions called internally
 */
//---------------------------------------------------------------------------
static int WritePhysicalUSBRegister(int PhysRegNum, int DataR, int forceSensorWrite)
{
    int index, iResult;

    // get index to the register
    index = GetIndexReg(PhysRegNum);

    // insert the register in the writing queue
    RegWriteList[ListIdx].regnum = (BYTE)PhysRegNum;
    RegWriteList[ListIdx++].data = (BYTE)DataR;

    // if a real writing has been requested, or the queue is full,
    // or the register is volatile -> write the list
    if(forceSensorWrite || (ListIdx >= MAX_WRITING_BLOCKS) || (index < 0) || IsVolatile(index))
    {
        // flush any pending writes
        iResult = FlushPendingWrites();
        if(iResult!=STERR_OK)
        {
            return iResult;
        }
    }

    // update the internal table
    if(index >= 0)
    {
        SetRegData(index, DataR);
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function writes all the registers queued in the list
 *
 *  @return @ref STERR_OK upon success
 *  @return (other values) Errors from functions called internally
 */
//---------------------------------------------------------------------------
static int FlushPendingWrites()
{
    int iResult;

    if(ListIdx > 0)
    {
        iResult = USB_Write_Register_Table(ListIdx, (BYTE*)RegWriteList);
        if(iResult!=STERR_OK)
        {
            return iResult;
        }

        // reset the queue
        memset(RegWriteList, 0, sizeof(RegWriteList));
        ListIdx = 0;
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function loads the register table with the actual values 
 *  of the physical registers.
 *  
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_FIRMWAREREV if not supported by firmware
 *  @return (other values) Errors from functions called internally
 */
// --------------------------------------------------------------------------
int LoadRegTable()
{
    BYTE reg_buffer[MAX_READING_BLOCKS];
    int i, nreg, iResult;

    nreg = ((MaxNumReg<=MAX_READING_BLOCKS)?(MaxNumReg):(MAX_READING_BLOCKS));
    for(i=0; i<nreg; i++)
    {
        reg_buffer[i] = RegTable[i].regnum;
    }

    // read the registers
    iResult = USB_Read_Register_Table(nreg, reg_buffer);
    if(iResult!=STERR_OK)
    {
        return iResult;
    }

    // reload the register table
    for(i=0; i<nreg; i++)
    {
        SetRegData(i, reg_buffer[i]);
    }

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function read a set of registers in one USB transaction.
 *
 *  The buffer has the following format: Addr0,Addr1,...AddrN where N is 
 *  @a Num_Regs. Maximum value of @a Num_Regs is 58. The ST9 will perform the 
 *  reads in the buffer sequence. Firmware registers can also be read. The 
 *  function works only on USB with firmware 0x17 or later.
 *
 *  @param Num_Regs   Number of registers to read (max value is 58)
 *  @param reg_table  Pointer to a table of addresses to read.
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_BADPARAMETER if the buffer is too large
 *  @return (other values) USB errors
 */
//---------------------------------------------------------------------------
static int USB_Read_Register_Table(int Num_Regs, BYTE *reg_table)
{
    int retval;
    BYTE buffer64[64];

    // check max value of Num_Regs
    if(Num_Regs > MAX_READING_BLOCKS)
    {
        return STERR_BADPARAMETER;
    }

    memcpy(buffer64, reg_table, Num_Regs*sizeof(BYTE));

    retval = OnSendCmdTableUSB((ST_TOUCH_GET_REG_BLOCK|0x80), 0, buffer64, Num_Regs);
    if(Check_USB_CommunicationError(&retval) != STERR_OK)
    {
        return retval;
    }

    memcpy(reg_table, &buffer64[1], Num_Regs*sizeof(BYTE));

    return STERR_OK;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function write a set of registers in one USB transaction.
 *
 *  The buffer has the following format: Addr0,Data0,Addr1,Data1,...AddrN,DataN
 *  Where N is @a Num_Regs. Maximum value of @a Num_Regs is 25. The ST9 will 
 *  perform the writes in the buffer sequence. There is no limitation on repeat 
 *  writes to the same address.  The function works only on USB with firmware 0x17 
 *  or later
 *
 *  @param Num_Regs     Number of registers to write (max value is 25)
 *  @param reg_table    Pointer to a buffer of data to write.
 *
 *  @return @ref STERR_OK upon success
 *  @return @ref STERR_BADPARAMETER if the buffer is too large
 *  @return (other values) USB errors
 */
//---------------------------------------------------------------------------
int USB_Write_Register_Table(int Num_Regs, BYTE *reg_table)
{
    int retval;

    // check max value of Num_Regs
    if(Num_Regs > 25)
    {
        return STERR_BADPARAMETER;
    }

    retval = OnSendCmdTableUSB(ST_TOUCH_SET_REG_BLOCK, 0, reg_table, (Num_Regs*2));
    return Check_USB_CommunicationError(&retval);
}

//---------------------------------------------------------------------------
/**
 *  @brief The function builds the binary data to be written in the TC register.
 *
 *  @param WholeData    Value of the whole register (8 bits)
 *  @param LogData      Value of the logical register
 *  @param numbit       Number of bits for the data in the register
 *  @param shift        Position of the data in the register
 *  @param Bytedata     Value of the register to be written.  Must be pre-allocated
 *                      in memory.
 */
//---------------------------------------------------------------------------
static void GetByteData(int WholeData, int LogData, int numbit, int shift, BYTE *Bytedata)
{
    int i;
    int ZeroMul=0xffff, DataS=0;

    // building an hexadecimal number with numbit zeros
    for(i=0; i<numbit; i++)
    {
        ZeroMul &= (0xfffe << i);
    }

    // shifting the zeros at the right place
    for(i=0; i<shift; i++)
    {
        ZeroMul = ((ZeroMul << 1) | 0x0001);
    }

    DataS = (LogData << shift);

    // replacing the concerned value in WholeData
    WholeData = (WholeData & ZeroMul);
    WholeData |= DataS;

    *Bytedata = (BYTE)WholeData;
}

//---------------------------------------------------------------------------
/**
 *  @brief The function looks for the register number in the register table,
 *  and returns the index.
 *
 *  If the register is not found -> returns -1.
 *
 *  @param PhysRegNum   Number of the physical register
 *
 *  @return int >= 0 if index of the register in the internal table
 *  @return -1 if no entry was found in the internal table
 */
//---------------------------------------------------------------------------
static int GetIndexReg(int PhysRegNum)
{
    int i;

    for(i=0; i<MaxNumReg; i++)
    {
        if(RegTable[i].regnum == (BYTE)PhysRegNum)
        {
            return i;
        }
    }

    return -1;
}
