/****************************************************************************
 Module
   LEDService.c

 Revision
   0.0.1

 Description
   This is a service that operates LEDs for the mechanical flipbook features

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/16/16 11:12 afb     Initial draft
 11/26/16 13:07 afs     added static functions, added response to ES_NO_WATER
 11/27/16 9:07  afs     modified rampWaterLEDs to respond to acc value
 11/28/16 9:33  hr      changed LED PIN
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/

#include <cmath>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "LEDService.h"

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

/*----------------------------- Module Defines ----------------------------*/

#define SEED_LED_ON BIT2HI		//PD2
#define PORT_D    BIT3HI      // for the seed LED

#define PWM_Flip1LED_CHAN  6    	//which is PDO
#define PWM_Flip1LED_GROUP 3		//grouped with 6 and 7
#define PWM_Flip1LED_FREQ  1000		//Initial PWM frequency for testing
#define PWM_Flip1LED_DUTY  0		//Initial PWM Duty for testing

#define PWM_Flip2LED_CHAN  7     	//which is PD1
#define PWM_Flip2LED_GROUP 3		//grouped with 6 and 7
#define PWM_Flip2LED_FREQ  1000		//Initial PWM frequency
#define PWM_Flip2LED_DUTY  0		//Initial PWM Duty for testing

#define PWM_Flip3LED_CHAN  5     	//which is PE5
#define PWM_Flip3LED_GROUP 2		//grouped with 4 and 5
#define PWM_Flip3LED_FREQ  1000		//Initial PWM frequency for testing
#define PWM_Flip3LED_DUTY  0		//Initial PWM Duty for testing

#define PWM_WATER_LED_CHAN  4     	//which is PE4
#define PWM_WATER_LED_GROUP 2		//grouped with 4 and 5
#define PWM_WATER_LED_FREQ  1000	//Initial PWM frequency for testing
#define PWM_WATER_LED_DUTY  0		//Initial PWM Duty for testing

#define MAX_SAFE_PWM_DUTY	70		//limit pwm to 85 of 99 to keep effective voltage below 12V, assuming 13.8V supply
#define MIN_PWM_DUTY	0 

// parameters for converting acc value to water brightness
#define MIN_TILT_CHANGE 2300 //2600
#define MIN_ACC   2610
#define MAX_ACC   1999

// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)

// timers for blinkin
#define BLINK_SEED_TIME  ONE_SEC
#define BLINK_WATER_TIME ONE_SEC

#define ALL_BITS (0xff<<2)



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

static void RampF1LEDS( void );
static void RampF2LEDS( void );
static void RampF3LEDS( void );
static void RampWaterLEDS ( uint8_t brightness );
static void BlinkSeedLEDS(bool);
static void BlinkWaterLEDS(bool);
static void BlinkAllLEDS(bool);
static void F1SetFullBrightness( void );
static void F2SetFullBrightness( void );
static void F3SetFullBrightness( void );

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match that of enum in header file
static LEDState_t CurrentState;

