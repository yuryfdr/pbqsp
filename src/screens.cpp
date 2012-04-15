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
#include "screens.h"
#include "pbtk/pbfilechooser.h"

MainScreen mainScreen("mainScreen", 0);

enum mmm {
  MAINMENU_OPEN = 100,
  MAINMENU_FONT,
  MAINMENU_SELECTFONT,
  MAINMENU_ORIENTATION,
  MAINMENU_QUICKSAVE,
  MAINMENU_QUICKLOAD,
  MAINMENU_RESTART,
  MAINMENU_ABOUT,
  MAINMENU_EXIT
};

static imenu fontSizeMenu[] = {
  {ITEM_ACTIVE, 16, "16", NULL},
  {ITEM_ACTIVE, 18, "18", NULL},
  {ITEM_ACTIVE, 20, "20", NULL},
  {ITEM_ACTIVE, 22, "22", NULL},
  {ITEM_ACTIVE, MAINMENU_SELECTFONT, "Выбрать шрифт", NULL},
  {0, 0, NULL, NULL}
};

static imenu mainMenu[] = {
  {ITEM_HEADER, 0, "QSP", NULL},
  {ITEM_ACTIVE, MAINMENU_OPEN, "Открыть книгу", NULL},
  {ITEM_SUBMENU, MAINMENU_FONT, "Шрифт", fontSizeMenu},
  {ITEM_ACTIVE, MAINMENU_ORIENTATION, "Ориентация", NULL},
  {ITEM_ACTIVE, MAINMENU_QUICKSAVE, "Быстрое сохранение", NULL},
  {ITEM_ACTIVE, MAINMENU_QUICKLOAD, "Быстрая загрузка", NULL},
  {ITEM_ACTIVE, MAINMENU_RESTART, "Начать заново", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, MAINMENU_ABOUT, "О...", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, MAINMENU_EXIT, "Выход", NULL},
  {0, 0, NULL, NULL}
};

bool IsQuestOpened()
{
  return QSPGetCurLoc() != 0;
}

bool CompareStr(std::string str1, char *str2)
{
  if (str2 == 0)
    return str1.size() == 0;
  return str1 == str2;
}

/*void dir_selected(char *path)
{
  std::string fileName;

  DIR *dir = iv_opendir(path);
  if (dir != 0){
    struct dirent *ep;
    printf("%s\n",path);
    while (ep = iv_readdir(dir)){
      std::string ext = GetFileExtension(ep->d_name);
      if (ext == "QSP" || ext == "qsp" || ext == "GAM" || ext == "gam"){
        fileName += path;
        fileName += "/";
        fileName += ep->d_name;
        break;
      }
    }
    iv_closedir(dir);
  }else
    Message(ICON_ERROR, "Error", "Не удалось открыть выбранный каталог", 2000);
  printf("%s\n",fileName.c_str());
  if (fileName.size() == 0)
    Message(ICON_ERROR, "Error", "Не найден файл qsp или gam", 2000);
  else{
    //fileName = utf8_to((const unsigned char *)fileName.c_str(), koi8_to_unicode);
    if (!QSPLoadGameWorld(fileName.c_str()))
      ShowError();
    chdir(GetQuestPath().c_str());
    QSPRestartGame(QSP_TRUE);
  }
}*/
void dir_selected(int isok, PBFileChooser * fc)
{
  if (isok) {
    std::string path = fc->getPath();
    if (!QSPLoadGameWorld(path.c_str()))
      ShowError();
    chdir(GetQuestPath().c_str());
    QSPRestartGame(QSP_TRUE);
    mainScreen.getGameScreen()->hideImageScreen();
  }
}

void SetDefaultFont(std::string name, int size)
{
  ifont *font = OpenFont((char *)name.c_str(), size, 1);
  ifont *oldFont = defaultFont;
  defaultFont = font;
  mainScreen.setWidgetFont(defaultFont);
  mainScreen.updateUI();
  if (oldFont != 0)
    CloseFont(oldFont);
}

