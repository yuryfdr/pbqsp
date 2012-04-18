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
#include "helper.h"
#include "qspthread.h"

#include <stdio.h>
#include <stdlib.h>

#include "htmlcxx/html/CharsetConverter.h"
#include "htmlcxx/html/Uri.h"
#include "htmlcxx/html/ParserDom.h"
#include "htmlcxx/html/utils.h"

#include <sstream>

void ShowError()
{
  char message[2048];
  QSP_CHAR *loc;
  int code, where, line;
  QSPGetLastErrorData(&code, &loc, &where, &line);
  const QSP_CHAR *desc = QSPGetErrorDesc(code);
  if (loc)
    sprintf(message,
            "Location: %s\nIndex: %d\nLine: %d\nCode: %d\nDesc: %s\n",
            loc, where, line, code, desc);
  else
    sprintf(message, "Code: %d\nDesc: %s\n", code, desc);
  SendIntEvent(INT_EVT_ERROR, message);
}

std::string GetQuestPath()
{
  std::string fullPath(QSPGetQstFullPath());

  size_t slash_pos = fullPath.find_last_of("/\\");
  if (slash_pos != std::string::npos)
    return fullPath.substr(0, slash_pos + 1);

  fullPath.clear();
  return fullPath;
}

std::string GetFileExtension(const std::string & path)
{
  size_t dot_pos = path.find_last_of(".");
  if (dot_pos != std::string::npos)
    return path.substr(dot_pos + 1);
  return "";
}

std::string ClearHTMLTags(std::string & text)
{
  while (1) {
    size_t pos1 = text.find_first_of('<', 0);
    if (pos1 == std::string::npos)
      break;

    size_t pos2 = text.find_first_of('>', pos1);
    if (pos2 == std::string::npos)
      break;

    text.erase(pos1, pos2 - pos1 + 1);
  }
  return text;
}

struct insert_op {
  ifont *font;
  int align;
  bool is_link;
   std::string tag;
  PBListBoxItem *item;
  bool to_parent;
   insert_op():font(defaultFont), align(8), is_link(false), item(NULL), to_parent(false) {
  } insert_op(const insert_op & i):font(i.font), align(i.align),
      is_link(i.is_link), tag(i.tag), item(NULL), to_parent(i.to_parent) {
  }
};

