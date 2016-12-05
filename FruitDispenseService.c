
/****************************************************************************
 Module
   FruitDispenseService.c

 Revision
   1.0.1

 Description
   Controls Fruit motor

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/17/16 11:49 hariner     started coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include <cmath>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TemplateService.h"

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
#include "FruitDispenseService.h"
#include "ADMulti.h"

/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff<<2)

#define PORT_B    BIT1HI      // for the motor
#define FRUITMOTOR_PIN GPIO_PIN_5  //PB5
#define PWM_CHAN  3           // which is PB5
#define PWM_GROUP 1						//for PB5
#define PWM_FREQ  50

#define MS_TO_US( A )    A/(0.0008*1000)    // converts to ticks of 0.8 us
#define PWM_PULSE        MS_TO_US( 1550 )  //  (defined in ticks of 0.8 us)


// these times assume 1ms/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static FruitMotorState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFruitService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
	Harine Ravichandiran
****************************************************************************/
bool InitFruitService ( uint8_t Priority )
{
  ES_Event ThisEvent;
	//Initialize the MyPriority variable with the passed in parameter.
  MyPriority = Priority;
	
	//Set CurrentState to be InitFruitDisp
	CurrentState = InitFruitDisp;
	
	//Post Event ES_Init to PostFruitService queue (this service)
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
     PostFruitService

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
bool PostFruitService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFruitService

 Parameters
   ES_Event : the event to process
	 Input event can be: ES_F3_DONE, ES_INIT, ES_RESET

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Takes in event and dispenses one fruit at a time. Moves motor to forward extreme and backward extreme

 Notes
   
 Author
  Harine Ravichandiran
****************************************************************************/
ES_Event RunFruitService( ES_Event ThisEvent )
{
	//Local Variables: NextState
	ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
	//Set NextFruitState to CurrentState
  FruitMotorState_t NextState = CurrentState;
	
	//Based on the state of the CurrentState variable choose one of the following blocks of code:
	switch ( CurrentState ){
		//CurrentState is InitFruitService
		case InitFruitDisp:
			//if ThisEvent is ES_INIT
			if (ThisEvent.EventType == ES_INIT){
                //Sets the frequency of the PWM pin
				PWM8_TIVA_SetFreq( PWM_FREQ, PWM_GROUP );
				//set NextState to Wait4Flip3DoneFr
				NextState = Wait4Flip3DoneFr;
				#if DEBUG_FRUIT
				printf("Fruit: Initialized. \n\r\n");
				#endif
			}//Endif
		break; //End case Initialize Fruit Dispensing block
		
		//CurrentState is Wait4Flip3Done
		case Wait4Flip3DoneFr:
			//if ThisEvent event type is ES_F3_DONE,
			if (ThisEvent.EventType == ES_F3_DONE){
				//Turn motor on
				PWM8_TIVA_SetPulseWidth (PWM_PULSE, PWM_CHAN);
				//set NextState to FruitDispensing
				NextState = Wait4FrDispDone;
				#if DEBUG_FRUIT
				printf("Fruit: Flip3Done. Fruit motor moving to dispense fruit. \n\r\n");
				#endif
			}//EndIf
		break; //End Wait4Flip3Done block 
			
		case Wait4FrDispDone:
			//if ThisEvent event type is ES_TIMEOUT,
			if (ThisEvent.EventType == ES_FR_DISP_DONE){
				//Stop the motor
				PWM8_TIVA_SetDuty( 0, PWM_CHAN );
				#if DEBUG_FRUIT
				printf("Fruit: Fruit motor stopped after dispensing one fruit. \n\r\n");
				#endif
				//Set NextState to InitFruitDisp
				NextState = InitFruitDisp;
			}//Endif
		break; //End of Wait4FrDispDone block
			
		default :
				printf("Error in FruitMotor State");
	  ;
	}
	CurrentState = NextState;
  return ReturnEvent;
} // End of RunFruitService

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