void font_selected(char *fontr, char *fontb, char *fonti, char *fontbi)
{
  std::string font(fontr);

  size_t div_pos = font.find_first_of(',');
  if (div_pos != std::string::npos) {
    int size = atoi(font.substr(div_pos + 1).c_str());
    SetDefaultFont(font.substr(0, div_pos), size);
  }
}

void orientation_selected(int direction)
{
  SetOrientation(direction);
  mainScreen.updateUI(true);
}

void HandleRestartDialog(int button)
{
  if (button == 1 && IsQuestOpened()) {
    if (!QSPRestartGame(QSP_TRUE))
      ShowError();
  }
}

bool GetVarValue(const QSP_CHAR * name, int *num, QSP_CHAR ** str)
{
  if (QSPGetVarValuesCount(name, num) && *num) {
    QSPGetVarValues(name, 0, num, str);
    return true;
  }
  return false;
}

//static char dirbuf[1024];
void HandleMainMenuItem(int index)
{
  std::cerr<<__PRETTY_FUNCTION__<<index<<std::endl;
  std::string fileName;
  IntEventProcessed = true;
  switch (index) {
  case MAINMENU_OPEN:
    //OpenDirectorySelector("Выберите каталог", dirbuf, 1024, dir_selected);
    OpenFileChooser("Открыть игру", IsQuestOpened()?GetQuestPath().c_str():"/mnt/ext1", "{*.qsp|*.gam}\n*", 0,
                    (pb_dialoghandler) dir_selected);
    break;
  case MAINMENU_QUICKSAVE:
    if (!IsQuestOpened()) {
      Message(ICON_INFORMATION, "QSP",
              "Перед загрузкой состояния необходимо открыть книгу",
              3000);
      break;
    }
    int numVal;
    QSP_CHAR *strVal;
    if (!(GetVarValue(QSP_FMT("NOSAVE"), &numVal, &strVal) && numVal)) {
      SendQSPEvent(QSP_EVT_SAVEGAME, "quicksave.sav");
    } else {
      Message(ICON_INFORMATION, "QSP",
              "Возможность сохранения отключена", 3000);
    }
    break;
  case MAINMENU_QUICKLOAD:
    if (!IsQuestOpened()) {
      Message(ICON_INFORMATION, "QSP", "Нет открытой книги", 3000);
      break;
    }
    SendQSPEvent(QSP_EVT_OPENSAVEDGAME, "quicksave.sav");
    break;
  case MAINMENU_RESTART:
    if (!IsQuestOpened()) {
      Message(ICON_INFORMATION, "QSP", "Нет открытой книги", 3000);
      break;
    }
    Dialog(ICON_QUESTION, "QSP",
           "Вы действительно хотите начать заново?", "Да",
           "Нет", HandleRestartDialog);

    break;
  case MAINMENU_EXIT:
    CloseApp();
    break;
  case MAINMENU_SELECTFONT:
    OpenFontSelector("Выберите шрифт", (char *)std::string(defaultFont->name).c_str(),
                     defaultFont->size, font_selected);
    break;
  case MAINMENU_ORIENTATION:
    OpenRotateBox(orientation_selected);
    break;
  case MAINMENU_ABOUT:
    Message(ICON_INFORMATION, "QSP", "QspPlayer for PocketBook v"
            APP_VERSION
            "\nАлександр Грибанов (AI)© 2009\n"
            "Юрий Федорченко (yuryfdr)© 2011-2012", 5000);
    break;
  default:
    if(-1!=index)
    SetDefaultFont(defaultFont->name, index);
    break;
  }
}

 MainScreen::MainScreen(std::string name, PBWidget * parent):PBWidget(name, parent),
gameScreen("gameScreen", this)
{
  links_in_act_dialog = false;
  _drawBorder = false;
  addWidget(&gameScreen);

  gameScreen.setVisible(false);
}

int MainScreen::handle(int type, int par1, int par2)
{
  if (type == EVT_EXIT) {
    QSPCallbacks::CloseFile(NULL);
    CloseApp();
  } else if (type == EVT_SHOW) {
    setFocused(true);

    gameScreen.setVisible(true);
    gameScreen.setFocused(true);
    placeWidgets();
    updateUI();
  }
  return PBWidget::handle(type, par1, par2);
}

