/****************************************************************************
 Module
   AirService.c

 Revision
   1.0.1

 Description
   Listens to the IR receivers, interprets hand movement speed and posts 
	 to flipbook3 service. Also controls the LEDs that tell the user which 
	 IR sensor they should be triggering with their hand.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/13/16 10:32 afs     started coding
 11/26/16 15:52 afs     added reset functionality
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TemplateService.h"

// include the PWM library
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
#include "AirService.h"
#include "Flipbook3Service.h"
#include "MainStoryService.h"
#include "LEDService.h"

/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff<<2)
#define PORT_A    BIT0HI      // for IR
#define PORT_C    BIT2HI      // for LEDs

#define IR1_PIN   GPIO_PIN_4  //PA4
#define IR1_HI    BIT4HI
#define IR1_LO    BIT4LO

#define IR2_PIN   GPIO_PIN_5  //PA5
#define IR2_HI    BIT5HI
#define IR2_LO    BIT5LO

#define LED1_PIN  GPIO_PIN_6  //PC6
#define LED1_HI   BIT6HI
#define LED1_LO   BIT6LO

#define LED2_PIN  GPIO_PIN_7  //PC7
#define LED2_HI   BIT7HI
#define LED2_LO   BIT7LO

#define THRESHOLD 10

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static uint8_t UpdateIR1State ( void );
static uint8_t UpdateIR2State ( void );

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static AirState_t CurrentState;
static uint8_t LastIR1State;
static uint8_t LastIR2State;
static ES_EventTyp_t PrevEvent;
static uint8_t IR_Count;

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
bool InitAirService ( uint8_t Priority )
{
  ES_Event ThisEvent;
	//Initialize the MyPriority variable with the passed in parameter.
  MyPriority = Priority;
	
	//Initialize the port line to sense the IR
  HWREG(SYSCTL_RCGCGPIO) |= PORT_A;   // enable port A  
	// wait for the port to be ready
	while( (HWREG(SYSCTL_PRGPIO) & PORT_A) != PORT_A );  
	
	// enable IR1 pin and set as input
	HWREG(GPIO_PORTA_BASE + GPIO_O_DEN ) |= ( IR1_HI );
	HWREG( GPIO_PORTA_BASE + GPIO_O_DIR ) &= ( IR1_LO );
	LastIR1State = UpdateIR1State();
	
	// enable IR2 pin and set as input
	HWREG(GPIO_PORTA_BASE + GPIO_O_DEN ) |= ( IR2_HI );
	HWREG( GPIO_PORTA_BASE + GPIO_O_DIR ) &= ( IR2_LO );
	LastIR2State = UpdateIR2State();
	
	//Initialize the port line to control the Air LEDs 
  HWREG(SYSCTL_RCGCGPIO) |= PORT_C;   // enable portA    
	// wait for the port to be ready
	while( (HWREG(SYSCTL_PRGPIO) & PORT_C) != PORT_C );  
	
	// enable LED1 pin and set as output
	HWREG(GPIO_PORTC_BASE + GPIO_O_DEN ) |= ( LED1_HI );
	HWREG( GPIO_PORTC_BASE + GPIO_O_DIR ) |= ( LED1_HI );
	// set LED1 LO
	HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED1_LO;

	// enable LED2 pin and set as output
	HWREG(GPIO_PORTC_BASE + GPIO_O_DEN ) |= ( LED2_HI );
	HWREG( GPIO_PORTC_BASE + GPIO_O_DIR ) |= ( LED2_HI );
	// set LED2 LO
	HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED2_LO;
	
	//Set CurrentState to be InitAir
	CurrentState = InitAir;
	
	//Post Event ES_Init to AirService queue (this service)
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
     PostAirService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue

 Author
****************************************************************************/
bool PostAirService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunAirService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description

 Author
	A. Siu
