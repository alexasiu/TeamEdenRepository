/****************************************************************************
 
  Header file for template Flat Sate Machine 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef LEDService_H
#define LEDService_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum { InitLEDState, Waiting4Seed, F1Run, Wait4Watering, 
	           F2Run, F3Run, Celebration } LEDState_t ;

// Public Function Prototypes
bool InitLEDService ( uint8_t Priority );
bool PostLEDService( ES_Event ThisEvent );
ES_Event RunLEDService( ES_Event ThisEvent );
LEDState_t QueryLEDState ( void );

#endif /* LEDService_H */