void MainScreen::placeWidgets()
{
  printf("%s\n", __PRETTY_FUNCTION__);
#ifdef NETBOOK                  //netbook debuh height
  setSize(0, 0, ScreenWidth(), 580);
  gameScreen.setSize(0, 0, ScreenWidth(), 580);
#else
  setSize(0, 0, ScreenWidth(), ScreenHeight());
  gameScreen.setSize(0, 0, ScreenWidth(), ScreenHeight());
#endif
}

long oldFullRefreshCount = 0;
bool IsFullRefresh()
{
  return oldFullRefreshCount != QSPGetFullRefreshCount();
}

void MainScreen::updateUI(bool forceUpdate)
{
  if (IsFullRefresh())
    SendQSPEvent(QSP_EVT_SAVEGAME, "autosave.sav");

  bool updateNeeded = gameScreen.reload();
  if (forceUpdate || updateNeeded) {
    update(forceUpdate);
  }
  oldFullRefreshCount = QSPGetFullRefreshCount();
}

GameScreen *MainScreen::getGameScreen()
{
  return &gameScreen;
}

void GameScreen::initMessage()
{
  if (!messageDialog) {
    messageDialog = new MessageDialog();
    messageDialog->onQuit.connect(sigc::mem_fun(this, &GameScreen::message_end));
  }
}

 GameScreen::GameScreen(std::string name, PBWidget * parent):PBWidget(name, parent),
menuButton("QSP", this), commandBoxButton(" K ", this),
objectsButton("objectsButton", this), versionLabel("versionLabel", this),
locationDescription("locationDescription", this), actionsDialog("actionsDialog", this),
objectsScreen("objectsScreen", this), imageScreen("imageScreen", this), messageDialog(NULL)
{
  _drawBorder = false;
  _leaveOnKeys = false;
  objectsScreen.onLeave.connect(sigc::mem_fun(this, &GameScreen::DialogLeavedHandler));
  //imageScreen.onLeave.connect(sigc::mem_fun(this,  &GameScreen::DialogLeavedHandler));

  addWidget(&actionsDialog);
  addWidget(&menuButton);
  addWidget(&commandBoxButton);
  addWidget(&objectsButton);
  addWidget(&locationDescription);
  addWidget(&objectsScreen);
  addWidget(&imageScreen);

  objectsScreen.setVisible(false);
  objectsButton.onPress.connect(sigc::mem_fun(this, &GameScreen::ButtonPressedHandler));

  imageScreen.setVisible(false);

  menuButton.onPress.connect(sigc::mem_fun(this, &GameScreen::ButtonPressedHandler));
  //menuButton.setFocused(true);
  commandBoxButton.onPress.connect(sigc::mem_fun(this, &GameScreen::ButtonPressedHandler));
}

void GameScreen::ActionExecutedHandler(PBWidget * sender)
{
  reload();
  update();
}

void GameScreen::ButtonPressedHandler(PBWidget * sender)
{
  if (sender == &menuButton) {
    IntEventProcessed = false;
    OpenMenu(mainMenu, 0, menuButton.x(), menuButton.y(), HandleMainMenuItem);
  } else if (sender == &commandBoxButton) {
    showCommandBox();
  } else if (sender == &objectsButton) {
    switchObjectsScreen();
  }
}

void GameScreen::DialogLeavedHandler(PBWidget * sender, bool next)
{
  if (sender == &objectsScreen)
    switchObjectsScreen();
  else if (sender == &imageScreen) {
    sender->setVisible(false);
    if (objectsScreen.isVisible())
      objectsScreen.setFocused(true);
    else
      actionsDialog.setFocused(true);
    update();
  }
}

