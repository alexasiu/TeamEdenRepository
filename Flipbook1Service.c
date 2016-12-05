/****************************************************************************
 Module
   Flipbook1Service.c

 Revision
   1.0.1

 Description
   Controls F1 motor that spins the flipbook's animations.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/14/16 10:32 afs     started coding
 11/15/16 12:00 afs     tested
 11/26/16 16:39  afs    added reset functionality
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TemplateService.h"

// include the PWM library
#include "PWM8Tiva.h"

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "BITDEFS.H"
#include "Flipbook1Service.h"
#include "MainStoryService.h"

/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff<<2)
#define PORT_B    BIT1HI      // for the motor
#define PORT_F    BIT5HI      // for the 595 + LEDs

#define MOTOR_PIN GPIO_PIN_6  //PB6
#define PWM_CHAN   0           // which is PB6
#define PWM_GROUP  0
#define PWM_FREQ  50
#define MS_TO_US( A )  A/(0.0008*1000)  // converts to ticks of 0.8 us
#define PWM_PULSE MS_TO_US( 1530 )      //  (defined in ticks of 0.8 us)
// 1550 and above is 1.937 ms so it goes counter-clockwise (direction we want)
// larger than 1550 will be faster in the counter-clockwise direction
// 1550 and below goes clockwise. Closer to 0 is faster
#define PWM_CELEB_PULSE  MS_TO_US( 1600 )
#define PWM_RESET_PULSE  MS_TO_US( 1550 )

// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static Flip1State_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFlip1Service

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
	A. Siu
****************************************************************************/
bool InitFlip1Service ( uint8_t Priority )
{
  ES_Event ThisEvent;
	//Initialize the MyPriority variable with the passed in parameter.
  MyPriority = Priority;
	
	//Make sure PWM is initialized in Main
	
	//Initialize the port line to control the LEDs on Flipbook 1
  HWREG(SYSCTL_RCGCGPIO) |= PORT_F;   // enable port F  
	// wait for the port to be ready
	while( (HWREG(SYSCTL_PRGPIO) & PORT_F) != PORT_F );  

	//Set CurrentState to be InitFlipbook1Service
	CurrentState = InitFlip1;
	
	//Post Event ES_Init to Flipbook3Service queue (this service)
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
     PostFlip1Service

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
bool PostFlip1Service( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFlip1Service

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   A.Siu
****************************************************************************/
ES_Event RunFlip1Service( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  Flip1State_t NextState = CurrentState;
  switch ( CurrentState )
  {
		case InitFlip1:
			if ( ThisEvent.EventType == ES_INIT ) {
				#if DEBUG_F1
				printf( "F1S: Init starting.\n\r\n" );
				#endif
				PWM8_TIVA_SetFreq( PWM_FREQ, PWM_GROUP );
				NextState = Wait4Seed;
			}
		break;
		
		case Wait4Seed:
			if ( ThisEvent.EventType == ES_SEED_DETECTED ) {
				//Start the motor with PWM duty cycle at 
				//printf( "pulse %f\n\r\n", PWM_PULSE );
				PWM8_TIVA_SetPulseWidth( PWM_PULSE, PWM_CHAN );
				//Set NextState Wait4Stop
				NextState = Wait4Stop;
				#if DEBUG_F1
				printf( "F1S: seed detected, starting motor\n\r\n" );
				#endif
			} 
			else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set NextState to wait for reset
				NextState = Wait4ResetF1;
			}
		break;
		
		case Wait4Stop:
			//if ThisEvent is ES_F1_DONE
			if ( ThisEvent.EventType == ES_F1_DONE ) {
				#if DEBUG_F1
				printf( "F1S: F1 done spinning stopping motor F1\n\r\n" );
				#endif
				//Stop the motor
				PWM8_TIVA_SetDuty( 0, PWM_CHAN );
				//Set NextState Wait4Celebration
				NextState = Wait4CelebrationF1;
			} 
			else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set NextState to wait for reset
				NextState = Wait4ResetF1;
			}
			break;
		
		case Wait4CelebrationF1:
			// if it's a celebration event
			if ( ThisEvent.EventType == ES_CELEBRATION ) {
				// start the F1 motor to make 2 flips
				PWM8_TIVA_SetPulseWidth( PWM_CELEB_PULSE, PWM_CHAN );
				#if DEBUG_F1
				printf("F1S: celebration mode!\n\r\n");
				#endif
			} 
			// if it's time to reset
			else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set NextState to wait for reset
				NextState = Wait4ResetF1;
			}
			break;

		case Wait4ResetF1:
			if ( ThisEvent.EventType == ES_F1_DONE ) {
				//Turn off motor
				PWM8_TIVA_SetDuty( 0, PWM_CHAN );
				#if DEBUG_F1
				printf("F1S: Wait4ResetF1 - done resetting flipbook\n\r\n");
				#endif
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Set NextState to InitFlipbook3Service
				NextState = InitFlip1;
			}
		break;
		
		default :
	  ;
		
	} // end SM
	CurrentState = NextState;
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryFlip1Service

 Parameters
     None

 Returns
     Flip1State_t The current state of the Flipvook1Service state machine

 Description
     returns the current state of the Flipbook1 state machine
 Notes

 Author
     A. Siu, 11/26/16, 19:21
****************************************************************************/
Flip1State_t QueryFlip1Service ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

