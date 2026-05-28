#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>
#include <Arduino.h>
#include "main.h"
#include "msg.h"
#include "atask.h"
#include "io.h"
#include "relay.h"

#define MSG_MAX_FIELDS          10
#define MSG_MAX_FIELD_LEN       16
#define MSG_MAX_RAW_MSG_LEN     80



typedef struct 
{
    char        raw[MSG_MAX_RAW_MSG_LEN];
    char        fields[MSG_MAX_FIELDS][MSG_MAX_FIELD_LEN];
    uint8_t     field_count;
} msg_st;

extern main_ctrl_st main_ctrl;
msg_st msg = {0};

bool msg_is_valid_char(char c) {
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == ';' || c == '#' || c == '.' || c == '-') return true;
    return false;
}

int msg_strip_to_raw(char *msg_inp)
{
    int len = strlen(msg_inp);
    // Serial.printf("len1: %d ",len);
    if(len == 0 ) return 0;
    int indx = len-1;
    while(msg_inp[indx] == '\n' || msg_inp[indx] == '\r'){
        msg_inp[indx--] = 0x00;
        if(indx < 3) break;
    }
    len = strlen(msg_inp);
    int i = 0;
    while(msg_inp[i] == '\n' || msg_inp[i] == '\r'){
        i++;
        if(i > len -3) break;
    }
    len -= i;
    strncpy(msg.raw, &msg_inp[i],MSG_MAX_RAW_MSG_LEN);
    return len;
}

uint8_t msg_split(char *msg_inp, char separator = ';') {

    int len = msg_strip_to_raw(msg_inp);
    // Serial.printf("len1: %d ",len);
    if(len < 3) return 0;
    
    int f = 0;   // field index
    int i = 0;   // 
    int p = 0;   // position inside field

    // Serial.printf("len3: %d ",len);
    // Serial.printf("Start - End: %c %c ",raw_msg[i], raw_msg[len - 1] );
    if (len < 2 || msg.raw[i] != '<' || msg.raw[len - 1] != '>') return 0;
    i++;
    while (i < len - 1 && f < MSG_MAX_FIELDS) {
        char c = msg.raw[i];

        if (c == separator) {
            // End of field
            msg.fields[f][p] = '\0';
            f++;
            p = 0;
        }
        else {
            if (p < MSG_MAX_FIELD_LEN - 1) {
                msg.fields[f][p++] = c;
            }
        }

        i++;
    }

    // Final field
    if (f < MSG_MAX_FIELDS) {
        msg.fields[f][p] = '\0';
        f++;
    }
    // Serial.printf("..end return %d\n",f);
    return f;
}


void msg_sub_print(void)
{
    Serial.print("Message fields; count = ");
    Serial.println(msg.field_count);
    for (uint8_t i = 0; i < msg.field_count; i++) {
        Serial.print("["); Serial.print(i); Serial.print("] = ");
        Serial.println(msg.fields[i]);    
    }
}

uint8_t str_to_int16(const char *str, int16_t *out)
{
    if (!str || !*str) return 0;

    char *endptr;
    errno = 0;

    long val = strtol(str, &endptr, 10);

    // Check full consumption and no conversion errors
    if (errno != 0 || *endptr != '\0')
        return 1;

    // Check 16-bit signed range
    if (val < INT16_MIN || val > INT16_MAX)
        return 2;

    *out = (int16_t)val;
    return 0;
}

uint8_t str_to_int16(const char *str, int16_t *out, int min, int max)
{
    uint8_t res = str_to_int16(str, out);
    if(res == 0)
    {
        if ((*out < min) || (*out > max)) res = 3;
    }
    return res;
}

// <R;RANTA;SMS1;PUMP;11>
// Message fields; count = 5
// [0] = R
// [1] = RANTA
// [2] = SMS1
// [3] = PUMP
// [4] = 11

void msg_handle(char *in_msg)
{
    int16_t i16;
    uint8_t res;
    msg_strip_to_raw(in_msg);
    msg.field_count = msg_split(msg.raw);
    msg_sub_print(); 
    if (msg.field_count > 2)
    {
        switch(msg.fields[0][0])
        {
            case 'R':               
                //if(strncmp(msg.fields[1], main_ctrl.my_addr, MY_ADDR_LEN) == 0)
                if(strncmp(msg.fields[1],"RANTA", MY_ADDR_LEN) == 0)
                {
                    Serial.println(F("To me"));
                    uint8_t rindx = 99;
                    if(strncmp(msg.fields[3], "PUMP", MSG_MAX_FIELD_LEN) == 0) rindx = 1;
                    if(strncmp(msg.fields[3], "PEER", MSG_MAX_FIELD_LEN) == 0) rindx = 0;
                    if (rindx < MAX_RELAY)
                    {
                        Serial.println(F("Ranta message"));
                        res = str_to_int16(msg.fields[4], &i16, 0, 7200);
                        if(res == 0) {
                            Serial.print(F("Seconds: ")); Serial.println(i16);
                            relay_on_of_timer(rindx, i16);
                        }
                        else {
                            Serial.print(F("Incorrect value ")); Serial.println(res);
                        }

                    }

                }
                break;
        }
    }
}

