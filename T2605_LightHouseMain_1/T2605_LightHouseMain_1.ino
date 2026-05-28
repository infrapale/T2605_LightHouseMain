/*****************************************************************************
T2605_LightHouseMain_1
HW: Arduino Pro Mini + RFM69 @ 2017

RFM69 Address: RANTA 
From RFM60:
  Set relay (2) with timer function
    <R;RANTA;SMS1;PUMP;1>     pump on (infinite)
    <R;RANTA;SMS1;PUMP;120>   pump on for 120 seconds
    <R;RANTA;SMS1;PEER;0>     peer off

To RFM69
  Send Water Temperature  
    <S;#;RANTA;W;13.4>
*******************************************************************************
https://github.com/infrapale/T2310_RFM69_TxRx
https://learn.adafruit.com/adafruit-feather-m0-radio-with-rfm69-packet-radio
https://learn.sparkfun.com/tutorials/rfm69hcw-hookup-guide/all

*******************************************************************************
**/

#include <Arduino.h>
#include "main.h"
#if defined(ADA_M0_RFM69) | defined(ADA_RFM69_WING)
#include <wdt_samd21.h>
#endif
#ifdef PRO_MINI_RFM69
#include "avr_watchdog.h"
#endif
#include "secrets.h"
#include <RH_RF69.h>
#include <VillaAstridCommon.h>
#include "atask.h"
#include "rfm69.h"
#include "uart.h"
#include "rfm_receive.h"
#include "rfm_send.h"
#include "io.h"
#include "relay.h"

#define ZONE  "OD_1"
//*********************************************************************************************
#define SERIAL_BAUD   9600
#define ENCRYPTKEY    RFM69_KEY   // defined in secret.h


RH_RF69         rf69(RFM69_CS, RFM69_INT);
RH_RF69         *rf69p;
module_data_st  me = {'X','1'};   // deprecated ??
time_type       MyTime = {2023, 11,01,1,01,55}; 

main_ctrl_st main_ctrl = {
    .timeout = 0,
    //.my_addr = "SMS1",
};


void debug_print_task(void);
void run_100ms(void);
void rfm_receive_task(void); 


atask_st debug_print_handle        = {"Debug Print    ", 5000,0, 0, 255, 0, 1, debug_print_task};
atask_st clock_handle              = {"Tick Task      ", 100,0, 0, 255, 0, 1, run_100ms};
atask_st rfm_receive_handle        = {"Receive <- RFM ", 100,0, 0, 255, 0, 1, rfm_receive_task};


#ifdef PRO_MINI_RFM69
//AVR_Watchdog watchdog(4);
#endif

rfm_receive_msg_st  *receive_p;
rfm_send_msg_st     *send_p;
uart_msg_st         *uart_p;



void initialize_tasks(void)
{
  atask_initialize();
  //atask_add_new(&debug_print_handle);
  atask_add_new(&clock_handle);
  atask_add_new(&rfm_receive_handle);

  #ifdef SEND_TEST_MSG
  atask_add_new(&send_test_data_handle);
  #endif
}


void setup() 
{
    //while (!Serial); // wait until serial console is open, remove if not tethered to computer
    delay(2000);
    Serial.begin(9600);
    SerialX.begin(9600);

    Serial.print(__APP__); Serial.print(F(" Compiled: "));
    Serial.print(__DATE__); Serial.print(" ");
    Serial.print(__TIME__); Serial.println();

    
    uart_initialize();
    uart_p = uart_get_data_ptr();
    send_p = rfm_send_get_data_ptr();

    rf69p = &rf69;
    rfm69_initialize(&rf69);
    rfm_receive_initialize();
    io_initialize();
    // Hard Reset the RFM module
    
    initialize_tasks();
    relay_initialize();

    #if defined(ADA_M0_RFM69) | defined(ADA_RFM69_WING)
    // Initialze WDT with a 2 sec. timeout
    // wdt_init ( WDT_CONFIG_PER_16K );
    #endif
    #ifdef PRO_MINI_RFM69
    //watchdog.set_timeout(4);
    #endif

    SerialX.println("Bluetooth Relay");
}



void loop() 
{
    //SerialX.println("Hello World"); delay(4000);
    atask_run();  
}


void rfm_receive_task(void) 
{
    //SerialX.print("@");
    uart_read_uart();    // if available -> uart->prx.str uart->rx.avail
    if(uart_p->rx.avail)
    {
        uart_parse_rx_frame();
        #ifdef DEBUG_PRINT
        Serial.println(uart_p->rx.str);
        uart_print_rx_metadata();
        #endif
        if ( uart_p->rx.status == STATUS_OK_FOR_ME)
        {
            uart_exec_cmnd(uart_p->rx.cmd);
        }
        uart_p->rx.avail = false;
    }
    rfm_receive_message();
    #if defined(ADA_M0_RFM69) | defined(ADA_RFM69_WING)
    //wdt_reset();
    #endif
    #ifdef PRO_MINI_RFM69
    // watchdog.clear();
    #endif
}


void run_100ms(void)
{
    io_run_100ms();
}

void debug_print_task(void)
{
  atask_print_status(true);
}
