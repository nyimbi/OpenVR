/**
 * @file crypt.c
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#include "crypt.h"
#include "ktab.h"
#include "bits.h"
#define LFSR_MASK 0x0000D9F8

  // Galois configuration is faster in software
void step_lfsr(   unsigned char * history, unsigned short * shifter) {

  if ( *shifter & BIT0 ){
    *shifter = ((*shifter ^ LFSR_MASK) >> 1 ) | BIT15;
    *history |= BIT0;
  }
  else {
    *shifter = ( *shifter >> 1 );
  }
  *history = ( *history << 1 );
}


void encrypt( unsigned short key, unsigned char * clear_text, unsigned char numbytes){

  unsigned char i;
  unsigned char j;
  unsigned char history = 0;
  unsigned short shifter;


  shifter = key;

  for ( i=0;i<numbytes;i++){
    for ( j=0;j<8;j++){
      step_lfsr( &history, &shifter); // cycle 8 times for the next byte
    }
    *clear_text ^= history;
    clear_text++;
  }
}


//  map the lower 5 bits of an incoming number onto one of the characters A-Z,0-9
unsigned char num_to_char(unsigned char in){

  in &= 0x1F; // mask off any extra bits
  if (in > 25) {
    in += ('0'-26); // map 26-32 to '0'-'6'
  }
  else {
    in += 'A';  // map the rest to 'A'-'Z'
  }
return in;
}



void char_to_num(unsigned char * in){

  if (*in < 'A') {
    *in -= ('0'-26); // map '0'-'6' to 26-32 
  }
  else {
    *in -= 'A'; // map 'A'-'Z' to 0-25 
  }
}

int nums( unsigned char * buff, int numbytes){
  while (numbytes-- > 0) {
    char_to_num(buff);
    buff++;
  }
  return 0;
}


int chars( unsigned char * buff, int numbytes){
  while (numbytes-- > 0) {
    *buff = num_to_char(*buff);
    buff++;

  }
  return 0;
}

int crypt( unsigned short key, unsigned char * clear_text, unsigned char numbytes, int isPresetKey) {
  unsigned short realKey=key;
  if (!isPresetKey) realKey=key;
  else {
    if (key==0) realKey=KEYS_TABLE0;
    if (key==1) realKey=KEYS_TABLE1;
    if (key==2) realKey=KEYS_TABLE2;
    if (key==3) realKey=KEYS_TABLE3;
    if (key==4) realKey=KEYS_TABLE4;
    if (key==5) realKey=KEYS_TABLE5;
    if (key==6) realKey=KEYS_TABLE6;
    if (key==7) realKey=KEYS_TABLE7;
    if (key==8) realKey=KEYS_TABLE8;
    if (key==9) realKey=KEYS_TABLE9;
    if (key==10) realKey=KEYS_TABLE10;
    if (key==11) realKey=KEYS_TABLE11;
    if (key==12) realKey=KEYS_TABLE12;
    if (key==13) realKey=KEYS_TABLE13;
    if (key==14) realKey=KEYS_TABLE14;
    if (key==15) realKey=KEYS_TABLE15;
    if (key>15) return -1;
  }
  encrypt( realKey, clear_text, numbytes);
  return 0;
}
