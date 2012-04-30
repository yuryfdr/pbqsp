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
#include "main.h"
#include "pbtk/pbfilechooser.h"

MainScreen mainScreen("mainScreen", 0);
extern imenu rsntFile[];

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

void MainScreen::openGame(const char*file){
  if (!QSPLoadGameWorld(file)){
    ShowError();
    return;
  }
  chdir(GetQuestPath().c_str());
  QSPOpenSavedGame("autosave.sav",QSP_TRUE);
  addRecent(file);
  //QSPRestartGame(QSP_TRUE);
  getGameScreen()->hideImageScreen();
}

void dir_selected(int isok, PBFileChooser * fc)
{
  if (isok) {
    std::string path = fc->getPath();
    mainScreen.openGame(path.c_str());
  }
}

void SetDefaultFont(std::string name, int size)
{
  ifont *font = OpenFont((char *)name.c_str(), size, 1);
  ifont *oldFont = defFont;
  defFont = font;
  ifont *oldbFont = boldFont;
  boldFont = OpenFont((char *)(name+"-Bold").c_str(), size, 1);
  ifont *oldbigFont = biggerFont;
  biggerFont = OpenFont((char *)(name + "-Bold").c_str(), size + 2, 1);
  mainScreen.setWidgetFont(defFont);
  mainScreen.replaceWidgetFont(oldbigFont,biggerFont); 
  mainScreen.replaceWidgetFont(oldbFont,boldFont); 
  mainScreen.updateUI();
  if (oldFont != 0)
    CloseFont(oldFont);
  if (oldbFont != 0)
    CloseFont(oldbFont);
  if (oldbigFont != 0)
    CloseFont(oldbigFont);
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
  mainScreen.orient=direction;
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
void GameScreen::HandleMainMenuItem(int index)
{
  std::string fileName;
  IntEventProcessed = true;
  switch (index) {
  case MAINMENU_OPEN:
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
  case MAINMENU_AUTOLOAD:
    if (!IsQuestOpened()) {
      Message(ICON_INFORMATION, "QSP", "Нет открытой книги", 3000);
      break;
    }
    SendQSPEvent(QSP_EVT_OPENSAVEDGAME, "autosave.sav");
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
    OpenFontSelector("Выберите шрифт", (char *)std::string(defFont->name).c_str(),
                     defFont->size, font_selected);
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
  case ITM_RSNT0:
  case ITM_RSNT1:
  case ITM_RSNT2:
  case ITM_RSNT3:
  case ITM_RSNT4:
  case ITM_RSNT5:
  case ITM_RSNT6:
  case ITM_RSNT7:
  case ITM_RSNT8:
  case ITM_RSNT9:
    if(rsntFile[index-ITM_RSNT0].text){
      mainScreen.openGame(rsntFile[index-ITM_RSNT0].text);
    }
    break;
  case MAINMENU_LAYOUTOLD:
      mainScreen.screenLayout=MainScreen::OLD_L;
      mainScreen.updateUI(true);
    break;
  case MAINMENU_LAYOUTALL:
      mainScreen.screenLayout=MainScreen::ALL_L;
      jkhfkytd
      mainScreen.updateUI(true);
    break;
  default:
    if(-1!=index)
    SetDefaultFont(defFont->name, index);
    break;
  }
}

MainScreen::MainScreen(std::string name, PBWidget * parent):PBWidget(name, parent),
gameScreen("gameScreen", this),orient(0)
{
  links_in_act_dialog = false;
  _drawBorder = false;
  addWidget(&gameScreen);
  //gameScreen.setVisible(false);
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
    /*if(1){
      objectsScreen.setVisible(true);
    }*/
    placeWidgets();
    updateUI();
  }
  return PBWidget::handle(type, par1, par2);
}

