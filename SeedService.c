/****************************************************************************
 Module
   SeedService.c

 Description
   Does debouncing on the seed switch. Posts to Flipbook1 service.
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

#include "BITDEFS.H"
#include "SeedService.h"
#include "Flipbook1Service.h"
#include "MainStoryService.h"

#define ALL_BITS (0xff<<2)

#define PORT_B    BIT1HI      // for the motor
#define SEED_PIN   BIT0HI //The input pin from seed switch is PB0
#define SEED_SWITCH_TIME  100 //Time set for the seed switch timer

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t LastSeedSwitchState;
static SeedSwitchState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitSeedService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, initializes the port pin, and 
 Notes

 Author
     Harine Ravichandiran
****************************************************************************/
bool InitSeedService ( uint8_t Priority )
{
	//	Initialize the MyPriority variable with the passed in parameter.
	MyPriority = Priority;

	//Initialize the port line to monitor the button
	HWREG(SYSCTL_RCGCGPIO) |= PORT_B; //enable port B
	//wait for peripheral clock
	while ((HWREG(SYSCTL_PRGPIO) & PORT_B) != PORT_B);
	
	//set seed pin to input
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (SEED_PIN);
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) &= !(SEED_PIN);
	
	//Sample the button port pin and use it to initialize LastButtonState
	LastSeedSwitchState = (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) & SEED_PIN);
	
	//Set CurrentState to be Debouncing
	CurrentState = Debouncing;
	
	//Start debounce timer (timer posts to ButtonDebounceSM)
	ES_Timer_InitTimer( SEED_TIMER, SEED_SWITCH_TIME );
  ES_Event ThisEvent;
  
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
		return true;
  } else {
    return false;
  }
}

/****************************************************************************
 Function
     CheckSeedSwitchEvents

 Parameters
     Takes no parameters

 Returns
     bool, returns True if an event posted, false otherwise

 Description
     
 Notes

 Author
     Harine Ravichandiran
****************************************************************************/
bool CheckSeedSwitchEvents (void)
{
	//Local ReturnVal = False, CurrentSwitchState
	bool ReturnVal = false;
	uint8_t CurrentSwitchState;
    
	//Set CurrentSwitchState to state read from port pin
	CurrentSwitchState = (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) & SEED_PIN);
    
	//If the CurrentSwitchState is different from the LastSeedSwitchState
	if(CurrentSwitchState != LastSeedSwitchState){
		//Set ReturnVal = True
		ReturnVal = true;
        
		//	If the CurrentSeedSwitchState is down, (the value of the seed pin will be high)
		if( (CurrentSwitchState & SEED_PIN) == SEED_PIN ){
			//PostEvent SeedSwitchDown to SeedSwitch queue
			ES_Event SeedSwitchEvent;
			SeedSwitchEvent.EventType = ES_SeedSwitchDown;
			PostSeedService(SeedSwitchEvent);
		}
        //If the pin is not down (the switch is open, pin to ground)
		else{
			//PostEvent SeedSwitchUp to SeedSwitch queue
			ES_Event SeedSwitchEvent;
			SeedSwitchEvent.EventType = ES_SeedSwitchUp;
			PostSeedService( SeedSwitchEvent );
		}//Endif
   }//Endif
    
    //Set LastSwitchState to the CurrentSwitchState
	LastSeedSwitchState = CurrentSwitchState;
    //Return ReturnVal
	return ReturnVal;
}//End of CheckButtonEvents

/****************************************************************************
 Function
     PostSeedService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Harine Ravichandiran
****************************************************************************/
bool PostSeedService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunSeedService

 Parameters
   ES_Event : the event to process
	 The EventType field of ThisEvent will be one of: SeedSwitchUp, SeedSwitchDown, or ES_TIMEOUT

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   implements a 2-state state machine for debouncing timing
 Notes
   
 Author
   Harine Ravichandiran
****************************************************************************/
ES_Event RunSeedService( ES_Event ThisEvent ){  

	//If CurrentState is Debouncing
	if (CurrentState == Debouncing){
		//If EventType is ES_TIMEOUT & parameter is debounce timer number
		if (ThisEvent.EventType == ES_TIMEOUT){
			//	Set CurrentState to Ready2Sample
			CurrentState = Ready2Sample;
		}
	}
  //Else if CurrentState is Ready2Sample
	else if (CurrentState == Ready2Sample){
		//	If EventType is SeedSwitchUp
		if(ThisEvent.EventType == ES_SeedSwitchUp){
			//Start debounce timer
			ES_Timer_InitTimer(SEED_TIMER, SEED_SWITCH_TIME);
      //Set CurrentState to Debouncing
			CurrentState = Debouncing;
		}//	End if
    //	If EventType is SeedSwitchDown
    else if ( ThisEvent.EventType == ES_SeedSwitchDown ) {
			//Start debounce timer
      ES_Timer_InitTimer(SEED_TIMER, SEED_SWITCH_TIME);
      //Set CurrentState to Debouncing
      CurrentState = Debouncing;
      //Post ES_SEED_DETECTED to Flip1Service, MainStoryService
      ES_Event SeedEvent;
      SeedEvent.EventType = ES_SEED_DETECTED;
      // Post to all services that are triggered by the seed event
      ES_PostList03( SeedEvent );
      #if DEBUG_SEED  
			printf("SS: Posting seed detected to FS1\n\r\n");
			#endif
     }  //	End if
	}//End Else

  //Return ES_NO_EVENT
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;
	return returnVal;
}  //End of RunSeedService

/***************************************************************************
 private functions
 ***************************************************************************/


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

