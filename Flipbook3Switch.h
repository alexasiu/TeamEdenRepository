/****************************************************************************
 
  Header file for Flipbook3Switch

 ****************************************************************************/

#ifndef Flipbook3Switch_H
#define Flipbook3Switch_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// Public Function Prototypes
bool InitFlip3Switch ( uint8_t Priority );
bool CheckFlip3SwitchEvents (void);
ES_Event RunFlip3Switch( ES_Event ThisEvent );
bool PostFlip3Switch( ES_Event ThisEvent );

// typedefs for the states of the flip3 switch
typedef enum {Ready2SampleF3, DebouncingF3} Flip3SwitchState_t ;

#endif /*Flipbook3Switch_H */
