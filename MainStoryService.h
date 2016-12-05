/****************************************************************************
 
  Header file for the MainStoryService

 ****************************************************************************/

#ifndef MainServ_H
#define MainServ_H

#include "ES_Types.h"

// typedefs for the states
typedef enum { InitMain, Wait4Seed_M, Wait4AllFlips, Celebrating, Wait4Reset } MainState_t ;

// Public Function Prototypes
bool InitMainService ( uint8_t Priority );
bool PostMainService( ES_Event ThisEvent );
ES_Event RunMainService( ES_Event ThisEvent );

#endif /* MainServ_H */

