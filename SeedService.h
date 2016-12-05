/****************************************************************************
 
  Header file for SeedService

 ****************************************************************************/

#ifndef SeedService_H
#define SeedService_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// Public Function Prototypes
bool InitSeedService ( uint8_t Priority );
bool CheckSeedSwitchEvents (void);
ES_Event RunSeedService( ES_Event ThisEvent );
bool PostSeedService( ES_Event ThisEvent );

// typedefs for the states of the seed switch
typedef enum {Ready2Sample, Debouncing} SeedSwitchState_t ;

#endif /* SeedService_H */

