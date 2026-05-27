#include "main.h"
#include "io.h"
#include <Arduino.h>
#include "relay.h"
#include "atask.h"

struct relay_status_struct {
   boolean is_on;
   byte cntr_10ms;
};

//atask_st rfm_receive_handle      = {"Receive <- RFM ", 100,0, 0, 255, 0, 1, rfm_receive_task};
atask_st relay_th                  = {"Relay 10ms     ", 100,0, 0, 255, 0, 1, relay_do_every_10ms};

relay_status_struct relay_status[MAX_RELAY]; 

byte relay_off_on[MAX_RELAY][2]={
    {RELAY_1A,RELAY_1B},
    {RELAY_2A,RELAY_2B},      
};

void relay_initialize(void) {
    for (byte i=0;i < MAX_RELAY;i++){
      pinMode( relay_off_on[i][0], OUTPUT); 
      pinMode( relay_off_on[i][1], OUTPUT); 
      digitalWrite(relay_off_on[i][0],LOW);
      digitalWrite(relay_off_on[i][1],LOW);

      relay_status[i].is_on = false; 
      relay_status[i].cntr_10ms = 0; 
   }
    atask_add_new(&relay_th);
}

void relay_test(void)
{
  for (byte n=0; n<10; n++)
  {
    for (byte i=0;i < MAX_RELAY;i++)
    {
      for (byte j = 0; j<2; j++)
      {
        digitalWrite(relay_off_on[i][j],LOW);
        delay(50);
        digitalWrite(relay_off_on[i][j],HIGH);
        Serial.println(relay_off_on[i][j]);
        delay(1000);
        digitalWrite(relay_off_on[i][j],LOW);
        delay(1000);
      }
    }
  }
}

void  relay_toggle(byte relay_indx) {
      if (relay_status[relay_indx].is_on) { 
         relay_status[relay_indx].is_on = false;
      } else { 
         relay_status[relay_indx].is_on = true; 
      }   
      Serial.print("Relay: ");Serial.print(relay_indx);Serial.print(" = ");Serial.println(relay_status[relay_indx].is_on);
      relay_status[relay_indx].cntr_10ms = SWITCH_TIME_x10ms; 
};

void relay_turn_on(byte relay_indx) {
      relay_status[relay_indx].is_on = true; 
      relay_status[relay_indx].cntr_10ms = SWITCH_TIME_x10ms; 
};
void relay_turn_off(byte relay_indx) {
      relay_status[relay_indx].is_on = false; 
      relay_status[relay_indx].cntr_10ms = SWITCH_TIME_x10ms; 
};

void relay_do_every_10ms(void){
   byte i;

   for(i=0; i<MAX_RELAY; i++){
      if (relay_status[i].cntr_10ms > 0){
         relay_status[i].cntr_10ms--;
         if (relay_status[i].cntr_10ms > 0){
            if (relay_status[i].is_on) digitalWrite(relay_off_on[i][1],HIGH);
            else digitalWrite(relay_off_on[i][0],HIGH); 
         } 
         else {
            digitalWrite(relay_off_on[i][0],LOW);
            digitalWrite(relay_off_on[i][1],LOW);
         }
      }
   } 
}
