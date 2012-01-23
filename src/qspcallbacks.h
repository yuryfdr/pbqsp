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
#ifndef QSPCALLBACKS_H
#define QSPCALLBACKS_H

#include <vector>
//#include "convert.h"
#include "inkview.h"
#include "qsp/qsp.h"
#include "qspthread.h"

#define DYN_MENU_SIZE 20

class QSPCallbacks
{
public:
	static void SetQSPCallbacks();
	static void RefreshInt(QSP_BOOL isRedraw);
	static void SetTimer(long msecs);
	static void SetInputStrText(const QSP_CHAR *text);
	static QSP_BOOL IsPlay(const QSP_CHAR *file);
	static void CloseFile(const QSP_CHAR *file);
	static void PlayFile(const QSP_CHAR *file, long volume);
	static void ShowPane(long type, QSP_BOOL isShow);
	static void Sleep(long msecs);
	static long GetMSCount();
	static void Msg(const QSP_CHAR *str);
	static void DeleteMenu();
	static void AddMenuItem(const QSP_CHAR *name, const QSP_CHAR *imgPath);
	static int ShowMenu();
	static void Input(const QSP_CHAR *text, QSP_CHAR *buffer, long maxLen);
	static void ShowImage(const QSP_CHAR *file);
	static void OpenGameStatus();
	static void SaveGameStatus();
	static void DeInit();
};

#endif

