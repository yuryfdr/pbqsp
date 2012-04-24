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
#ifndef SCREENS_H
#define SCREENS_H
#include <stdlib.h>
#include "pbtk/pbcontrols.h"
#include "helper.h"
#include "qspthread.h"
#include "qsp/qsp.h"

#define COMMAND_BUF_SIZE 512

#ifdef __EMU__
#define KBDOPTS KBD_SCREENTOP
#else
#define KBDOPTS 0
#endif

void handleMainMenuItem(int index);

class ObjectsDialog:public PBListBox {
 protected:
  std::vector < std::string > _rawValues;
 public:
  ObjectsDialog(std::string name, PBWidget * parent);

  virtual int handle(int type, int par1, int par2);
  bool reload();
  int getObjectsCount();
  std::string getCurrentObjectDesc();
};

typedef std::vector < std::pair < std::string, std::string > >links_vector;
typedef std::vector < std::pair < std::string, std::string > >::iterator link_it;

class LocationDescription:public PBListBox {
 protected:
  links_vector _links;
  std::string _rawValue;
 public:
  virtual int handle(int type, int par1, int par2);
  LocationDescription(const std::string & name, PBWidget * parent):PBListBox(name, parent) {
    scrollDelta = 0;
    _graylinks = true;
  };
  bool reload();
  links_vector getLinks();
};

class AdditionalDescription:public PBListBox {
 protected:
  links_vector _links;
  std::string _rawValue;
 public:
  AdditionalDescription(const std::string & name, PBWidget * parent):PBListBox(name, parent) {
    setCaption("Дополнительное описание");
  } bool reload();
  links_vector getLinks();
};

class ActionsDialog:public PBListBox {
 protected:
  std::vector < std::string > _rawValues;
  virtual void FocusedItemChangedHandler(PBWidget * sender);
 public:
  ActionsDialog(std::string name, PBWidget * parent);

  virtual int handle(int type, int par1, int par2);
  bool reload(bool force = false);
  void addLinkItem(std::string text, std::string link);
};

class ObjectsScreen:public PBWidget {
 protected:
  ObjectsDialog objectsDialog;
  AdditionalDescription additionalDescription;

  virtual void placeWidgets();
 public:
  ObjectsScreen(std::string name, PBWidget * parent);
  bool reload();

  ObjectsDialog *getObjectsDialog();
};

class ImageScreen:public PBWidget {
 public:
  ImageScreen(std::string name, PBWidget * parent):PBWidget(name, parent) {
  } virtual void setSize(int X, int Y, int W, int H) {
    PBWidget::setSize(X, Y, W, H);
    if (_image && W > _image->w()) {
      w(_image->w());
    }
  }
  virtual void draw();
  virtual int handle(int type, int par1, int par2);
};

class MessageDialog:public PBDialog {
  PBButton bt_ok;
  PBListBox text;
  void on_ok(PBButton *) {
    quit(true);
 } public:
  MessageDialog():PBDialog(""), bt_ok("Ok", this), text("", this) {
    addWidget(&text);
    addWidget(&bt_ok);
    bt_ok.onPress.connect(sigc::mem_fun(this, &MessageDialog::on_ok));
  }
  void setMessage(const std::string & msg);
  void placeWidgets() {
    int sw = ScreenWidth();
    int sh = ScreenHeight();
    int ch = captionHeight();
    setSize(sw / 6, sh / 6 - ch, 2 * sw / 3, 2 * sh / 3);
    text.setSize(x() + 2, y() + ch, w() - 4, h() - 30 - ch);
    text.placeWidgets();
    PBWidget *li = text.getItem(text.itemsCount() - 1);
    if (li && (li->x() + li->h()) < (h() - 32)) {
      setSize(sw / 6, sh / 6 - ch, 2 * sw / 3, (li->x() + li->h()) + 32);
      text.setSize(x() + 2, y() + ch, w() - 4, h() - 30 - ch);
    }
    bt_ok.setSize(x() + w() / 2, y() + h() - 28, w() / 2 - 2, 25);
  }
};

class GameScreen:public PBWidget {
 protected:
  PBButton menuButton;
  PBButton commandBoxButton;
  PBButton objectsButton;
  PBLabel versionLabel;
  LocationDescription locationDescription;
  ActionsDialog actionsDialog;
  ObjectsScreen objectsScreen;
  ImageScreen imageScreen;
 public:
  MessageDialog * messageDialog;
  void initMessage();
  void hideImageScreen() {
    imageScreen.setVisible(false);
  } 
 protected:
  bool image_shown;
  virtual void placeWidgets();
  void switchObjectsScreen();
  void showCommandBox();

  void ActionExecutedHandler(PBWidget * sender);
  void ButtonPressedHandler(PBWidget * sender);
  void onMenuButton(PBWidget*);
  static void HandleMainMenuItem(int index);
  void DialogLeavedHandler(PBWidget * sender, bool next);
  void message_end(PBDialog *, bool);
 public:

  GameScreen(std::string name, PBWidget * parent);

  virtual int handle(int type, int par1, int par2);
  virtual void update(bool refresh = false);

  bool reload();
  std::string getLastCommand();
  void setLastCommand(std::string value);
  void showWindow(int window, bool show);
  void showImage(boost::shared_ptr < PBImage > image);
};

class MainScreen:public PBWidget {
 protected:
  GameScreen gameScreen;
  virtual void placeWidgets();

 public:
  enum layout{
    OLD_L,
    ALL_L
  }screenLayout;
  int orient;
  
  bool links_in_act_dialog;
  bool show_act_dlg;
  MainScreen(std::string name, PBWidget * parent);
  virtual int handle(int type, int par1, int par2);
  void loadConfig();
  void saveConfig();
  static void addRecent(const char*path);
  void openGame(const char*file);
  GameScreen *getGameScreen();
  void updateUI(bool forceUpdate = true);
};

#endif

