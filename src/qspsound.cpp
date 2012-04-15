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
#include "pbwildmidi.h"

#ifdef __EMU__
#define KBDOPTS KBD_SCREENTOP
#else
#define KBDOPTS 0
#endif

#ifdef HAVE_SOUND

static struct played_file {
  std::string name;
  bool played;
  pthread_mutex_t mt;
  pthread_t th;
  pid_t pid;
   played_file():played(false), th(-1), pid(-1) {
    pthread_mutex_init(&mt, NULL);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  } pthread_attr_t attr;
} played;

#ifdef __EMU__
static const char *const timidity[] = { "timidity", "-s", "22050", NULL, NULL };
static const char *const sox[] = { "sox", "-d", "-S", NULL, NULL };
static const char *const mpg123[] = { "mpg123", "-q", "-q", NULL, NULL };
static const char *const ogg123[] = { "ogg123", "-v", "-v", NULL, NULL };

static const int narg = 3;
#else
static const char *const timidity[] = { "timidity", "-s", "11025", NULL, NULL };
static const char *const sox[] = { "sndfile-play", "", "", NULL, NULL };
static const char *const mpg123[] = { "mpg123", "-a", "/dev/sound/dsp", NULL, NULL };
static const char *const ogg123[] = { "ogg123", "-d", "/dev/sound/dsp", NULL, NULL };

static const int narg = 3;
#endif

void *playfn(void *pf)
{
  if (played.name.size() != 0 && played.name.size() > 4) {
    pthread_mutex_lock(&played.mt);
    std::string ext = played.name.substr(played.name.size() - 4, played.name.size() - 1);
    fprintf(stderr, "ext :%s:\n", ext.c_str());
    std::string name =
        (to_utf8(played.name.substr(GetQuestPath().size()).c_str(), koi8_to_unicode));
    name = GetQuestPath() + name;
    std::string runstr;
    const char *const *targs;
    if (ext == ".mp3") {
      runstr = "mpg123";
      targs = mpg123;
    } else if (ext == ".mid" || ext == ".MID") {
#if 0
      played.pid = -1;
      played.played = true;
      pthread_mutex_unlock(&played.mt);
      play_midi(convertbackslash(name).c_str(), 22050, 127);
      return pf;
#endif
      runstr = "timidity";
      targs = timidity;
    }                           /*else if(ext==".ogg" || ext==".Ogg"){
                                   runstr="ogg123";
                                   //timidity[narg]=convertbackslash(name).c_str();
                                   targs=ogg123;
                                   //runstr+=convertbackslash(name);
                                   } */
    else {
      runstr = "sndfile-play";
      targs = sox;
    }
    for (int i = 0; i < narg + 2; ++i) {
      fprintf(stderr, "arg %d :%s\n", i, targs[i]);
    }
    if (played.pid != -1)
      kill(played.pid, SIGKILL);
    pid_t pid = fork();
    played.played = true;
    pthread_mutex_unlock(&played.mt);

    if (pid != -1) {
      if (pid == 0) {
        fprintf(stderr, "run %s %s %s\n", runstr.c_str(), convertbackslash(name).c_str(), targs[1]);
#ifndef __EMU__
        execlp((std::string("/mnt/ext1/system/bin/") + runstr).c_str(), targs[0], targs[1]
               , targs[2], convertbackslash(name).c_str(), (char *)0);
#else
        execlp((std::string("/usr/bin/") + runstr).c_str(), targs[0], targs[1]
               , targs[2], convertbackslash(name).c_str(), (char *)0);
#endif
        printf("child %d %s exited\n", errno, strerror(errno));
      } else {
        played.pid = pid;
        int status;
        waitpid(pid, &status, 0);
        played.pid = -1;
        fprintf(stderr, "afwp child %s exited\n", runstr.c_str());
      }
    }
  }
  pthread_mutex_lock(&played.mt);
  played.played = false;
  pthread_mutex_unlock(&played.mt);
  return pf;
}

/*void* playfn_aaa(void* pf){
  if(played.name.empty())return pf;
  if(played.name.size()<=4)
  
  pthread_mutex_lock(&played.mt);
  std::string ext=played.name.substr(played.name.size()-4,played.name.size()-1);
  //fprintf(stderr,"ext :%s:\n",ext.c_str());
  std::string name=played.name.substr(GetQuestPath().size());
  played.played=true;
  pthread_mutex_unlock(&played.mt);
  if(ext==".mp3"){
  }else if(ext==".mid" || ext==".MID"){
    play_midi(convertbackslash(name).c_str(),22050,127);
  }else if(ext==".ogg" || ext==".Ogg"){
  }
  pthread_mutex_lock(&played.mt);
  if(name==played.name.substr(GetQuestPath().size())){
    played.played=false;
    played.name="";
  }
  pthread_mutex_unlock(&played.mt);
  return pf;
} */

QSP_BOOL QSPCallbacks::IsPlay(const QSP_CHAR * file)
{
  return (played.name == file);
}

void QSPCallbacks::CloseFile(const QSP_CHAR * file)
{
  pthread_mutex_lock(&played.mt);
  fprintf(stderr, "close %s\n", file);
#if 0
  stop_play();
#endif
  played.played = false;
  if (played.pid != -1) {
    fprintf(stderr, "kill %d\n", played.pid);
    kill(played.pid, SIGKILL);
    played.pid = -1;
  }
  void *ret;
  fprintf(stderr, "before join %d\n", played.th);
  pthread_mutex_unlock(&played.mt);
  if (played.th != -1)
    pthread_join(played.th, &ret);
  fprintf(stderr, "after join %d\n", played.th);
  played.th = -1;
}

void QSPCallbacks::PlayFile(const QSP_CHAR * file, long volume)
{
  fprintf(stderr, "Play:%s :%d\n", file, played.played);
  if (IsPlay(file))
    return;
  else
    CloseFile(NULL);
  fprintf(stderr, "Play:%s :%d\n", file, played.played);
  pthread_mutex_lock(&played.mt);
  played.name = file;
  played.played = true;
  pthread_mutex_unlock(&played.mt);
  pthread_create(&played.th, &played.attr, playfn, NULL);
}

#else

QSP_BOOL QSPCallbacks::IsPlay(const QSP_CHAR * file)
{
  return true;
}

void QSPCallbacks::CloseFile(const QSP_CHAR * file)
{
}

void QSPCallbacks::PlayFile(const QSP_CHAR * file, long volume)
{
}
#endif