void GameScreen::placeWidgets()
{
  int buttonsHeight = 38;
  int left = x() + BORDER_SPACE, top = y() + BORDER_SPACE,
      width = w() - BORDER_SPACE * 2, height = h() - BORDER_SPACE * 2;
  //int left = x(), top = y(), width = w(), height = h();

  menuButton.setMaxWidth(width / 4);
  menuButton.setSize(left, top, 0, buttonsHeight);
  if (!commandBoxButton.isVisible())
    objectsButton.setSize(menuButton.x() + menuButton.w() + BORDER_SPACE, top,
                          width - menuButton.w() - BORDER_SPACE, buttonsHeight);
  else {
    commandBoxButton.setMaxWidth(width / 4);
    commandBoxButton.setSize(menuButton.x() + menuButton.w() + BORDER_SPACE, top, 0, buttonsHeight);
    objectsButton.setSize(menuButton.x() + menuButton.w() + commandBoxButton.w() + BORDER_SPACE * 2,
                          top, width - menuButton.w() - commandBoxButton.w() - BORDER_SPACE * 2,
                          buttonsHeight);
  }
  if (actionsDialog.isVisible()) {
    if (imageScreen.isVisible()) {
      imageScreen.setSize(left, top + buttonsHeight + BORDER_SPACE, width / 3,
                          height * 5 / 7 - buttonsHeight - BORDER_SPACE);
      left += imageScreen.w();
      width -= imageScreen.w();
    }
    locationDescription.setSize(left, top + buttonsHeight + BORDER_SPACE, width,
                                height * 5 / 7 - buttonsHeight - BORDER_SPACE);

    left = x() + BORDER_SPACE;
    width = w() - BORDER_SPACE * 2;
    actionsDialog.setSize(left, locationDescription.y() + locationDescription.h() + BORDER_SPACE,
                          width, height - locationDescription.h() - buttonsHeight - BORDER_SPACE);
  } else {
    if (imageScreen.isVisible()) {
      imageScreen.setSize(left, top + buttonsHeight + BORDER_SPACE, width,
                          height - buttonsHeight - BORDER_SPACE);
      left += imageScreen.w();
      width -= imageScreen.w();
    }
    locationDescription.setSize(left, top + buttonsHeight + BORDER_SPACE, width,
                                height - buttonsHeight - BORDER_SPACE);

    left = x() + BORDER_SPACE;
    width = w() - BORDER_SPACE * 2;
    actionsDialog.setSize(left, locationDescription.y() + locationDescription.h() + BORDER_SPACE,
                          width, 10 - BORDER_SPACE);
  }
  left = x() + BORDER_SPACE;
  width = w() - BORDER_SPACE * 2;
  objectsScreen.setSize(left, top + buttonsHeight + BORDER_SPACE, width,
                        height - buttonsHeight - BORDER_SPACE);
  //imageScreen.setSize(left, top, width, height);

  versionLabel.setSize(objectsButton.x() + objectsButton.w() - 140 - BORDER_SPACE,
                       objectsButton.y() + BORDER_SPACE, 140, objectsButton.h() - BORDER_SPACE * 2);
  versionLabel.setVisible(!IsQuestOpened());
}

void GameScreen::switchObjectsScreen()
{
  if (objectsScreen.isVisible()) {
    objectsScreen.setVisible(false);
    locationDescription.setVisible(true);
    if (mainScreen.show_act_dlg) {
      actionsDialog.setVisible(true);
      actionsDialog.setFocused(true);
    } else
      locationDescription.setFocused(true);
    if (image_shown)
      imageScreen.setVisible(true);
    update(true);
  } else {
    objectsScreen.setVisible(true);
    locationDescription.setVisible(false);
    actionsDialog.setVisible(false);
    objectsScreen.setFocused(true);
    image_shown = imageScreen.isVisible();
    imageScreen.setVisible(false);
    update(true);
  }
}

static char commandBuf[COMMAND_BUF_SIZE + 1] = "";
static std::string lastCommand;
void HandleCommandBox(char *s)
{
  std::string text = utf8_to(s, koi8_to_unicode);

  SendQSPEvent(QSP_EVT_SETUSERINPUT, text);

  SendQSPEvent(QSP_EVT_EXECUSERINPUT);

  if (s != 0)
    lastCommand.assign(s);
  else
    lastCommand.clear();
}

void GameScreen::showCommandBox()
{
  SetStringToCharString(commandBuf, lastCommand, COMMAND_BUF_SIZE);
  OpenKeyboard("Введите команду", commandBuf, COMMAND_BUF_SIZE / 2, KBDOPTS,
               HandleCommandBox);
}

