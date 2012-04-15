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

ifont *defaultFont;
ifont *boldFont, *biggerFont;
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
#define CONFIG_FONT "Font"
#define CONFIG_GAME "Game"
iconfigedit QSPConfig[] = {
  {CFG_TEXT, NULL, "Шрифт", NULL, CONFIG_FONT, "LiberationSans, 22", NULL, NULL},
  {CFG_TEXT, NULL, "Книга", NULL, CONFIG_GAME, "", NULL, NULL},
  {0, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};

void LoadConfig()
{
#ifdef __EMU__
  LoadKeyboard("EN_RU.kbd");
#endif
  config = OpenConfig(CONFIGPATH "/qsp.cfg", QSPConfig);
  printf("%s\n", CONFIGPATH "/qsp.cfg");
  char *fontName = ReadString(config, CONFIG_FONT, "LiberationSans, 22");
  std::string font(fontName);

  size_t div_pos = font.find_first_of(',');
  if (div_pos != std::string::npos) {
    int size = atoi(font.substr(div_pos + 1).c_str());
    std::string fontname = font.substr(0, div_pos);
    ifont *oldFont = defaultFont;
    ifont *newFont = OpenFont((char *)fontname.c_str(), size, 1);
    if (newFont)
      defaultFont = newFont;
    if (oldFont)
      CloseFont(oldFont);
    boldFont = OpenFont((char *)(fontname + "-Bold").c_str(), size, 1);
    biggerFont = OpenFont((char *)(fontname + "-Bold").c_str(), size + 2, 1);
  }
  if (!defaultFont) {
    defaultFont = OpenFont(DEFAULTFONT, 18, 1);
    boldFont = OpenFont(DEFAULTFONTB, 18, 1);
    defaultFont = OpenFont(DEFAULTFONTB, 20, 1);
  }

  if (prevGame.size() == 0) {
    char *gameName = ReadString(config, CONFIG_GAME, 0);
    if (gameName != 0)
      prevGame = gameName;
  }
}

void SaveConfig()
{
  char fontName[80];
  sprintf(fontName, "%s,%d", defaultFont->name, defaultFont->size);
  WriteString(config, CONFIG_FONT, fontName);

  WriteString(config, CONFIG_GAME, (char *)QSPGetQstFullPath());

  SaveConfig(config);
  CloseConfig(config);
}

pthread_mutex_t int_mutex;
pthread_mutex_t qsp_mutex;

int main_handler(int type, int par1, int par2)
{

  if (type == EVT_INIT) {
    LoadConfig();
    pthread_t qspThread;
    pthread_mutex_init(&int_mutex, NULL);
    pthread_mutex_init(&qsp_mutex, NULL);
    int res = pthread_create(&qspThread, NULL, QSPThreadProc, NULL);

    mainScreen.setWidgetFont(defaultFont);
    SetOrientation(GetGlobalOrientation());

    QSPInit();
    QSPCallbacks::SetQSPCallbacks();

    InterfaceEventsTimer();

    if (prevGame.size() > 0)
      SendQSPEvent(QSP_EVT_OPENGAME, prevGame);
  }

  mainScreen.handle(type, par1, par2);

  if (type == EVT_EXIT) {
    SaveConfig();
    QSPCallbacks::DeInit();
    QSPDeInit();
    CloseFont(defaultFont);
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
