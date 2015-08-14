#ifndef ALSA_ENCAPSULATION_ALSA_CONTROL_H_
#define ALSA_ENCAPSULATION_ALSA_CONTROL_H_

#include <iostream>
#include <future>
#include <functional>
#include <alsa/asoundlib.h>
#include <wav_functions.h>


#define STEREO 2
#define MONO 1

class alsa_control {
public:
  void show_ALSA_parameters();
  void listen();
  void listen(std::string filename);
  void listen_with_callback(std::function<void(void *, int)> func);
  void listen_with_callback(std::function<void(void *, int)> func, std::string filename);
  void record_to_file(std::string filename, int const &duration_in_us);

  void force_period_size(int const &value);

  void stop();

  alsa_control(unsigned int const &rate, unsigned long const &frames, int const &bits, unsigned int const &stereo_mode);
  ~alsa_control();

private:
  unsigned int rate_;
  unsigned int stereo_mode_;
  int bits_;
  unsigned int time_period_;

  snd_pcm_hw_params_t *params_;
  snd_pcm_t *handle_;
  snd_pcm_uframes_t frames_;
  snd_pcm_uframes_t period_size_;

  std::atomic<bool> continue_listening_;
  std::future<void> thread_;

  void open_pcm_device();
  void set_parameters_ALSA();

  void thread_listen(std::string filename);
  void thread_listen_with_callback(std::function<void(void *, int)> func, std::string filename);
  void thread_record_to_file(std::string filename, int const &duration_in_us);

  alsa_control() = delete;
  alsa_control(const alsa_control &) = delete;
};

#endif //ALSA_ENCAPSULATION_ALSA_CONTROL_H_