//module level variables to set brightness of leds that can be reset easily and persists across function calls
static uint8_t F1LED_Brightness = 0;				//flipbook 1 LED brightness
static uint8_t F2LED_Brightness = 0;				//flipbook 2 LED brightness
static uint8_t F3LED_Brightness = 0;				//flipbook 3 LED brightness
static uint8_t WaterLED_Brightness = 0;			//water LED brightness

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitLEDService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
bool InitLEDService ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
	
	//Make sure PWM is initialized in Main
	
  // put us into the Initial PseudoState
  CurrentState = InitLEDState;
	
	//initialize and turn on all PWM LEDs
			//set frequency and duty for F1
					PWM8_TIVA_SetFreq( PWM_Flip1LED_FREQ, PWM_Flip1LED_GROUP);
					PWM8_TIVA_SetDuty( PWM_Flip1LED_DUTY, PWM_Flip1LED_CHAN );
	
			//set frequency and duty for F2
					PWM8_TIVA_SetFreq( PWM_Flip2LED_FREQ, PWM_Flip2LED_GROUP);
					PWM8_TIVA_SetDuty( PWM_Flip2LED_DUTY, PWM_Flip2LED_CHAN );
	
			//set frequency and duty for F3
					PWM8_TIVA_SetFreq( PWM_Flip3LED_FREQ, PWM_Flip3LED_GROUP);
					PWM8_TIVA_SetDuty( PWM_Flip3LED_DUTY, PWM_Flip3LED_CHAN );
					
			//set frequency and duty for water LED
					PWM8_TIVA_SetFreq( PWM_WATER_LED_FREQ, PWM_WATER_LED_GROUP);
					PWM8_TIVA_SetDuty( PWM_WATER_LED_DUTY, PWM_WATER_LED_CHAN );
					
	//set seed GPIO LED to on

			//Initialize the port line to control the seed LED
					HWREG(SYSCTL_RCGCGPIO) |= PORT_D;   // enable port D  
			// wait for the port to be ready
					while( (HWREG(SYSCTL_PRGPIO) & PORT_D) != PORT_D );  
			//Write to the digital enable register to connect pins 6 to digital I/O ports
					HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= (SEED_LED_ON);
			//Set PA6 to be an output
					HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) |= (SEED_LED_ON);
			// Start with seed LED on
					HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) |= SEED_LED_ON;
	
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
     PostLEDService

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostLEDService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunLEDService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   Drew Bell, 11/16/16, 15:23
****************************************************************************/
ES_Event RunLEDService( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
	//Set NextState to CurrentState
	LEDState_t NextState = CurrentState;
	
	
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch ( CurrentState )
  {
    case InitLEDState :       // If current state is initial Psedudo State
			if ( ThisEvent.EventType == ES_INIT ) {
				//set all LEDS to be off OFF
				PWM8_TIVA_SetDuty( MIN_PWM_DUTY, PWM_Flip1LED_CHAN );
				PWM8_TIVA_SetDuty( MIN_PWM_DUTY, PWM_Flip2LED_CHAN );
				PWM8_TIVA_SetDuty( MIN_PWM_DUTY, PWM_Flip3LED_CHAN );
				PWM8_TIVA_SetDuty( MIN_PWM_DUTY, PWM_WATER_LED_CHAN);
				HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~SEED_LED_ON;
						
				//set Ramp LED values back to zero
				F1LED_Brightness = 0;				//flipbook 1 LED brightness
				F2LED_Brightness = 0;				//flipbook 2 LED brightness
				F3LED_Brightness = 0;				//flipbook 3 LED brightness
				WaterLED_Brightness = 0;			//water LED brightness
				
				//Blink all LEDs in Wait for seed / Welcome Mode
				BlinkSeedLEDS(true);
			
				// now put the machine into the actual initial state
				NextState = Waiting4Seed;
				#if DEBUG_LED
				printf("LS: InitLEDState Done.\n\r\n");
				#endif
      }
				
      break;

    case Waiting4Seed :       // This is the welcoming state
			//if ThisEvent is a timeout caused by BlinkSeedLEDS
			if ( (ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == BlinkSeedLEDS_TIMER) ) {
				//call blink seed again and pass true to keep blinking
				BlinkSeedLEDS(true);
				#if DEBUG_LED
				printf("LS: Blink Seed LEDS | Waiting for Seed.\n\r\n");
				#endif
			}			
			//if ES_SEED_DETECTED
			else if ( ThisEvent.EventType == ES_SEED_DETECTED ) {
				//stop seed LED blink 
				BlinkSeedLEDS(false);
				//start RampF1LEDS and transition to F1Run State
				RampF1LEDS();
				NextState = F1Run;
				#if DEBUG_LED
				printf("LS: Move to F1Run | Waiting4Seed.\n\r\n");
				#endif
			}
			//if ES_RESET
			else if ( ThisEvent.EventType == ES_RESET ) {
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Return to InitLEDState
				NextState = InitLEDState;				
			}
			break;
	  
	 case F1Run : 
			//if the timer ran out and event is from RampF1LEDS
			if ((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == RampF1LEDS_TIMER)) {
				//keep ramping
				RampF1LEDS();
				#if DEBUG_LED
				printf("LS: Ramping LEDs | F1 Run.\n\r\n");
				#endif
			}
			//if ES_F1_DONE
			else if ( ThisEvent.EventType == ES_F1_DONE ) {
				// make sure F1 LEDs are set to their full brightness
				F1SetFullBrightness();
				//start blinking of water leds
				BlinkWaterLEDS(true);
				//change to wait for water state
				NextState = Wait4Watering;
				#if DEBUG_LED
				printf("LS: ES_F1_DONE - Moving to Watering | F1Run.\n\r\n");
				#endif
			}	
			//else if ES_RESET
			else if ( ThisEvent.EventType == ES_RESET ) {
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Return to InitLEDState
				NextState = InitLEDState;
			}
			break;
			
	case Wait4Watering : 	//for now, blink the whole time for water
			//if event is blink water LEDS, keep blinking
			if ( (ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == BlinkWaterLEDS_TIMER) ) {
				//keep blinking water LEDs by calling and passing true
				BlinkWaterLEDS(true);
				#if DEBUG_LED
				printf("LS: Timeout - Blink Water LEDs | Waiting4Watering.\n\r\n");
				#endif
			}
			
			//if ES_WATER, go to F2 Run
			else if ( ThisEvent.EventType == ES_WATER ) {
				if ( ThisEvent.EventParam <= MIN_TILT_CHANGE ) {
					//stop blinking the water LED
					BlinkWaterLEDS(false);
					//Start ramping F2 LEDs
					RampF2LEDS();
					//move to F2Run state
					NextState = F2Run;
					#if DEBUG_LED
					printf("LS: ES_WATER - Move to F2Run | Waiting4Seed.\n\r\n");
					#endif
				}
			}
			
			else if ( ThisEvent.EventType == ES_RESET ) {
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Return to InitLEDState
				NextState = InitLEDState;
			}
			break;
	  
	case F2Run : 
			//if the timer ran out and event is from RampF2LEDS
			if ( (ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == RampF2LEDS_TIMER) ) {
				//keep ramping
				RampF2LEDS();
				#if DEBUG_LED
				printf("LS: Timeout - Ramp F2 LEDs | F2Run.\n\r\n");
				#endif
			}
			else if ( ThisEvent.EventType == ES_WATER ) {
				// if there is enough tilt
				if ( ThisEvent.EventParam <= MIN_TILT_CHANGE ) {
					// stop blinking the LEDS
					BlinkWaterLEDS(false);
					// calculate the water brightness based on the acc value
					uint16_t WaterPulse = ((MAX_SAFE_PWM_DUTY*(ThisEvent.EventParam - MIN_ACC))/(MAX_ACC-MIN_ACC));
					// start ramping water leds
					RampWaterLEDS( WaterPulse );
				} else { // else if it's not enough tilt
					// start blinking again
					BlinkWaterLEDS(true);
				}
			}
			// if it's a timeout for blinking the water LEDs, continue blinking
			else if ((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == BlinkWaterLEDS_TIMER)){
				//keep blinking water LEDs by calling and passing true
				BlinkWaterLEDS(true);
				#if DEBUG_LED
				printf("LS: Timeout - Blink Water LEDs | F2Run.\n\r\n");
				#endif
			}
			//if ES_F2_DONE move to F3Run
			else if(ThisEvent.EventType == ES_F2_DONE){
				// make sure F2 LEDs are set to their full brightness
				F2SetFullBrightness();
				// turn off the water LEDs
				BlinkWaterLEDS(false);
//				//Start ramping F3 LEDS
//				RampF3LEDS();
				//Change to F3 state
				NextState = F3Run;
				#if DEBUG_LED
				printf("LS: ES_F2_DONE - Moving to F3Run | F2Run.\n\r\n");
				#endif
			}	
			// else if it's a reset event
			else if(ThisEvent.EventType == ES_RESET){
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Return to InitLEDState
				NextState = InitLEDState;
			}
			break; 
			
	 case F3Run : 
			if ( ThisEvent.EventType == ES_DONE_HARVEST )  {
				//Start ramping F3 LEDS
				RampF3LEDS();
			}
			//if the timer ran out and event is from RampF3LEDS
			else if((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == RampF3LEDS_TIMER)){
				//keep ramping
				RampF3LEDS();
				#if DEBUG_LED
				printf("LS: Timeout - Ramp F3 LEDs | F3Run.\n\r\n");
				#endif
			}
			//if ES_F3_DONE, go to celebrate
			else if(ThisEvent.EventType == ES_F3_DONE){
				// make sure F3 LEDs are set to their full brightness
				F3SetFullBrightness();
				// set next state to celebration
				NextState = Celebration;
				//blink all LEDs to celebrate
				BlinkAllLEDS(true);
				#if DEBUG_LED
				printf("LS: ES_F3_DONE - Moving to Celebration | F3Run.\n\r\n");
				#endif
			}
			//if event is a reset, reset 
			else if(ThisEvent.EventType == ES_RESET){
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Return to InitLEDState
				NextState = InitLEDState;
			}
			break;
			
		//No case for harvest. Leave harvest LEDs in airservice
	
	case Celebration : 
			// if timeout to keep blinkinh,
			if((ThisEvent.EventType == ES_TIMEOUT) && ThisEvent.EventParam == BlinkAllLEDS_TIMER){
				//call blink all again and pass true to keep blinking
				BlinkAllLEDS(true);
				#if DEBUG_LED
				printf("LS: Timeout - Blinking all LEDs | Celebration.\n\r\n");
				#endif
			}		
			//if event is a reset, reset 
			else if(ThisEvent.EventType == ES_RESET){
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Return to InitLEDState
				NextState = InitLEDState;
			}
			break;

	  
    // repeat state pattern as required for other states
    default :
      ;
  }      // end switch on Current State
  
  CurrentState = NextState;
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryLEDService

 Parameters
     None

 Returns
     LEDState_t The current state of the LEDService state machine

 Description
     returns the current state of the LEDService state machine
 Notes

 Author
     Drew Bell, 11/16/16, 19:21
****************************************************************************/
LEDState_t QueryLEDService ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

/****************************************************************************
 Function
    RampF1LEDS

 Parameters
   none

 Returns
   none

 Description
  Ramps the duty cycle of Flipbook 1 LEDs from 0 to full brightness over time with non-blocking code

 Notes
 	(1) F1LED_Brightness is a module-level variable that is reset to 0 by the RESET state
	(2) The maximum suggested duty cycle is 85. With a 13.8V supply, this correlates to an effective voltage of 12V, the rating for the LED strips
	(3) Duty cycle controls brightness, so the terms are used interchangably in this module
	 
 Author
   Drew Bell, 11/20/16, 15:23
****************************************************************************/

static void RampF1LEDS( void ) {
// module level variable F1LED_Brightness is intialized to zero at compile, listed at top of module in defines
	static uint8_t Inc = 4;			//increments this many per step
	static uint8_t FullBrightness = MAX_SAFE_PWM_DUTY;	//sets the max brightness for the ramp

	//duty cycle controls brightness, so brightness and duty cycle are interchangable
	//will use brightness of clarity
	//if brightness for this LED + inc is less than or equal to the desired brightness, then add inc to LED brightness
	if( (F1LED_Brightness + Inc) < FullBrightness){
		F1LED_Brightness = F1LED_Brightness + Inc;
	}
	//else if LED brightness + inc is more than desired brightness, 
	else if( (F1LED_Brightness + Inc) >= FullBrightness){
		//set brightness = bmax
		F1LED_Brightness = MAX_SAFE_PWM_DUTY;
	}
	
	//set duty to Brightness
	PWM8_TIVA_SetDuty( F1LED_Brightness, PWM_Flip1LED_CHAN );
	
	//if LEDs need to have not reached the full brightness, set timer to wait before posting to LEDService to trigger another increment
	if( F1LED_Brightness < FullBrightness){
		ES_Timer_InitTimer(RampF1LEDS_TIMER, HALF_SEC);
	}

  	return;	
}

/****************************************************************************
 Function
    F1SetFullBrightness

 Parameters
   none

 Returns
   none

 Description
  Sets flipbook1 LEDs to their max safe brightness

 Author
   A. Siu, 11/27/16, 9:26
****************************************************************************/	
static void F1SetFullBrightness( void ) {
	//set duty to full Brightness
	PWM8_TIVA_SetDuty( MAX_SAFE_PWM_DUTY, PWM_Flip1LED_CHAN );
	return;
}
	
/****************************************************************************
 Function
    RampF2LEDS

 Parameters
   none

 Returns
   none

 Description
  Ramps the duty cycle of Flipbook 2 LEDs from 0 to full brightness over time with non-blocking code

 Notes
 	(1) F2LED_Brightness is a module-level variable that is reset to 0 by the RESET state
	(2) The maximum suggested duty cycle is 85. With a 13.8V supply, this correlates to an effective voltage of 12V, the rating for the LED strips
	(3) Duty cycle controls brightness, so the terms are used interchangably in this module
	 
 Author
   Drew Bell, 11/20/16, 15:23
****************************************************************************/			
static void RampF2LEDS( void ) {
// module level variable F2LED_Brightness is intialized to zero at compile, listed at top of module in defines
	static uint8_t Inc = 4;			//increments this many per step
	static uint8_t FullBrightness = MAX_SAFE_PWM_DUTY;	//sets the max brightness for the ramp

	//duty cycle controls brightness, so brightness and duty cycle are interchangable
	//will use brightness of clarity
	//if brightness for this LED + inc is less than or equal to the desired brightness, then add inc to LED brightness
	if( (F2LED_Brightness + Inc) < FullBrightness){
		F2LED_Brightness = F2LED_Brightness + Inc;
	}
	//else if LED brightness + inc is more than desired brightness, 
	else if( (F2LED_Brightness + Inc) >= FullBrightness){
		//set brightness = bmax
		F2LED_Brightness = MAX_SAFE_PWM_DUTY;
	}
	
	//set duty to Brightness
	PWM8_TIVA_SetDuty( F2LED_Brightness, PWM_Flip2LED_CHAN );
	
	//if LEDs need to have not reached the full brightness, set timer to wait before posting to LEDService to trigger another increment
		if( F2LED_Brightness < FullBrightness){
		ES_Timer_InitTimer(RampF2LEDS_TIMER, HALF_SEC);
	}

  	return;	
}

/****************************************************************************
 Function
    F2SetFullBrightness

 Parameters
   none

 Returns
   none

 Description
  Sets flipbook2 LEDs to their max safe brightness

 Author
   A. Siu, 11/27/16, 9:26
****************************************************************************/		
static void F2SetFullBrightness( void ) {
	//set duty to full Brightness
	PWM8_TIVA_SetDuty( MAX_SAFE_PWM_DUTY, PWM_Flip2LED_CHAN );
	return;
}
			
/****************************************************************************
 Function
    RampF3LEDS

 Parameters
   none

 Returns
   none

 Description
  Ramps the duty cycle of Flipbook 3 LEDs from 0 to full brightness over time with non-blocking code

 Notes
 	(1) F3LED_Brightness is a module-level variable that is reset to 0 by the RESET state
	(2) The maximum suggested duty cycle is 85. With a 13.8V supply, this correlates to an effective voltage of 12V, the rating for the LED strips
	(3) Duty cycle controls brightness, so the terms are used interchangably in this module
	 
 Author
   Drew Bell, 11/20/16, 15:23
****************************************************************************/			
static void RampF3LEDS( void ) {
// module level variable F3LED_Brightness is intialized to zero at compile, listed at top of module in defines
	static uint8_t Inc = 4;			//increments this many per step
	static uint8_t FullBrightness = MAX_SAFE_PWM_DUTY;	//sets the max brightness for the ramp

	//duty cycle controls brightness, so brightness and duty cycle are interchangable
	//will use brightness of clarity
	//if brightness for this LED + inc is less than or equal to the desired brightness, then add inc to LED brightness
	if( (F3LED_Brightness + Inc) < FullBrightness){
		F3LED_Brightness = F3LED_Brightness + Inc;
	}
	//else if LED brightness + inc is more than desired brightness, 
	else if( (F3LED_Brightness + Inc) >= FullBrightness){
		//set brightness = bmax
		F3LED_Brightness = MAX_SAFE_PWM_DUTY;
	}
	
	//set duty to Brightness
	PWM8_TIVA_SetDuty( F3LED_Brightness, PWM_Flip3LED_CHAN );
	
	//if LEDs need to have not reached the full brightness, set timer to wait before posting to LEDService to trigger another increment
		if( F3LED_Brightness < FullBrightness){
		ES_Timer_InitTimer(RampF3LEDS_TIMER, HALF_SEC);
	}

  	return;	
}

/****************************************************************************
 Function
    F3SetFullBrightness

 Parameters
   none

 Returns
   none

 Description
  Sets flipbook3 LEDs to their max safe brightness

 Author
   A. Siu, 11/27/16, 9:26
****************************************************************************/			
static void F3SetFullBrightness( void ) {
	//set duty to full Brightness
	PWM8_TIVA_SetDuty( MAX_SAFE_PWM_DUTY, PWM_Flip3LED_CHAN );
	return;
}
		
/****************************************************************************
 Function
    RampWaterLEDS

 Parameters
   none

 Returns
   none

 Description
  Ramps the duty cycle of Water LEDs from 0 to full brightness over time with non-blocking code

 Notes
 	(1) WaterLED_Brightness is a module-level variable that is reset to 0 by the RESET state
	(2) The maximum suggested duty cycle is 85. With a 13.8V supply, this correlates to an effective voltage of 12V, the rating for the LED strips
	(3) Duty cycle controls brightness, so the terms are used interchangably in this module
	 
 Author
   Drew Bell, 11/20/16, 15:23
****************************************************************************/			
static void RampWaterLEDS( uint8_t Inc ) {
	
	static uint8_t FullBrightness = MAX_SAFE_PWM_DUTY;	//sets the max brightness for the ramp

	//duty cycle controls brightness, so brightness and duty cycle are interchangable
	//will use brightness of clarity
	//if brightness for this LED + inc is less than or equal to the desired brightness, then add inc to LED brightness
	if( (WaterLED_Brightness + Inc) < FullBrightness){
		WaterLED_Brightness = WaterLED_Brightness + Inc;
	}
	//else if LED brightness + inc is more than desired brightness, 
	else if( (WaterLED_Brightness + Inc) >= FullBrightness){
		//set brightness = bmax
		WaterLED_Brightness = MAX_SAFE_PWM_DUTY;
	}
	
	//set duty to Brightness
	PWM8_TIVA_SetDuty( WaterLED_Brightness, PWM_WATER_LED_CHAN );

  	return;	
	
	/*
	// module level variable WaterLED_Brightness is intialized to zero at compile, listed at top of module in defines
	static uint8_t Inc = 4;			//increments this many per step
	static uint8_t FullBrightness = MAX_SAFE_PWM_DUTY;	//sets the max brightness for the ramp

	//duty cycle controls brightness, so brightness and duty cycle are interchangable
	//will use brightness of clarity
	//if brightness for this LED + inc is less than or equal to the desired brightness, then add inc to LED brightness
	if( (WaterLED_Brightness + Inc) < FullBrightness){
		WaterLED_Brightness = WaterLED_Brightness + Inc;
	}
	//else if LED brightness + inc is more than desired brightness, 
	else if( (WaterLED_Brightness + Inc) >= FullBrightness){
		//set brightness = bmax
		WaterLED_Brightness = MAX_SAFE_PWM_DUTY;
	}
	
	//set duty to Brightness
	PWM8_TIVA_SetDuty( WaterLED_Brightness, PWM_WATER_LED_CHAN );
	
	//if LEDs need to have not reached the full brightness, set timer to wait before posting to LEDService to trigger another increment
		if( WaterLED_Brightness < FullBrightness){
		ES_Timer_InitTimer(RampWaterLEDS_TIMER, HALF_SEC);
	}

  	return;	
		*/
		
}
	
/****************************************************************************
 Function
    BlinkAllLEDS

 Parameters
   bool indicating start or stop blink

 Returns
   none

 Description
	This function makes all LEDs blink until a stop signal is given

 Notes
 	(1) passing a boolean true to the function starts blinking, which toggles between an upper duty and lower duty
	(2) the time given to the timer specifies half period of each blink cycle
	(3) passing a boolean false stops the blinking and sets LEDs in the off state
	(4) IMPORTANT - A STOP COMMAND MUST BE GIVEN TO STOP THE BLINKING, I.E. CHANGING FROM A BLINKING STATE TO A STEADY LED STATE.
									OTHERWISE, BLINKING WILL CONTINUE. 
									This design was chosen to increase flexibility. It can be changed to a state-based system or layered with a state checker, if desired.
 Author
   Drew Bell, 11/20/16, 15:23
****************************************************************************/			
static void BlinkAllLEDS (bool command) {
	
	static bool BlinkState = 0;	//declare a static boolean variable BlinkState to determine on/off, start with false
		
	//if command is true, continue with blink
	if (command == true){
		//if BlinkState is false
		if(BlinkState == 0){
			//set PWM duty to high duty 
			PWM8_TIVA_SetDuty( MAX_SAFE_PWM_DUTY, PWM_Flip1LED_CHAN );
			PWM8_TIVA_SetDuty( MAX_SAFE_PWM_DUTY, PWM_Flip2LED_CHAN );
			PWM8_TIVA_SetDuty( MAX_SAFE_PWM_DUTY, PWM_Flip3LED_CHAN );
			PWM8_TIVA_SetDuty( MAX_SAFE_PWM_DUTY, PWM_WATER_LED_CHAN);
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) |= SEED_LED_ON;
			//set BlinkStatePWM to 1 (true)-> LEDS have been turned on
			BlinkState = 1;
		}
		//else if BlinkStatePWM is equal to true
		else if (BlinkState == 1){
			//set PWM duty to low duty
			PWM8_TIVA_SetDuty( MIN_PWM_DUTY, PWM_Flip1LED_CHAN );
			PWM8_TIVA_SetDuty( MIN_PWM_DUTY, PWM_Flip2LED_CHAN );
			PWM8_TIVA_SetDuty( MIN_PWM_DUTY, PWM_Flip3LED_CHAN );
			PWM8_TIVA_SetDuty( MIN_PWM_DUTY, PWM_WATER_LED_CHAN);
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~SEED_LED_ON;
			//set BlinkStatePWM to 0 (false)-> LEDS have been turned off
			BlinkState = 0;
		//end if
		}
		//set timer to wait a half second before posting to LEDService
		ES_Timer_InitTimer(BlinkAllLEDS_TIMER, HALF_SEC);
	}
	//else if command is false meaning the blinking should shop
	else if(command == false){
		//set LEDs low and return
		PWM8_TIVA_SetDuty( 0, PWM_Flip1LED_CHAN );
		PWM8_TIVA_SetDuty( 0, PWM_Flip2LED_CHAN );
		PWM8_TIVA_SetDuty( 0, PWM_Flip3LED_CHAN );
		PWM8_TIVA_SetDuty( 0, PWM_WATER_LED_CHAN);
		HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~SEED_LED_ON;
	}
					
	return;
}

