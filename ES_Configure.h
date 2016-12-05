/****************************************************************************
 Module
     ES_Configure.h
 Description
     This file contains macro definitions that are edited by the user to
     adapt the Events and Services framework to a particular application.
 Notes
     
 History
 When           Who     What/Why
 -------------- ---     --------
  10/11/15 18:00 jec      added new event type ES_SHORT_TIMEOUT
  10/21/13 20:54 jec      lots of added entries to bring the number of timers
                         and services up to 16 each
 08/06/13 14:10 jec      removed PostKeyFunc stuff since we are moving that
                         functionality out of the framework and putting it
                         explicitly into the event checking functions
 01/15/12 10:03 jec      started coding
 11/13/16 15:48 afs      added airservice and flp3service
*****************************************************************************/

#ifndef CONFIGURE_H
#define CONFIGURE_H

#define DEBUG_MAIN  1
#define DEBUG_F1    0
#define DEBUG_SEED  0  // flipbook1 + seed service
#define DEBUG_F2    0  // flipbook2 service
#define DEBUG_WATER 0  // water service checkers only
#define DEBUG_F3    0
#define DEBUG_AIR   0  // flipbook3 + air service
#define DEBUG_IR    0  // IR event checkers only
#define DEBUG_FLIP1SWITCH 0 //flipbook1 switch
#define DEBUG_FLIP2SWITCH 0 //flipbook1 switch
#define DEBUG_FLIP3SWITCH 0 //flipbook1 switch
#define DEBUG_LED   0
#define DEBUG_FRUIT 0  //Fruit Dispensing motor
#define DEBUG_FRUIT_SWITCH   0  // debug fruit switch
#define DEBUG_ACC   0  // debug accelerometer readings

/****************************************************************************/
// The maximum number of services sets an upper bound on the number of 
// services that the framework will handle. Reasonable values are 8 and 16
// corresponding to an 8-bit(uint8_t) and 16-bit(uint16_t) Ready variable size
#define MAX_NUM_SERVICES 16

/****************************************************************************/
// This macro determines that nuber of services that are *actually* used in
// a particular application. It will vary in value from 1 to MAX_NUM_SERVICES
#define NUM_SERVICES 13

/****************************************************************************/
// These are the definitions for Service 0, the lowest priority service.
// Every Events and Services application must have a Service 0. Further 
// services are added in numeric sequence (1,2,3,...) with increasing 
// priorities
// the header file with the public function prototypes
#define SERV_0_HEADER "AirService.h"
// the name of the Init function
#define SERV_0_INIT InitAirService
// the name of the run function
#define SERV_0_RUN RunAirService
// How big should this services Queue be?
#define SERV_0_QUEUE_SIZE 3

/****************************************************************************/
// The following sections are used to define the parameters for each of the
// services. You only need to fill out as many as the number of services 
// defined by NUM_SERVICES
/****************************************************************************/
// These are the definitions for Service 1
#if NUM_SERVICES > 1
// the header file with the public function prototypes
#define SERV_1_HEADER "Flipbook3Service.h"
// the name of the Init function
#define SERV_1_INIT InitFlip3Service
// the name of the run function
#define SERV_1_RUN RunFlip3Service
// How big should this services Queue be?
#define SERV_1_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 2
#if NUM_SERVICES > 2
// the header file with the public function prototypes
#define SERV_2_HEADER "SeedService.h"
// the name of the Init function
#define SERV_2_INIT InitSeedService
// the name of the run function
#define SERV_2_RUN RunSeedService
// How big should this services Queue be?
#define SERV_2_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 3
#if NUM_SERVICES > 3
// the header file with the public function prototypes
#define SERV_3_HEADER "Flipbook1Service.h"
// the name of the Init function
#define SERV_3_INIT InitFlip1Service
// the name of the run function
#define SERV_3_RUN RunFlip1Service
// How big should this services Queue be?
#define SERV_3_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 4
#if NUM_SERVICES > 4
// the header file with the public function prototypes
#define SERV_4_HEADER "MainStoryService.h"
// the name of the Init function
#define SERV_4_INIT InitMainService
// the name of the run function
#define SERV_4_RUN RunMainService
// How big should this services Queue be?
#define SERV_4_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 5
#if NUM_SERVICES > 5
// the header file with the public function prototypes
#define SERV_5_HEADER "WaterBucketService.h"
// the name of the Init function
#define SERV_5_INIT InitWaterService
// the name of the run function
#define SERV_5_RUN RunWaterService
// How big should this services Queue be?
#define SERV_5_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 6
#if NUM_SERVICES > 6
// the header file with the public function prototypes
#define SERV_6_HEADER "Flipbook2Service.h"
// the name of the Init function
#define SERV_6_INIT InitFlip2Service
// the name of the run function
#define SERV_6_RUN RunFlip2Service
// How big should this services Queue be?
#define SERV_6_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 7
#if NUM_SERVICES > 7
// the header file with the public function prototypes
#define SERV_7_HEADER "Flipbook1Switch.h"
// the name of the Init function
#define SERV_7_INIT InitFlip1Switch
// the name of the run function
#define SERV_7_RUN RunFlip1Switch
// How big should this services Queue be?
#define SERV_7_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 8
#if NUM_SERVICES > 8
// the header file with the public function prototypes
#define SERV_8_HEADER "Flipbook2Switch.h"
// the name of the Init function
#define SERV_8_INIT InitFlip2Switch
// the name of the run function
#define SERV_8_RUN RunFlip2Switch
// How big should this services Queue be?
#define SERV_8_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 9
#if NUM_SERVICES > 9
// the header file with the public function prototypes
#define SERV_9_HEADER "Flipbook3Switch.h"
// the name of the Init function
#define SERV_9_INIT InitFlip3Switch
// the name of the run function
#define SERV_9_RUN RunFlip3Switch
// How big should this services Queue be?
#define SERV_9_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 10
#if NUM_SERVICES > 10
// the header file with the public function prototypes
#define SERV_10_HEADER "LEDService.h"
// the name of the Init function
#define SERV_10_INIT InitLEDService
// the name of the run function
#define SERV_10_RUN RunLEDService
// How big should this services Queue be?
#define SERV_10_QUEUE_SIZE 5
#endif

