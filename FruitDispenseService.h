/****************************************************************************
 
  Header file for FruitDispenseService

 ****************************************************************************/

#ifndef FRUITDISPENSESERVICE_H
#define FRUITDISPENSESERVICE_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
typedef enum { InitFruitDisp, Wait4Flip3DoneFr, Wait4FrDispDone} FruitMotorState_t ;

// Public Function Prototypes
bool InitFruitService ( uint8_t Priority );
bool PostFruitService ( ES_Event ThisEvent );
ES_Event RunFruitService ( ES_Event ThisEvent );

#endif /* FRUITDISPENSESERVICE_H */

