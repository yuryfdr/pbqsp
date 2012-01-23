#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h>

class PBSound{
  struct played_file{
    std::string name;
    std::string path;
    std::string ext;
    bool played;
    int pos;
    int volume;
    int stop;
    pthread_t thid;
    pid_t pid;
    played_file(const char* nm);
  };
  static std::vector<played_file> files;
  static int audio_fd;
  static bool wm_ready;

  static int open_oss();
  static int close_oss();
  static int write_oss(char * output_data, int output_size);
  static int set_param(int rate);

  static void* play_ogg(played_file* pf);
  static void* play_mp3(played_file* pf);
  static void* play_wav(played_file* pf);
  static void* play_mid(played_file* pf);
  public:
  static void stop_play(const char* fname);
  static void play(const char* fname,int vol);
  static bool is_play(const char* fname);
};
