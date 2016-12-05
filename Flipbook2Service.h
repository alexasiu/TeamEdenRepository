/****************************************************************************
 
  Header file for flipbook 2 service

 ****************************************************************************/

#ifndef FLIPBOOK_2_H
#define FLIPBOOK_2_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
typedef enum { InitFlipbook2Service, AwaitFlip1Finished, AwaitingWater, 
			   Wait4Celebration, Wait4ResetF2 } Flip2State_t ;

// Public Function Prototypes
bool InitFlip2Service ( uint8_t Priority );
bool PostFlip2Service ( ES_Event ThisEvent );
ES_Event RunFlip2Service ( ES_Event ThisEvent );
Flip2State_t QueryFlip2Service ( void );

#endif /* FLIPBOOK_2_H */
