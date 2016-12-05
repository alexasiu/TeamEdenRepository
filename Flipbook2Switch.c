/****************************************************************************
 Module
   Flipbook2Switch.c

 Description
   Does Debouncing on Flipbook2 switch; Posts to Flipbook2 service
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
#include "Flipbook2Switch.h"
#include "Flipbook2Service.h"
#include "WaterBucketService.h"
#include "Flipbook3Service.h"

#define ALL_BITS (0xff<<2)

#define PORT_B  BIT1HI  // for the motor
#define FLIPBOOK2SWITCH_PIN  BIT3HI //The input pin from flipbook2 switch is PB3
#define FLIPBOOK2_SWITCH_TIME  100 //Time set for the flip2 switch timer

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t LastFlipbook2SwitchState;
static Flip2SwitchState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFlip2Switch

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
bool InitFlip2Switch ( uint8_t Priority )
{
	//	Initialize the MyPriority variable with the passed in parameter.
	MyPriority = Priority;

	//Initialize the port line to monitor the button
	HWREG(SYSCTL_RCGCGPIO) |= PORT_B; //enable port B
	//wait for peripheral clock
	while ((HWREG(SYSCTL_PRGPIO) & PORT_B) != PORT_B);
	
	//set flip2 switch pin to input
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (FLIPBOOK2SWITCH_PIN);
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) &= !(FLIPBOOK2SWITCH_PIN);
	
	//Sample the button port pin and use it to initialize LastFlipbook2SwitchState
	LastFlipbook2SwitchState = (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) & FLIPBOOK2SWITCH_PIN);
	
	//Set CurrentState to be Debouncing
	CurrentState = DebouncingF2;
	
	//Start debounce timer (timer posts to PostFlip2Switch)
	ES_Timer_InitTimer( FLIPBOOK2_SWITCH_TIMER, FLIPBOOK2_SWITCH_TIME );
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
     CheckFlip2SwitchEvents

 Parameters
     Takes no parameters

 Returns
     bool, returns True if an event posted, false otherwise

 Description
     
 Notes

 Author
     Harine Ravichandiran
****************************************************************************/
bool CheckFlip2SwitchEvents (void)
{
	//Local ReturnVal = False, CurrentSwitchState
	bool ReturnVal = false;
	uint8_t CurrentSwitchState;
    
	//Set CurrentSwitchState to state read from port pin
	CurrentSwitchState = (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) & FLIPBOOK2SWITCH_PIN);
    
	//If the CurrentSwitchState is different from the LastFlipbook2SwitchState
	if(CurrentSwitchState != LastFlipbook2SwitchState){
		//Set ReturnVal = True
		ReturnVal = true;
        
		//	If the CurrentSwitchState is down, (the value of the pin will be high)
		if( (CurrentSwitchState & FLIPBOOK2SWITCH_PIN) == FLIPBOOK2SWITCH_PIN ){
			//PostEvent Flipbook2SwitchDown to Flipbook2Switch queue
			ES_Event Flipbook2SwitchEvent;
			Flipbook2SwitchEvent.EventType = ES_Flipbook2SwitchDown;
			PostFlip2Switch(Flipbook2SwitchEvent);
		}
        //If the pin is not down (the switch is open, pin to ground)
		else{
			//PostEvent Flipbook2SwitchUp to Flipbook2Switch queue
			ES_Event Flipbook2SwitchEvent;
			Flipbook2SwitchEvent.EventType = ES_Flipbook2SwitchUp;
			PostFlip2Switch(Flipbook2SwitchEvent);
		}//Endif
   }//Endif
    
    //Set LastSwitchState to the CurrentSwitchState
	LastFlipbook2SwitchState = CurrentSwitchState;
    //Return ReturnVal
	return ReturnVal;
}//End of CheckFlip2SwitchEvents

/****************************************************************************
 Function
     PostFlip2Switch

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
bool PostFlip2Switch( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFlip2Switch

 Parameters
   ES_Event : the event to process
	 The EventType field of ThisEvent will be one of: ES_Flipbook2SwitchDown, ES_Flipbook2SwitchUp, or ES_TIMEOUT

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   implements a 2-state state machine for debouncing timing
 Notes
   
 Author
   Harine Ravichandiran
****************************************************************************/
ES_Event RunFlip2Switch( ES_Event ThisEvent ){  

	//If CurrentState is Debouncing
	if (CurrentState == DebouncingF2){
		//If EventType is ES_TIMEOUT & parameter is debounce timer number
		if (ThisEvent.EventType == ES_TIMEOUT){
			//	Set CurrentState to Ready2Sample
			CurrentState = Ready2SampleF2;
		}
	}
  //Else if CurrentState is Ready2Sample
	else if (CurrentState == Ready2SampleF2){
		//	If EventType is ES_Flipbook2SwitchUp
		if(ThisEvent.EventType == ES_Flipbook2SwitchUp){
			//Start debounce timer
			ES_Timer_InitTimer( FLIPBOOK2_SWITCH_TIMER, FLIPBOOK2_SWITCH_TIME );
      //Set CurrentState to Debouncing
			CurrentState = DebouncingF2;
		}//	End if
    //	If EventType is ES_Flipbook2SwitchDown
    else if(ThisEvent.EventType == ES_Flipbook2SwitchDown){
			//Start debounce timer
      ES_Timer_InitTimer( FLIPBOOK2_SWITCH_TIMER, FLIPBOOK2_SWITCH_TIME );
      //Set CurrentState to Debouncing
      CurrentState = DebouncingF2;
      //Post ES_F2_DONE to Flipbook2Service, WaterBucketService, Flipbook3Service, LEDService
      ES_Event Flip2SwitchEvent;
      Flip2SwitchEvent.EventType = ES_F2_DONE;
	  ES_PostList05( Flip2SwitchEvent );
      #if DEBUG_FLIP2SWITCH  
	  	printf("Flip2switch: ES_F2_DONE - posting to list 05\n\r\n");
	  #endif
     }  //	End if
	}//End Else

  //Return ES_NO_EVENT
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;
	return returnVal;
}  //End of RunFlip2SwitchService

/***************************************************************************
 private functions
 ***************************************************************************/


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