int GameScreen::handle(int type, int par1, int par2)
{

  int handled = PBWidget::handle(type, par1, par2);

  if (!handled && type == EVT_KEYPRESS) {
    switch (par1) {
    case KEY_MENU:
      IntEventProcessed = false;
      OpenMenu(mainMenu, 0, menuButton.x(), menuButton.y(), HandleMainMenuItem);
      return 1;
    case KEY_BACK:
      showCommandBox();
      return 1;
    case KEY_DELETE:
      // show|hide objectsScreen
      switchObjectsScreen();
      break;
    }
  }

  return handled;
}

void GameScreen::update(bool refresh)
{
  PBWidget::update(refresh);
  FineUpdate();
}

bool GameScreen::reload()
{
  bool updateNeeded = false;

  char objButtonCaptionBuf[40];
  sprintf(objButtonCaptionBuf, "Предметы: %d",
          objectsScreen.getObjectsDialog()->getObjectsCount());
  std::string objButtonCaption(objButtonCaptionBuf);

  std::string curObject(objectsScreen.getObjectsDialog()->getCurrentObjectDesc());
  if (curObject.size() > 0) {
    objButtonCaption += " | ";
    objButtonCaption += curObject;
  }
  objectsButton.setText(objButtonCaption);

  updateNeeded = locationDescription.reload();
  if (updateNeeded) {
    actionsDialog.reload(true);
    if (mainScreen.links_in_act_dialog) {
      links_vector links = locationDescription.getLinks();
      for (link_it it = links.begin(); it != links.end(); it++)
        actionsDialog.addLinkItem(it->first, it->second);
    }
    objectsScreen.reload();
    return true;
  } else
    updateNeeded = actionsDialog.reload();

  updateNeeded = objectsScreen.reload() || updateNeeded;

  return updateNeeded;
}

std::string GameScreen::getLastCommand()
{
  return lastCommand;
}

void GameScreen::setLastCommand(std::string value)
{
  lastCommand = value;
}

void GameScreen::showWindow(int window, bool show)
{
  switch (window) {
  case QSP_WIN_INPUT:
    commandBoxButton.setVisible(show);
    break;
  case QSP_WIN_OBJS:
    //objectsButton.setVisible(show);
    //objectsScreen.setVisible(show);
    break;
  case QSP_WIN_ACTS:
    if (!mainScreen.links_in_act_dialog) {
      actionsDialog.setVisible(show);
      mainScreen.show_act_dlg = show;
    }
    break;
  }
  update_needed = true;
  //update(true);
}

void GameScreen::showImage(boost::shared_ptr < PBImage > image)
{
  imageScreen.setImage(image);
  imageScreen.setVisible(true);
  update(true);
}

 ObjectsScreen::ObjectsScreen(std::string name, PBWidget * parent):PBWidget(name, parent),
objectsDialog("objectsDialog", this), additionalDescription("additionalDescription",
                                                            this)
{
  _drawBorder = false;
  addWidget(&objectsDialog);
  addWidget(&additionalDescription);
}

void ObjectsScreen::placeWidgets()
{
  objectsDialog.setSize(x(), y(), w(), h() / 2);
  additionalDescription.setSize(x(), objectsDialog.y() + objectsDialog.h() + BORDER_SPACE, w(),
                                h() - objectsDialog.h() - BORDER_SPACE);
}

bool ObjectsScreen::reload()
{
  bool updateNeeded = false;
  updateNeeded = objectsDialog.reload() || updateNeeded;
  updateNeeded = additionalDescription.reload() || updateNeeded;
  return updateNeeded;
}

ObjectsDialog *ObjectsScreen::getObjectsDialog()
{
  return &objectsDialog;
}