/****************************************************************************/
// These are the definitions for Service 11
#if NUM_SERVICES > 11
// the header file with the public function prototypes
#define SERV_11_HEADER "FruitDispenseService.h"
// the name of the Init function
#define SERV_11_INIT InitFruitService
// the name of the run function
#define SERV_11_RUN RunFruitService
// How big should this services Queue be?
#define SERV_11_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 12
#if NUM_SERVICES > 12
// the header file with the public function prototypes
#define SERV_12_HEADER "FruitSwitch.h"
// the name of the Init function
#define SERV_12_INIT InitFruitSwitch
// the name of the run function
#define SERV_12_RUN RunFruitSwitch
// How big should this services Queue be?
#define SERV_12_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 13
#if NUM_SERVICES > 13
// the header file with the public function prototypes
#define SERV_13_HEADER "TestHarnessService13.h"
// the name of the Init function
#define SERV_13_INIT InitTestHarnessService13
// the name of the run function
#define SERV_13_RUN RunTestHarnessService13
// How big should this services Queue be?
#define SERV_13_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 14
#if NUM_SERVICES > 14
// the header file with the public function prototypes
#define SERV_14_HEADER "TestHarnessService14.h"
// the name of the Init function
#define SERV_14_INIT InitTestHarnessService14
// the name of the run function
#define SERV_14_RUN RunTestHarnessService14
// How big should this services Queue be?
#define SERV_14_QUEUE_SIZE 3
#endif

/****************************************************************************/
// These are the definitions for Service 15
#if NUM_SERVICES > 15
// the header file with the public function prototypes
#define SERV_15_HEADER "TestHarnessService15.h"
// the name of the Init function
#define SERV_15_INIT InitTestHarnessService15
// the name of the run function
#define SERV_15_RUN RunTestHarnessService15
// How big should this services Queue be?
#define SERV_15_QUEUE_SIZE 3
#endif


/****************************************************************************/
// Name/define the events of interest
// Universal events occupy the lowest entries, followed by user-defined events
typedef enum {  ES_NO_EVENT = 0,
                ES_ERROR,  /* used to indicate an error from the service */
                ES_INIT,   /* used to transition from initial pseudo-state */
                ES_TIMEOUT, /* signals that the timer has expired */
                ES_SHORT_TIMEOUT, /* signals that a short timer has expired */
                /* User-defined events start here */
								ES_RESET,
								ES_SeedSwitchDown,
								ES_SeedSwitchUp,
								ES_SEED_DETECTED,
								ES_F1_DONE,
								ES_Flipbook1SwitchDown,
								ES_Flipbook1SwitchUp,
								ES_WATER,
								ES_NO_WATER,
								ES_F2_DONE,
								ES_Flipbook2SwitchDown,
								ES_Flipbook2SwitchUp,
								ES_F3_DONE,
								ES_Flipbook3SwitchDown,
								ES_Flipbook3SwitchUp,
								ES_IR1_HI, 
								ES_IR2_HI,
								ES_AIR,
								ES_NO_AIR,
								ES_START_HARVEST,
								ES_DONE_HARVEST,
								ES_FR_DISP_DONE,
								ES_FruitSwitchUp,
								ES_FruitSwitchDown,
								ES_CELEBRATION,
								ES_DONE_INIT,  // services post this when they're ready to start
                ES_NEW_KEY /* signals a new key received from terminal */
                } ES_EventTyp_t ;