****************************************************************************/
ES_Event RunAirService( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  AirState_t NextState = CurrentState;
  switch ( CurrentState )
  {
		case InitAir:
			if ( ThisEvent.EventType == ES_INIT ) {
				// set all LEDs lo
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED1_LO;
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED2_LO;
				NextState = Wait4HarvestingIR;
				#if DEBUG_AIR
				printf("AS: Init starting.\n\r\n");
				#endif
			}
			break;
		
		//CurrentState is Wait4Harvesting
		case Wait4HarvestingIR:
			//if ThisEvent is ES_START_HARVEST
			if ( ThisEvent.EventType == ES_START_HARVEST ) {
				#if DEBUG_AIR
				printf("AS: Start harvesting\n\r\n");
				#endif
				//Turn on LED corresponding to IR1
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) |= LED1_HI;
				// save the PrevEvent as IR2
				PrevEvent = ES_IR2_HI;
				IR_Count = 0;
				//Set NextState to Harvesting
				NextState = Harvesting_IR1;
			} 
			if ( ThisEvent.EventType == ES_RESET ) {
				// turn off all the LEDs
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED1_LO;
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED2_LO;
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Set NextState to InitAirService
				NextState = InitAir;
			}
		break;
		
	//CurrentState is Harvesting_IR1
		case Harvesting_IR1:
			//if ThisEvent is IR1_HI, and this is a new event
			if ( (ThisEvent.EventType == ES_IR1_HI) && (PrevEvent != ThisEvent.EventType) ) {
				//Increment IR_Count
				IR_Count++;
				#if DEBUG_AIR
				printf("AS: Count so far %u\n\r\n", IR_Count);
				#endif
				//Save the event type to PrevEvent
				PrevEvent = ThisEvent.EventType;
				// check if this is the end of harvesting
				if ( IR_Count >= THRESHOLD ) {
					// turn off all the LEDs
					HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED1_LO;
					HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED2_LO;
					#if DEBUG_AIR
					printf("AS: posting ES_DONE_HARVEST to FS3\n\r\n");
					#endif
					//Post ES_DONE_HARVEST event to Flipbook3Service and LED Service
					ES_Event Event2Post;
					Event2Post.EventType = ES_DONE_HARVEST;
					PostFlip3Service( Event2Post );
					PostLEDService( Event2Post );
					//Set NextState to Wait4Celebration
					NextState = Wait4CelebrationIR;
				// else continue to toggle the LEDs
				} else {
					//Turn off the LED corresponding to IR1
					HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED1_LO;
					//Turn on the LED corresponding to IR2
					HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) |= LED2_HI;
					//Set NextState to Harvesting_IR2
					NextState = Harvesting_IR2;
				}
			} else if ( ThisEvent.EventType == ES_RESET ) {
				// turn off all the LEDs
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED1_LO;
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED2_LO;
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Set NextState to InitAirService
				NextState = InitAir;
			}
			break;
			
		case Harvesting_IR2:
			//if ThisEvent is IR2_HI, and this is a new event
			if ( (ThisEvent.EventType == ES_IR2_HI) && (PrevEvent != ThisEvent.EventType) ) {
				//Increment IR_Count
				IR_Count++;
				#if DEBUG_AIR
				printf("AS: Count so far %u\n\r\n", IR_Count);
				#endif
				//Save the event type to PrevEvent
				PrevEvent = ThisEvent.EventType;
				// check if this is the end of harvesting
				if ( IR_Count >= THRESHOLD ) {
					// turn off all the LEDs
					HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED1_LO;
					HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED2_LO;
					#if DEBUG_AIR
					printf("AS: harvesting done\n\r\n");
					printf("AS: posting ES_DONE_HARVEST to FS3\n\r\n");
					#endif
					//Post ES_DONE_HARVEST event to Flipbook3Service and LED Service
					ES_Event Event2Post;
					Event2Post.EventType = ES_DONE_HARVEST;
					PostFlip3Service( Event2Post );
					PostLEDService( Event2Post );
					//Set NextState to Wait4Celebration
					NextState = Wait4CelebrationIR;
				// else continue to toggle the LEDs
				} else {
					//Turn off the LED corresponding to IR2
					HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED2_LO;
					//Turn on the LED corresponding to IR1
					HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) |= LED1_HI;
					//Set NextState to Harvesting_IR1
					NextState = Harvesting_IR1;
				}
			} else if ( ThisEvent.EventType == ES_RESET ) {
				// turn off all the LEDs
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED1_LO;
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED2_LO;
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Set NextState to InitAirService
				NextState = InitAir;
			}
			break;
		
		case Wait4CelebrationIR:
			if ( ThisEvent.EventType == ES_RESET ) {
				// turn off all the LEDs
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED1_LO;
				HWREG( GPIO_PORTC_BASE + ( GPIO_O_DATA + ALL_BITS )) &= LED2_LO;
				// post an ES_DONE_INIT to the MainService
				ES_Event Event2Post;
				Event2Post.EventType = ES_DONE_INIT;
				PostMainService( Event2Post );
				//Set NextState to InitAirService
				NextState = InitAir;
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
     QueryAirService

 Parameters
     None

 Returns
     AirState_t The current state of the AirService state machine

 Description
     returns the current state of the AirService state machine
 Notes

 Author
     A. Siu, 11/26/16, 19:21
****************************************************************************/
AirState_t QueryAirService ( void )
{
   return ( CurrentState );
}

/****************************************************************************
 Function
    Check4IR_1

 Parameters
	 None

 Returns
	 Returns true if hand was detected. Posts an ES_IR1_HI event.

 Author
	A. Siu
****************************************************************************/
bool Check4IR_1 ( void ) {
	
	bool ReturnVal = false;
	//Set CurrentIR_1State to state read from port pin
	uint8_t CurrentIR1State = UpdateIR1State();
	
	//If the CurrentIR_1State is different from LastIR_1State
	if ( CurrentIR1State != LastIR1State ) {
		ES_Event ThisEvent;
		ReturnVal = true;
		//If CurrentIR_1State is Hi (hand is there)
		if ( CurrentIR1State == IR1_HI ) {
			#if DEBUG_IR
				printf("IR1 change detected\n\r\n");
			#endif
			//Post ES_IR1_HI to AirService
			ThisEvent.EventType = ES_IR1_HI;
			PostAirService( ThisEvent );
		}
	}
	//Set LastAccState to the CurrentAccState
	LastIR1State = CurrentIR1State;

	return ReturnVal;
}

/****************************************************************************
 Function
    Check4IR_2

 Parameters
		None

 Returns
		Returns true if hand was detected. Posts an ES_IR2_HI event.

 Author
****************************************************************************/
bool Check4IR_2 ( void ) {
	
	bool ReturnVal = false;
	//Set CurrentIR_1State to state read from port pin
	uint8_t CurrentIR2State = UpdateIR2State();
	
	//If the CurrentIR_2State is different from LastIR_2State
	if ( CurrentIR2State != LastIR2State ) {
		ES_Event ThisEvent;
		ReturnVal = true;
		//If CurrentIR_1State is Hi (hand is there)
		if ( CurrentIR2State == IR2_HI ) {
			#if DEBUG_IR
				printf("IR2 change detected\n\r\n");
			#endif
			//Post ES_IR1_HI to AirService
			ThisEvent.EventType = ES_IR2_HI;
			PostAirService( ThisEvent );
		}
	}
	//Set LastAccState to the CurrentAccState
	LastIR2State = CurrentIR2State;

	return ReturnVal;
}

/***************************************************************************
 private functions
 ***************************************************************************/
/****************************************************************************
 Function
     UpdateIR1State

 Parameters
     None

 Returns
	Return the current state of the IR1 pin

 Author
     A. Siu, 11/03/16
****************************************************************************/
static uint8_t UpdateIR1State ( void )
{
	return ( HWREG( GPIO_PORTA_BASE + ( GPIO_O_DATA + ALL_BITS )) &= IR1_PIN );
}


/****************************************************************************
 Function
     UpdateIR2State

 Parameters
     None

 Returns
	Return the current state of the IR2 pin

 Author
     A. Siu, 11/03/16
****************************************************************************/
static uint8_t UpdateIR2State ( void )
{
	return ( HWREG( GPIO_PORTA_BASE + ( GPIO_O_DATA + ALL_BITS )) &= IR2_PIN );
}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