/****************************************************************************
 Function
    BlinkWaterLEDS

 Parameters
   bool indicating start or stop blink

 Returns
   none

 Description
	This function makes the water LEDs blink until a stop signal is given

 Notes
 	(1) passing a boolean true to the function starts blinking, which toggles between an upper duty and lower duty
	(2) the time given to the timer specifies half period of each blink cycle
	(3) passing a boolean false stops the blinking and sets LEDs in the off state
	(4) IMPORTANT - A STOP COMMAND MUST BE GIVEN TO STOP THE BLINKING, I.E. CHANGING FROM A BLINKING STATE TO A STEADY LED STATE.
									OTHERWISE, BLINKING WILL CONTINUE. 
									This design was chosen to increase flexibility. It can be changed to a state-based system or layered with a state checker, if desired.
 Author
   Drew Bell, 11/20/16, 16:23
****************************************************************************/		

static void BlinkWaterLEDS (bool command) {

	static bool BlinkState = 0;	//declare a static boolean variable BlinkState to determine on/off, start with false
		
	//if command is true, continue with blink
	if (command == true){
		//if BlinkState is false
		if(BlinkState == 0){
			//set PWM duty to high duty 
			PWM8_TIVA_SetDuty( MAX_SAFE_PWM_DUTY, PWM_WATER_LED_CHAN);
			//set BlinkStatePWM to 1 (true)-> LEDS have been turned on
			BlinkState = 1;
		}
		//else if BlinkStatePWM is equal to true
		else if (BlinkState == 1){
			//set PWM duty to low duty
			PWM8_TIVA_SetDuty( MIN_PWM_DUTY, PWM_WATER_LED_CHAN);
			//set BlinkStatePWM to 0 (false)-> LEDS have been turned off
			BlinkState = 0;
		//end if
		}
		//set timer to wait one second before posting to LEDService
		ES_Timer_InitTimer(BlinkWaterLEDS_TIMER, BLINK_WATER_TIME);
	}
	//else if command is false meaning the blinking should shop
	else if(command == false){
		//set LEDs low and return
		PWM8_TIVA_SetDuty( 0, PWM_WATER_LED_CHAN);
	}
					
	return;
}

