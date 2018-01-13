/********************************************************************\
  Name:         feccusb.cxx
  Created by:   Pierre-Andre Amaudruz

  Contents:     Experiment specific readout code (user part) of
                Midas frontend. This example provide a template 
                for data acquisition using the CC-USB from Wiener
   This frontend runs in PERIODIC mode. The fastest is 1ms period.
   CCUSB doesn't provide a data valid condition to be evaluated in the POLLING 
   function. But as multiple trigger(lam) can be collected in the buffer. 
   Test shows that with a buffer of 4KD16 (mode:0) acquisition up to
   350KB/s for 8.3Kevt/s -> about 6us per D16 camac transfer average.
  
\********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <string.h>
#include "midas.h"
extern "C"{
#include <libxxusb.h>
}

#include "ccusb.h"
#include "OdbCCusb.h"

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
char const *frontend_name = "feccusb";
/* The frontend file name, don't change it */
char const *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 000;

/* maximum event size produced by this frontend */
INT max_event_size = 10000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 100 * 10000;
  //End of MIDAS specific globals , following lines are experiment specific global values

/* number of channels */
#define N_ADC8  12
// #define N_ADCD  12
// #define N_TDC  4
// #define N_SCLR 4

/* CAMAC crate and slots */
#define SLOT_IO   23
//#define SLOT_ADC8  4
#define SLOT_ADC8  10
//#define SLOT_ADCD  13
//#define SLOT_TDC   6
//define SLOT_SCLR  3

xxusb_device_type devices[10];
struct usb_device *dev;
usb_dev_handle *udev;       // Device Handle

HNDLE hDB, hSetCC;          //!< Midas equipment/<name>/Settings info handles
CCUSB_SETTINGS tscc;      //!< Settings local structure (see OdbCCusb.h)

/*-- Function declarations -----------------------------------------*/

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();

INT read_trigger_event(char *pevent, INT off);
INT read_scaler_event(char *pevent, INT off);

INT poll_event(INT source, INT count, BOOL test);
INT interrupt_configure(INT cmd, INT source, POINTER_T adr);

/*-- Equipment list ------------------------------------------------*/
EQUIPMENT equipment[] = {

   {"CCUSB",               /* equipment name */
    {1, 0,                   /* event ID, trigger mask */
     "BUF8",               /* event buffer */
     EQ_PERIODIC,            /* equipment type */
     LAM_SOURCE(0, 0xFFFFFF),/* event source crate 0, all stations */
     "MIDAS",                /* format */
     TRUE,                   /* enabled */
     RO_RUNNING,             /* read only when running */
     10,                     /* poll for 10ms */
     0,                      /* stop run after this event limit */
     0,                      /* number of sub events */
     0,                      /* don't log history */
     "", "", "",},
    read_trigger_event,      /* readout routine */
    },

   {""}
};

#ifdef __cplusplus
}
#endif

/********************************************************************\
              Callback routines for system transitions

  These routines are called whenever a system transition like start/
  stop of a run occurs. The routines are called on the following
  occations:

  frontend_init:  When the frontend program is started. This routine
                  should initialize the hardware.

  frontend_exit:  When the frontend program is shut down. Can be used
                  to releas any locked resources like memory, commu-
                  nications ports etc.

  begin_of_run:   When a new run is started. Clear scalers, open
                  rungates, etc.

  end_of_run:     Called on a request to stop a run. Can send
                  end-of-run event and close run gates.

  pause_run:      When a run is paused. Should disable trigger events.

  resume_run:     When a run is resumed. Should enable trigger events.
\********************************************************************/
// 
//----------------------------------------------------------------
// callback when the key in ODB is touched (see db_open_record below)
void seq_callback(INT hDB, INT hseq, void *info)
{
  KEY key;
  db_get_key(hDB, hseq, &key);
  printf("odb ... %s touched (%d)\n", key.name, hseq);
}

//
//----------------------------------------------------------------
//
int ccUsbFlush(void) {
  short IntArray [10000];  //for FIFOREAD
  
  int ret=1;
  int k=0;
  while(ret>0) {
    ret = xxusb_bulk_read(udev, IntArray, 8192, 100);
    if(ret>0) {
      printf("drain loops: %i (result %i)\n ",k,ret);                                     
      k++;
      if (k>100) ret=0;

      // Last event dump
      for(int i =0; i < ret/2; i++){
	printf("%x ",IntArray[i] & 0xffff);
	if(i%8==7) printf("\n");
      }
    }
  }
  printf("\n");
  printf("First bin %x\n",IntArray[0]);
  return (IntArray[0] & 0xfff);

}

