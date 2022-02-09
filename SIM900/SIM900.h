/*
Library:	              SIM900.h
Written by:		      Edison (Edison YouTube Channel)
Date Written:		       Feb,9, 2022
Last modified:
Description:		      This is an STM32 device driver library for the SIM900, using STM HAL libraries
YouTube link:                  https://youtu.be/dUMIH2YShrg
										
* Copyright (C) 2021 Edison Ngunjiri
   This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
   of the GNU General Public Licenseversion 3 as published by the Free Software Foundation.
	
   This software library is shared with puplic for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
   or indirectly by this software, read more about this on the GNU General Public License.
*/

#include <stdint.h>

#ifndef SRC_SIM900_H_
#define SRC_SIM900_H_

#define GSMTest                "AT\r\n"
#define CheckMSGMode           "AT+CMGS=?\r\n"
#define checkMem               "AT+CPMS=?\r\n"
#define Activate_SMS_Mode      "AT+CMGF=1\r\n"
#define TestSmsRead            "AT+CMGR=?\r\n"
#define ReadALLSms             "AT+CMGL=\"ALL\"\r\n"
#define ReadOneSms             "AT+CMGR=1\r\n"
#define SimMemory              "AT+CPMS=\"SM\"\r\n"
#define SaveMsgSetting         "AT+CSAS\r\n"
#define ReciveMsg              "AT+CNMI=1,1,0,0,0\r\n"
#define DeleteAllSms           "AT+CMGDA=\"DEL ALL\"\r\n"
#define SetClock               "AT+CCLK?\r\n"



typedef enum
{
	GSM_Ok=0,
}GSM_STATUS;
typedef enum
{
	Text_OK=0,
	Text_NOT_OK,
	Sender_OK=0,
	TokenCode_OK,
	TokenUnits_OK,
	Search_OK,
	SMS_SENT,
	COMMAND_OK,
	SMS_READ_OK,

}Message_Status;

typedef enum
{
	Text=0,
	Sender,
	TokenCode,
	TokenUnits,
	COMMAND,
}SIM900_FIND;

typedef enum
{
	READ=0,
	DELETE,
	INBOX_MEM,
	CAPACITY_MEM,
}MESSAGE_CHOICE;

typedef struct Contact
{
	char Message_Sender[15];
	char Message_Body[30];
	char Time_Received[11];
	char Date_Received[11];
}Message_Typedef;

void clear_buffer(void);
int SIM900Init(void);
void ReadSmsInbox(int SMS_NUMBER);
void ReadAllSmsInbox(void);
int Send_Message(char Phone_Number[13],char *TextMsg);
void DeleteSingleMessage(int SMS_NUMBER);
void DeleteAllMessages(void);
int Find_in_Text(char Search_in_name[20],char *string);
int Check_No_of_message_In_Memory(int MEMORY);
void Message_Interrupt(void);
int Read_Delete_LastMsg(int READ_DELETE);
void clear_MessageDetails(Message_Typedef *Message_Storage);

#endif /* SRC_SIM900_H_ */
