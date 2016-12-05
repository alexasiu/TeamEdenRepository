/****************************************************************************
 Module
   MainStoryService.c

 Revision
   1.0.1

 Description
   This handles celebration and reset of all services.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/15/16 09:58 afs     started coding
 11/26/16 16:46 afs     added reset functionality
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TemplateService.h"

#include "MainStoryService.h"

/*----------------------------- Module Defines ----------------------------*/

// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define GAME_TIME 60000

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static MainState_t CurrentState;
static uint8_t NumDoneInit;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMainService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service

 Author
     A. Siu
****************************************************************************/
bool InitMainService ( uint8_t Priority )
{
  ES_Event ThisEvent;
  MyPriority = Priority;
	CurrentState = InitMain;
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostMainService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     A. Siu
****************************************************************************/
bool PostMainService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMainService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   A. Siu
****************************************************************************/
ES_Event RunMainService( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  MainState_t NextState = CurrentState;
  switch ( CurrentState )
  {
		case InitMain:
			if ( ThisEvent.EventType == ES_INIT ) {
				// reset the count of states done resetting
				NumDoneInit = 0;
				// set next state to wait4seed
				NextState = Wait4Seed_M;
				#if DEBUG_MAIN
				printf( "Main: Init \n\r\n" );
				#endif
			}
			break;
			
		case Wait4Seed_M:
			if ( ThisEvent.EventType == ES_SEED_DETECTED ) {
				#if DEBUG_MAIN
				printf( "Main: got seed \n\r\n" );
				#endif
				ES_Timer_InitTimer( GAME_TIMER, GAME_TIME );
				NextState = Wait4AllFlips;
			} else if ( ThisEvent.EventType == ES_RESET ) {
				// post reset to all services
				ES_Event Event2Post;
				Event2Post.EventType = ES_RESET;
				ES_PostList00( Event2Post );
				NextState = Wait4Reset;
			}
			break;
	
		case Wait4AllFlips:
			if ( ThisEvent.EventType == ES_F3_DONE ) {
				#if DEBUG_MAIN
				printf( "Main: all flips done. time to celebrate \n\r\n" );
				#endif
				// post to all services to celebrate
				ES_Event Event2Post;
				Event2Post.EventType = ES_CELEBRATION;
				ES_PostList01( Event2Post );
				// start the celebration timer
				ES_Timer_InitTimer( CELEB_TIMER, ONE_SEC*10 );
				NextState = Celebrating;
			}
			// if game time is up == timeout
			else if ( (ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == GAME_TIMER) )  { 
				#if DEBUG_MAIN
				printf( "Main: timeout. reset all \n\r\n" );
				#endif
				// post reset to all services
				ES_Event Event2Post;
				Event2Post.EventType = ES_RESET;
				ES_PostList00( Event2Post );
				// set the next state to wait for all services to reset
				NextState = Wait4Reset;
			}
			else if ( ThisEvent.EventType == ES_RESET ) {
				// post reset to all services
				ES_Event Event2Post;
				Event2Post.EventType = ES_RESET;
				ES_PostList00( Event2Post );
				NextState = Wait4Reset;
			}
			break;
			
		case Celebrating:
			// if it's a timeout (from game timer or celeb timer), we need to reset
			if ( ThisEvent.EventType == ES_TIMEOUT ) { 
				#if DEBUG_MAIN
				printf( "Main: timeout. reset all \n\r\n" );
				#endif
				// post reset to all services
				ES_Event Event2Post;
				Event2Post.EventType = ES_RESET;
				ES_PostList00( Event2Post );
				// set next state to wait for all services to reset
				NextState = Wait4Reset;
			} else if ( ThisEvent.EventType == ES_RESET ) {
				// post reset to all services
				ES_Event Event2Post;
				Event2Post.EventType = ES_RESET;
				ES_PostList00( Event2Post );
				NextState = Wait4Reset;
			}
			break;

		case Wait4Reset:
			// is an ES_DONE_INIT is posted, a service has finished
			if ( ThisEvent.EventType == ES_DONE_INIT ) {
				// increment the counter to keep track of services done
				NumDoneInit++;
				#if DEBUG_MAIN
				printf( "Main: services done: %u\n\r\n", NumDoneInit );
				#endif
				// if all services are done
				if ( NumDoneInit >= 6 ) {
					#if DEBUG_MAIN
					printf( "Main: all services done init\n\r\n" );
					#endif
					// post an ES_INIT event so all services can re-initialize
					ES_Event Event2Post;
					Event2Post.EventType = ES_INIT;
					ES_PostList00( Event2Post );
					// transition to the init state
					NextState = InitMain;
				}
			}
		break;
		
		default :
	    ;
		
	} // end SM
	CurrentState = NextState;
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

