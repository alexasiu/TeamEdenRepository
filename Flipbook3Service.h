/****************************************************************************
 
  Header file for Flipbook3Service

 ****************************************************************************/

#ifndef FLIP3_SERV_H
#define FLIP3_SERV_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
typedef enum { InitFlip3, Wait4Flip2Done, Wait4ShortTimeout, Wait4Harvesting, 
               Wait4Flip3Done, Wait4CelebrationF3, Wait4ResetF3 } Flip3State_t ;

// Public Function Prototypes
bool InitFlip3Service ( uint8_t Priority );
bool PostFlip3Service ( ES_Event ThisEvent );
ES_Event RunFlip3Service ( ES_Event ThisEvent );
Flip3State_t QueryFlip3Service ( void );
bool QueryInitFlip3Service ( void );

#endif /* FLIP3_SERV_H */

