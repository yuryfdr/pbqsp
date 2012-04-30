/* Copyright (C) 2009 AI */
/* Copyright (C) 2011-2012 Yury P. Fedorchenko (yuryfdr at users.sf.net)  */
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
#ifndef MAIN_H
#define MAIN_H

#include "screens.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"
#include "qspcallbacks.h"

#include "qsp/qsp.h"

enum mmm {
  MAINMENU_OPEN = 100,
  MAINMENU_FONT,
  MAINMENU_SELECTFONT,
  MAINMENU_ORIENTATION,
  MAINMENU_LAYOUTALL,
  MAINMENU_LAYOUTOLD,
  MAINMENU_QUICKSAVE,
  MAINMENU_QUICKLOAD,
  MAINMENU_AUTOLOAD,
  MAINMENU_RESTART,
  MAINMENU_ABOUT,
  MAINMENU_EXIT,
  ITM_RESN,//resent files
  ITM_RSNT0,
  ITM_RSNT1,
  ITM_RSNT2,
  ITM_RSNT3,
  ITM_RSNT4,
  ITM_RSNT5,
  ITM_RSNT6,
  ITM_RSNT7,
  ITM_RSNT8,
  ITM_RSNT9
};

extern pthread_mutex_t int_mutex;
extern pthread_mutex_t qsp_mutex;

#endif

