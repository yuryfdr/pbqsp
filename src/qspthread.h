/* Copyright (C) 2009 AI */
/* Copyright (C) 2011 Yury P. Fedorchenko (yuryfdr at users.sf.net)  */
/*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef QSPTHREAD_H
#define QSPTHREAD_H

#include "helper.h"
#include "qspcallbacks.h"
#include "qsp/qsp.h"
#include <stdlib.h>

#define QSP_EVT_EXIT	101
#define QSP_EVT_OPENGAME	102
#define QSP_EVT_EXECSTRING	103
#define QSP_EVT_EXECSELACTION	104
#define QSP_EVT_RESTART	105
#define QSP_EVT_SAVEGAME	106
#define QSP_EVT_OPENSAVEDGAME	107
#define QSP_EVT_SETUSERINPUT	108
#define QSP_EVT_EXECUSERINPUT	109
#define QSP_EVT_SETOBJINDEX	110
#define QSP_EVT_DONEMENU	111

#define INT_EVT_UPDATE 201
#define INT_EVT_MESSAGE 202
#define INT_EVT_INPUT 203
#define INT_EVT_MENU 204
#define INT_EVT_ERROR 205

void SendQSPEvent(int type, std::string str1 = "", int int1 = 0);
void* QSPThreadProc(void *ptr);
void QSPEventsTimer();

void SendIntEvent(int type, std::string str1 = "");
void InterfaceEventsTimer();
extern bool IntEventProcessed;
void ShowError();
#endif

