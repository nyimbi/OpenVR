/**
 * @file defines.h
 *
 * Values, which can be set/changed by the user of the Comm library
 * 
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#ifndef __DEFINES_H__
#define __DEFINES_H__


//========================================================================
//      Constants
//========================================================================

/**
 * Communication bit rate for communication with ESS/TFM.
 * Max. 115200 bps, for ESS 2.x even 230400 bps.
 * Has to be one of the LL_SIO_BAUDRATE_xxxx constants
 */
#define COMM_BIT_RATE       LL_SIO_BAUDRATE_115200

/**
 * AWAKE after WAKEUP timeout in milliseconds.
 */
#define COMM_AWAKE_TIMEOUT   4000


/**
 * Communication timeout in milliseconds. After the timeout the frame will be considered lost and resent.
 * The length of timeout depends on comm bit rate (it has to allow at least enough time
 * to safely send biggest possible frame there and back). And to keep overhead low
 * it should not be less than 1 second.
 */
#define DEFAULT_TIMEOUT        5000
//1500

/**
 * WTX (keep-alive) timeout in milliseconds. 
 * Time, after which ESS/TFM sends WTX packets to avoid timeouts.
 * Recommended length is 70% of COMM_TIMEOUT
 */
#define COMM_WTX_TIMEOUT    1000


/**
 * Number of tries to send a frame before the connection is considered broken. 
 * Recommended 2-3 tries. Minimum 1. Value 0 is illegal!
 */
#define COMM_TRIES          2


/**
 * Max. length of the data part of one communication frame.
 * Determines the length of the buffer, which will be statically allocated for the frame.
 *
 * IMPORTANT: 
 *  MAX_FRAME_DATA_SIZE must not be longer than 2047
 *  MAX_FRAME_DATA_SIZE must not be shorter than the longest expected received data (with the exception of PTGrab command)
 */
#define MAX_FRAME_DATA_SIZE   2047

#define MAX_COMM_DATA_SIZE    131072    //128KB  


/**
 * Flag whether active state of AWAKE signal is in logical 0 or 1 (0 is default)
 */
#define COMM_AWAKE_ACTIVE_STATE     0


#endif // __DEFINES_H__



