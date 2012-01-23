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

class ObjectsDialog : public PBListBox
{
protected:
	std::vector<std::string> _rawValues;
	
//	CppSlot1<ObjectsDialog, void, PBWidget *> FocusedItemChangedSlot;//
//	virtual void FocusedItemChangedHandler(PBWidget *sender);//
public:
	ObjectsDialog(std::string name, PBWidget *parent);
	
	virtual int handle(int type, int par1, int par2);
	bool reload();
	int getObjectsCount();
	std::string getCurrentObjectDesc();
};

typedef std::vector<std::pair<std::string, std::string> > links_vector;
typedef std::vector<std::pair<std::string, std::string> >::iterator link_it;

class LocationDescription : public PBListBox
{
protected:
  links_vector _links;
  std::string _rawValue;
public:
  virtual int handle(int type, int par1, int par2);
  //virtual void draw();
  LocationDescription(const std::string& name, PBWidget *parent):PBListBox(name, parent){
    scrollDelta=0;
    _graylinks=true;
  };
  bool reload();
  links_vector getLinks();
};

class AdditionalDescription : public PBListBox
{
protected:
  links_vector _links;
  std::string _rawValue;
public:
  AdditionalDescription(const std::string& name, PBWidget *parent):PBListBox(name, parent){
    setCaption("Дополнительное описание");
  }
  bool reload();
  links_vector getLinks();
};

class ActionsDialog : public PBListBox
{
protected:
  std::vector<std::string> _rawValues;
  virtual void FocusedItemChangedHandler(PBWidget *sender);
public:
  ActionsDialog(std::string name, PBWidget *parent);

  virtual int handle(int type, int par1, int par2);
  bool reload(bool force = false);
  void addLinkItem(std::string text, std::string link);
};


class ObjectsScreen : public PBWidget
{
protected:
  ObjectsDialog objectsDialog;
  AdditionalDescription additionalDescription;

  virtual void placeWidgets();
public:
  ObjectsScreen(std::string name, PBWidget *parent);
  bool reload();

  ObjectsDialog *getObjectsDialog();
};

class ImageScreen : public PBWidget
{
public:
  ImageScreen(std::string name, PBWidget *parent): PBWidget(name, parent){}
  virtual void setSize(int X,int Y,int W,int H){
    PBWidget::setSize(X,Y,W,H);
    if(_image && W>_image->w()){
      w(_image->w());
    }
  }
  virtual void draw();
  virtual int handle(int type, int par1, int par2);
};

class GameScreen : public PBWidget
{
protected:
  PBButton menuButton;
  PBButton commandBoxButton;
  PBButton objectsButton;
  PBLabel versionLabel;
  LocationDescription locationDescription;
  ActionsDialog actionsDialog;
  ObjectsScreen objectsScreen;
  ImageScreen imageScreen;
  bool image_shown;
  virtual void placeWidgets();
  void switchObjectsScreen();
  void showCommandBox();

  void ActionExecutedHandler(PBWidget * sender);
  void ButtonPressedHandler(PBWidget * sender);
  void DialogLeavedHandler(PBWidget * sender, bool next);
public:

  GameScreen(std::string name, PBWidget *parent);

  virtual int handle(int type, int par1, int par2);
  virtual void update(bool refresh=false);

  bool reload();
  std::string getLastCommand();
  void setLastCommand(std::string value);
  void showWindow(int window, bool show);
  void showImage(PBImage *image);
};

class MainScreen : public PBWidget
{
protected:
  GameScreen gameScreen;
  virtual void placeWidgets();

public:
  bool links_in_act_dialog;
  bool show_act_dlg;
  MainScreen(std::string name, PBWidget *parent);
  virtual int handle(int type, int par1, int par2);

  GameScreen *getGameScreen();
  void updateUI(bool forceUpdate = true);
};


#endif

