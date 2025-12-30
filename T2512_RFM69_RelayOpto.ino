/*****************************************************************************
T2512_RFM69_RelayOpto
*******************************************************************************

HW: Adafruit M0 RFM69 Feather or Arduino Pro Mini + RFM69

Send and receive data via UART

*******************************************************************************
https://github.com/infrapale/T2310_RFM69_TxRx
https://learn.adafruit.com/adafruit-feather-m0-radio-with-rfm69-packet-radio
https://learn.sparkfun.com/tutorials/rfm69hcw-hookup-guide/all
*******************************************************************************


*******************************************************************************
Sensor Radio Message:   {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                        {"Z":"Dock","S":"T_dht22","V":"8.7","R":"-"}
Relay Radio Message     {"Z":"MH1","S":"RKOK1","V":"T","R":"-"}
Sensor Node Rx Mesage:  <#X1N:OD1;Temp;25.0;->
Relay Node Rx Mesage:   <#X1N:RMH1;RKOK1;T;->

Relay Mesage      <#R12=x>   x:  0=off, 1=on, T=toggle

*******************************************************************************
**/

#include <Arduino.h>
#include "main.h"
#ifdef ADAFRUIT_FEATHER_M0
#include <wdt_samd21.h>
#endif
#ifdef PRO_MINI_RFM69
#include "avr_watchdog.h"
#endif
#include "secrets.h"
#include <RH_RF69.h>
#include "Rfm69Modem.h"
#include "atask.h"
#include "io.h"
#include "opto.h"

#define ZONE  "OD_1"
//*********************************************************************************************
#define SERIAL_BAUD   9600
#define ENCRYPTKEY    RFM69_KEY   // defined in secret.h
RH_RF69         rf69(RFM69_CS, RFM69_INT);
Rfm69Modem      rfm69_modem(&rf69, RFM69_RST, PIN_LED_ONBOARD );
modem_data_st   modem_data = {MY_MODULE_TAG, MY_MODULE_ADDR};

#define NBR_TEST_MSG  4
#define LEN_TEST_MSG  32

void debug_print_task(void);
void run_100ms(void);
void send_test_msg_task(void);
void rfm_receive_task(void); 
void modem_task(void);


atask_st debug_print_handle        = {"Debug Print    ", 5000,0, 0, 255, 0, 1, debug_print_task};
atask_st clock_handle              = {"Tick Task      ", 100,0, 0, 255, 0, 1, run_100ms};
atask_st modem_handle              = {"Radio Modem    ", 100,0, 0, 255, 0, 1, modem_task};
atask_st send_test_handle          = {"Send Test      ", 100,0, 0, 255, 0, 1, send_test_msg_task};

#ifdef PRO_MINI_RFM69
//AVR_Watchdog watchdog(4);
#endif


void initialize_tasks(void)
{
    atask_add_new(&debug_print_handle); 
    atask_add_new(&clock_handle);
    atask_add_new(&modem_handle);
    //atask_add_new(&send_test_handle);
}


void setup() 
{
    //while (!Serial); // wait until serial console is open, remove if not tethered to computer
    delay(2000);
    Serial.begin(9600);
    Serial.print(__APP__); Serial.print(F(" Compiled: "));
    Serial.print(__DATE__); Serial.print(" ");
    Serial.print(__TIME__); Serial.println();

    //SerialX.begin(9600);
    atask_initialize();
    io_initialize();
    opto_initialize();
    if(io_wd_is_enabled()) Serial.println(F("Watchdog is enabled"));
    // modem_data.addr = io_get_unit_addr();
    initialize_tasks();
    uint8_t key[] = RFM69_KEY;
    rfm69_modem.initialize(modem_data.tag, modem_data.addr, key);
    rfm69_modem.enable_uart(false, false);
    rfm69_modem.radiate(__APP__);
    #ifdef PRO_MINI_RFM69
    //watchdog.set_timeout(4);
    #endif
}

#define BUFF_LEN   80
char mbuff[BUFF_LEN];
void loop() 
{
    atask_run();  
    if(rfm69_modem.msg_is_avail())
    {
        io_led_flash(LED_INDX_BLUE, 20);
        rfm69_modem.get_msg(mbuff, BUFF_LEN, true);
        Serial.println(mbuff);
    }
}

void modem_task(void)
{
    rfm69_modem.modem_task();
}

void send_test_msg_task(void)
{
    static uint32_t delay_timeout;
 
    switch(send_test_handle.state)
    {
        case 0:
            send_test_handle.state = 10;
            break;
        case 10:
            rfm69_modem.radiate("HelloWorld");
            io_led_flash(LED_INDX_ONBOARD, 20);
            delay_timeout = millis() + 4000;
            send_test_handle.state = 20;
            break;
        case 20:
            if (millis() > delay_timeout)
            {   
                // rfm69_modem.radiate_node_json("Dock;T_bmp1;9.1;-");
                io_led_flash(LED_INDX_BLUE, 10);
                delay_timeout = millis() + 4000;
                send_test_handle.state = 30;
            } 
            break; 
        case 30:
            if (millis() > delay_timeout) send_test_handle.state = 10;
            break;
    }
}

void run_100ms(void)
{
    io_run_100ms();
}

void debug_print_task(void)
{
//   atask_print_status(true);
//   opto_debug_print();
}