void ParseChildren(PBListBox & listBox,
                   std::vector < std::pair < std::string,
                   std::string > >&links, tree < htmlcxx::HTML::Node > &tr,
                   tree < htmlcxx::HTML::Node >::iterator & itp, insert_op & iop)
{
  tree < htmlcxx::HTML::Node >::iterator it = itp.begin();
  insert_op ciop = iop;
  for (; tr.is_valid(it) && it != itp.end(); it = tr.next_sibling(it)) {
#ifdef HTMLDEBUG
    std::cerr << it->isTag() << '\t' << tr.depth(it) << "\t:tag:" <<
        to_utf8(it->text().c_str(), koi8_to_unicode) << ":/tag:" << std::endl;
#endif
    if (it->isTag()) {
      ciop.to_parent = false;
      if (it->tagName() == "a" || it->tagName() == "A") {
        ciop.is_link = true;
        it->parseAttributes();
        //std::cerr<<it->attribute("href").second<<'\t'<<(it.begin()!=it.end())<<std::endl;
        if (it.begin() != it.end())
          links.push_back(make_pair
                          (std::string("[") + to_utf8(it.begin()->text().c_str(), koi8_to_unicode)
                           + std::string("]"), it->attribute("href").second));
        else
          ciop.to_parent = true;
        ciop.tag = std::string("link:") + it->attribute("href").second;
      } else if (it->tagName() == "center" || it->tagName() == "CENTER") {
        ciop.align = 2;
      } else if (it->tagName() == "left" || it->tagName() == "LEFT") {
        ciop.align = 1;
      } else if (it->tagName() == "right" || it->tagName() == "RIGHT") {
        ciop.align = 4;
      } else if (it->tagName() == "img" || it->tagName() == "IMG") {
        it->parseAttributes();
        std::string path;       // = GetQuestPath();
        path += it->attribute("src").second;
        to_utf8(path.c_str(), &path, koi8_to_unicode);
#ifdef HTMLDEBUG
        std::cerr << "Before create image:" << path << std::endl;
#endif
        PBImage* image = PBImage::createFromFile(convertbackslash(path).c_str());
        PBListBoxItem *newItem = listBox.addItem(boost::shared_ptr<PBImage>(image), ciop.tag, ciop.align);
        if (!ciop.is_link)
          newItem->setCanBeFocused(false);
      } else if (it->tagName() == "div" || it->tagName() == "DIV") {
        it->parseAttributes();
        if (it->attribute("align").second == "center") {
          ciop.align = 2;
        } else if (it->attribute("align").second == "left") {
          ciop.align = 1;
        } else if (it->attribute("align").second == "right") {
          ciop.align = 4;
        }
      } else if (it->tagName() == "h2"
                 || it->tagName() == "H2" || it->tagName() == "h1" || it->tagName() == "H1") {
        ciop.font = biggerFont;
      } else if (it->tagName() == "h3"
                 || it->tagName() == "H3" || it->tagName() == "h4" || it->tagName() == "H4") {
        ciop.font = boldFont;
      } else if (it->tagName() == "b" || it->tagName() == "B") {
        ciop.font = boldFont;
        ciop.to_parent = true;
      } else if (it->tagName() == "font" || it->tagName() == "FONT") {
        it->parseAttributes();
        std::string sz = it->attribute("size").second;
        int size = atoi(sz.c_str());
        if (sz.size() == 1) {
          if (size > 3)
            ciop.font = biggerFont;
        } else {
          if (size > 0)
            ciop.font = biggerFont;
        }
      }
    } else {
#ifdef HTMLDEBUG
      std::cerr << "parbefore:" << it->text() << ":"<<std::endl;
#endif
      std::string par(to_utf8(it->text().c_str(), koi8_to_unicode));
      if (par.size() > 0) {
        if (ciop.to_parent && (iop.item || ciop.item)) {
          PBListBoxItem *itm = (ciop.item) ? ciop.item : iop.item;
#ifdef HTMLDEBUG
          std::cerr << "item present:" << itm->getText() << std::endl;
#endif
          PBWidget *ctl = new PBWidget(to_utf8(it->text().c_str(),
                                               koi8_to_unicode),
                                       itm);
          ctl->setWidgetFont(ciop.font);
          //if (!ciop.is_link)ctl->setCanBeFocused(false);
          itm->addWidget(ctl);
          if (ciop.item) {
            ciop.item = NULL;
            ciop.to_parent = false;
            ciop.is_link = false;
          }
        } else {
          if (it->text().compare(0, 2, "</") != 0 && it->text()!="\n") {  //skip broken html tags
#ifdef HTMLDEBUG
            std::cerr << "it text:" << it->text() << ":"<<std::endl;
#endif
            std::string tta=to_utf8(it->text().c_str(),koi8_to_unicode);
            
            PBListBoxItem *newItem=NULL;
            if (!ciop.is_link){
              std::stringstream sss(tta);
              int inl=0;
              while(!sss.eof() || !sss.fail()){
                std::string ppar;
                std::getline(sss,ppar);
                if(ppar.empty()){//skip multiple empty lines
                  //std::cerr<<"elc:"<<inl<<std::endl;
                  if((++inl)!=1)continue;
                }else inl=0;
                newItem = listBox.addItem(ppar,ciop.tag,ciop.align);
                //std::cerr<<"add ppar:"<<ppar<<":"<<inl<<":\n";
                newItem->setCanBeFocused(false);
                if (ciop.font != defaultFont)
                  newItem->setWidgetFont(ciop.font);
              }
            }else{
              newItem = listBox.addItem(tta,ciop.tag,ciop.align);
              if (ciop.font != defaultFont)
                newItem->setWidgetFont(ciop.font);
            }

            ciop.item = newItem;
            ciop.to_parent = false;
            ciop.is_link = false;
          }
        }
      }
    }
    ParseChildren(listBox, links, tr, it, ciop);
    ciop.is_link = false;
  }                             //for
}

void ParseTextH(const char *src_text, PBListBox & listBox,
                std::vector < std::pair < std::string, std::string > >&links)
{
  links.clear();

  if (src_text == 0 || strlen(src_text) == 0)
    return;

#ifdef HTMLDEBUG
  std::cerr << "text to parce:" << to_utf8(src_text, koi8_to_unicode) <<":"<<std::endl;
#endif
  std::string text(src_text);   //=to_utf8((const unsigned char *)src_text, koi8_to_unicode);
  size_t aux_symb_pos;
  do {
    aux_symb_pos = text.find_first_of("\r");
    if (aux_symb_pos != std::string::npos)
      text.erase(aux_symb_pos, 1);
  } while (aux_symb_pos != std::string::npos);

  tree < htmlcxx::HTML::Node > tr;
  htmlcxx::HTML::ParserDom parser;
  parser.parse(text);
  tr = parser.getTree();
#ifdef HTMLDEBUG
  std::cerr << tr << std::endl;
#endif
  tree < htmlcxx::HTML::Node >::iterator it = tr.begin();
  for (; tr.is_valid(it) && it != tr.end(); it = tr.next_sibling(it)) {
#ifdef HTMLDEBUG
    std::cerr << it->isTag() << '\t' << tr.depth(it) << "\t:tag:" <<
        to_utf8(it->text().c_str(), koi8_to_unicode) << ":/tag:" << std::endl;
#endif
    insert_op op;
    ParseChildren(listBox, links, tr, it, op);
  }
}

void SetStringToCharString(char *dest, const std::string & src, int dest_size)
{
  int size = src.size() > dest_size - 1 ? dest_size - 1 : src.size();
  strncpy(dest, src.c_str(), size);
  dest[size] = 0;
}
