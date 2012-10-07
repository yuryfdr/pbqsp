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
#include "main.h"
#include <sstream>

ifont *defFont,*boldFont,*biggerFont;
namespace {
  struct initializer{
    initializer(){
      OpenScreen();
    }
  }iii;
};
extern MainScreen mainScreen;

std::string prevGame;

iconfig *config = 0;
/*#define CONFIG_FONT "Font"
#define CONFIG_GAME "Game"
iconfigedit QSPConfig[] = {
  {CFG_TEXT, NULL, "Шрифт", NULL, CONFIG_FONT, "LiberationSans, 22", NULL, NULL},
  {CFG_TEXT, NULL, "Книга", NULL, CONFIG_GAME, "", NULL, NULL},
  {0, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};*/

imenu rsntFile[] = {
  {0, ITM_RSNT0, NULL, NULL},
  {0, ITM_RSNT1, NULL, NULL},
  {0, ITM_RSNT2, NULL, NULL},
  {0, ITM_RSNT3, NULL, NULL},
  {0, ITM_RSNT4, NULL, NULL},
  {0, ITM_RSNT5, NULL, NULL},
  {0, ITM_RSNT6, NULL, NULL},
  {0, ITM_RSNT7, NULL, NULL},
  {0, ITM_RSNT8, NULL, NULL},
  {0, ITM_RSNT9, NULL, NULL},
  {0, 0, NULL, NULL}
};

static imenu fontSizeMenu[] = {
  {ITEM_ACTIVE, 16, "16", NULL},
  {ITEM_ACTIVE, 18, "18", NULL},
  {ITEM_ACTIVE, 20, "20", NULL},
  {ITEM_ACTIVE, 22, "22", NULL},
  {ITEM_ACTIVE, MAINMENU_SELECTFONT, "Выбрать шрифт", NULL},
  {0, 0, NULL, NULL}
};

static imenu optionsMenu[] = {
  {ITEM_SUBMENU, MAINMENU_FONT, "Шрифт", fontSizeMenu},
  {ITEM_ACTIVE, MAINMENU_ORIENTATION, "Ориентация", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, MAINMENU_LAYOUTOLD, "2 экрана", NULL},
  {ITEM_ACTIVE, MAINMENU_LAYOUTALL, "1 экран", NULL},
  {0, 0, NULL, NULL}
};

static imenu mainMenu[] = {
  {ITEM_HEADER, 0, "QSP", NULL},
  {ITEM_ACTIVE, MAINMENU_OPEN, "Открыть книгу", NULL},
  {ITEM_HIDDEN, MAINMENU_FONT, "Последние открытые", rsntFile},
  {ITEM_ACTIVE, MAINMENU_QUICKSAVE, "Быстрое сохранение", NULL},
  {ITEM_ACTIVE, MAINMENU_QUICKLOAD, "Быстрая загрузка", NULL},
  {ITEM_ACTIVE, MAINMENU_AUTOLOAD, "Загрузка автосохранения", NULL},
  {ITEM_ACTIVE, MAINMENU_RESTART, "Начать заново", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_SUBMENU, MAINMENU_FONT, "Настройки", optionsMenu},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, MAINMENU_ABOUT, "О...", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, MAINMENU_EXIT, "Выход", NULL},
  {0, 0, NULL, NULL}
};

void GameScreen::onMenuButton(PBWidget*){
  IntEventProcessed = false;
  OpenMenu(mainMenu, 0, menuButton.x(), menuButton.y(), HandleMainMenuItem);
}

void MainScreen::addRecent(const char *s)
{
  for (int i = 0; i < 9; ++i) {
    if (rsntFile[i].text && strcmp(rsntFile[i].text, s) == 0) {
      if (i != 0)
        std::swap(rsntFile[i].text, rsntFile[0].text);
      return;
    }
  }
  if (rsntFile[9].text)
    free(rsntFile[9].text);
  for (int i = 8; i >= 0; --i) {
    if (rsntFile[i].text) {
      rsntFile[i + 1].text = rsntFile[i].text;
      rsntFile[i + 1].type = ITEM_ACTIVE;
    }
  }
  rsntFile[0].text = strdup(s);
  rsntFile[0].type = ITEM_ACTIVE;
  mainMenu[2].type = ITEM_SUBMENU;
}

