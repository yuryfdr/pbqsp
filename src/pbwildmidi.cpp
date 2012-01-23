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

char* output_buffer=NULL;

int audio_fd=-1;
int rate = 22050;

static void shutdown_output ( void ) {
	printf("Shutting Down Sound System\r\n");
	if (audio_fd != -1)
		close(audio_fd);
}
char *buffer = NULL;
unsigned long int max_buffer;
unsigned long int buffer_delay;
unsigned long int counter;
struct audio_buf_info info;

static int write_oss_output (char * output_data, int output_size);
static void close_oss_output ( void );

static int open_oss_output( void ) {
  int caps, rc, tmp;
//  unsigned long int omode = O_RDWR;
  unsigned long int omode = O_WRONLY;

#ifndef __EMU__
  if ((audio_fd = open("/dev/sound/dsp", omode)) < 0) {
    printf("ERROR: Unable to open /dev/sound/dsp (%s)\n",strerror(errno));
#else
  if ((audio_fd = open("/dev/dsp", omode)) < 0) {
    printf("ERROR: Unable to open /dev/dsp (%s)\n",strerror(errno));
#endif
    return -1;
  }
  ioctl(audio_fd, SNDCTL_DSP_RESET, 0);
  rc = AFMT_S16_LE;
  ioctl (audio_fd, SNDCTL_DSP_SETFMT, &rc);
  int stereo = 1;
  ioctl (audio_fd, SNDCTL_DSP_STEREO, &stereo);
  ioctl (audio_fd, SNDCTL_DSP_SPEED, &rate);
  int snd_bitsize=16;
  ioctl (audio_fd, SNDCTL_DSP_SAMPLESIZE, &snd_bitsize);
  int snd_fragment=0x00080009;
  ioctl (audio_fd, SNDCTL_DSP_SETFRAGMENT, &snd_fragment);
  return 1;
}

static int write_oss_output(char * output_data, int output_size) {
  write(audio_fd,output_data,output_size);
  return 0;
}

static void close_oss_output(void) {
	shutdown_output();
/*	if (buffer != NULL)
		munmap (buffer, info.fragstotal * info.fragsize);*/
	audio_fd = -1;
}
volatile bool _stop_play;
void stop_play(){
  _stop_play=true;
  printf("%s %d\n",__PRETTY_FUNCTION__,_stop_play);
}
static bool wm_init=false;
void play_midi(const char* fname,int samplerate,int volume){
  if(audio_fd<0){
    open_oss_output();
  }
  if(!wm_init){
#ifndef __EMU__
  WildMidi_Init("/mnt/ext1/system/config/wildmidi.cfg",22050,0);
#else
  WildMidi_Init("/etc/wildmidi.cfg",22050,0);
#endif
  wm_init=true;
  }
  WildMidi_MasterVolume(volume);
  if(!output_buffer)output_buffer=(char*)malloc(16384);
  void* midi_ptr=WildMidi_Open(fname);
  if(!midi_ptr){
    //close_oss_output();
    printf("err open file %s\n",fname);
    return;
  }
  _WM_Info* wm_info=WildMidi_GetInfo(midi_ptr);
  //printf("cpr %s\n",wm_info->copyright);
  //_stop_play=false;
  while(!_stop_play){
    wm_info=WildMidi_GetInfo(midi_ptr);
    int count_diff=wm_info->approx_total_samples-wm_info->current_sample;
    if(count_diff==0||_stop_play)
      break;
    int output_result;
    if(count_diff<4096){
      output_result=WildMidi_GetOutput(midi_ptr,output_buffer,count_diff*4);
    }else{
      output_result=WildMidi_GetOutput(midi_ptr,output_buffer,16384);
    }
    if(output_result<=0||_stop_play)
      break;
    printf("play %s %g %d stop %d\n",fname,(double)count_diff/wm_info->approx_total_samples,output_result,_stop_play);
    write_oss_output(output_buffer,output_result);
  }
  WildMidi_Close(midi_ptr);
  //printf("End %s \n",fname);
  close_oss_output();
}
void play_mp3(const char* fname,int samplerate,int volume){
}