#ifndef __MSG_H__
#define __MSG_H__

void msg_initialize(void);

bool msg_is_valid_char(char c);

uint8_t msg_split(const char *msg);

void msg_sub_print(void);

void msg_handle(char *in_msg);

#endif