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
#include "sys/wait.h"
#include "string.h"
#include "qspcallbacks.h"
#include "screens.h"
#include "qspthread.h"

#ifdef __EMU__
#define KBDOPTS KBD_SCREENTOP
#else
#define KBDOPTS 0
#endif

extern MainScreen mainScreen;

void QSPCallbacks::SetQSPCallbacks()
{
  QSPSetCallBack(QSP_CALL_SETTIMER, (QSP_CALLBACK) & SetTimer);
  QSPSetCallBack(QSP_CALL_REFRESHINT, (QSP_CALLBACK) & RefreshInt);
  QSPSetCallBack(QSP_CALL_SETINPUTSTRTEXT, (QSP_CALLBACK) & SetInputStrText);
  QSPSetCallBack(QSP_CALL_ISPLAYINGFILE, (QSP_CALLBACK) & IsPlay);
  QSPSetCallBack(QSP_CALL_PLAYFILE, (QSP_CALLBACK) & PlayFile);
  QSPSetCallBack(QSP_CALL_CLOSEFILE, (QSP_CALLBACK) & CloseFile);
  QSPSetCallBack(QSP_CALL_SHOWMSGSTR, (QSP_CALLBACK) & Msg);
  QSPSetCallBack(QSP_CALL_SLEEP, (QSP_CALLBACK) & Sleep);
  QSPSetCallBack(QSP_CALL_GETMSCOUNT, (QSP_CALLBACK) & GetMSCount);
  QSPSetCallBack(QSP_CALL_DELETEMENU, (QSP_CALLBACK) & DeleteMenu);
  QSPSetCallBack(QSP_CALL_ADDMENUITEM, (QSP_CALLBACK) & AddMenuItem);
  QSPSetCallBack(QSP_CALL_SHOWMENU, (QSP_CALLBACK) & ShowMenu);
  QSPSetCallBack(QSP_CALL_INPUTBOX, (QSP_CALLBACK) & Input);
  QSPSetCallBack(QSP_CALL_SHOWIMAGE, (QSP_CALLBACK) & ShowImage);
  QSPSetCallBack(QSP_CALL_SHOWWINDOW, (QSP_CALLBACK) & ShowPane);
  QSPSetCallBack(QSP_CALL_OPENGAMESTATUS, (QSP_CALLBACK) & OpenGameStatus);
  QSPSetCallBack(QSP_CALL_SAVEGAMESTATUS, (QSP_CALLBACK) & SaveGameStatus);
}

long timer_interval = 0;
void timer_proc()
{
  if (!QSPExecCounter(QSP_TRUE))
    ShowError();
  SetHardTimer("QSPTIMER", timer_proc, timer_interval);
}

void QSPCallbacks::SetTimer(long msecs)
{
  timer_interval = msecs;
  if (msecs <= 0) {
    ClearTimer(timer_proc);
  } else {
    SetHardTimer("QSPTIMER", timer_proc, msecs);
  }
}

void QSPCallbacks::RefreshInt(QSP_BOOL isRedraw)
{
  std::cerr<<__PRETTY_FUNCTION__<<isRedraw<<std::endl;

  if(isRedraw){
    mainScreen.getGameScreen()->locationDescription.reload();
    mainScreen.getGameScreen()->locationDescription.update();
    usleep(10000);
  }
  else SendIntEvent(INT_EVT_UPDATE);
  //if(isRedraw)SetHardTimer("INTERFACE_EVENTS_TIMER", InterfaceEventsTimerU,1);
}

void QSPCallbacks::SetInputStrText(const QSP_CHAR * text)
{
  std::string text_str;
  to_utf8(text, &text_str, koi8_to_unicode);
  mainScreen.getGameScreen()->setLastCommand(text_str);
}

void QSPCallbacks::ShowPane(long type, QSP_BOOL isShow)
{
  mainScreen.getGameScreen()->showWindow(type, isShow);
}

void QSPCallbacks::Sleep(long msecs)
{
  usleep(msecs);
}

