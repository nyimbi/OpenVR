/**
 * @file reg_map.c
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */


//-------------------------- Function definitions ---------------------------

/**
 * @brief Translates logical register to physical register location
 *
 * @param Regnum Register number (logical)
 * @param indexreg Register number (physical).  Must be pre-allocated in memory
 * @param numbit Number of bits for the data in the register.  Must be pre-allocated in memory
 * @param shift Position of the data in the register.  Must be pre-allocated in memory
 */
void MapRegsF(int Regnum, int *indexreg, int *numbit, int *shift)
{
    switch(Regnum)
    {
    case 0:
        *indexreg = 0; *numbit = 1; *shift = 7;
        break;
    case 1:
        *indexreg = 0; *numbit = 6; *shift = 0;
        break;
    case 2:
        *indexreg = 1; *numbit = 6; *shift = 0;
        break;
    case 3:
        *indexreg = 2; *numbit = 5; *shift = 0;
        break;
    case 4:
        *indexreg = 4; *numbit = 5; *shift = 0;
        break;
    case 5:
        *indexreg = 5; *numbit = 3; *shift = 5;
        break;
    case 113:
        *indexreg = 5; *numbit = 2; *shift = 2;
        break;
    case 6:
        *indexreg = 5; *numbit = 1; *shift = 1;
        break;
    case 7:
        *indexreg = 6; *numbit = 5; *shift = 0;
        break;
    case 8:
        *indexreg = 7; *numbit = 6; *shift = 0;
        break;
    case 9:
        *indexreg = 8; *numbit = 3; *shift = 0;
        break;
    case 10:
        *indexreg = 9; *numbit = 1; *shift = 7;
        break;
    case 11:
        *indexreg = 9; *numbit = 3; *shift = 4;
        break;
    case 12:
        *indexreg = 9; *numbit = 1; *shift = 3;
        break;
    case 13:
        *indexreg = 9; *numbit = 2; *shift = 1;
        break;
    case 14:
        *indexreg = 9; *numbit = 1; *shift = 0;
        break;
    case 15:
        *indexreg = 10;  *numbit = 1; *shift = 7;
        break;
    case 16:
        *indexreg = 10;  *numbit = 1; *shift = 4;
        break;
    case 17:
        *indexreg = 10; *numbit = 1; *shift = 3;
        break;
    case 18:
        *indexreg = 10; *numbit = 1; *shift = 2;
        break;
    case 19:
        *indexreg = 10; *numbit = 1; *shift = 1;
        break;
    case 20:
        *indexreg = 10; *numbit = 1; *shift = 0;
        break;
    case 21:
        *indexreg = 11; *numbit = 6; *shift = 0;
        break;
    case 22:
        *indexreg = 11; *numbit = 1; *shift = 7;
        break;
    case 23:
        *indexreg = 11; *numbit = 1; *shift = 6;
        break;
    case 24:
        *indexreg = 12; *numbit = 1; *shift = 7;
        break;
    case 25:
        *indexreg = 12; *numbit = 1; *shift = 6;
        break;
    case 26:
        *indexreg = 12; *numbit = 1; *shift = 5;
        break;
    case 27:
        *indexreg = 12; *numbit = 1; *shift = 4;
        break;
    case 28:
        *indexreg = 12; *numbit = 4; *shift = 0;
        break;
    case 29:
        *indexreg = 13; *numbit = 8; *shift = 0;
        break;
    case 30:
        *indexreg = 14; *numbit = 1; *shift = 7;
        break;
    case 31:
        *indexreg = 14; *numbit = 1; *shift = 2;
        break;
    case 32:
        *indexreg = 14; *numbit = 1; *shift = 1;
        break;
    case 33:
        *indexreg = 14; *numbit = 1; *shift = 0;
        break;
    case 34:
        *indexreg = 15; *numbit = 6; *shift = 2;
        break;
    case 35:
        *indexreg = 15; *numbit = 2; *shift = 0;
        break;
    case 36:
        *indexreg = 16; *numbit = 8; *shift = 0;
        break;
    case 360:
        *indexreg = 16; *numbit = 8; *shift = 0;
        break;  
    case 37:
        *indexreg = 17; *numbit = 1; *shift = 0;
        break;
    case 38:
        *indexreg = 18; *numbit = 8; *shift = 0;
        break;
    case 39:
        *indexreg = 19; *numbit = 1; *shift = 0;
        break;
    case 40:
        *indexreg = 20; *numbit = 8; *shift = 0;
        break;
    case 41:
        *indexreg = 21; *numbit = 8; *shift = 0;
        break;
    case 42:
        *indexreg = 22; *numbit = 1; *shift = 0;
        break;
    case 43:
        *indexreg = 23; *numbit = 8; *shift = 0;
        break;
    case 44:
        *indexreg = 24; *numbit = 1; *shift = 0;
        break;
    case 45:
        *indexreg = 25; *numbit = 8; *shift = 0;
        break;
    case 46:
        *indexreg = 26; *numbit = 8; *shift = 0;
        break;
    case 47:
        *indexreg = 27; *numbit = 8; *shift = 0;
        break;
    case 48:
        *indexreg = 28; *numbit = 8; *shift = 0;
        break;
    case 49:
        *indexreg = 29; *numbit = 1; *shift = 7;
        break;
    case 50:
        *indexreg = 29; *numbit = 1; *shift = 5;
        break;
    case 51:
        *indexreg = 29; *numbit = 1; *shift = 4;
        break;
    case 52:
        *indexreg = 29; *numbit = 1; *shift = 3;
        break;
    case 53:
        *indexreg = 29; *numbit = 1; *shift = 2;
        break;
    case 54:
        *indexreg = 29; *numbit = 2; *shift = 0;
        break;
    case 55:    // VendorCode[5:0]
        *indexreg = 31; *numbit = 6; *shift = 2;
        break;
    case 56:
        *indexreg = 31; *numbit = 2; *shift = 0;
        break;
    case 57:
        *indexreg = 32; *numbit = 1; *shift = 4;
        break;
    case 58:
        *indexreg = 32; *numbit = 4; *shift = 0;
        break;
    case 59:
        *indexreg = 33; *numbit = 1; *shift = 7;
        break;
    case 60:
        *indexreg = 33; *numbit = 1; *shift = 6;
        break;
    case 61:
        *indexreg = 33; *numbit = 1; *shift = 5;
        break;
    case 62:
        *indexreg = 33; *numbit = 1; *shift = 4;
        break;
    case 63:
        *indexreg = 33; *numbit = 3; *shift = 1;
        break;
    case 64:
        *indexreg = 34; *numbit = 1; *shift = 3;
        break;		
    case 65:
        *indexreg = 34; *numbit = 3; *shift = 0;
        break;		     
    case 66:
        *indexreg = 35; *numbit = 3; *shift = 3;
        break;
    case 67:
        *indexreg = 35; *numbit = 3; *shift = 0;
        break;
    case 68:
        *indexreg = 36; *numbit = 5; *shift = 0;
        break;
    case 120:
        *indexreg = 36; *numbit = 1; *shift = 5;
        break;
    case 69:
        *indexreg = 39; *numbit = 4; *shift = 0;
        break;
    case 70:
        *indexreg = 40; *numbit = 4; *shift = 0;
        break;
    case 71:
        *indexreg = 41; *numbit = 4; *shift = 0;
        break;
    case 72:
        *indexreg = 42; *numbit = 4; *shift = 0;
        break;
    case 73:
        *indexreg = 43; *numbit = 4; *shift = 0;
        break;
    case 74:
        *indexreg = 44; *numbit = 4; *shift = 0;
        break;
    case 75:
        *indexreg = 45; *numbit = 4; *shift = 0;
        break;
    case 76:
        *indexreg = 46; *numbit = 4; *shift = 0;
        break;
    case 77:
        *indexreg = 47; *numbit = 6; *shift = 2;
        break;
    case 78:
        *indexreg = 47; *numbit = 2; *shift = 0;
        break;
    case 79:
        *indexreg = 48; *numbit = 1; *shift = 7;
        break;
    case 80:
        *indexreg = 48; *numbit = 1; *shift = 6;
        break;
    case 81:
        *indexreg = 48; *numbit = 1; *shift = 4;
        break;
    case 82:
        *indexreg = 48; *numbit = 1; *shift = 3;
        break;
    case 83:
        *indexreg = 48; *numbit = 1; *shift = 2;
        break;
    case 84:
        *indexreg = 48; *numbit = 2; *shift = 0;
        break;
    case 85:
        *indexreg = 49; *numbit = 1; *shift = 5;
        break;
    case 86:
        *indexreg = 49; *numbit = 1; *shift = 4;
        break;
    case 87:
        *indexreg = 49; *numbit = 1; *shift = 3;
        break;
    case 88:
        *indexreg = 49; *numbit = 1; *shift = 2;
        break;
    case 89:
        *indexreg = 49; *numbit = 1; *shift = 1;
        break;
    case 90:
        *indexreg = 49; *numbit = 1; *shift = 0;
        break;
    case 91:
        *indexreg = 50; *numbit = 8; *shift = 0;
        break;
    case 92:
        *indexreg = 51; *numbit = 4; *shift = 4;
        break;
    case 93:
        *indexreg = 51; *numbit = 4; *shift = 0;
        break;
    case 94:
        *indexreg = 52; *numbit = 4; *shift = 4;
        break;
    case 95:
        *indexreg = 52; *numbit = 4; *shift = 0;
        break;
    case 96:
        *indexreg = 53; *numbit = 1; *shift = 7;
        break;
    case 97:
        *indexreg = 53; *numbit = 1; *shift = 6;
        break;
    case 98:
        *indexreg = 53; *numbit = 1; *shift = 5;
        break;
    case 99:
        *indexreg = 53; *numbit = 1; *shift = 4;
        break;
    case 100:
        *indexreg = 53; *numbit = 4; *shift = 0;
        break;
    case 101:
        *indexreg = 54; *numbit = 1; *shift = 1;
        break;
    case 102:
        *indexreg = 54; *numbit = 1; *shift = 0;
        break;
    case 103:
        *indexreg = 55; *numbit = 1; *shift = 3;
        break;
    case 104:
        *indexreg = 55; *numbit = 1; *shift = 2;
        break;
    case 105:
        *indexreg = 55; *numbit = 1; *shift = 1;
        break;
    case 106:
        *indexreg = 55; *numbit = 1; *shift = 0;
        break;
    case 107:
        *indexreg = 56; *numbit = 6; *shift = 0;
        break;
    case 108:
        *indexreg = 57; *numbit = 6; *shift = 0;
        break;
    case 109:
        *indexreg = 58; *numbit = 6; *shift = 0;
        break;
    case 110:
        *indexreg = 63; *numbit = 6; *shift = 2;
        break;
    case 111:
        *indexreg = 63; *numbit = 2; *shift = 0;
        break;
    case 114:
        *indexreg = 61; *numbit = 1; *shift = 5;
        break;
    case 115:
        *indexreg = 61; *numbit = 1; *shift = 4;
        break;
    case 116:
        *indexreg = 61; *numbit = 1; *shift = 3;
        break;
    case 117:
        *indexreg = 61; *numbit = 1; *shift = 2;
        break;
    case 118:
        *indexreg = 61; *numbit = 1; *shift = 1;
        break;
    case 119:
        *indexreg = 61; *numbit = 1; *shift = 0;
        break;
    case 112:
        *indexreg = 255; *numbit = 8; *shift = 0;
        break;
    case 121:
        *indexreg = 29; *numbit = 1; *shift = 6;
        break;
    case 122:
        *indexreg = 0xFB; *numbit = 8; *shift = 0;
        break;
    case 123:
        *indexreg = 0xFA; *numbit = 8; *shift = 0;
        break;
    case 124:
        *indexreg = 0xF9; *numbit = 8; *shift = 0;
        break;
    case 125:
        *indexreg = 0xF8; *numbit = 8; *shift = 0;
        break;
    case 126:
        *indexreg = 10;*numbit = 1; *shift = 6;
        break;
    case 127:
        *indexreg = 10;*numbit = 1; *shift = 5;
        break;
    case 128:
        *indexreg = 14;*numbit = 1; *shift = 6;
        break;
    case 129:
        *indexreg = 14;*numbit = 1; *shift = 5;
        break;
    case 130:
        *indexreg = 14;*numbit = 1; *shift = 4;
        break;
    case 131:
        *indexreg = 14;*numbit = 1; *shift = 3;
        break;
    case 132:
        *indexreg = 0xF7; *numbit = 8; *shift = 0;
        break;
    case 133:
        *indexreg = 0xF6; *numbit = 8; *shift = 0;
        break;
    case 134:
        *indexreg = 0xF5; *numbit = 8; *shift = 0;
        break;
    case 135:
        *indexreg = 0xF4; *numbit = 8; *shift = 0;
        break;
    case 136:
        *indexreg = 0xF3; *numbit = 8; *shift = 0;
        break;
    case 137:
        *indexreg = 0xF2; *numbit = 8; *shift = 0;
        break;
    case 138:
        *indexreg = 0xF1; *numbit = 8; *shift = 0;
        break;
    case 139:
        break;
    case 140:
        *indexreg = 0xF0; *numbit = 8; *shift = 0;
        break;
    }
}
