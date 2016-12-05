/****************************************************************************
 
  Header file for Flipbook1Switch

 ****************************************************************************/

#ifndef Flipbook1Switch_H
#define Flipbook1Switch_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// Public Function Prototypes
bool InitFlip1Switch ( uint8_t Priority );
bool CheckFlip1SwitchEvents (void);
ES_Event RunFlip1Switch( ES_Event ThisEvent );
bool PostFlip1Switch( ES_Event ThisEvent );

// typedefs for the states of the flip1 switch
typedef enum {Ready2SampleF1, DebouncingF1} Flip1SwitchState_t ;

#endif /*Flipbook1Switch_H */