/****************************************************************************/
// These are the definitions for the Distribution lists. Each definition
// should be a comma separated list of post functions to indicate which
// services are on that distribution list. Summary:
//    ES_PostList00  --> RESET list
//    ES_PostList01  --> Celebration list
//    ES_PostList02  --> F1 Done list
//    ES_PostList03  --> Seed list
//    ES_PostList04  --> Water list
//    ES_PostList05  --> F2 done list
//    ES_PostList06  --> F3 done list
//    ES_PostList07  --> Unused      
#define NUM_DIST_LISTS 7
#if NUM_DIST_LISTS > 0 
// RESET list. Post to this to reset all services.
#define DIST_LIST0 PostAirService, PostFlip3Service, PostFlip1Service, PostMainService, PostWaterBucketService, PostFlip2Service, PostLEDService, PostFruitService
#endif
#if NUM_DIST_LISTS > 1 
// CELEBRATION list. Post to make all services celebrate.
#define DIST_LIST1 PostAirService, PostFlip3Service, PostFlip1Service, PostMainService, PostWaterBucketService, PostFlip2Service, PostLEDService, PostFruitService
#endif
#if NUM_DIST_LISTS > 2 
// F1 Done list.
#define DIST_LIST2 PostFlip1Service, PostWaterBucketService, PostFlip2Service, PostLEDService
#endif
#if NUM_DIST_LISTS > 3 
// Seed detected list.
#define DIST_LIST3 PostFlip1Service, PostMainService, PostLEDService
#endif
#if NUM_DIST_LISTS > 4 
// Water list
#define DIST_LIST4 PostFlip2Service, PostLEDService, PostWaterBucketService
#endif
#if NUM_DIST_LISTS > 5 
// F2 Done list
#define DIST_LIST5 PostFlip2Service, PostFlip3Service, PostLEDService, PostWaterBucketService
#endif
#if NUM_DIST_LISTS > 6 
// F3 Done list
#define DIST_LIST6 PostFlip3Service, PostMainService, PostLEDService, PostFruitService             
#endif
#if NUM_DIST_LISTS > 7 
#define DIST_LIST7 PostTemplateFSM
#endif

/****************************************************************************/
// This are the name of the Event checking funcion header file. 
#define EVENT_CHECK_HEADER "AllEventCheckers.h"

/****************************************************************************/
// This is the list of event checking functions 
#define EVENT_CHECK_LIST Check4Keystroke, Check4IR_1, Check4IR_2, CheckSeedSwitchEvents, Check4Water, CheckFlip1SwitchEvents,CheckFlip2SwitchEvents, CheckFlip3SwitchEvents, CheckFruitSwitchEvents

/****************************************************************************/
// These are the definitions for the post functions to be executed when the
// corresponding timer expires. All 16 must be defined. If you are not using
// a timer, then you should use TIMER_UNUSED
// Unlike services, any combination of timers may be used and there is no
// priority in servicing them
#define TIMER_UNUSED ((pPostFunc)0)
#define TIMER0_RESP_FUNC PostSeedService
#define TIMER1_RESP_FUNC PostFlip3Service
#define TIMER2_RESP_FUNC PostMainService
#define TIMER3_RESP_FUNC PostMainService
#define TIMER4_RESP_FUNC PostFlip1Switch
#define TIMER5_RESP_FUNC PostFlip2Switch
#define TIMER6_RESP_FUNC PostFlip3Switch
#define TIMER7_RESP_FUNC PostLEDService
#define TIMER8_RESP_FUNC PostLEDService
#define TIMER9_RESP_FUNC PostLEDService
#define TIMER10_RESP_FUNC PostLEDService
#define TIMER11_RESP_FUNC PostLEDService
#define TIMER12_RESP_FUNC PostLEDService
#define TIMER13_RESP_FUNC PostLEDService
#define TIMER14_RESP_FUNC PostFruitSwitch
#define TIMER15_RESP_FUNC PostLEDService

/****************************************************************************/
// Give the timer numbers symbolc names to make it easier to move them
// to different timers if the need arises. Keep these definitions close to the
// definitions for the response functions to make it easier to check that
// the timer number matches where the timer event will be routed
// These symbolic names should be changed to be relevant to your application 

#define SEED_TIMER             0
#define FLIPBOOK3_INIT_TIMER   1   //timer for briefly starting the flipbook
#define GAME_TIMER             2
#define CELEB_TIMER            3
#define FLIPBOOK1_SWITCH_TIMER 4
#define FLIPBOOK2_SWITCH_TIMER 5
#define FLIPBOOK3_SWITCH_TIMER 6
#define SEED_LED_TIMER    		 7
#define RampF1LEDS_TIMER       8
#define RampF2LEDS_TIMER       9 
#define RampF3LEDS_TIMER      10
#define RampWaterLEDS_TIMER   11
#define BlinkAllLEDS_TIMER    12
#define BlinkWaterLEDS_TIMER  13
#define FRUIT_SWITCH_TIMER		14
#define BlinkSeedLEDS_TIMER   15

#endif /* CONFIGURE_H */
