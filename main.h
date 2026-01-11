#ifndef __MAIN_H__
#define __MAIN_H__
#include "WString.h"
#define   __APP__ ((char*)"T2512_RFM69_RelayOpto")

#define DEBUG_PRINT 
//#define SEND_TEST_MSG 
//#define ADA_M0_RFM69 
#define PRO_MINI_RFM69
#include <Arduino.h>
#include "rfm69.h"

#ifdef  ADA_M0_RFM69
#define SerialX  Serial1
#else
#define SerialX Serial
#endif

#define TASK_NBR_OF  3
#define LED_INDICATION

#define MY_MODULE_TAG       'R'
#define MY_MODULE_ADDR      '0'
#define RELAY_MODULE_TAG    'Y'
#define RELAY_MODULE_ADDR   '0'
#define OPTO_FUNCTION       'O'
#define RELAY_FUNCTION      'R'
#define ACTION_SET          '='
#define ACTION_GET          '?'
#define ACTION_REPLY        ':'
#define WILD_CHAR           '*'

typedef struct
{
    char            tag;
    char            addr;         
} modem_data_st;

// <R1F1O2:L>\r\n
typedef struct
{
    char to_tag;
    char to_addr;
    char from_tag;
    char from_addr;
    char function;
    char func_indx;
    char action;
    char value;
} uart_msg_st;


#endif