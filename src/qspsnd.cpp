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
#include "sys/wait.h"
#include "string.h"
#include "qspcallbacks.h"
#include "screens.h"
#include "qspthread.h"
#include "pbsound.h"

#ifdef __EMU__
#define KBDOPTS KBD_SCREENTOP
#else
#define KBDOPTS 0
#endif



QSP_BOOL QSPCallbacks::IsPlay(const QSP_CHAR *file)
{
  return PBSound::is_play(file);
}

void QSPCallbacks::CloseFile(const QSP_CHAR *file)
{
  PBSound::stop_play(file);
}

void QSPCallbacks::PlayFile(const QSP_CHAR *file, long volume)
{
  PBSound::play(file,volume);
}

