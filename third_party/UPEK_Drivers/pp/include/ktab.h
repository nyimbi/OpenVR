/**
 * @file ktab.h
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef _KTAB_H_
#define _KTAB_H_




#define KTPA5 (('™'<<8) + 'õ')
#define KTPA6 (('@'<<8) + 'š')
#define KTPA7 (('»'<<8) + 'þ')
#define KTPA10 (('œ'<<8) + 'ð')
#define KTPA2 (('·'<<8) + 'º')
#define KTPA1 (('Ó'<<8) + 'Ï')
#define KTPA13 (('ã'<<8) + 'Œ')
#define KTPA8 (('È'<<8) + 'å')
#define KTPA9 (('Õ'<<8) + 'Ï')
#define KTPA0 ((''<<8) + 'À')
#define KTPA15 (('.'<<8) + 'ƒ')
#define KTPA3 ((''<<8) + '¥')
#define KTPA4 (('Æ'<<8) + 'ž')
#define KTPA14 (('‡'<<8) + '¾')
#define KTPA12 (('–'<<8) + 'Ò')
#define KTPA11 (('Ð'<<8) + '@')






#define KTPB9   0x5ea6
#define KTPB10   0x2f6d
#define KTPB0   0xa983
#define KTPB1   0x351a
#define KTPB8   0x15ad
#define KTPB2   0x9669
#define KTPB5   0xd053
#define KTPB11   0xec75
#define KTPB4   0x5594
#define KTPB13   0x9aec
#define KTPB14   0x504b
#define KTPB15   0x07b7
#define KTPB7   0xef09
#define KTPB12   0x8a52
#define KTPB3   0x1fd2
#define KTPB6   0x8c95





#define KTPC10  7918-9462+4915     
#define KTPC9   8952+1946-7895    
#define KTPC12  3269+4529-8941  
#define KTPC11  7949-7982+4069    
#define KTPC0   1563+9245-4124
#define KTPC4   3017-3256+9032
#define KTPC14  1026+4976-7859     
#define KTPC1   23478+2345-12345
#define KTPC8   7774-8956+4465    
#define KTPC7   9865-7549+2659  
#define KTPC2   9823+0535-2234
#define KTPC13  5689-8956+4659    
#define KTPC3   1536+31284-28-9283
#define KTPC15  4565-7981+2983     
#define KTPC5   5659-4157+3692
#define KTPC6   4785+7826-1145   



#define KEYS_TABLE0 (KTPA0 + KTPB0 - KTPC0 )&0xFFFF
#define KEYS_TABLE1 (KTPA1 + KTPB1 - KTPC1 )&0xFFFF
#define KEYS_TABLE2 (KTPA2 + KTPB2 - KTPC2 )&0xFFFF
#define KEYS_TABLE3 (KTPA3 + KTPB3 - KTPC3 )&0xFFFF
#define KEYS_TABLE4 (KTPA4 + KTPB4 - KTPC4 )&0xFFFF
#define KEYS_TABLE5 (KTPA5 + KTPB5 - KTPC5 )&0xFFFF
#define KEYS_TABLE6 (KTPA6 + KTPB6 - KTPC6 )&0xFFFF
#define KEYS_TABLE7 (KTPA7 + KTPB7 - KTPC7 )&0xFFFF
#define KEYS_TABLE8 (KTPA8 + KTPB8 - KTPC8 )&0xFFFF
#define KEYS_TABLE9 (KTPA9 + KTPB9 - KTPC9 )&0xFFFF
#define KEYS_TABLE10 (KTPA10 + KTPB10 - KTPC10 )&0xFFFF
#define KEYS_TABLE11 (KTPA11 + KTPB11 - KTPC11 )&0xFFFF
#define KEYS_TABLE12 (KTPA12 + KTPB12 - KTPC12 )&0xFFFF
#define KEYS_TABLE13 (KTPA13 + KTPB13 - KTPC13 )&0xFFFF
#define KEYS_TABLE14 (KTPA14 + KTPB14 - KTPC14 )&0xFFFF
#define KEYS_TABLE15  (KTPA15 + KTPB15 - KTPC15 )&0xFFFF


#endif

