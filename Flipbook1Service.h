/****************************************************************************
 
  Header file for Flipbook1Service

 ****************************************************************************/

#ifndef FLIP1_SERV_H
#define FLIP1_SERV_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
typedef enum { InitFlip1, Wait4Seed, Wait4Stop, 
		       Wait4CelebrationF1, Wait4ResetF1 } Flip1State_t ;

// Public Function Prototypes
bool InitFlip1Service ( uint8_t Priority );
bool PostFlip1Service ( ES_Event ThisEvent );
ES_Event RunFlip1Service ( ES_Event ThisEvent );
Flip1State_t QueryFlip1Service ( void );

#endif /* FLIP1_SERV_H */

