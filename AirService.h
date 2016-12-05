/****************************************************************************
 
  Header file for AirService

 ****************************************************************************/

#ifndef AIR_SERV_H
#define AIR_SERV_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
typedef enum { InitAir, Wait4HarvestingIR, Harvesting_IR1, 
							 Harvesting_IR2, Wait4CelebrationIR } AirState_t ;

// Public Function Prototypes
bool InitAirService ( uint8_t Priority );
bool PostAirService ( ES_Event ThisEvent );
ES_Event RunAirService ( ES_Event ThisEvent );
AirState_t QueryAirService ( void );

//Event checkers
bool Check4IR_1 ( void );
bool Check4IR_2 ( void );

#endif /* AIR_SERV_H */