void MainScreen::placeWidgets()
{
  //printf("%s\n", __PRETTY_FUNCTION__);
#ifdef NETBOOK                  //netbook debug height
  if(orient==0 || orient == 3){
    setSize(0, orient==3?220:0, ScreenWidth(), 580);
    gameScreen.setSize(0, orient==3?220:0, ScreenWidth(), 580);
  }else{
    setSize(orient==2?220:0, 0, 580, ScreenHeight());
    gameScreen.setSize( orient==2?220:0, 0, 580, ScreenHeight());
  }
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
  //std::cerr<<__PRETTY_FUNCTION__<<std::endl;
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
  objectsButton("objectsButton", this),
  locationDescription("locationDescription", this), actionsDialog("actionsDialog", this),
  objectsScreen("objectsScreen", this), imageScreen("imageScreen", this), messageDialog(NULL)
{
  _drawBorder = false;
  _leaveOnKeys = false;
  objectsScreen.onLeave.connect(sigc::mem_fun(this, &GameScreen::DialogLeavedHandler));
  objectsDialog=&objectsScreen.objectsDialog;
  additionalDescription=&objectsScreen.additionalDescription;
  addWidget(&actionsDialog);
  addWidget(&menuButton);
  addWidget(&commandBoxButton);
  addWidget(&objectsButton);
  addWidget(&locationDescription);
  addWidget(&objectsScreen);
  addWidget(&imageScreen);

  if(mainScreen.screenLayout==MainScreen::ALL_L)objectsScreen.setVisible(true);
  else objectsScreen.setVisible(false);
  objectsButton.onPress.connect(sigc::mem_fun(this, &GameScreen::ButtonPressedHandler));

  imageScreen.setVisible(false);

  menuButton.onPress.connect(sigc::mem_fun(this, &GameScreen::onMenuButton));
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
  if (sender == &commandBoxButton) {
    showCommandBox();
  } else if (sender == &objectsButton) {
    switchObjectsScreen();
  }
}

void GameScreen::DialogLeavedHandler(PBWidget * sender, bool next)
{
  if (sender == &objectsScreen && 0)
    switchObjectsScreen();
  else if (sender == &imageScreen) {//never?
    std::cerr<<"Never must appeer!"<<std::endl;
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
  int delim=(mainScreen.screenLayout==MainScreen::ALL_L && objectsScreen.isVisible())?2:1;
  int left = x() + BORDER_SPACE, top = y() + BORDER_SPACE,
      width = (w() - BORDER_SPACE * 2)/delim, height = h() - BORDER_SPACE * 2;
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
    width = (w() - BORDER_SPACE * 2)/delim;
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
    width = (w() - BORDER_SPACE * 2)/delim;
    actionsDialog.setSize(left, locationDescription.y() + locationDescription.h() + BORDER_SPACE,
                          width, 10 - BORDER_SPACE);
  }
  width = (w() - BORDER_SPACE * 2)/delim;
  left = (mainScreen.screenLayout==MainScreen::ALL_L)?w()/2:0 + BORDER_SPACE;
  //width = w() - BORDER_SPACE * 2;
  objectsScreen.setSize(left, top + buttonsHeight + BORDER_SPACE, width,
                        height - buttonsHeight - BORDER_SPACE);
}

void GameScreen::switchObjectsScreen()
{
  if(mainScreen.screenLayout==MainScreen::ALL_L)return;
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
      onMenuButton(&menuButton);
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
  std::cerr<<__PRETTY_FUNCTION__<<std::endl;
  PBWidget::update(refresh);
#ifndef _NEW_DEV_
  FineUpdate();
#endif
}

bool GameScreen::reload()
{
  //std::cerr<<__PRETTY_FUNCTION__<<std::endl;
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
  //std::cerr<<__PRETTY_FUNCTION__<<"un"<<updateNeeded<<std::endl;
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

  case QSP_WIN_ACTS:
    if (!mainScreen.links_in_act_dialog) {
      actionsDialog.setVisible(show);
      mainScreen.show_act_dlg = show;
    }
  case QSP_WIN_OBJS:
    objectsButton.setVisible(show);
    objectsDialog->setVisible(show);
    if(!objectsDialog->isVisible() && 
       !additionalDescription->isVisible())
       objectsScreen.setVisible(false);
    else objectsScreen.setVisible(true);
    break;
  case QSP_WIN_VARS:
    additionalDescription->setVisible(show);
    if(!objectsDialog->isVisible() && 
       !additionalDescription->isVisible())
       objectsScreen.setVisible(false);
    else objectsScreen.setVisible(true);
    break;
  default:
    std::cerr<<"not handled:"<<window<<std::endl;
  }
  update_needed = true;
  //update();
}

void GameScreen::showImage(boost::shared_ptr < PBImage > image)
{
  imageScreen.setImage(image);
  imageScreen.setVisible(true);
  update(true);
}

ObjectsScreen::ObjectsScreen(std::string name, PBWidget * parent):PBWidget(name, parent),
objectsDialog("objectsDialog", this), additionalDescription("additionalDescription",this)
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
  //std::cerr<<__PRETTY_FUNCTION__<<std::endl;
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
          //update();
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
  //std::cerr<<__PRETTY_FUNCTION__<<std::endl;
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
      //std::cerr<<"add text need scroll!"<<ois<<'\t'<<_items.size()<<std::endl;
      placeWidgets();
      selectItem(_items.size()-1,false);
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
  //std::cerr<<__PRETTY_FUNCTION__<<std::endl;
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
        std::string tag((*it)->getTag());
        if (tag.size() > 0) {
          index = atoi(tag.c_str());
        }

        if (index != QSPGetSelObjectIndex()) {
          (*it)->setFocused(true);
          //update();
          SendQSPEvent(QSP_EVT_SETOBJINDEX, "", index);
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
  //std::cerr<<__PRETTY_FUNCTION__<<std::endl;
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

      /*if (i == sel_index) {
        newItem->setFocused(true);
      }*/
    }
    selectItem(sel_index,false);
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
{
  onFocusedWidgetChanged.connect(sigc::mem_fun(this, &ActionsDialog::FocusedItemChangedHandler));
}

void ActionsDialog::FocusedItemChangedHandler(PBWidget * sender)
{
  int index = -1;
  PBWidget *fc = getFocusedWidget();
  if (fc != 0) {
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
          //update();
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
  //std::cerr<<__PRETTY_FUNCTION__<<std::endl;
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

      //if (i == sel_index)newItem->setFocused(true);
    }
    selectItem(sel_index,false);
    fprintf(stderr, "\n ActionsDialog reloaded");
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
