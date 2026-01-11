#include "main.h"
#include "uart.h"

uint8_t uart_read_uart(char *buff)
{
    uint8_t len = 0;
    if (Serial.available())
    {
        String Str;
        Str  = Serial.readStringUntil('\n');
        // Serial.println(Str);
        if (Str.length()> 0)
        {
            Str.trim();
            len = Str.length();
            Str.toCharArray(buff, UART_BUFF_LEN);
        }
    } 
    return len;
}
