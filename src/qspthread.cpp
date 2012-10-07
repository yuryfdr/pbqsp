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
#include "pthread.h"

#include "qspthread.h"

#include "screens.h"
#include "main.h"

extern MainScreen mainScreen;

pthread_t MainThread;

bool QSPEventProcessed;

struct QspEvent {
  int type;
   std::string p_str;
  int p_int;
};
#include <queue>

std::queue < QspEvent > qsp_events;

void SendQSPEvent(int type, std::string str1, int int1)
{
  //pthread_mutex_lock(&qsp_mutex);
  QSPEventProcessed = false;
  QspEvent ev;
  ev.p_str = str1;
  ev.p_int = int1;
  ev.type = type;
  qsp_events.push(ev);
  //pthread_mutex_unlock(&qsp_mutex);
}

void *QSPThreadProc(void *ptr)
{
// (QSPEvent != QSP_EVT_EXIT)
  while (true) {
    //pthread_mutex_lock(&qsp_mutex);
    while (!qsp_events.empty()) {
      QspEvent ev = qsp_events.front();
      switch (ev.type) {
      case QSP_EVT_EXIT:
        return 0;
      case QSP_EVT_OPENGAME:
        if (!QSPLoadGameWorld(ev.p_str.c_str()))
          ShowError();
        else {
          chdir(GetQuestPath().c_str());
          QSPRestartGame(QSP_TRUE);
        }
        break;
      case QSP_EVT_RESTART:
        if (!QSPRestartGame(QSP_TRUE))
          ShowError();
        break;
      case QSP_EVT_SAVEGAME:
        if (!QSPSaveGame((QSP_CHAR *) ev.p_str.c_str(), QSP_FALSE))
          ShowError();
        break;
      case QSP_EVT_OPENSAVEDGAME:
        if (!QSPOpenSavedGame((QSP_CHAR *) ev.p_str.c_str(), QSP_TRUE))
          ShowError();
        break;
      case QSP_EVT_EXECSTRING:
        if (!QSPExecString((const QSP_CHAR *)ev.p_str.c_str(), QSP_TRUE))
          ShowError();
        else if(ev.p_str.find("OPENQST")!=ev.p_str.npos){
          chdir(GetQuestPath().c_str());
        }
        break;
      case QSP_EVT_EXECSELACTION:
        if (!QSPExecuteSelActionCode(QSP_TRUE))
          ShowError();
        break;
      case QSP_EVT_SETOBJINDEX:
        if (!QSPSetSelObjectIndex(ev.p_int, QSP_TRUE))
          ShowError();
        break;
      case QSP_EVT_SETUSERINPUT:
        QSPSetInputStrText(ev.p_str.c_str());
        break;
      case QSP_EVT_EXECUSERINPUT:
        if (!QSPExecUserInput(QSP_TRUE))
          ShowError();
        break;
      case QSP_EVT_DONEMENU:
        QSPSelectMenuItem(ev.p_int);
        break;
      default:
        usleep(500);
      }
      qsp_events.pop();
    }
    QSPEventProcessed = true;
    //pthread_mutex_unlock(&qsp_mutex);
    usleep(500);
  }
  return 0;
}

/*
void QSPEventsTimer()
{
	int QSPEvent1 = QSPEvent;
	
	switch (QSPEvent)
	{
		case QSP_EVT_OPENGAME:
			if (!QSPLoadGameWorld(QSPEventStr1.c_str()))
				ShowError();
			else
			{
				chdir(GetQuestPath().c_str());
				QSPRestartGame(QSP_TRUE);
			}
			break;
			
		case QSP_EVT_RESTART:
			if (!QSPRestartGame(QSP_TRUE))
				ShowError();
			break;
			
		case QSP_EVT_SAVEGAME:
			if (!QSPSaveGame((QSP_CHAR*)QSPEventStr1.c_str(), QSP_FALSE))
				ShowError();
			break;
			
		case QSP_EVT_OPENSAVEDGAME:
			if (!QSPOpenSavedGame((QSP_CHAR*)QSPEventStr1.c_str(), QSP_TRUE))
				ShowError();
			break;
			
		case QSP_EVT_EXECSTRING:
			if (!QSPExecString((const QSP_CHAR *)QSPEventStr1.c_str(), QSP_TRUE))
				ShowError();
			break;
			
		case QSP_EVT_EXECSELACTION:
			if (!QSPExecuteSelActionCode(QSP_TRUE))
				ShowError();
			break;
			
		case QSP_EVT_EXECUSERINPUT:
			if (!QSPExecUserInput(QSP_TRUE))
				ShowError();
			break;
	}
		
	
	if (QSPEvent != QSP_EVT_EXIT)
		SetHardTimer("QSP_EVENTS_TIMER", QSPEventsTimer, 500);
	
	if (QSPEvent != 0 && QSPEvent1 != 0)
	{
		QSPEventProcessed = true;
		QSPEvent = 0;
	}
}
*/
struct IntEvent {
  int type;
  std::string IntEventStr1;
};
bool IntEventProcessed = true;
std::queue < IntEvent > IntEventQue;

void keyboard_entry(char *s);
void RealInput();
void RealMenu();

void SendIntEvent(int type, std::string str1)
{
  pthread_mutex_lock(&int_mutex);
  IntEvent ev;
  ev.type = type;
  ev.IntEventStr1 = str1;
  IntEventQue.push(ev);
  pthread_mutex_unlock(&int_mutex);
}

void dlg_cb(int i)
{
  IntEventProcessed = true;
}

void GameScreen::message_end(PBDialog *, bool)
{
  IntEventProcessed = true;
}

void MessageDialog::setMessage(const std::string & msg)
{
  text.clear();
  std::vector < std::pair < std::string, std::string > >lnks;
  ParseTextH(msg.c_str(), text, lnks);
}

void InterfaceEventsTimerU()
{
  if (IntEventProcessed) {
    pthread_mutex_lock(&int_mutex);
    while (!IntEventQue.empty() ) {
      if(!IntEventProcessed)break;
      IntEvent ev = IntEventQue.front();
      IntEventQue.pop();
      switch (ev.type) {
      case INT_EVT_UPDATE:
        std::cerr<<__PRETTY_FUNCTION__<<"bui"<<std::endl;
        IntEventProcessed = false;
        mainScreen.updateUI(false);
        IntEventProcessed = true;
        break;
      case INT_EVT_INPUT:
        IntEventProcessed = false;
        RealInput();
        break;
      case INT_EVT_MENU:
        IntEventProcessed = false;
        RealMenu();
        break;
      case INT_EVT_ERROR:
        Message(ICON_ERROR, "Error", (char *)ev.IntEventStr1.c_str(), 50000);
        break;
      case INT_EVT_MESSAGE:
        IntEventProcessed = false;
        GameScreen *gs = mainScreen.getGameScreen();
        gs->initMessage();
        static std::string str;
        str = ev.IntEventStr1.c_str();
        gs->messageDialog->setMessage(str);
        gs->messageDialog->run();
        break;
      }
    }
    pthread_mutex_unlock(&int_mutex);
  }
}

void InterfaceEventsTimer(){
  InterfaceEventsTimerU();
  SetHardTimer("INTERFACE_EVENTS_TIMER", InterfaceEventsTimer, 500);
}