int LocationDescription::handle(int type, int par1, int par2)
{
  int ret = PBListBox::handle(type, par1, par2);
  if (!ret && type == EVT_KEYPRESS) {
    switch (par1) {
    case KEY_OK:{
        PBListBoxItem *item = (PBListBoxItem *) getFocusedWidget();
        if (item == 0 || (item->y() + item->h() > y() + h()) || item->y() < y()) {
          return 0;
        }
        std::cerr << "Tag:" << item->getTag() << std::endl;
        if (item->getTag().size() > 5) {
          if (item->getTag().substr(5, 5) == "exec:" || item->getTag().substr(5, 5) == "EXEC:") {
            SendQSPEvent(QSP_EVT_EXECSTRING, item->getTag().substr(5 + 5));
          }
        } else {
          SendQSPEvent(QSP_EVT_EXECSELACTION);
        }
        return 1;
      }
    }
  }
  if (!ret && type == EVT_POINTERUP && eventInside(par1, par2)) {
    for (lbitem_cit it = _items.begin(); it < _items.end() ; ++it) {
      if ((*it)->eventInside(par1, par2)) {
        if ( (*it)->canBeFocused()) {
          (*it)->setFocused(true);
          update();
          if ((*it)->getTag().size() > 5) {
            if ((*it)->getTag().substr(5, 5) == "exec:" || (*it)->getTag().substr(5, 5) == "EXEC:") {
              SendQSPEvent(QSP_EVT_EXECSTRING, (*it)->getTag().substr(5 + 5));
            }
          } else {
            SendQSPEvent(QSP_EVT_EXECSELACTION);
          }
          return 1;
        }
      }
    }
  }
  return 0;
}

bool LocationDescription::reload()
{
  if (QSPGetMainDesc() == 0) {
    scrollDelta = 0;
    clear();
    bool updateNeeded = _rawValue.size() > 0;
    _rawValue.clear();
    return updateNeeded;
  }
  if (QSPIsMainDescChanged() || _rawValue != QSPGetMainDesc()) {
    scrollDelta = 0;
    //int ois=_items.size();
    clear();
    _rawValue = QSPGetMainDesc();
    ParseTextH(QSPGetMainDesc(), *this, _links);
    // scroll if text was added
    if (QSPIsMainDescChanged() && !IsFullRefresh() && _items.size() > 0){
      std::cerr<<"add text need scroll!"<<ois<<'\t'<<_items.size()<<std::endl;
      placeWidgets();
      selectItem(_items.size()-1);
    }
    return true;
  }
  return false;
}

links_vector LocationDescription::getLinks()
{
  return _links;
}

bool AdditionalDescription::reload()
{
  if (QSPGetVarsDesc() == 0) {
    clear();
    bool updateNeeded = _rawValue.size() > 0;
    _rawValue.clear();
    return updateNeeded;
  }
  if (QSPIsVarsDescChanged() || _rawValue != QSPGetVarsDesc()) {
    scrollDelta = 0;
    clear();
    _rawValue = QSPGetVarsDesc();
    ParseTextH(QSPGetVarsDesc(), *this, _links);
    return true;
  }
  return false;
}

links_vector AdditionalDescription::getLinks()
{
  return _links;
}

 ObjectsDialog::ObjectsDialog(std::string name, PBWidget * parent):PBListBox(name, parent)
    //,FocusedItemChangedSlot(this, &ObjectsDialog::FocusedItemChangedHandler)
{
  //OnFocusedWidgetChanged.connect(&FocusedItemChangedSlot);
}

/*
void ObjectsDialog::FocusedItemChangedHandler(PBWidget *sender)
{
	int index = -1;
	if (listBox.GetFocusedWidget() != 0)
	{
		std::string tag (((PBListBoxItem *)listBox.GetFocusedWidget())->getTag());
		if (tag.size() > 0)
		{
			index = atoi(tag.c_str());
		}
		
	}
	
	if (index != QSPGetSelObjectIndex())
		if (!QSPSetSelObjectIndex(index, QSP_TRUE))
			ShowError();
}
*/

