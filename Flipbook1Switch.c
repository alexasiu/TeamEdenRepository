/****************************************************************************
 Module
   Flipbook1Switch.c

 Description
   Does Debouncing on Flipbook1 switch; Posts to Flipbook1 service
 
Author
 Harine Ravichandiran
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
#include "Flipbook1Switch.h"
#include "Flipbook1Service.h"
#include "WaterBucketService.h"
#include "Flipbook2Service.h"

#define ALL_BITS (0xff<<2)

#define PORT_B  BIT1HI  // for the motor
#define FLIPBOOK1SWITCH_PIN  BIT1HI //The input pin from flipbook1 switch is pin 1(PB1)
#define FLIPBOOK1_SWITCH_TIME  100 //Time set for the seed switch timer

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t LastFlipbook1SwitchState; //stores the last state of the Flipbook 1 switch
static Flip1SwitchState_t CurrentState; //stores the current state of Flipbook 1 switch

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFlip1Switch

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
bool InitFlip1Switch ( uint8_t Priority )
{
	//	Initialize the MyPriority variable with the passed in parameter.
	MyPriority = Priority;

	//Initialize the port line to monitor the button
	HWREG(SYSCTL_RCGCGPIO) |= PORT_B; //enable port B
	//wait for peripheral clock
	while ((HWREG(SYSCTL_PRGPIO) & PORT_B) != PORT_B);
	
	//set seed pin to input
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (FLIPBOOK1SWITCH_PIN);
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) &= !(FLIPBOOK1SWITCH_PIN);
	
	//Sample the button port pin and use it to initialize LastFlipbook1SwitchState
	LastFlipbook1SwitchState = (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) & FLIPBOOK1SWITCH_PIN);
	
	//Set CurrentState to be Debouncing
	CurrentState = DebouncingF1;
	
	//Start debounce timer (timer posts to PostFlip1Switch)
	ES_Timer_InitTimer( FLIPBOOK1_SWITCH_TIMER, FLIPBOOK1_SWITCH_TIME );
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
     CheckFlip1SwitchEvents

 Parameters
     Takes no parameters

 Returns
     bool, returns True if an event posted, false otherwise

 Description
     
 Notes

 Author
     Harine Ravichandiran
****************************************************************************/
bool CheckFlip1SwitchEvents (void)
{
	//Local ReturnVal = False, CurrentSwitchState
	bool ReturnVal = false;
	uint8_t CurrentSwitchState;
    
	//Set CurrentSwitchState to state read from port pin
	CurrentSwitchState = (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) & FLIPBOOK1SWITCH_PIN);
    
	//If the CurrentSwitchState is different from the LastFlipbook1SwitchState
	if(CurrentSwitchState != LastFlipbook1SwitchState){
		//Set ReturnVal = True
		ReturnVal = true;
        
		//	If the CurrentSwitchState is down, (the value of the seed pin will be high)
		if( (CurrentSwitchState & FLIPBOOK1SWITCH_PIN) == FLIPBOOK1SWITCH_PIN ){
			//PostEvent Flipbook1SwitchDown to Flipbook1Switch queue
			ES_Event Flipbook1SwitchEvent;
			Flipbook1SwitchEvent.EventType = ES_Flipbook1SwitchDown;
			PostFlip1Switch(Flipbook1SwitchEvent);
		}
        //If the pin is not down (the switch is open, pin to ground)
		else{
			//PostEvent Flipbook1SwitchUp to Flipbook1Switch queue
			ES_Event Flipbook1SwitchEvent;
			Flipbook1SwitchEvent.EventType = ES_Flipbook1SwitchUp;
			PostFlip1Switch(Flipbook1SwitchEvent);
		}//Endif
   }//Endif
    
    //Set LastSwitchState to the CurrentSwitchState
	LastFlipbook1SwitchState = CurrentSwitchState;
    //Return ReturnVal
	return ReturnVal;
}//End of CheckButtonEvents

/****************************************************************************
 Function
     PostFlip1Switch

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
bool PostFlip1Switch( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFlip1Switch

 Parameters
   ES_Event : the event to process
	 The EventType field of ThisEvent will be one of: ES_Flipbook1SwitchDown, ES_Flipbook1SwitchUp, or ES_TIMEOUT

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   implements a 2-state state machine for debouncing timing
 Notes
   
 Author
   Harine Ravichandiran
****************************************************************************/
ES_Event RunFlip1Switch( ES_Event ThisEvent ){  

	//If CurrentState is Debouncing
	if (CurrentState == DebouncingF1){
		//If EventType is ES_TIMEOUT & parameter is debounce timer number
		if (ThisEvent.EventType == ES_TIMEOUT){
			//	Set CurrentState to Ready2Sample
			CurrentState = Ready2SampleF1;
		}
	}
  //Else if CurrentState is Ready2Sample
	else if (CurrentState == Ready2SampleF1){
		//	If EventType is ES_Flipbook1SwitchUp
		if(ThisEvent.EventType == ES_Flipbook1SwitchUp){
			//Start debounce timer
			ES_Timer_InitTimer( FLIPBOOK1_SWITCH_TIMER, FLIPBOOK1_SWITCH_TIME );
      //Set CurrentState to Debouncing
			CurrentState = DebouncingF1;
		}//	End if
    //	If EventType is ES_Flipbook1SwitchDown
    else if(ThisEvent.EventType == ES_Flipbook1SwitchDown){
			//Start debounce timer
      ES_Timer_InitTimer( FLIPBOOK1_SWITCH_TIMER, FLIPBOOK1_SWITCH_TIME );
      //Set CurrentState to Debouncing
      CurrentState = DebouncingF1;
      //Post ES_F1_DONE to Flipbook1Service, WaterBucketService, Flipbook2Service
      ES_Event Flip1SwitchEvent;
      Flip1SwitchEvent.EventType = ES_F1_DONE;
      // Post to all services that are triggered by F1 done
      ES_PostList02( Flip1SwitchEvent );
      #if DEBUG_FLIP1SWITCH  
			printf("Flip1switch: ES_F1_Done - posting to F1S, WaterBucketService, F2S\n\r\n");
			#endif
     }  //	End if
	}//End Else

  //Return ES_NO_EVENT
	ES_Event returnVal;
	returnVal.EventType = ES_NO_EVENT;
	return returnVal;
}  //End of RunFlip1switchService

/***************************************************************************
 private functions
 ***************************************************************************/


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

