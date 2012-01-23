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
#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include "pbtk/convert.h"
#include "pbtk/pbcontrols.h"

#include "qsp/qsp.h"
#include "qsp/bindings/bindings_config.h"

#define APP_VERSION "5.7.0.0_rc2_20120108"

extern ifont *defaultFont,*boldFont,*biggerFont;

void ShowError();
std::string GetQuestPath();
std::string GetFileExtension(const std::string& path);
void ParseTextH(const char *src_text, PBListBox &listBox, std::vector<std::pair<std::string, std::string> > &links);
std::string ClearHTMLTags(std::string &text);
void SetStringToCharString(char *dest,const std::string& src, int dest_size);

inline std::string convertbackslash(const std::string& strin){
  std::string str(strin);
  size_t pos=str.find_first_of("\\");
  while(pos!=str.npos){
    str[pos]='/';
    pos=str.find_first_of("\\");
  }
  return str;
}
inline std::string convertPath(const std::string& strin){
  std::string str = convertbackslash(strin);
  if(str.find(GetQuestPath())!=str.npos){
    std::string path=str.substr(GetQuestPath().size());
    std::cerr<<"Path:"<<path<<":"<<std::endl;
    return GetQuestPath()+to_utf8(path,koi8_to_unicode);
  } else {
    return to_utf8(str,koi8_to_unicode);
  }
  return "/";
}

#endif

