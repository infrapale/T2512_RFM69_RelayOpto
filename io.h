#ifndef __IO_H__
#define __IO_H__

#ifdef  ADA_M0_RFM69
#define RFM69_CS      8
#define RFM69_INT     3
// #define RFM69_IRQN    0  // Pin 2 is IRQ 0!
#define RFM69_RST     4
#endif

#ifdef PRO_MINI_RFM69
#define RFM69_CS      10
#define RFM69_INT     2
#define RFM69_IRQN    0  // Pin 2 is IRQ 0!
#define RFM69_RST     9
#endif
// LED Definitions
#define PIN_LED_ONBOARD 13  // onboard blinky
#define LED_NBR_OF      4
#define PIN_LED_BLUE    A0  // connected to A7
// DIP Switch U2
#define PIN_SW1         0 // TXO
#define PIN_SW2         1 // RXI
#define PIN_SW3         0 // RST
#define PIN_SW4         3
#define PIN_SW5         4
#define PIN_SW6         5
#define PIN_SW7         6
#define PIN_SW8         7

#define SW_INDX_OPT1    3
#define SW_INDX_ADDR0   3
#define SW_INDX_ADDR1   4
#define SW_INDX_ADDR2   5
#define SW_INDX_WD_EN   7


typedef enum
{
    LED_INDX_ONBOARD = 0,
    LED_INDX_BLUE,
    LED_INDX_NBR_OF
} led_index_et;

void io_initialize(void);

void io_led_flash(led_index_et led_indx, uint16_t nbr_ticks );

void io_run_100ms(void);

bool io_wd_is_enabled(void);

uint8_t io_get_unit_addr(void);


#endif