
/*
Library:	              SIM900.c
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


#include "SIM900.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "stm32f4xx_hal.h"
#define GsmHuart  huart1

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

Message_Typedef Message_Details={0};


int No_of_messages=0;
char Reply[200]= {0};



void clear_buffer(void)
{
	for(int n=0; n<=200; n++)
	{
		Reply[n]='\0';
	}
}


int SIM900Init(void)
{
	clear_buffer();
	while(1)
	{

		HAL_UART_Transmit(&GsmHuart,(uint8_t*)GSMTest,strlen(GSMTest),HAL_MAX_DELAY);
		HAL_UART_Receive(&GsmHuart,(uint8_t*)Reply,20,1000);
		HAL_Delay(1000);
		if(Reply[2]=='O' && Reply[3]=='K')
		{

			HAL_Delay(1000);
			break;
		}
	}
	//Activate TEXT MODE FOR SMS
	HAL_UART_Transmit(&GsmHuart,(uint8_t*)Activate_SMS_Mode,strlen(Activate_SMS_Mode),HAL_MAX_DELAY);
	HAL_Delay(1000);
	//check memory
	//Save memory setting
	HAL_UART_Transmit(&GsmHuart,(uint8_t*)SaveMsgSetting,strlen(SaveMsgSetting),HAL_MAX_DELAY);
	HAL_Delay(1000);
	//Get time
	Message_Interrupt();
	return GSM_Ok;

}

void ReadSmsInbox(int SMS_NUMBER)
{
	int Sep_quotes=0,comma=0,Char_Position_in_Reply=0,Sender_Digit=0,Date_Digit=0,Body_Digit=0,Time_Digit=0;
	char MessageSender[14]={0},DateReceived[9]={0},TimeReceived[9]={0},MessageBody[30]={0};
	char msgnumber[30];
	clear_buffer();
	sprintf(msgnumber,"AT+CMGR=%d\r\n",SMS_NUMBER);
	HAL_UART_Transmit(&GsmHuart,(uint8_t*)msgnumber,strlen(msgnumber),HAL_MAX_DELAY);
	HAL_UART_Receive(&GsmHuart,(uint8_t*)Reply,100,1000);
	HAL_Delay(1000);
	for(Char_Position_in_Reply=0;Char_Position_in_Reply<strlen(Reply);Char_Position_in_Reply++)
	{
		if(Reply[Char_Position_in_Reply]=='"')
		{
			Sep_quotes++;
			Char_Position_in_Reply++;
		}
		if(Reply[Char_Position_in_Reply]==',')
		{
			comma++;
		}
		if(Sep_quotes==3)
		{
			MessageSender[Sender_Digit]=Reply[Char_Position_in_Reply];
			Sender_Digit++;
		}
		if(Sep_quotes==6 && comma==3 && Date_Digit<8)
		{
			DateReceived[Date_Digit]=Reply[Char_Position_in_Reply];
			Date_Digit++;
		}
		if(Sep_quotes==6 && comma==4 &&Time_Digit<8)
		{
			TimeReceived[Time_Digit]=Reply[Char_Position_in_Reply+1];
			Time_Digit++;
		}
		if(Sep_quotes==7)
		{
			MessageBody[Body_Digit]=Reply[Char_Position_in_Reply];
			Body_Digit++;
		}
	}

	sprintf(Message_Details.Message_Sender,"%s\t",MessageSender);
	sprintf(Message_Details.Date_Received,"%s\t",DateReceived);
	sprintf(Message_Details.Time_Received,"%s\t",TimeReceived);
	sprintf(Message_Details.Message_Body,"%s\r\n",MessageBody);
}


int Check_No_of_message_In_Memory(int MEMORY)
{
	char unit[15]= {0};
	uint8_t count=0,n=0,comma=0,Value_To_Return=0;
	clear_buffer();
	//Check sim memory
	HAL_UART_Transmit(&GsmHuart,(uint8_t*)SimMemory,strlen(SimMemory),HAL_MAX_DELAY);
	HAL_UART_Receive(&GsmHuart,(uint8_t*)Reply,100,1000);
	HAL_Delay(100);
	if(Find_in_Text("CPMS",Reply)==Text_OK)
	{
		while(n<=strlen(Reply))
		{
			if(isdigit(Reply[n])!=0)
			{
				unit[count]=Reply[n];
				count++;
			}

			if(Reply[n]==',')
			{

				if(comma==0)
				{
					No_of_messages=atoi(unit);
					count=0;
					if(MEMORY==INBOX_MEM)
					{
						Value_To_Return= No_of_messages;
						break;
					}
				}
				if(comma==1 &&MEMORY==CAPACITY_MEM)
				{
					No_of_messages=atoi(unit);
					Value_To_Return= No_of_messages;
					break;
				}
				{
					No_of_messages=0;
				}
				comma++;
			}
			n++;

		}
	}
	return Value_To_Return;
}


int Send_Message(char Phone_Number[13],char *TextMsg)
{
	int Ctrl_z=26;
	char ch[1];
	char Phone[30];
	ch[0]=Ctrl_z;
	strcpy(Phone,"AT+CMGS=\"");
	strcat(Phone,Phone_Number);
	strcat(Phone,"\"\r\n");
	HAL_UART_Transmit(&GsmHuart,(uint8_t*)Phone,strlen(Phone),HAL_MAX_DELAY);
	HAL_Delay(1000);
	HAL_UART_Transmit(&GsmHuart,(uint8_t*)TextMsg,strlen(TextMsg),HAL_MAX_DELAY);
	HAL_Delay(1000);
	HAL_UART_Transmit(&GsmHuart,(uint8_t*)ch,1,HAL_MAX_DELAY);
	HAL_Delay(5000);
	return SMS_SENT;
}

void clear_MessageDetails(Message_Typedef *Message_Storage)
{
	memcpy(Message_Storage->Date_Received,NULL,strlen(Message_Storage->Date_Received));
	memcpy(Message_Storage->Message_Body,NULL,strlen(Message_Storage->Message_Body));
	memcpy(Message_Storage->Message_Sender,NULL,strlen(Message_Storage->Message_Sender));
	memcpy(Message_Storage->Time_Received,NULL,strlen(Message_Storage->Time_Received));
}

void DeleteSingleMessage(int SMS_NUMBER)
{
	char msgnumber[30];
	HAL_UART_Transmit(&GsmHuart,(uint8_t*)Activate_SMS_Mode,strlen(Activate_SMS_Mode),HAL_MAX_DELAY);
	HAL_Delay(2000);
	sprintf(msgnumber,"AT+CMGD=%d\r\n",SMS_NUMBER);
	HAL_UART_Transmit(&GsmHuart,(uint8_t*)msgnumber,strlen(msgnumber),HAL_MAX_DELAY);
	HAL_Delay(1000);
}


void DeleteAllMessages(void)
{
	clear_buffer();
	HAL_UART_Transmit_IT(&GsmHuart,(uint8_t*)Activate_SMS_Mode,strlen(Activate_SMS_Mode));
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&GsmHuart,(uint8_t*)DeleteAllSms,strlen(DeleteAllSms));
	HAL_Delay(3000);
}

int Read_Delete_LastMsg(int READ_DELETE)
{
	int No_Sms_Inbox=0;
	No_Sms_Inbox=Check_No_of_message_In_Memory(INBOX_MEM);
	if(No_Sms_Inbox>=20)
	{
		DeleteAllMessages();
	}
	switch(No_Sms_Inbox)
	{
	case 0:
		clear_MessageDetails(&Message_Details);
		return 0;
		break;
	case 1:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(1);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(1);
		}
		break;
	case 2:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(2);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(2);
		}
		break;
	case 3:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(3);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(3);
		}
		break;
	case 4:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(4);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(4);
		}
		break;
	case 5:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(5);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(5);
		}
		break;
	case 6:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(6);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(6);
		}
		break;
	case 7:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(7);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(7);
		}
		break;

	case 8:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(8);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(8);
		}
		break;

	case 9:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(9);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(9);
		}
		break;

	case 10:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(10);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(10);
		}
		break;

	case 11:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(11);
		}

		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(11);
		}
		break;
	case 12:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(12);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(12);
		}
		break;
	case 13:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(13);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(13);
		}

		break;
	case 14:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(14);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(14);
		}

		break;
	case 15:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(15);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(15);
		}
		break;
	case 16:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(16);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(16);
		}
		break;
	case 17:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(17);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(17);
		}
		break;
	case 18:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(18);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(18);
		}
		break;
	case 19:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(19);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(19);
		}
		break;
	case 20:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(20);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(20);
		}
		break;
	case 21:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(21);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(21);
		}
		break;
	case 22:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(22);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(22);
		}
		break;
	case 23:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(23);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(23);
		}
		break;
	case 24:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(24);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(24);
		}
		break;
	case 25:
		if(READ_DELETE==READ)
		{
			ReadSmsInbox(25);
		}
		if(READ_DELETE==DELETE)
		{
			DeleteSingleMessage(25);
		}
		break;

	}
	return 1;

}

int Find_in_Text(char Search_in_name[20],char *string)
{

	int n=0,q=0,match=0;
	char test[20]={0};
	char Search_name[20]={0};
	strcpy(Search_name,Search_in_name);
	for(n=0; n<=strlen(string); n++)
	{
		if(string[n]==Search_name[0])
		{
			q=n;
			match=0;
			while(match<strlen(Search_name))
			{
				test[match]=string[q];
				match++;
				q++;
			}
			if(memcmp(Search_name,test,strlen(Search_name))==0)
			{
				return Text_OK;
			}
			else
			{
				return Text_NOT_OK;
			}
		}
	}
	return Search_OK;
}

void Message_Interrupt(void)
{
	HAL_UART_Transmit(&GsmHuart,(uint8_t*)ReciveMsg,strlen(ReciveMsg),100);
	HAL_Delay(1000);
}