void MainScreen::loadConfig()
{
#ifdef __EMU__
  LoadKeyboard("EN_RU.kbd");
#endif
  config = OpenConfig(CONFIGPATH "/qsp.cfg", NULL/*QSPConfig*/);
  printf("%s\n", CONFIGPATH "/qsp.cfg");
  char *fontName = ReadString(config, "Font", "LiberationSans, 22");
  std::string font(fontName);

  size_t div_pos = font.find_first_of(',');
  if (div_pos != std::string::npos) {
    int size = atoi(font.substr(div_pos + 1).c_str());
    std::string fontname = font.substr(0, div_pos);
    ifont *oldFont = defFont;
    ifont *newFont = OpenFont((char *)fontname.c_str(), size, 1);
    if (newFont)
      defFont = newFont;
    if (oldFont)
      CloseFont(oldFont);
    boldFont = OpenFont((char *)(fontname + "-Bold").c_str(), size, 1);
    biggerFont = OpenFont((char *)(fontname + "-Bold").c_str(), size + 2, 1);
  }
  if (!defFont) {
    defFont = OpenFont(DEFAULTFONT, 18, 1);
    boldFont = OpenFont(DEFAULTFONTB, 18, 1);
    biggerFont = OpenFont(DEFAULTFONTB, 20, 1);
  }

  for (int i = 9; i >= 0; --i) {
    std::stringstream str;
    str << "RF" << i;
    char *s = ReadString(config, str.str().c_str(), NULL);
    if (s && *s) {
      MainScreen::addRecent(s);
      if(i==0 && prevGame.empty()){
        prevGame=s;
      }
    }
  }
  
  if (prevGame.size() == 0) {//old compat remove!
    char *gameName = ReadString(config, "Game", 0);
    if (gameName != 0){
      prevGame = gameName;
      DeleteString(config, "Game");
    }
  }
  orient = ReadInt(config, "Orientation",GetGlobalOrientation());
  screenLayout = (MainScreen::layout)ReadInt(config, "Layout",ALL_L);

  setWidgetFont(defFont);
  SetOrientation(orient);
}

void MainScreen::saveConfig()
{
  char fontName[80];
  sprintf(fontName, "%s,%d", defFont->name, defFont->size);
  WriteString(config, "Font", fontName);

  for (int i = 9; i >= 0; --i) {
//    printf("sr:%d\n",i);
    if (rsntFile[i].text) {
      std::stringstream str1;
      str1 << "RF" << i;
      WriteString(config, str1.str().c_str(), rsntFile[i].text);
    }
  }
  WriteInt(config, "Orientation",orient);
  WriteInt(config, "Layout",screenLayout);

  SaveConfig(config);
  CloseConfig(config);
}

pthread_mutex_t int_mutex;
pthread_mutex_t qsp_mutex;

int main_handler(int type, int par1, int par2)
{

  if (type == EVT_INIT) {
    mainScreen.loadConfig();
    pthread_t qspThread;
    pthread_mutex_init(&int_mutex, NULL);
    pthread_mutex_init(&qsp_mutex, NULL);
    int res = pthread_create(&qspThread, NULL, QSPThreadProc, NULL);

    QSPInit();
    QSPCallbacks::SetQSPCallbacks();

    InterfaceEventsTimer();

    if (prevGame.size() > 0){
      mainScreen.openGame(prevGame.c_str());
    }
  }

  mainScreen.handle(type, par1, par2);

  if (type == EVT_EXIT) {
    mainScreen.saveConfig();
    QSPCallbacks::DeInit();
    QSPDeInit();
    CloseFont(defFont);
    CloseFont(boldFont);
    CloseFont(biggerFont);
  }
  return 0;
}

int main(int argc, char **argv)
{
  if (argc > 1) {
    prevGame = argv[1];
  }

  InkViewMain(main_handler);
  return 0;
}