BOOL in_deferred_transition = FALSE;
BOOL finished_clearing_buffer = FALSE;
int number_extra_reads = 0;

// This function used to setup deferred transition, where we read the USB buffer 
// a couple extra times at the end of the run to clear out the events.
BOOL clear_buffer_events(int transition, BOOL first){

  if(first){

    //  Stop DAQ mode
    int ret = xxusb_register_write(udev, 1, 0x0);
    printf("stopping CAMAC xxusb_register_write=%d\n", ret);

    finished_clearing_buffer = FALSE;
    in_deferred_transition = TRUE;
    number_extra_reads = 0;
    printf("Starting deferred to clear USB buffer\n");
  }

  if(finished_clearing_buffer){
    printf("Finished deferred transition after %i extra reads.\n",number_extra_reads);
    return TRUE;
  }

  if(number_extra_reads > 10){
    cm_msg(MERROR, "clear_buffer_events", "Didn't manage to clear CCUSB buffer after 10 extra reads.\n");
    return TRUE;
  }

  return FALSE;
}


/*-- Frontend Init -------------------------------------------------*/
// Executed once at the start of the applicatoin
INT frontend_init() {

  int size, status;
  char set_str[80];
  CCUSB_SETTINGS_STR(ccusb_settings_str);

  cm_get_experiment_database(&hDB, &status);
  
  // Map /equipment/<eq_name>/settings 
  sprintf(set_str, "/Equipment/CCUSB/Settings");
  
  // create the ODB Settings if not existing under Equipment/<eqname>/
  // If already existing, don't change anything 
  status = db_create_record(hDB, 0, set_str, strcomb(ccusb_settings_str));
  if (status != DB_SUCCESS) {
    cm_msg(MERROR, "ccusb", "cannot create record (%s)", set_str);
    return FE_ERR_ODB;
  }
  
  // Get the equipment/<eqname>/Settings key
  status = db_find_key(hDB, 0, set_str, &hSetCC);
  
  // Enable hot-link on /Settings of the equipment
  // Anything changed under /Settings will be triggering the callback (above)
  size = sizeof(CCUSB_SETTINGS);
  if ((status = db_open_record(hDB, hSetCC, &tscc, size, MODE_READ
			       , seq_callback, NULL)) != DB_SUCCESS) {
    cm_msg(MERROR, "ccusb", "cannot open record (%s)", set_str);
    return CM_DB_ERROR;
  }

  // Setup a deferred transition so that we can read out any events that remain in
  // buffer at the end of the run.
  cm_register_deferred_transition(TR_STOP,clear_buffer_events);

  // hardware initialization   
  //Find XX_USB devices and open the first one found
  printf("Initializing USB device\n");
  int ret = xxusb_devices_find(devices);
  printf("Testing first USB device; ret=%i\n",ret);
  dev = devices[0].usbdev;
  udev = xxusb_device_open(dev);

  // Make sure CC_USB opened OK
  if(!udev) {
    cm_msg(MERROR, "ccusb", "Failed to Open CC_USB");
    return FE_ERR_HW;
  } 

  //  Stop DAQ mode in case it was left running (after crash)
  ret = xxusb_register_write(udev, 1, 0x0);

  if(ret < 0){
    cm_msg(MERROR, "ccusb", "Error writing to CCUSB; error message = %i",ret);
    return FE_ERR_HW;
  }

  //  printf("stop DAQ return:%d\n", ret);

  // Flush old data first
  ccUsbFlush();
  //  printf("Flush return:%d\n", ret);

  // CAMAC CLEAR
  CAMAC_C(udev);
  // printf("CAMAC_C return:%d\n", ret);
  // CAMAC Z
  CAMAC_Z(udev);
  // printf("CAMAC_Z return:%d\n", ret);
  
  /* print message and return FE_ERR_HW if frontend should not be started */
  cm_msg(MINFO, "ccusb", "CC-USB ready");
  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/
INT frontend_exit()
{
  // Close device
  xxusb_device_close(udev);
  return SUCCESS;
}

// Keep track of number of events in the run
int EventsInRun = 0;

/*-- Begin of Run --------------------------------------------------*/
// Done on every begin of run 
INT begin_of_run(INT run_number, char *error) {

  //
  // Construct the CAMAC list to be loaded into the Stack
  long int stack[100];
  int i, ret;
  int q, x;
  long d24;
  short d16;

  // Create the CAMAC list to be performed on each LAM
  StackCreate(stack);

  // StackXxx(), MRAD16, WMARKER, etc and macros are defined in 
  // in ccusb.h
  //                   n       a   f  data  
  StackFill(MRAD16 , SLOT_ADC8, 0 , 0, 12, stack);// read 12 ADC8 
  // StackFill(MRAD16 , SLOT_ADCD, 0 , 0, 12, stack);// read 12 ADCD 
  //  StackFill(MRAD16 , SLOT_TDC, 0 , 0, 7, stack); // read  8 TDC
  // Use of F2 can prevent the F9 if last readout is last A  
  //StackFill(CD16   , SLOT_TDC, 0 , 9, 0, stack); // clear TDC, data is ignored (command)  
  StackFill(CD16   , SLOT_ADC8, 0 , 9, 0, stack); // clear ADC8
  // StackFill(CD16   , SLOT_ADCD, 0 , 9, 0, stack); // clear ADCD
  StackFill(WMARKER, 0, 0, 0, 0xFEED, stack);    // Our marker 0xfeed
  StackClose(stack);

  #if 1
  // Debugging Check stack before writing
  printf("stack[0]:%ld\n", stack[0]);
  for (i = 0; i < stack[0]+1; i++) {
    printf("stack[%i]=0x%lx\n", i, stack[i]);
  }

  // Load stack into CC-USB
  ret = xxusb_stack_write(udev, 2, stack);
  ret -= 2;
  if (ret<0) {
    printf("err on stack_write:%d\n", ret);
  } else {
    printf("Nbytes(ret) from stack_write:%d\n", ret);
    for (i = 0; i < (ret/2); i++) {
      if(0) printf("Wstack[%i]=0x%lx\n", i, stack[i]);
    }
  }
  #endif

  // Debugging Read stack 
  ret = xxusb_stack_read(udev, 2, stack);
  if (ret<0) {
    printf("err on stack_read:%d\n", ret);
  } else {
    printf("Nbytes(ret) from stack_read:%d\n", ret);
    for (i = 0; i < (ret/2); i++) {
      if(0) printf("Rstack[%i]=0x%lx\n", i, stack[i]);
    }
  }
  
  printf("FInished stacking...\n");
  // Remove Inhibit
  CAMAC_I(udev, FALSE);
  
  // LAM mask from the Equipment
  ret = CAMAC_write(udev, 25, 9, 16, LAM_SOURCE_STATION(equipment[0].info.source), &q, &x);

  printf("FInished stacking...\n");
  // delay LAM timeout[15..8], trigger delay[7..0]
  d16 = ((tscc.lam_timeout & 0xFF) << 8) | (tscc.trig_delay & 0xFF);
  printf ("lam, trigger delay : 0x%x\n", d16);
  ret = CAMAC_write(udev, 25, 2, 16, d16, &q, &x);

  printf("FInished stacking...\n");
  //  Enable LAM
  // ret = CAMAC_read(udev, SLOT_TDC, 0, 26, &d24, &q, &x);
  ret = CAMAC_read(udev, SLOT_ADC8, 0, 26, &d24, &q, &x);
  //ret = CAMAC_read(udev, SLOT_ADCD, 0, 26, &d24, &q, &x);
  // printf("ret:%d q:%d x:%d\n", ret, q, x);

  // Opt in Global Mode register N(25) A(1) F(16)    
  // Buffer size 
  // 0:4096, 1:2048, 2:1024, 3:512, 4:256, 5:128, 6:64, 7:single event
  ret = CAMAC_write(udev, 25, 1, 16, tscc.buffer_size, &q, &x);

  printf("FInished stacking...\n");
  // CAMAC_DGG creates a gate pulse with control of delay, width 
  // in 10ns increments. 
  // BUT range is limited to 16bit anyway annd only GG-A (0) provides
  // the delay extension.
  // Maximum delay 1e6 -> minimum frq 1KHz 
  //                DGG_A Pulser NimO1               invert latch
  ret = CAMAC_DGG(udev, 0,     7,    1, tscc.delay/10, tscc.width/10,     0,    0); 

  printf("FInished stacking...\n");
  //  First clear before first LAM/readout
  ret = CAMAC_read(udev, SLOT_ADC8, 0, 9, &d24, &q, &x);
  // ret = CAMAC_read(udev, SLOT_ADCD, 0, 9, &d24, &q, &x);
  // ret = CAMAC_read(udev, SLOT_TDC, 0, 9, &d24, &q, &x);

  //  Start DAQ mode
  ret = xxusb_register_write(udev, 1, 0x1);
  //
  // NO CAMAC calls ALLOWED beyond this point!!!!!!!!
  //  as the Module is in acquisition mode now
  //

  // Count total events in run.
  EventsInRun = 0;
  // Set value for deferred transition (to readout last events).
  in_deferred_transition = FALSE;
  finished_clearing_buffer = FALSE;

  return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/
INT end_of_run(INT run_number, char *error)
{
  //  Stop DAQ mode
  int ret = xxusb_register_write(udev, 1, 0x0);
  printf("End of run Stop DAQ return:%d\n", ret);

  // Set Inhibit
  //  CAMAC_I(udev, TRUE);

  // Flush data.
  // This shouldn't do anything, since we flushed all the events with deferred transition.
  ret = ccUsbFlush();
  if(ret > 0) cm_msg(MINFO, "ccusb", "Flushed %d events; surprising", ret);
  EventsInRun += ret;
  cm_msg(MINFO, "ccusb", "Total number of events read in this run: %i \n",EventsInRun);
  return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/
INT pause_run(INT run_number, char *error)
{
  //  Stop DAQ mode
  xxusb_register_write(udev, 1, 0x0);

  // Set Inhibit
  CAMAC_I(udev, TRUE);

  // -PAA-
  // flush data, these data are lost as the run is already closed.
  // will implement deferred transition later to fix this issue
  ccUsbFlush();
   return SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/
INT resume_run(INT run_number, char *error)
{
  int q, x;
  long d24;

  //  Clear module
  CAMAC_read(udev, SLOT_ADC8, 0, 9, &d24, &q, &x);
  //CAMAC_read(udev, SLOT_ADCD, 0, 9, &d24, &q, &x);
  // CAMAC_read(udev, SLOT_TDC, 0, 9, &d24, &q, &x);

  // Remove Inhibit
  CAMAC_I(udev, FALSE);

  //  Start DAQ mode
  xxusb_register_write(udev, 1, 0x1);

  return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/
INT frontend_loop()
{
   /* if frontend_call_loop is true, this routine gets called when
      the frontend is idle or once between every event */
   return SUCCESS;
}

/*------------------------------------------------------------------*/

/********************************************************************\

  Readout routines for different events

\********************************************************************/

/*-- Trigger event routines ----------------------------------------*/
extern "C" INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
   int i;
   DWORD lam=0;

   for (i = 0; i < count; i++) {
     ss_sleep(100);
     if (lam & LAM_SOURCE_STATION(source))
       if (!test)
	 return lam;
   }
   
   return 0;
}

/*-- Interrupt configuration ---------------------------------------*/
extern "C" INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
{
   switch (cmd) {
   case CMD_INTERRUPT_ENABLE:
      break;
   case CMD_INTERRUPT_DISABLE:
      break;
   case CMD_INTERRUPT_ATTACH:
      break;
   case CMD_INTERRUPT_DETACH:
      break;
   }
   return SUCCESS;
}

/*-- Event readout -------------------------------------------------*/
INT read_trigger_event(char *pevent, INT off)
{
  WORD *pdata;
  int ret, nd16=0;
  
  if(in_deferred_transition){
    number_extra_reads++;
  }

  /* init bank structure */
  bk_init(pevent);
  
    /* create Midas bank named ADC8 for ADC card in slot 8*/
  bk_create(pevent, "ADC8", TID_WORD, (void **)&pdata);

 /* create Midas bank named ADCD D=13 for ADC card in slot 13*/
  //bk_create(pevent, "ADCD", TID_WORD, (void **)&pdata);

 /* create Midas bank named ADTD */
//  bk_create(pevent, "ADTD", TID_WORD, (void **)&pdata);

  // Read CC-USB buffer, returns nbytes, use for 32-bit data
  ret = xxusb_bulk_read(udev, pdata, 8192, 500);  
  if (ret > 0) {
    nd16 = ret / 2;                 // # of d16
    int nevents = (pdata[0]& 0xfff);   // # of LAM in the buffer
    EventsInRun += nevents;
 #if 0
    int evtsize = (pdata[1] & 0xffff);  // # of words per event
    printf("Read data: ret:%d  nd16:%d nevent:%d, evtsize:%d\n", ret, nd16, nevents, evtsize);
#endif

    //    if (nevents & 0x8000) return 0;  // Skip event
    if(pdata[0] & 0x8000)cm_msg(MINFO, "read_trigger_event", "Readout last CAMAC event in run.");
      
    
    if (nd16 > 0) {
      // Adjust pointer (include nevents, evtsize)
      pdata += nd16;
    }

    // Close bank
    bk_close(pevent, pdata);

    printf("readout non-zero bytes from CAMAC usb\n");
    // Done with a valid event
    return bk_size(pevent); 

 } else {
    //    printf("no read ret:%d\n", ret);

    // we finished the deferred transition when we do a read of USB buffer 
    // and don't get any extra data.
    if(in_deferred_transition){      
      finished_clearing_buffer = TRUE; 
    }
    return 0;
  }
}