int ObjectsDialog::handle(int type, int par1, int par2)
{
  if (PBListBox::handle(type, par1, par2))
    return 1;
  int index = -1;
  if (type == EVT_KEYPRESS) {
    PBListBoxItem *focusedItem = (PBListBoxItem *) getFocusedWidget();
    switch (par1) {
    case KEY_OK:
      if (focusedItem != 0) {
        std::string tag(focusedItem->getTag());
        if (tag.size() > 0) {
          index = atoi(tag.c_str());
        }

        if (index != QSPGetSelObjectIndex()) {
          SendQSPEvent(QSP_EVT_SETOBJINDEX, "", index);
        } else
          onLeave.emit(this, false);
      } else
        onLeave.emit(this, false);
      break;
    }
  }
  if (type == EVT_POINTERUP && eventInside(par1, par2)) {
    for (lbitem_cit it = _items.begin(); it < _items.end(); ++it) {
      if ((*it)->eventInside(par1, par2)) {
        //if( (*it)->isVisible() && (*it)->GetCanBeFocused() ){
        //(*it)->setFocused(true);
        //WidgetLeaveHandler(this,false);
        //Update();
        //}
        std::string tag((*it)->getTag());
        if (tag.size() > 0) {
          index = atoi(tag.c_str());
        }

        if (index != QSPGetSelObjectIndex()) {
          (*it)->setFocused(true);
          update();
          SendQSPEvent(QSP_EVT_SETOBJINDEX, "", index);
          //(*it)->setFocused(true);
          //Update();
        } else {
          onLeave.emit(this, false);
          update();
        }

        return 1;
      }
    }
  }
  return 0;
}

bool ObjectsDialog::reload()
{
  bool updateNeeded = false;

  long n_objects = QSPGetObjectsCount();
  long sel_index = QSPGetSelObjectIndex();
  char *obj_image, *obj_desc;

  if (QSPIsObjectsChanged())
    updateNeeded = true;
  else {
    if (_rawValues.size() != n_objects)
      updateNeeded = true;
    else
      for (size_t i = 0; i < _rawValues.size(); i++) {
        QSPGetObjectData(i, &obj_image, &obj_desc);
        if (!CompareStr(_rawValues[i], obj_desc)) {
          updateNeeded = true;
          break;
        }
      }
  }

  if (updateNeeded) {
    scrollDelta = 0;
    clear();
    _rawValues.clear();
    for (long i = 0; i < n_objects; i++) {
      QSPGetObjectData(i, &obj_image, &obj_desc);
      _rawValues.push_back(std::string(obj_desc));

      std::string str_desc;
      to_utf8(obj_desc, &str_desc, koi8_to_unicode);
      char tag[20];
      sprintf(tag, "%ld", i);
      PBListBoxItem *newItem = addItem(ClearHTMLTags(str_desc), std::string(tag));

      if (i == sel_index) {
        newItem->setFocused(true);
      }
    }
  }

  return updateNeeded;
}

int ObjectsDialog::getObjectsCount()
{
  return QSPGetObjectsCount();
}

std::string ObjectsDialog::getCurrentObjectDesc()
{
  long sel_index = QSPGetSelObjectIndex();
  char *obj_image, *obj_desc;
  std::string str_desc;

  if (sel_index >= 0) {
    QSPGetObjectData(sel_index, &obj_image, &obj_desc);
    to_utf8(obj_desc, &str_desc, koi8_to_unicode);
  }
  return str_desc;
}

 ActionsDialog::ActionsDialog(std::string name, PBWidget * parent):PBListBox(name, parent)
                                //,
    //FocusedItemChangedSlot(this, &ActionsDialog::FocusedItemChangedHandler)
{
  onFocusedWidgetChanged.connect(sigc::mem_fun(this, &ActionsDialog::FocusedItemChangedHandler));
}

void ActionsDialog::FocusedItemChangedHandler(PBWidget * sender)
{
  int index = -1;
  PBWidget *fc = getFocusedWidget();
  if (fc != 0) {
    //std::string tag (((PBListBoxItem *)listBox.GetFocusedWidget())->getTag());
    //index = atoi(tag.c_str());
    index = atoi(fc->getTag().c_str());
  }
  QSPSetSelActionIndex(index, QSP_FALSE);
}

