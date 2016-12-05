/****************************************************************************
 
  Header file for water bucket service

 ****************************************************************************/

#ifndef WATER_BUCKET_H
#define WATER_BUCKET_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
typedef enum { InitWaterBucketService, Wait4Flip1Done, 
	           Wait4Water, DoneWatering } WaterBucketState_t ;

// Public Function Prototypes
bool InitWaterService ( uint8_t Priority );
bool PostWaterBucketService ( ES_Event ThisEvent );
ES_Event RunWaterService ( ES_Event ThisEvent );
WaterBucketState_t QueryWaterService ( void );

//Event checkers
bool Check4Water ( void );

#endif /* WATER_BUCKET_H */
