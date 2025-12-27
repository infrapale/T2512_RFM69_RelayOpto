#include "main.h"
#include "io.h"



typedef struct
{
    uint8_t  pin;
    uint16_t tick_cntr;
} led_ctrl_st;

typedef struct {
    uint8_t  dip_sw_bm;
    uint8_t  unit_addr;
} io_ctrl_st;

io_ctrl_st io_ctrl = {0};


led_ctrl_st led_ctrl[LED_NBR_OF] = 
{
    [LED_INDX_ONBOARD]   = {PIN_LED_ONBOARD, 0},
    [LED_INDX_BLUE]      = {PIN_LED_BLUE, 0}
};

uint8_t dip_sw_pin[8] = {PIN_SW1, PIN_SW2, PIN_SW3, PIN_SW4, PIN_SW5, PIN_SW6, PIN_SW7, PIN_SW8};

void io_initialize(void)
{
    pinMode(PIN_LED_ONBOARD, OUTPUT);
    digitalWrite(PIN_LED_ONBOARD, HIGH);

    pinMode(A7, INPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
    digitalWrite(PIN_LED_BLUE, HIGH);
    io_led_flash(LED_INDX_BLUE,10);

    for (uint8_t i= SW_INDX_OPT1; i <= SW_INDX_WD_EN; i++){
        pinMode(dip_sw_pin[i], INPUT_PULLUP);
        if (digitalRead(dip_sw_pin[i])==LOW) io_ctrl.dip_sw_bm |= (1<<i);
        if(digitalRead(dip_sw_pin[SW_INDX_ADDR0])==LOW) io_ctrl.unit_addr |= (1<<0);
        if(digitalRead(dip_sw_pin[SW_INDX_ADDR1])==LOW) io_ctrl.unit_addr |= (1<<1);
        if(digitalRead(dip_sw_pin[SW_INDX_ADDR2])==LOW) io_ctrl.unit_addr |= (1<<2);
    }
    // Serial.print("Dip Switch Bitmap: "); Serial.println(io_ctrl.dip_sw_bm,HEX);
    // Serial.print("Unit Address:      "); Serial.println(io_ctrl.unit_addr);
}

void io_led_flash(led_index_et led_indx, uint16_t nbr_ticks )
{
    // duration = nbr_ticks * 100ms
    digitalWrite(led_ctrl[led_indx].pin, HIGH);
    led_ctrl[led_indx].tick_cntr = nbr_ticks;
}

bool io_wd_is_enabled(void)
{
    return (((1 << SW_INDX_WD_EN) & io_ctrl.dip_sw_bm) != 0);
}
uint8_t io_get_unit_addr(void)
{
    return io_ctrl.unit_addr;
}

void io_run_100ms(void)
{
    for (uint8_t lindx = 0; lindx < LED_INDX_NBR_OF; lindx++ )
    {
       if (led_ctrl[lindx].tick_cntr > 0)
       {
          led_ctrl[lindx].tick_cntr--;
          if (led_ctrl[lindx].tick_cntr == 0)
          {
                digitalWrite(led_ctrl[lindx].pin, LOW);
          }
       } 
    }
}
