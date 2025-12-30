
#include "main.h"
#include "atask.h"
#include "uart.h"
#include "Rfm69Modem.h"

#define  NBR_OF_RELAY_MOD     2
#define  NBR_OF_OPT_INP       2
#define  OPTO_MAX_RADIATE_INTERVAL   (60000)
#define  OPTO_MIN_RADIATE_INTERVAL   (4000)

extern Rfm69Modem      rfm69_modem;

typedef struct
{
    uint32_t    timeout;
    uint8_t     relay_module_indx;
    uint8_t     relay_indx;
    uint8_t     opto_indx;
    uart_msg_st decoded_opto;
    uart_msg_st decoded_relay;
    uart_msg_st decoded_rec;
    uint32_t    radiate_timeout;
} opto_ctrl_st;

typedef struct
{
    uint8_t state;
    uint8_t new_val;
    uint8_t prev_val;
    char    label[16];
    uint32_t  timeout;
    uint32_t  next_update;
} opto_state_st;

extern modem_data_st   modem_data;

opto_state_st opto[NBR_OF_RELAY_MOD][NBR_OF_OPT_INP] = 
{
    {
        {0,0,0,"Piha1",0,0},
        {0,0,0,"Piha2",0,0},
    },
    {
        {0,0,0,"Ranta1",0,0},
        {0,0,0,"Ranta2",0,0},
    }
};

char tx_buff[UART_BUFF_LEN] = {0};
char rx_buff[UART_BUFF_LEN] = {0};

void opto_task(void);
void opto_radiate_task(void);

//atask_st debug_print_handle = {"Debug Print    ", 5000,0, 0, 255, 0, 1, debug_print_task};
atask_st opto_handle          = {"Opto Task      ", 100,0, 0, 255, 0, 1, opto_task};
atask_st opto_radiate_handle  = {"Radiate Task   ", 1000,0, 0, 255, 0, 1, opto_radiate_task};

opto_ctrl_st octrl = 
{
    .timeout = 0,
    .relay_module_indx = 0,
    .relay_indx = 0,
    .opto_indx = 0,
    .decoded_opto = {
        RELAY_MODULE_TAG, RELAY_MODULE_ADDR, 
        MY_MODULE_TAG, MY_MODULE_ADDR,
        OPTO_FUNCTION, WILD_CHAR,
        ACTION_GET, WILD_CHAR
    },
    .decoded_relay = {
        RELAY_MODULE_TAG, RELAY_MODULE_ADDR, 
        MY_MODULE_TAG, MY_MODULE_ADDR,
        RELAY_FUNCTION, '0',
        ACTION_SET, WILD_CHAR
    },
    .decoded_rec = {'*','*','*','*','*','*','*','*'},
    .radiate_timeout = 0
};

void opto_build_uart_msg(char *buff, uart_msg_st *decod)
{
    buff[0] = '<';
    buff[1] = decod->to_tag;
    buff[2] = decod->to_addr;
    buff[3] = decod->from_tag;
    buff[4] = decod->from_addr;
    buff[5] = decod->function;
    buff[6] = decod->func_indx;
    buff[7] = decod->action;
    buff[8] = decod->value;
    buff[9] = '>';
    buff[10] = '\r';
    buff[11] = '\n';
    buff[12] = 0x00;
}

void opto_print_decoded(uart_msg_st *decod)
{
    Serial.print(decod->to_tag);
    Serial.print(decod->to_addr);
    Serial.print(decod->from_tag);
    Serial.print(decod->from_addr);
    Serial.print(decod->function);
    Serial.print(decod->func_indx);
    Serial.print(decod->action);
    Serial.print(decod->value);
    Serial.println();
}

bool opto_decode_uart_msg(char *buff, uart_msg_st *decod)
{
    bool frame_ok = false;
    if((buff[0] == '<')  && (buff[9] == '>'))
    {
        decod->to_tag     = buff[1];
        decod->to_addr    = buff[2];
        decod->from_tag   = buff[3];
        decod->from_addr  = buff[4];
        decod->function   = buff[5];
        decod->func_indx  = buff[6];
        decod->action     = buff[7];
        decod->value      = buff[8];
        frame_ok = true;
    }
    return frame_ok;
}

void opto_inp_state_machine(uint8_t mod_indx, uint8_t inp_indx)
{

}

void opto_update_io_state(char from_addr, char value)
{
    uint8_t indx = from_addr - '1';
    uint8_t bm = value - '0';
    uint8_t new_val;

    for (uint8_t i = 0; i < NBR_OF_OPT_INP; i++)
    {
        if((bm & (1 << i)) != 0) new_val = 1;
        else new_val = 0;
        opto[indx][i].new_val = new_val;
    }
}

void opto_debug_print(void)
{
    for(uint8_t mod_indx = 0; mod_indx < NBR_OF_RELAY_MOD; mod_indx++)
    {
        for(uint8_t inp_indx = 0; inp_indx < NBR_OF_OPT_INP; inp_indx++)
        {
            Serial.print(opto[mod_indx][inp_indx].label);
            Serial.print(": State ");
            Serial.print(opto[mod_indx][inp_indx].state);
            Serial.print("-");
            Serial.print(opto[mod_indx][inp_indx].prev_val);
            Serial.print("->");
            Serial.print(opto[mod_indx][inp_indx].new_val);
            Serial.println();
        }
    }
}