/****************************************************************************
 Function
    BlinkSeedLEDS

 Parameters
   bool indicating start or stop blink

 Returns
   none

 Description
	This function makes the seed LEDs blink until a stop signal is given

 Notes
 	(1) passing a boolean true to the function starts blinking, which toggles between an upper duty and lower duty
	(2) the time given to the timer specifies half period of each blink cycle
	(3) passing a boolean false stops the blinking and sets LEDs in the off state
	(4) IMPORTANT - A STOP COMMAND MUST BE GIVEN TO STOP THE BLINKING, I.E. CHANGING FROM A BLINKING STATE TO A STEADY LED STATE.
									OTHERWISE, BLINKING WILL CONTINUE. 
									This design was chosen to increase flexibility. It can be changed to a state-based system or layered with a state checker, if desired.
 Author
   Drew Bell, 11/20/16, 16:23
****************************************************************************/	
static void BlinkSeedLEDS(bool command) {

	static bool BlinkState = 0;	//declare a static boolean variable BlinkState to determine on/off, start with false
		
	//if command is true, continue with blink
	if (command == true){
		//if BlinkState is false
		if(BlinkState == 0){
			//set pin on
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) |= SEED_LED_ON;
			//set BlinkStatePWM to 1 (true) -> LEDS have been turned on
			BlinkState = 1;
		}
		//else if BlinkStatePWM is equal to true
		else {
			//set pin low
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~SEED_LED_ON;
			//set BlinkStatePWM to 0 (false)-> LEDS have been turned off
			BlinkState = 0;
		//end if
		}
		//set timer to wait a half second before posting to LEDService
		ES_Timer_InitTimer(BlinkSeedLEDS_TIMER, BLINK_SEED_TIME);
	}
	//else if command is false meaning the blinking should shop
	else if(command == false){
		//set LEDs low and return
		HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~SEED_LED_ON;
	}
					
	return;
}