long QSPCallbacks::GetMSCount()
{
  static time_t lastTime = 0;
  time_t now = time(0);
  long interval;

  if (lastTime == 0)
    interval = 0;
  else
    interval = (long)(difftime(now, lastTime) * 1000);

  lastTime = now;

  return interval;
}

void QSPCallbacks::Msg(const QSP_CHAR * str)
{
  std::string text(str);
  SendIntEvent(INT_EVT_MESSAGE, text);
}

static imenu dynamicMenu[DYN_MENU_SIZE];
static int dynamicMenuSize = 0;

void QSPCallbacks::DeleteMenu()
{
  dynamicMenuSize = 0;
  dynamicMenu[0].type = 0;
}

void QSPCallbacks::AddMenuItem(const QSP_CHAR * name, const QSP_CHAR * imgPath)
{
  if (dynamicMenuSize >= DYN_MENU_SIZE)
    return;

  std::string itemName;
  to_utf8(name, &itemName, koi8_to_unicode);
  if (itemName == "-") {
    dynamicMenu[dynamicMenuSize].type = ITEM_SEPARATOR;
    dynamicMenu[dynamicMenuSize].index = 0;
  } else {
    char *text = new char[strlen(itemName.c_str()) + 1];
    strcpy(text, itemName.c_str());
    dynamicMenu[dynamicMenuSize].type = ITEM_ACTIVE;
    dynamicMenu[dynamicMenuSize].index = dynamicMenuSize;
    dynamicMenu[dynamicMenuSize].text = text;
    dynamicMenu[dynamicMenuSize].submenu = 0;
  }

  dynamicMenuSize++;

  dynamicMenu[dynamicMenuSize].type = 0;
}

volatile int curmenuindex;
volatile bool keyb_closed;

void HandleDynamicMenuItem(int index)
{
  SendQSPEvent(QSP_EVT_DONEMENU, "", index);
  IntEventProcessed = true;
}

int QSPCallbacks::ShowMenu()
{
  SendIntEvent(INT_EVT_MENU, "");
  return 0;
}

int RealMenu()
{
  OpenMenu(dynamicMenu, 0, ScreenWidth() / 3, ScreenHeight() / 5, HandleDynamicMenuItem);
  return 0;
}

static char inputBuf[1024] = "";
QSP_CHAR *buff;
long inputMaxLen;
std::string inputTitle;

void keyboard_entry(char *s)
{
  keyb_closed = true;
  if (s) {
    std::string encoded_text = utf8_to(s, koi8_to_unicode);
    SetStringToCharString(buff, encoded_text, inputMaxLen * 2);
  } else {
    buff[0] = '\0';
  }
  inputDone(buff, inputMaxLen * 2);
  IntEventProcessed = true;
}

void RealInput()
{
  OpenKeyboard((char *)inputTitle.c_str(), inputBuf, inputMaxLen, KBDOPTS, keyboard_entry);
}

void QSPCallbacks::Input(const QSP_CHAR * text, QSP_CHAR * buffer, long maxLen)
{
  buff = buffer;
  inputMaxLen = maxLen / 2;
  to_utf8(text, &inputTitle, koi8_to_unicode);
  keyb_closed = false;
  SendIntEvent(INT_EVT_INPUT, inputTitle);
}

void QSPCallbacks::ShowImage(const QSP_CHAR * file)
{
  if (file == 0 || strlen(file) == 0)
    return;
  std::string path;             // = GetQuestPath();
  path = convertPath(std::string(file));
  PBImage* image = PBImage::createFromFile(path.c_str());
  if (image!=0) {
#ifdef HTMLDEBUG
    std::cerr << "before show image:" << file << ":" << path << std::endl;
#endif
    mainScreen.getGameScreen()->showImage(boost::shared_ptr<PBImage>(image));
  }
}

void QSPCallbacks::OpenGameStatus()
{
}

void QSPCallbacks::SaveGameStatus()
{
}

void QSPCallbacks::DeInit()
{
  for (int i = 0; i < DYN_MENU_SIZE; i++)
    delete[]dynamicMenu[i].text;
}
