#ifndef ALSA_ENCAPSULATION_ALSA_CONTROL_H_
#define ALSA_ENCAPSULATION_ALSA_CONTROL_H_

#include <iostream>
#include <future>
#include <functional>
#include <alsa/asoundlib.h>
#include <WavFunctions.h>


#define STEREO 2
#define MONO 1

class AlsaControl {
public:
  void ShowALSAParameters();
  void Listen();
  void Listen(std::string filename);
  void ListenWithCallback(std::function<void(void *, int)> func);
  void ListenWithCallback(std::function<void(void *, int)> func,
      std::string filename);
  void RecordToFile(std::string filename, int const &duration_in_us);

  void ForcePeriodSize(int const &value);

  void Stop();

  AlsaControl(unsigned int const &rate, unsigned long const &frames,
      int const &bits, unsigned int const &stereo_mode);
  virtual ~AlsaControl();

private:
  unsigned int rate_;
  snd_pcm_uframes_t frames_;
  int bits_;
  unsigned int stereo_mode_;

  unsigned int time_period_;
  snd_pcm_hw_params_t *params_;
  snd_pcm_t *handle_;
  snd_pcm_uframes_t period_size_;

  std::atomic<bool> continue_listening_;
  std::future<void> thread_;

  void OpenPcmDevice();
  void SetParametersALSA();

  void ThreadListen(std::string filename);
  void ThreadListenWithCallback(std::function<void(void *, int)> func,
      std::string filename);
  void ThreadRecordToFile(std::string filename, int const &duration_in_us);

  AlsaControl() = delete;
  AlsaControl(const AlsaControl &) = delete;
};

#endif //ALSA_ENCAPSULATION_ALSA_CONTROL_H_
