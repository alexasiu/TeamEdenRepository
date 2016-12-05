/****************************************************************************
 Module
   WaterBucketService.c

 Revision
   1.0.1

 Description
   Controls water bucket vibration motor

 Notes

 History
 When              Who           What/Why
 --------------    ---           --------
 11/14/16 4:30     chaim         started coding
 11/15/16          chaim & afs   integrating to framework
 11/15/16 18:06    chaim & afs   analog input with pots working
 11/26/16 16:46 afs     added reset functionality
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
#define MIN_TILT_CHANGE 2500 //2600
// define acceleration raw values that go roughly from 1800 (90 degrees) to 2600 (0 degrees)

#define PORT_C     BIT2HI
#define PORT_E     BIT4HI
#define Z_PIN      GPIO_PIN_0
#define VIB_PIN    GPIO_PIN_4
#define VIB_HI     BIT4HI
#define VIB_LO     BIT4LO

//WaterBucketService
//Listens to the water bucket accelerometer and controls the vibration motor. 
bool InitWaterService ( uint8_t Priority );
bool PostWaterBucketService ( ES_Event ThisEvent );
ES_Event RunWaterService ( ES_Event ThisEvent );

bool Check4Water ( void );
static bool Listen;
static uint16_t AccToTilt ( void );

static uint8_t MyPriority;
static WaterBucketState_t CurrentState;

//InitWaterService
//Takes a priority number, returns True. 
bool InitWaterService ( uint8_t Priority ) {
	ES_Event ThisEvent;
	//	Initialize the MyPriority variable with the passed in parameter.
	MyPriority = Priority;
	
	//	Initialize the port E line as output to run the vibration motor
  HWREG(SYSCTL_RCGCGPIO) |= PORT_C;   // enable port C 
	// wait for the port to be ready
	while( (HWREG(SYSCTL_PRGPIO) & PORT_C) != PORT_C );  
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= VIB_PIN;
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) |= VIB_PIN;
	
	//	Initialize the port E line for the accelerometer inputs
  HWREG(SYSCTL_RCGCGPIO) |= PORT_E;   // enable port E 
	// wait for the port to be ready
	while( (HWREG(SYSCTL_PRGPIO) & PORT_E) != PORT_E );  
	//We set pin PE0 to output the z position read by the accelerometer
	//We set pin PE0 to be the output that feeds into the flipbook2 motor.
	HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= (Z_PIN);
	//Initialize the analog pin that will be read from
	ADC_MultiInit(1);
	//	Initialize the port line to read the accelerometer input
	HWREG(GPIO_PORTE_BASE+GPIO_O_DIR) &= ~Z_PIN;
	
	//Set CurrentState to be InitWaterBucketService
	CurrentState = InitWaterBucketService;
	
	//Post Event ES_Init to InitWaterBucketService queue (this service)
	ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {  
    return true;
  }else
  {
      return false;
  }
}//End of InitWaterService (return True)

//RunWaterService (implements the state machine for Water Bucket Service)
//The EventType field of ThisEvent will be one of: ES_INIT, ES_WATER, ES_NO_WATER, ES_CELEBRATION
//Local Variables: NextState
ES_Event RunWaterService(ES_Event ThisEvent) {
//Set NextState to CurrentState
	WaterBucketState_t NextState = CurrentState;
//Based on the state of the CurrentState variable choose one of the following blocks of code:
	switch (CurrentState) {
//	CurrentState is InitWaterBucketService
		case InitWaterBucketService :
			//		if ThisEvent is ES_INIT
			if (ThisEvent.EventType == ES_INIT) {
				//Specify that we are not checking for water
				Listen = false;
				//Set NextState Wait4Flip1Done
				NextState = Wait4Flip1Done;
				#if DEBUG_WATER
				printf("WB: Init water bucket\n\r\n");
				#endif
			}//		Endif
		break;//End InitWaterBucketService block

		//CurrentState is Wait4Flip1Done
		case Wait4Flip1Done :
			//if ThisEvent is ES_F1_DONE
			if (ThisEvent.EventType == ES_F1_DONE) {
				Listen = true;
				//Set NextState Wait4Water
				NextState = Wait4Water;
			} //	End Wait4Flip1Done block
			if (ThisEvent.EventType == ES_RESET) {
				//Turn off motor
				HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA+ALL_BITS)) &= VIB_LO;	
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Set NextState InitWaterBucketService
				NextState = InitWaterBucketService;
			}
		break;//End Wait4Flip1Done block

		//	CurrentState is Wait4Water
		case Wait4Water :
			//if ThisEvent is ES_WATER
			if (ThisEvent.EventType == ES_WATER) {
				// if it's enough tilt for the vibration motor
				if ( ( ThisEvent.EventParam <= MIN_TILT_CHANGE ) ){
					// Turn on the vibration motor
					HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA+ALL_BITS)) |= VIB_HI;
					#if DEBUG_WATER
						printf("WB: Water!\n\r\n");
					#endif
				// else there isn't enough tilt	
				} else {
					// Turn off the vibration motor
					HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA+ALL_BITS)) &= VIB_LO;
					#if DEBUG_WATER
						printf("WB: No Water!\n\r\n");
					#endif
				}
			}
			
			// if ThisEvent is ES_F2_DONE
			else if (ThisEvent.EventType == ES_F2_DONE) {
				// stop checking for water
				Listen = false;
				//Turn off the vibration motor
				HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA+ALL_BITS)) &= VIB_LO;				
				//SetNextState DoneWatering
				NextState = DoneWatering;
			}
			//	if ThisEvent is ES_RESET
			else if (ThisEvent.EventType == ES_RESET) {
				//Turn off motor
				HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA+ALL_BITS)) &= VIB_LO;	
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Set NextState InitWaterBucketService
				NextState = InitWaterBucketService;
			}
		break; 	//End Wait4Water block

		//	CurrentState is DoneWatering
		case DoneWatering :
			#if DEBUG_WATER
				printf("WB: Done watering\n\r\n");
			#endif
			//if ThisEvent is ES_RESET
			if ( ThisEvent.EventType == ES_RESET) {
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Set NextState InitWaterBucketService
				NextState = InitWaterBucketService;
			}
		break; //	End DoneWatering
	}
	//	Set CurrentState to NextState
	CurrentState = NextState;
	//	Return ES_NO_EVENT
	ThisEvent.EventType = ES_NO_EVENT;
	return ThisEvent;
}//End of RunWaterBucketService

//Check4Water
//Takes no parameters, returns True if an event posted
//	Local ReturnVal = False, CurrentAccState
bool Check4Water ( void ) {
	ES_Event ThisEvent;
	bool ReturnVal = false;
	//	Set CurrentAccState to state read from port pin
	uint32_t CurrentAccState = AccToTilt();
	#if DEBUG_ACC
		printf("Acc val: %u\n\r\n", CurrentAccState);
	#endif
	if ( Listen ) { //if we are checking for water
		
		ThisEvent.EventParam = CurrentAccState;
		//PostEvent ES_WATER to water list
		ThisEvent.EventType = ES_WATER;
		ES_PostList04( ThisEvent );
	}
	//	Return ReturnVal
	return ReturnVal;
}//End of Check4Water

//private TiltToDutyCycle
//Returns the current tilt (Tilt), represented as an angle tan(z/x).
static uint16_t AccToTilt ( void ) {
	static double prevZVal = 2600;
	uint16_t ReturnVal;
	uint32_t ZArray[1];
	double currZVal;
	//Read the values for z from PE0;
	ADC_MultiRead(ZArray);
	currZVal = ZArray[0];
	// filter signal
	ReturnVal = (currZVal * 0.5) + (prevZVal * 0.5);
	// save prev value
	prevZVal = currZVal;
	// return the filtered value
	return ReturnVal;
}//EndTiltToPWM

//PostWaterBucketService
bool PostWaterBucketService ( ES_Event ThisEvent ) {
//Post Event to ES_SERVICES
	return ES_PostToService( MyPriority, ThisEvent);
}


/****************************************************************************
 Function
     QueryWaterService

 Parameters
     None

 Returns
     WaterBucketState_t The current state of the WaterBucketService state machine

 Description
     returns the current state of the WaterBucketService state machine
 Notes

 Author
     A. Siu, 11/26/16, 19:21
****************************************************************************/
WaterBucketState_t QueryWaterService ( void )
{
   return ( CurrentState );
}
