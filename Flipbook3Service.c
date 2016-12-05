/****************************************************************************
 Module
   Flipbook3Service.c

 Revision
   1.0.1

 Description
   Controls F3 motor, and F3 LEDs (for indicating time).

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/13/16 10:32 afs     started coding
 11/26/16 16:46 afs     added reset functionality
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
#include "Flipbook3Service.h"
#include "AirService.h"
#include "MainStoryService.h"

/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff<<2)
#define PORT_B    BIT1HI      // for the motor
#define PORT_F    BIT5HI      // for the 595 + LEDs
#define PORT_A    BIT0HI
#define PORT_E    BIT4HI

#define PWM_CHAN  2           // which is PB4
#define PWM_GROUP 1
#define PWM_FREQ  50
#define PWM_DUTY  90
#define PWM_FREQ  50
#define MS_TO_US( A )  A/(0.0008*1000)  // converts to ticks of 0.8 us
#define PWM_PULSE MS_TO_US( 1530 )      //  (defined in ticks of 0.8 us)
#define PWM_CELEB_PULSE  MS_TO_US( 1600 )
#define PWM_RESET_PULSE  MS_TO_US( 1550 )

// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define F3_SHORT_TIME (ONE_SEC)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static Flip3State_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTemplateService

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
bool InitFlip3Service ( uint8_t Priority )
{
  ES_Event ThisEvent;
	//Initialize the MyPriority variable with the passed in parameter.
  MyPriority = Priority;
	
	//Make sure PWM is initialized in Main
	
	//Initialize the port line to control the LEDs on Flipbook 3
  HWREG(SYSCTL_RCGCGPIO) |= PORT_F;   // enable port F  
	// wait for the port to be ready
	while( (HWREG(SYSCTL_PRGPIO) & PORT_F) != PORT_F );  
	// TODO Set LEDs Low

	//Set CurrentState to be InitFlipbook3Service
	CurrentState = InitFlip3;
	
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
     PostFlip3Service

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
bool PostFlip3Service( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFlip3Service

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
ES_Event RunFlip3Service( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  Flip3State_t NextState = CurrentState;
  switch ( CurrentState )
  {
		case InitFlip3:
			if ( ThisEvent.EventType == ES_INIT ) {
				#if DEBUG_F3
				printf( "FS3: Init starting.\n\r\n" );
				#endif
				// set PWM motor frequency
				PWM8_TIVA_SetFreq( PWM_FREQ, PWM_GROUP );
				//Start at 0 duty cycle
				PWM8_TIVA_SetDuty( 0, PWM_CHAN );
				NextState = Wait4Flip2Done;
			}
			break;
		
		//CurrentState is Wait4Flip2Done
		case Wait4Flip2Done:
			//if ThisEvent is ES_F2_DONE
			if ( ThisEvent.EventType == ES_F2_DONE ) {
				//Start the motor
				PWM8_TIVA_SetPulseWidth( PWM_PULSE, PWM_CHAN );
				//Start the FLIPBOOK3_INIT_TIMER
				ES_Timer_InitTimer( FLIPBOOK3_INIT_TIMER, F3_SHORT_TIME );
				//Set NextState to Wait4ShortTimeout
				NextState = Wait4ShortTimeout;
			} else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set next state to ResetF3
				NextState = Wait4ResetF3;
			}
		break; //End Wait4Flip2Done block
		
		//CurrentState is Wait4ShortTimeout
		case Wait4ShortTimeout:	
			//if ThisEvent is ES_TIMEOUT and time corresponds to FLIPBOOK3_INIT_TIMER
			if ( (ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == FLIPBOOK3_INIT_TIMER) ){
				//Stop motors
				PWM8_TIVA_SetDuty( 0, PWM_CHAN );
				//Post ES_START_HARVEST to AirService
				ES_Event Event2Post;
				Event2Post.EventType = ES_START_HARVEST;
				PostAirService( Event2Post );
				#if DEBUG_F3
				printf( "FS3: waiting for harvest\n\r\n" );
				#endif
				//Set NextState to Wait4Harvesting
				NextState = Wait4Harvesting;
			} else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set next state to ResetF3
				NextState = Wait4ResetF3;
			}
		break;
			
		case Wait4Harvesting:		
			//if ThisEvent is ES_DONE_HARVEST
			if ( ThisEvent.EventType == ES_DONE_HARVEST ) {
				//Start the F3 motor
				PWM8_TIVA_SetPulseWidth( PWM_PULSE, PWM_CHAN );
				#if DEBUG_F3
				printf( "FS3: starting flipbook3 motor\n\r\n" );
				#endif
				//NextState is Wait4FlipDone
				NextState = Wait4Flip3Done;
			}	else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set next state to ResetF3
				NextState = Wait4ResetF3;
			}
			break;
			
		case Wait4Flip3Done:
			//if ThisEvent is ES_F3_DONE
			if ( ThisEvent.EventType == ES_F3_DONE ) {
				//Stop the motor
				PWM8_TIVA_SetDuty( 0, PWM_CHAN );
				//Set NextState to Wait4Celebration
				NextState = Wait4CelebrationF3;
			}
			//if ThisEvent is ES_RESET
			else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set next state to ResetF3
				NextState = Wait4ResetF3;
			}
			break;
		
		case Wait4CelebrationF3:
			if ( ThisEvent.EventType == ES_CELEBRATION ) {
				// start the F3 motor 
				PWM8_TIVA_SetPulseWidth( PWM_CELEB_PULSE, PWM_CHAN );
				#if DEBUG_F3
				printf("F3S: celebration mode!\n\r\n");
				#endif
			} else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set next state to ResetF3
				NextState = Wait4ResetF3;
			}
			break;
		
		case Wait4ResetF3:
			if ( ThisEvent.EventType == ES_F3_DONE ) {
				//Turn off motor
				PWM8_TIVA_SetDuty( 0, PWM_CHAN );
				#if DEBUG_F3
				printf("F3S: Wait4ResetF3 - done resetting flipbook\n\r\n");
				#endif
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Set NextState to InitFlipbook3Service
				NextState = InitFlip3;
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
     QueryFlip3Service

 Parameters
     None

 Returns
     Flip3State_t The current state of the Flipvook3Service state machine

 Description
     returns the current state of the Flipbook3 state machine
 Notes

 Author
     A. Siu, 11/26/16, 19:21
****************************************************************************/
Flip3State_t QueryFlip3Service ( void )
{
   return ( CurrentState );
}


/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

