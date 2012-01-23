#include "pbsound.h"
#include "helper.h"
extern "C"{
#include <wildmidi_lib.h>
};
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/soundcard.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <vector>
#include <string>

PBSound::played_file::played_file(const char* nm):name(nm),played(false),stop(false),thid(-1),pid(-1){
  ext=name.substr(name.size()-4,name.size()-1);
  std::string path=convertbackslash(name);
}

std::vector<PBSound::played_file> PBSound::files;
int PBSound::audio_fd=-1;
bool PBSound::wm_ready=false;

int PBSound::open_oss(){
  unsigned long int omode = O_RDWR;
#ifndef __EMU__
  if ((audio_fd = open("/dev/sound/dsp", omode)) < 0) {
#else
  if ((audio_fd = open("/dev/dsp", omode)) < 0) {
#endif
    printf("ERROR: Unable to open /dev/dsp (%s)\r\n",strerror(errno));
    return -1;
  }
  ioctl(audio_fd, SNDCTL_DSP_RESET, 0);
  int rc = AFMT_S16_LE;
  ioctl (audio_fd, SNDCTL_DSP_SETFMT, &rc);
  int stereo = 1;
  ioctl (audio_fd, SNDCTL_DSP_STEREO, &stereo);
  //ioctl (audio_fd, SNDCTL_DSP_SPEED, &rate);
  int snd_bitsize=16;
  ioctl (audio_fd, SNDCTL_DSP_SAMPLESIZE, &snd_bitsize);
  int snd_fragment=0x00080009;
  ioctl (audio_fd, SNDCTL_DSP_SETFRAGMENT, &snd_fragment);
}

int PBSound::close_oss(){
  close(audio_fd);
  audio_fd=-1;
}
int PBSound::set_param(int rate){
  ioctl (audio_fd, SNDCTL_DSP_SPEED, &rate);
}

int PBSound::write_oss(char * output_data, int output_size) {
  write(audio_fd,output_data,output_size);
  return 0;
}


void* PBSound::play_ogg(played_file* pf){
}
void* PBSound::play_mp3(played_file* pf){
}
void* PBSound::play_wav(played_file* pf){
}
void* PBSound::play_mid(played_file* pf){
}

//  public:
void PBSound::stop_play(const char* fname){
}
void PBSound::play(const char* fname,int vol){
  played_file fl(fname);
  fl.volume=vol;
}
bool PBSound::is_play(const char* fname){
}

