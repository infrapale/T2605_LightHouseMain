#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>
#include <Arduino.h>
#include "main.h"
#include "uart.h"
#include "rfm_send.h"
#include "msg.h"
#include "atask.h"
#include "io.h"
#include "atask.h"


// case 0: SensorComSendStr("?TLake"); break;
// case 1: SensorComSendStr("?BMP180T"); break;

extern uart_msg_st         uart;

typedef struct
{
  float value;
  bool  updated;
} reading_st;

typedef struct {
    reading_st water_temp;
    reading_st bmp180_temp;
    uint32_t    timeout;
    char        send_buff[MAX_MESSAGE_LEN];
}sensor_ctrl_st;

void sensor_task(void);
void sensor_send_task(void);

//atask_st rfm_receive_handle      = {"Receive <- RFM ", 100,0, 0, 255, 0, 1, rfm_receive_task};
atask_st sensor_th                 = {"Sensor task    ", 100,0, 0, 255, 0, 1, sensor_task};
atask_st send_th                   = {"Sensor Send    ", 60000,0, 0, 255, 0, 1, sensor_send_task};

sensor_ctrl_st sensor_ctrl = {0};

void sensor_initialize(void)
{
    atask_add_new(&sensor_th);
    atask_add_new(&send_th);
}

void sensor_send_request(char *msg)
{
    SerialX.print('<');
    SerialX.print(msg);
    SerialX.println(">");
}

void sensor_task(void)
{
    int i1,i2;

    switch(sensor_th.state)
    {
        case 0:
            sensor_th.state = 10;
            break;
        case 10:
            sensor_send_request("?BMP180T");
            sensor_th.state = 20;
            break;
        case 20:
            uart_read_uart();
            if (uart.rx.avail){
                    //Serial.println(uart.rx.str);
                // <*T_BMP180=23.4>crlf  
                i1 = uart.rx.str.indexOf('=');
                i2 = uart.rx.str.indexOf('>', i1+1);
                if ((i1 > 0) && (i2 > i1)){
                    i1++;
                    //Serial.println(uart.rx.str.substring(i1,i2));
                    sensor_ctrl.bmp180_temp.value = uart.rx.str.substring(i1,i2).toFloat();
                    sensor_ctrl.bmp180_temp.updated = true;
                }
            }
            sensor_ctrl.timeout = millis() + 1000;
            sensor_th.state = 30;
            break;
        case 30:
            if (millis() > sensor_ctrl.timeout) sensor_th.state = 50;
            break;
        case 40:
            sensor_th.state = 10;
            break;
        case 50:
            sensor_send_request("?T_Lake");
            sensor_th.state = 60;
            break;
        case 60:
            uart_read_uart();
            if (uart.rx.avail){
                    //Serial.println(uart.rx.str);
                // <*T_BMP180=23.4>crlf  
                i1 = uart.rx.str.indexOf('=');
                i2 = uart.rx.str.indexOf('>', i1+1);
                if ((i1 > 0) && (i2 > i1)){
                    sensor_ctrl.water_temp.value = uart.rx.str.substring(i1,i2).toFloat();
                    sensor_ctrl.water_temp.updated = true;
                }
            }
            sensor_ctrl.timeout = millis() + 1000;
            sensor_th.state = 70;
            break;
        case 70:
            if (millis() > sensor_ctrl.timeout) sensor_th.state = 10;
            break;
    
    }

}

void sensor_send_task(void)
{
    switch(send_th.state)
    {
        case 0:
            send_th.state = 10;
            break;
        case 10:
            if((sensor_ctrl.bmp180_temp.updated) || (sensor_ctrl.water_temp.updated)){
                String Str = "<S;#;RANTA";
                if(sensor_ctrl.bmp180_temp.updated){
                    sensor_ctrl.bmp180_temp.updated = false;
                    Str += ";T1;";
                    Str += String(sensor_ctrl.bmp180_temp.value,1);
                }
                if(sensor_ctrl.water_temp.updated){
                    sensor_ctrl.water_temp.updated = false;
                    Str += ";W1;";
                    Str += String(sensor_ctrl.water_temp.value,1);
                }
                Str += ">";
                Str.toCharArray(sensor_ctrl.send_buff, MAX_MESSAGE_LEN);
                rfm_send_radiate_msg( sensor_ctrl.send_buff);
                //Serial.print(sensor_ctrl.bmp180_temp.value);
            }
            send_th.state = 10;
            break;
        case 20:
            send_th.state = 10;
            break;
    }
}
