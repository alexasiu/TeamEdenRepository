/****************************************************************************
 Module
   Flipbook3Switch.c

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
#include "Flipbook3Switch.h"
#include "Flipbook3Service.h"
#include "MainStoryService.h"

#define ALL_BITS (0xff<<2)

#define PORT_B  BIT1HI  
#define FLIPBOOK3SWITCH_PIN  BIT2HI //The input pin from flipbook3 switch is PB2
#define FLIPBOOK3_SWITCH_TIME  100 //Time set for the flip3 switch timer

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t LastFlipbook3SwitchState;
static Flip3SwitchState_t CurrentState;

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
bool InitFlip3Switch ( uint8_t Priority )
{
	//	Initialize the MyPriority variable with the passed in parameter.
	MyPriority = Priority;

	//Initialize the port line to monitor the button
	HWREG(SYSCTL_RCGCGPIO) |= PORT_B; //enable port B
	//wait for peripheral clock
	while ((HWREG(SYSCTL_PRGPIO) & PORT_B) != PORT_B);
	
	//set flip3 switch pin to input
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (FLIPBOOK3SWITCH_PIN);
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) &= !(FLIPBOOK3SWITCH_PIN);
	
	//Sample the button port pin and use it to initialize LastFlipbook3SwitchState
	LastFlipbook3SwitchState = (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) & FLIPBOOK3SWITCH_PIN);
	
	//Set CurrentState to be Debouncing
	CurrentState = DebouncingF3;
	
	//Start debounce timer (timer posts to PostFlip3Switch)
	ES_Timer_InitTimer( FLIPBOOK3_SWITCH_TIMER, FLIPBOOK3_SWITCH_TIME );
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
     CheckFlip3SwitchEvents

 Parameters
     Takes no parameters

 Returns
     bool, returns True if an event posted, false otherwise

 Description
     
 Notes

 Author
     Harine Ravichandiran
****************************************************************************/
bool CheckFlip3SwitchEvents (void)
{
	//Local ReturnVal = False, CurrentSwitchState
	bool ReturnVal = false;
	uint8_t CurrentSwitchState;
    
	//Set CurrentSwitchState to state read from port pin
	CurrentSwitchState = (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) & FLIPBOOK3SWITCH_PIN);
    
	//If the CurrentSwitchState is different from the LastFlipbook3SwitchState
	if(CurrentSwitchState != LastFlipbook3SwitchState){
		//Set ReturnVal = True
		ReturnVal = true;
        
		//	If the CurrentSwitchState is down, (the value of the pin will be high)
		if( (CurrentSwitchState & FLIPBOOK3SWITCH_PIN) == FLIPBOOK3SWITCH_PIN ){
			//PostEvent Flipbook3SwitchDown to Flipbook3Switch queue
			ES_Event Flipbook3SwitchEvent;
			Flipbook3SwitchEvent.EventType = ES_Flipbook3SwitchDown;
			PostFlip3Switch(Flipbook3SwitchEvent);
		}
        //If the pin is not down (the switch is open, pin to ground)
		else{
			//PostEvent Flipbook3SwitchUp to Flipbook3Switch queue
			ES_Event Flipbook3SwitchEvent;
			Flipbook3SwitchEvent.EventType = ES_Flipbook3SwitchUp;
			PostFlip3Switch( Flipbook3SwitchEvent );
		}//Endif
   }//Endif
    
    //Set LastSwitchState to the CurrentSwitchState
	LastFlipbook3SwitchState = CurrentSwitchState;
    //Return ReturnVal
	return ReturnVal;
}//End of CheckFlip3SwitchEvents

/****************************************************************************
 Function
     PostFlip3Switch

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
bool PostFlip3Switch( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFlip3Switch

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
ES_Event RunFlip3Switch( ES_Event ThisEvent ){  

	//If CurrentState is Debouncing
	if (CurrentState == DebouncingF3){
		//If EventType is ES_TIMEOUT & parameter is debounce timer number
		if (ThisEvent.EventType == ES_TIMEOUT){
			//	Set CurrentState to Ready2Sample
			CurrentState = Ready2SampleF3;
		}
	}
  //Else if CurrentState is Ready2Sample
	else if (CurrentState == Ready2SampleF3){
		//	If EventType is ES_Flipbook3SwitchUp
		if(ThisEvent.EventType == ES_Flipbook3SwitchUp){
			//Start debounce timer
			ES_Timer_InitTimer( FLIPBOOK3_SWITCH_TIMER, FLIPBOOK3_SWITCH_TIME );
      //Set CurrentState to Debouncing
			CurrentState = DebouncingF3;
		}//	End if
    //	If EventType is ES_Flipbook3SwitchDown
    else if(ThisEvent.EventType == ES_Flipbook3SwitchDown){
			//Start debounce timer
      ES_Timer_InitTimer( FLIPBOOK3_SWITCH_TIMER, FLIPBOOK3_SWITCH_TIME );
      //Set CurrentState to Debouncing
      CurrentState = DebouncingF3;
      //Post ES_F3_DONE to Flipbook3Service, AirService, FruitDispenseService, LEDService
      ES_Event Flip3SwitchEvent;
      Flip3SwitchEvent.EventType = ES_F3_DONE;
      ES_PostList06( Flip3SwitchEvent );
      #if DEBUG_FLIP3SWITCH  
			printf("Flip3Switch: ES_F3_DONE - posting to list 06 \n\r\n");
			#endif
     }  //	End if
	}//End Else

  //Return ES_NO_EVENT
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;
	return returnVal;
}  //End of RunFlip3Service

/***************************************************************************
 private functions
 ***************************************************************************/


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