/***************************************************************************
	Code Parts



//Turn off all LEDs TODO
PWM8_TIVA_SetDuty( 0, PWM_Flip1LED_CHAN );

PWM8_TIVA_SetFreq( PWM_Flip1LED_FREQ, PWM_Flip1LED_GROUP);

PWM8_TIVA_SetDuty( PWM_Flip1LED_DUTY, PWM_Flip1LED_CHAN );

#define Flip1LED GPIO_PIN_0		//PD0
#define PWM_Flip1LED_CHAN  6     // which is PDO
#define PWM_Flip1LED_GROUP 3			//grouped with 6 and 7
#define PWM_Flip1LED_FREQ  1000		//50Hz PWM frequency
#define PWM_Flip1LED_DUTY  50		//start at 80% duty for testing

	HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= ~SEED_LED_ON;
	
	#if DEBUG_LED
				printf("Init done.\n\r\n");
				#endif
	
	//void SetPWM_PinDuty ( channel, duty_cycle )

//bool SetPWM_PinDuty( uint8_t channel, uint8_t duty_cycle) {
	//how arge does Pin need to be to hold the address of the 

	//pass 
	
	//returns true if channel and duty cycle are valid
	//returns false if channel or duty are not valid
}

//bool SetPWM_GroupFreq ( uint16_t freq, uint8_t group )
		
		//note: 0&1, 2&3, 4&5, and 6&7 are all in the same groups, so changes to one value in a group will change the other
		//take two unsigned ints and return a boolean. Return true if the values are within the proper range, return false if the values are output of what they should be
		//max value for freq is 65535 (2^16-1)		
		//if freq is greater than zero and less then 65535 AND group is >= = and <=&
					//PWM8_TIVA_SetFreq( freq, group);
					//return true
		//else 
					//return false
		//end if

//end 



//These may not be very useful internally, prob use the HWREG funciton

//bool SetGPIO_Pin_HI ( uint32_t Port, uint32_t Pin., uint8_t value)	
		//set GPIO_Pin to on or off
		//anything that is nonzero will set the port high

//bool SetGPIO_Pin_LO ( uint32_t Port, uint32_t Pin., uint8_t value)

 ***************************************************************************/
