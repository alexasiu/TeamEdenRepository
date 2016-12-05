/****************************************************************************
 Module
   FruitSwitch.c

 Description
   Does Debouncing on Flipbook3 switch; Posts to Flipbook2 service
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
#include "FruitSwitch.h"
#include "FruitDispenseService.h"
#include "MainStoryService.h"

#define ALL_BITS (0xff<<2)

#define PORT_A  BIT0HI  
#define FRUIT_PIN  BIT7HI //The input pin for fruitswitch is PA7
#define FRUIT_SWITCH_TIME  100 //Time set for the fruit switch timer

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t LastFruitSwitchState;
static  FruitSwitchState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFlip3Switch

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
bool InitFruitSwitch ( uint8_t Priority )
{
	//	Initialize the MyPriority variable with the passed in parameter.
	MyPriority = Priority;

	//Initialize the port line to monitor the button
	HWREG(SYSCTL_RCGCGPIO) |= PORT_A; //enable port A
	//wait for peripheral clock
	while ((HWREG(SYSCTL_PRGPIO) & PORT_A) != PORT_A);
	
	//set fruit switch pin to input
	HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= (FRUIT_PIN);
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) &= !(FRUIT_PIN);
	
	//Sample the button port pin and use it to initialize LastFruitSwitchState
	LastFruitSwitchState = (HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) & FRUIT_PIN);
	
	//Set CurrentState to be Debouncing
	CurrentState = DebouncingFr;
	
	//Start debounce timer (timer posts to PostFruitSwitch)
	ES_Timer_InitTimer( FRUIT_SWITCH_TIMER, FRUIT_SWITCH_TIME );
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
     CheckFruitSwitchEvents

 Parameters
     Takes no parameters

 Returns
     bool, returns True if an event posted, false otherwise

 Description
     
 Notes

 Author
     Harine Ravichandiran
****************************************************************************/
bool CheckFruitSwitchEvents (void)
{
	//Local ReturnVal = False, CurrentSwitchState
	bool ReturnVal = false;
	uint8_t CurrentSwitchState;
    
	//Set CurrentSwitchState to state read from port pin
	CurrentSwitchState = (HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) & FRUIT_PIN);
    
	//If the CurrentSwitchState is different from the LastFruitSwitchState
	if(CurrentSwitchState != LastFruitSwitchState){
		//Set ReturnVal = True
		ReturnVal = true;
        
		//	If the CurrentSwitchState is down, (the value of the pin will be high)
		if( (CurrentSwitchState & FRUIT_PIN) == FRUIT_PIN ){
			//PostEvent FruitSwitchDown to FruitSwitch queue
			ES_Event FruitSwitchEvent;
			FruitSwitchEvent.EventType = ES_FruitSwitchDown;
			PostFruitSwitch(FruitSwitchEvent);
		}
        //If the pin is not down (the switch is open, pin to ground)
		else{
			//PostEvent FruitSwitchUp to FruitSwitch queue
			ES_Event FruitSwitchEvent;
			FruitSwitchEvent.EventType = ES_FruitSwitchUp;
			PostFruitSwitch( FruitSwitchEvent );
		}//Endif
   }//Endif
    
    //Set LastSwitchState to the CurrentSwitchState
	LastFruitSwitchState = CurrentSwitchState;
    //Return ReturnVal
	return ReturnVal;
}//End of CheckButtonEvents

/****************************************************************************
 Function
     PostFruitSwitch

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
bool PostFruitSwitch( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFruitSwitch

 Parameters
   ES_Event : the event to process
	 The EventType field of ThisEvent will be one of: ES_FruitSwitchDown, ES_FruitSwitchUp, or ES_TIMEOUT

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   implements a 2-state state machine for debouncing timing
 Notes
   
 Author
   Harine Ravichandiran
****************************************************************************/
ES_Event RunFruitSwitch( ES_Event ThisEvent ){  

	//If CurrentState is Debouncing
	if (CurrentState == DebouncingFr){
		//If EventType is ES_TIMEOUT & parameter is debounce timer number
		if (ThisEvent.EventType == ES_TIMEOUT){
			//	Set CurrentState to Ready2Sample
			CurrentState = Ready2SampleFr;
		}
	}
  //Else if CurrentState is Ready2Sample
	else if (CurrentState == Ready2SampleFr){
		//	If EventType is ES_FruitSwitchUp
		if(ThisEvent.EventType == ES_FruitSwitchUp){
			//Start debounce timer
			ES_Timer_InitTimer( FRUIT_SWITCH_TIMER, FRUIT_SWITCH_TIME );
      //Set CurrentState to Debouncing
			CurrentState = DebouncingFr;
		}//	End if
    //	If EventType is ES_FruitSwitchDown
    else if(ThisEvent.EventType == ES_FruitSwitchDown){
			//Start debounce timer
      ES_Timer_InitTimer( FRUIT_SWITCH_TIMER, FRUIT_SWITCH_TIME );
      //Set CurrentState to Debouncing
      CurrentState = DebouncingFr;
      //Post ES_FR_DISP_DONE  FruitDispenseService,
      ES_Event FruitSwitchEvent;
      FruitSwitchEvent.EventType = ES_FR_DISP_DONE;
      PostFruitService( FruitSwitchEvent );
      #if DEBUG_FLIP3SWITCH  
			printf("FruitSwitch: ES_FR_DISP_DONE - posting to list fruitservice \n\r\n");
			#endif
     }  //	End if
	}//End Else

  //Return ES_NO_EVENT
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;
	return returnVal;
}  //End of RunFruitSwitchService

/***************************************************************************
 private functions
 ***************************************************************************/


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

