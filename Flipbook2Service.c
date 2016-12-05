/****************************************************************************
 Module
   Flipbook2.c

 Revision
   1.0.1

 Description
   Controls water bucket vibration motor

 Notes

 History
 When             Who          What/Why
 --------------  ---           --------
 11/15/16 1:12   chaim         started coding
 11/15/16 18:32  chaim & afs   integrating to framework
 11/26/16 16:39  afs           added reset functionality
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/

#include <cmath>
#include "ES_Configure.h"
#include "ES_Framework.h"

// include the PWM library
#include "PWM8Tiva.h"
#include "PWMTiva.h"

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
#include "WaterBucketService.h"
#include "Flipbook2Service.h"
#include "ADMulti.h"
#include "MainStoryService.h"

#define ALL_BITS (0xff<<2)
#define PI 3.14159265

#define PORT_B       BIT1HI    // for the motor
#define MOTOR_PIN   GPIO_PIN_7
#define PWM_CHAN  1           // which is PB7
#define PWM_GROUP 0
#define PWM_FREQ  50
#define MS_TO_US( A )    A/(0.0008*1000)    // converts to ticks of 0.8 us
#define PWM_PULSE        MS_TO_US( 1550 )  //  (defined in ticks of 0.8 us)
#define PWM_CELEB_PULSE  MS_TO_US( 1600 )
#define PWM_RESET_PULSE  MS_TO_US( 1550 )

#define MIN_PWM   1800
#define MAX_PWM   2000
#define MIN_ACC   2700 //Formerly 2610
#define MAX_ACC   1999
#define MIN_TILT_CHANGE 2500 //2600

#define PORT_F    BIT5HI      // for the 595 + LEDs
#define LED_PIN   BIT3HI;
#define LED_LO    BIT3LO;

//Local variables: LastAccState, MyPriority, CurrentState
static uint8_t MyPriority;
static Flip2State_t CurrentState;

//InitFlip2Service
//Takes a priority number, returns True.
bool InitFlip2Service ( uint8_t Priority ) {
	ES_Event ThisEvent;
	//Initialize the MyPriority variable with the passed in parameter.
	MyPriority = Priority;
	
	//Initialize the port line to control the LEDs on Flipbook 2
	//LEDs belong to port F, pin 3
	HWREG(SYSCTL_RCGCGPIO) |= PORT_F;
	while ((HWREG(SYSCTL_PRGPIO) & PORT_F) != PORT_F);
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= LED_PIN;
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) |= LED_PIN;
	//Turn the LEDs off TODO
	
//	Set CurrentState to be InitFlipbook2Service
	CurrentState = InitFlipbook2Service;
//	Post Event ES_Init to Flipbook2Service queue (this service)
	ThisEvent.EventType = ES_INIT;
	if (ES_PostToService( MyPriority, ThisEvent) == true)
  {  
    return true;
  }else
  {
		return false;
  }
}//	End of InitFlip2Service (return True)

//RunFlip2Service (implements the state machine for Flipbook2 Service)
//The EventType field of ThisEvent will be one of: ES_INIT, ES_WATER, ES_NO_WATER, ES_F2_DONE, ES_CELEBRATION
//Local Variables: NextState
ES_Event RunFlip2Service ( ES_Event ThisEvent ) {
//Set NextState to CurrentState
	Flip2State_t NextState = CurrentState;
	//Based on the state of the CurrentState variable choose one of the following blocks of code:
	switch ( CurrentState ) {
		
		//CurrentState is InitFlipbook2Service
		case InitFlipbook2Service :
			//if ThisEvent is ES_INIT
			if (ThisEvent.EventType == ES_INIT) {
				// set the pwm frequency, turn of motors
				PWM8_TIVA_SetFreq( PWM_FREQ, PWM_GROUP );
				//Set NextState AwaitFlip1Finished
				NextState = AwaitFlip1Finished;
				#if DEBUG_F2
				printf("FS2: init starting. \n\r\n");
				#endif
			}
		break;

		//CurrentState is AwaitFlip1Finished
		case AwaitFlip1Finished :
			//if ThisEvent is ES_F1_DONE
			if (ThisEvent.EventType == ES_F1_DONE) {
				#if DEBUG_F2
				printf("F2S: F1 done, waiting 4 water\n\r\n");
				#endif
				//Set NextState Wait4Water
				NextState = AwaitingWater;
			} else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set NextState to InitFlipbook2Service
				NextState = Wait4ResetF2;
			}	
		break; //End Wait4Seed block

		//CurrentState is AwaitingWater
		case AwaitingWater :
			
			//if ThisEvent is ES_WATER
			if ( ThisEvent.EventType == ES_WATER ) {
				// if it's close to zero 
				if ( ThisEvent.EventParam >= 2595 ) {
					//Stop the motor
					PWM8_TIVA_SetDuty(0, PWM_CHAN);
				} else { // else start the motor
					// calculate the scaled parameter
					uint16_t AccPulse = (((MAX_PWM - MIN_PWM)*(ThisEvent.EventParam - MIN_ACC))/(MAX_ACC-MIN_ACC)) + MIN_PWM;
					//Start the motor with the scaled parameter
					PWM8_TIVA_SetPulseWidth( AccPulse, PWM_CHAN );
					#if DEBUG_F2
						printf("F2S: F2 motor running at %u\n\r\n", AccPulse );
					#endif
				}
			}
			
			//if ThisEvent is ES_F2_DONE
			else if ( ThisEvent.EventType == ES_F2_DONE ) {
				//Stop the motor
				PWM8_TIVA_SetDuty(0, PWM_CHAN);
				//Keep motor running now at a constant rate
				PWM8_TIVA_SetPulseWidth(PWM_PULSE, PWM_CHAN);
				#if DEBUG_F2
				printf("FS2: flipbook2 done, stopping motor\n\r\n");
				#endif
				//Set NextState Wait4Celebration
				NextState = Wait4Celebration;
			}
			//if ThisEvent is ES_RESET
			else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set NextState to InitFlipbook2Service
				NextState = Wait4ResetF2;
			}	
		break; //	End Wait4Water block

		//CurrentState is Wait4Celebration
		case Wait4Celebration :
			//if ThisEvent is ES_CELEBRATION
			if ( ThisEvent.EventType == ES_CELEBRATION ) {
				// start the F2 motor 
				PWM8_TIVA_SetPulseWidth( PWM_CELEB_PULSE, PWM_CHAN );
				#if DEBUG_F2
				printf("F2S: Celebration!\n\r\n");
				#endif
			} else if ( ThisEvent.EventType == ES_RESET ) {
				//Turn on the motor to reset to the beginning
				PWM8_TIVA_SetPulseWidth( PWM_RESET_PULSE, PWM_CHAN );
				//Set NextState to InitFlipbook2Service
				NextState = Wait4ResetF2;
			}			
		break; //	End Wait4Celebration block

		case Wait4ResetF2:
			if ( ThisEvent.EventType == ES_F2_DONE ) {
				//Turn off motor
				PWM8_TIVA_SetDuty( 0, PWM_CHAN );
				#if DEBUG_F2
				printf("F2S: Wait4ResetF2 - done resetting flipbook\n\r\n");
				#endif
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Set NextState to InitFlipbook3Service
				NextState = InitFlipbook2Service;
			}
		break;	
	} // end SM
	
	//Set CurrentState to NextState
	CurrentState = NextState;
	//eturn ES_NO_EVENT
	ThisEvent.EventType = ES_NO_EVENT;
	return ThisEvent;
}//End of RunFlip2Service

//PostFlip2Service
bool PostFlip2Service ( ES_Event ThisEvent ) {
//Post Event to ES_SERVICES
	return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
     QueryFlip2Service

 Parameters
     None

 Returns
     Flip2State_t The current state of the Flipbook2Service state machine

 Description
     returns the current state of the Flipbook2 state machine
 Notes

 Author
     A. Siu, 11/26/16, 19:21
****************************************************************************/
Flip2State_t QueryFlip2Service ( void )
{
   return(CurrentState);
}