bool opto_process_uart_msg(uart_msg_st *decod)
{
    //  <R1OY1*=3>    Opto 1 and 2 are high
    //opto_print_decoded(decod);
    bool do_continue = true;
    if (do_continue) if (decod->to_tag != modem_data.tag) do_continue = false;
    if (do_continue) if(decod->to_addr != modem_data.addr) do_continue = false;
    if (do_continue) if(decod->from_tag != octrl.decoded_opto.to_tag) do_continue = false;
    if (do_continue) if(decod->from_addr != octrl.decoded_opto.to_addr) do_continue = false;
    if (do_continue) if(decod->function != octrl.decoded_opto.function) do_continue = false;
    if (do_continue) if(decod->func_indx != octrl.decoded_opto.func_indx) do_continue = false;

    if (do_continue){
        //Serial.print("Opto value="); Serial.println(decod->value);
        opto_update_io_state(decod->from_addr,decod->value);
    }
    else {
        //Serial.println("opto_process_uart_msg - FAILED");
    }
}

void opto_initialize(void)
{
    atask_add_new(&opto_handle);
    atask_add_new(&opto_radiate_handle);
}

void opto_task(void)
{
    switch(opto_handle.state)
    {
        case 0:
            opto_handle.state = 10;
            break;
        case 10:
            opto_handle.state = 20;
            break;
        case 20: // request opto status
            octrl.decoded_opto.to_addr = octrl.relay_module_indx + '1';
            opto_build_uart_msg(tx_buff, &octrl.decoded_opto);
            Serial.print(tx_buff);
            octrl.timeout = millis() + 1000;
            opto_handle.state = 30;
            break;
        case 30:  // read opto status
            if (uart_read_uart(rx_buff) >= 10) {
                //Serial.print("rx_buff: ");  Serial.println(rx_buff);
                if (opto_decode_uart_msg(rx_buff, &octrl.decoded_rec)){
                    //Serial.println("Frame is OK");
                    opto_process_uart_msg(&octrl.decoded_rec);
                }
                opto_handle.state = 40;
            }
            else {
                if( millis() > octrl.timeout) opto_handle.state = 40;
            }           
            break;
        case 40:
            octrl.relay_module_indx++;
            if(octrl.relay_module_indx > 1) octrl.relay_module_indx = 0; 
            opto_handle.state = 10;
            break;
        case 50:
            opto_handle.state = 10;
            break;
        case 100:
            opto_handle.state = 10;
            break;
        case 200:
            opto_handle.state = 10;
            break;
    }
}
void opto_radiate_change(uint8_t mindx, uint8_t oindx, uint8_t oval)
{
    // <PIR;PIHA1;1>
    char buff[40] = {0};
    strcat(buff,"<PIR;");
    strcat(buff, opto[mindx][oindx].label);
    strcat(buff, ";");
    if(oval == 1) strcat(buff,"1>");
    else strcat(buff,"0>");
    Serial.println(buff);
    rfm69_modem.radiate(buff);
}

void opto_radiate_task(void)
{
    static uint8_t mod_indx = 0;
    static uint8_t opto_indx = 0;
    bool do_radiate = false;

    switch(opto_radiate_handle.state)
    {
        case 0:
            opto_radiate_handle.state = 10;
            for(mod_indx= 0; mod_indx < NBR_OF_RELAY_MOD; mod_indx++){
                for (opto_indx= 0; opto_indx < NBR_OF_OPT_INP; opto_indx++){
                    opto[mod_indx][opto_indx].next_update = millis() + OPTO_MAX_RADIATE_INTERVAL;
                }
            }
            mod_indx = 0; opto_indx = 0;
            break;    
        case 10:
            do_radiate = false;
            if(opto[mod_indx][opto_indx].new_val != opto[mod_indx][opto_indx].prev_val)
            {
                opto[mod_indx][opto_indx].prev_val = opto[mod_indx][opto_indx].new_val;
                do_radiate = true;
                // Serial.println("Changed value");
            }
            if(millis() > opto[mod_indx][opto_indx].next_update){
                do_radiate = true;
                // Serial.println("Next update");
            }    
            if(do_radiate){

                opto_radiate_change(mod_indx,opto_indx, opto[mod_indx][opto_indx].new_val);
                octrl.radiate_timeout = millis() + OPTO_MIN_RADIATE_INTERVAL;
                opto[mod_indx][opto_indx].next_update = millis() + OPTO_MAX_RADIATE_INTERVAL;
                do_radiate = false;
                opto_radiate_handle.state = 20;
            }
            if (++opto_indx >= NBR_OF_OPT_INP) {
                opto_indx = 0;
                if (++mod_indx >= NBR_OF_RELAY_MOD) mod_indx = 0;
            }
            break;    
        case 20:
            if(millis() > octrl.radiate_timeout) 
            opto_radiate_handle.state = 10;
            break;    
        case 30:
            opto_radiate_handle.state = 10;
            break;    
        case 40:
            opto_radiate_handle.state = 10;
            break;    
        case 100:
            opto_radiate_handle.state = 10;
            break;    
        case 200:
            opto_radiate_handle.state = 10;
            break;    

    }
}
