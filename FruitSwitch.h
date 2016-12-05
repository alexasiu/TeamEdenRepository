/****************************************************************************
 
  Header file for FruitSwitch

 ****************************************************************************/

#ifndef FruitSwitch_H
#define FruitSwitch_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// Public Function Prototypes
bool InitFruitSwitch ( uint8_t Priority );
bool CheckFruitSwitchEvents (void);
ES_Event RunFruitSwitch( ES_Event ThisEvent );
bool PostFruitSwitch( ES_Event ThisEvent );

// typedefs for the states of the fruit switch
typedef enum {Ready2SampleFr, DebouncingFr} FruitSwitchState_t ;

#endif /*FruitSwitch_H */