int ActionsDialog::handle(int type, int par1, int par2)
{
  int ret = PBListBox::handle(type, par1, par2);
  if (!ret && type == EVT_KEYPRESS) {
    switch (par1) {
    case KEY_OK:{
        PBListBoxItem *item = (PBListBoxItem *) getFocusedWidget();
        if (item == 0) {
          return 0;
        }
        if (item->getTag().size() > 5) {
          if (item->getTag().substr(5, 5) == "exec:" || item->getTag().substr(5, 5) == "EXEC:") {
            SendQSPEvent(QSP_EVT_EXECSTRING, item->getTag().substr(5 + 5));
          }
        } else {
          SendQSPEvent(QSP_EVT_EXECSELACTION);
        }
        return 1;
      }
    }
  }
  if (type == EVT_POINTERUP && eventInside(par1, par2)) {
    for (lbitem_cit it = _items.begin() /*+_topItemIndex */ ;
         it < _items.end() /*begin()+_bottomItemIndex+1 */ ; ++it) {

      if ((*it)->eventInside(par1, par2)) {
        if ((*it)->isVisible() && (*it)->canBeFocused()) {
          (*it)->setFocused(true);
          update();
        }
        if ((*it)->getTag().size() > 5) {
          if ((*it)->getTag().substr(5, 5) == "exec:" || (*it)->getTag().substr(5, 5) == "EXEC:") {
            SendQSPEvent(QSP_EVT_EXECSTRING, (*it)->getTag().substr(5 + 5));
          }
        } else {
          SendQSPEvent(QSP_EVT_EXECSELACTION);
        }
        return 1;
      }
    }
  }
  return 0;
}

bool ActionsDialog::reload(bool force)
{
  bool updateNeeded = false;

  long n_actions = QSPGetActionsCount();
  long sel_index = QSPGetSelActionIndex();
  char *act_image, *act_desc;

  if (QSPIsActionsChanged())
    updateNeeded = true;
  else {
    if (_rawValues.size() != n_actions)
      updateNeeded = true;
    else
      for (size_t i = 0; i < _rawValues.size(); i++) {
        QSPGetActionData(i, &act_image, &act_desc);
        if (!CompareStr(_rawValues[i], act_desc)) {
          updateNeeded = true;
          break;
        }
      }
  }

  if (force || updateNeeded) {
    scrollDelta = 0;
    clear();
    _rawValues.clear();
    for (long i = 0; i < n_actions; i++) {
      QSPGetActionData(i, &act_image, &act_desc);
      _rawValues.push_back(std::string(act_desc));

      std::string str_desc;
      to_utf8(act_desc, &str_desc, koi8_to_unicode);
      char tag[20];
      sprintf(tag, "%ld", i);
      PBListBoxItem *newItem = addItem(ClearHTMLTags(str_desc), std::string(tag));

      if (i == sel_index)
        newItem->setFocused(true);
    }
    //fprintf(stderr, "\n ActionsDialog reloaded");
  }
  return updateNeeded;
}

void ActionsDialog::addLinkItem(std::string text, std::string link)
{
  PBListBoxItem *newItem = addItem(text);
  std::string link_tag("link:" + link);
  newItem->setTag(link_tag);
}

void ImageScreen::draw()
{
  clearRegion();
  if (_drawBorder)
    DrawRect(x(), y(), w(), h(), BLACK);
  if (_image)
    //DrawBitmapRect(x()+3, y()+3, w()-3, h()-3, getImage(), ALIGN_CENTER | VALIGN_MIDDLE);
    _image->draw(x() + 3, y() + 3, w() - 6, h() - 6, true);
  if (_drawBorder && _focused)
    DrawRect(x() + BORDER_SPACE / 2, y() + BORDER_SPACE / 2, w() - BORDER_SPACE, h() - BORDER_SPACE,
             BLACK);
}

int ImageScreen::handle(int type, int par1, int par2)
{
  if (type == EVT_KEYPRESS) {
    switch (par1) {
    case KEY_LEFT:
    case KEY_UP:
    case KEY_PREV:
      onLeave.emit(this, false);
      break;
    case KEY_OK:
    case KEY_RIGHT:
    case KEY_DOWN:
    case KEY_NEXT:
      onLeave.emit(this, true);
      break;
    }
  }
  return 1;
}
