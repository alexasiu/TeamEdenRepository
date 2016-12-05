/****************************************************************************
 
  Header file for Flipbook2Switch

 ****************************************************************************/

#ifndef Flipbook2Switch_H
#define Flipbook2Switch_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// Public Function Prototypes
bool InitFlip2Switch ( uint8_t Priority );
bool CheckFlip2SwitchEvents (void);
ES_Event RunFlip2Switch( ES_Event ThisEvent );
bool PostFlip2Switch( ES_Event ThisEvent );

// typedefs for the states of the flip2 switch
typedef enum {Ready2SampleF2, DebouncingF2} Flip2SwitchState_t ;

#endif /*Flipbook2Switch_H */
