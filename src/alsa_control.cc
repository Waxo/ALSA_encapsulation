#include <alsa_control.h>

alsa_control::alsa_control(unsigned int const &rate, unsigned long const &frames, int const &bits,
                           unsigned int const &stereo_mode)
    :
    rate_(rate),
    frames_(frames),
    bits_(bits),
    stereo_mode_(stereo_mode) {

  this->continue_listening_.store(false, std::memory_order_relaxed);

  this->open_pcm_device();
  snd_pcm_hw_params_alloca(&this->params_);
  this->set_parameters_ALSA();
}

alsa_control::~alsa_control() {
  snd_pcm_drain(this->handle_);
  snd_pcm_close(this->handle_);

  if (this->continue_listening_.load(std::memory_order_relaxed)) {
    std::cout << std::endl << "ERROR - All process seems not finished" << std::endl;
    exit(1);
  }
}

void alsa_control::show_ALSA_parameters() {
  int val;

  std::cout << "ALSA library version: " << SND_LIB_VERSION_STR << std::endl;

  std::cout << std::endl << "PCM stream types:" << std::endl;
  for (val = 0; val <= SND_PCM_STREAM_LAST; val++)
    std::cout << " " << snd_pcm_stream_name((snd_pcm_stream_t) val) << std::endl;

  std::cout << std::endl << "PCM access types:" << std::endl;
  for (val = 0; val <= SND_PCM_ACCESS_LAST; val++)
    std::cout << " " << snd_pcm_access_name((snd_pcm_access_t) val) << std::endl;

  std::cout << std::endl << "PCM formats:" << std::endl;
  for (val = 0; val <= SND_PCM_FORMAT_LAST; val++) {
    if (snd_pcm_format_name((snd_pcm_format_t) val) != NULL) {
      std::cout << "  " << snd_pcm_format_name((snd_pcm_format_t) val) << " (" <<
      snd_pcm_format_description((snd_pcm_format_t) val) << ")" << std::endl;
    }
  }

  std::cout << std::endl << "PCM subformats:" << std::endl;
  for (val = 0; val <= SND_PCM_SUBFORMAT_LAST; val++) {
    std::cout << "  " << snd_pcm_subformat_name((snd_pcm_subformat_t) val) << " (" <<
    snd_pcm_subformat_description((snd_pcm_subformat_t) val) << ")" << std::endl;
  }

  std::cout << std::endl << "PCM states:" << std::endl;
  for (val = 0; val <= SND_PCM_STATE_LAST; val++) {
    std::cout << " " << snd_pcm_state_name((snd_pcm_state_t) val) << std::endl;
  }
}

void alsa_control::open_pcm_device() {
  int rc = snd_pcm_open(&this->handle_, "default", SND_PCM_STREAM_CAPTURE, 0);

  if (rc < 0) {
    std::cout << "ERROR :  unable to open pcm device: " << snd_strerror(rc) << std::endl;
    exit(1);
  }
}

void alsa_control::set_parameters_ALSA() {
  snd_pcm_hw_params_any(this->handle_, this->params_); // def values
  snd_pcm_hw_params_set_access(this->handle_, this->params_, SND_PCM_ACCESS_RW_INTERLEAVED); //non interleaved
  snd_pcm_hw_params_set_format(this->handle_, this->params_, SND_PCM_FORMAT_S16_LE); //16bits little-endian
  snd_pcm_hw_params_set_channels(this->handle_, this->params_, this->stereo_mode_); // stereo ou mono


  snd_pcm_hw_params_set_rate_near(this->handle_, this->params_, &this->rate_,
                                  NULL); // sample rate (freq echantillonage)
  snd_pcm_hw_params_set_period_size_near(this->handle_, this->params_, &this->frames_,
                                         NULL); //frames pour une période

  int rc = snd_pcm_hw_params(this->handle_, this->params_);
  if (rc < 0) {
    std::cout << "ERROR - unable to set hw parameters: " << snd_strerror(rc) << std::endl;
    exit(1);
  }

  snd_pcm_hw_params_get_period_size(this->params_, &this->period_size_, NULL);
  snd_pcm_hw_params_get_period_time(this->params_, &this->time_period_, NULL);
}

void alsa_control::listen() {
  if (!this->continue_listening_.load(std::memory_order_relaxed)) {
    this->continue_listening_.store(true, std::memory_order_relaxed);
    this->thread_ = std::async(std::launch::async, &alsa_control::thread_listen, this, "");
  } else {
    std::cout << "ERROR - System is already listening/recording use stop()" << std::endl;
  }
}

void alsa_control::listen(std::string filename) {
  if (!this->continue_listening_.load(std::memory_order_relaxed)) {
    this->continue_listening_.store(true, std::memory_order_relaxed);
    this->thread_ = std::async(std::launch::async, &alsa_control::thread_listen, this, filename);
  } else {
    std::cout << "ERROR - System is already listening/recording use stop()" << std::endl;
  }
}

void alsa_control::listen_with_callback(std::function<void(char *, int)> func) {
  if (!this->continue_listening_.load(std::memory_order_relaxed)) {
    this->continue_listening_.store(true, std::memory_order_relaxed);
    this->thread_ = std::async(std::launch::async, &alsa_control::thread_listen_with_callback, this, func, "");
  } else {
    std::cout << "ERROR - System is already listening/recording use stop()" << std::endl;
  }
}

void alsa_control::listen_with_callback(std::function<void(char *, int)> func, std::string filename) {
  if (!this->continue_listening_.load(std::memory_order_relaxed)) {
    this->continue_listening_.store(true, std::memory_order_relaxed);
    this->thread_ = std::async(std::launch::async, &alsa_control::thread_listen_with_callback, this, func,
                               filename);
  } else {
    std::cout << "ERROR - System is already listening/recording use stop()" << std::endl;
  }
}

void alsa_control::record_to_file(std::string filename, int const &duration_in_us) {
  if (!this->continue_listening_.load(std::memory_order_relaxed)) {
    this->continue_listening_.store(true, std::memory_order_relaxed);
    this->thread_ = std::async(std::launch::async, &alsa_control::thread_record_to_file, this, filename,
                               duration_in_us);
    this->thread_.get();
    this->continue_listening_.store(false, std::memory_order_relaxed);
  } else {
    std::cout << std::endl << "ERROR - System is already listening/recording use stop()";
  }
}

void alsa_control::stop() {
  this->continue_listening_.store(false, std::memory_order_relaxed);
  this->thread_.get();
}

void alsa_control::thread_listen(std::string filename) {
  std::ofstream f;
  int rc;
  int nb_ech = 0;

  if (filename != "") {
    filename += ".wav";
    f.open(filename, std::ios::binary);
    write_header_wav(f, this->rate_, static_cast<short>(this->bits_), static_cast<short>(this->stereo_mode_),
                     10000); //10000 is an arbitrary constant because we don't know the size of the recording
  }

  snd_pcm_uframes_t size = this->period_size_ * 2; /* 2 bytes/sample, 1 channels */
  char *buffer = (char *) malloc(size);


  while (this->continue_listening_.load(std::memory_order_relaxed)) {
    rc = (int) snd_pcm_readi(this->handle_, buffer, this->period_size_);
    if (rc == -EPIPE) {
      std::cout << std::endl << "ERROR - overrun occurred";
      snd_pcm_prepare(this->handle_);
    } else if (rc < 0) {
      std::cout << std::endl << "ERROR - error from read: " << snd_strerror(rc);
    } else if (rc != (int) this->period_size_) {
      std::cout << std::endl << "ERROR - short read, read " << rc << " frames";
    }

    if (rc > 0 && filename != "") {
      f.write(buffer, rc * 2);
      nb_ech += rc;
    }
  }

  free(buffer);

  if (filename != "") {
    f.close();
    f.open(filename, std::ios::binary | std::ios::in);
    write_header_wav(f, this->rate_, static_cast<short>(this->bits_), static_cast<short>(this->stereo_mode_),
                     nb_ech);
    f.close();
  }
}

void alsa_control::thread_listen_with_callback(std::function<void(char *, int)> func, std::string filename) {
  std::ofstream f;
  int rc;
  int nb_ech = 0;

  if (filename != "") {
    filename += ".wav";
    f.open(filename, std::ios::binary);
    //10000 is an arbitrary constant because we don't know the size of the recording
    write_header_wav(f, this->rate_, static_cast<short>(this->bits_), static_cast<short>(this->stereo_mode_), 10000);
  }

  snd_pcm_uframes_t size = this->period_size_ * 2; /* 2 bytes/sample, 1 channels */
  char *buffer = (char *) malloc(size);


  while (this->continue_listening_.load(std::memory_order_relaxed)) {
    rc = (int) snd_pcm_readi(this->handle_, buffer, this->period_size_);
    if (rc == -EPIPE) {
      std::cout << std::endl << "ERROR - overrun occurred";
      snd_pcm_prepare(this->handle_);
    } else if (rc < 0) {
      std::cout << std::endl << "ERROR - error from read: " << snd_strerror(rc);
    } else if (rc != (int) this->period_size_) {
      std::cout << std::endl << "ERROR - short read, read " << rc << " frames";
    }

    if (rc > 0 && filename != "") {
      f.write(buffer, rc * 2);
      nb_ech += rc;
    }

    func(buffer, rc);
  }

  free(buffer);

  if (filename != "") {
    f.close();
    f.open(filename, std::ios::binary | std::ios::in);
    write_header_wav(f, this->rate_, static_cast<short>(this->bits_), static_cast<short>(this->stereo_mode_),
                     nb_ech);
    f.close();
  }
}

void alsa_control::thread_record_to_file(std::string filename, int const &duration_in_us) {
  std::ofstream f;
  int rc;
  int nb_ech = 0;

  filename += ".wav";
  f.open(filename, std::ios::binary);
  write_header_wav(f, this->rate_, static_cast<short>(this->bits_), static_cast<short>(this->stereo_mode_),
                   10000); //10000 is an arbitrary constant because we don't know the size of the recording

  snd_pcm_uframes_t size = this->period_size_ * 2 * this->stereo_mode_; /* 2 bytes/sample, 1 channels */

  char *buffer = (char *) malloc(size);
  long loops = duration_in_us / this->time_period_;

  while (loops-- > 0) {
    rc = (int) snd_pcm_readi(this->handle_, buffer, this->period_size_);
    if (rc == -EPIPE) {
      std::cout << std::endl << "ERROR - overrun occurred";
      snd_pcm_prepare(this->handle_);
    } else if (rc < 0) {
      std::cout << std::endl << "ERROR - error from read: " << snd_strerror(rc);
    } else if (rc != (int) this->period_size_) {
      std::cout << std::endl << "ERROR - short read, read " << rc << " frames";
    }

    if (rc > 0) {
      f.write(buffer, rc * 2);
    }

    nb_ech += rc;
  }

  f.close();
  f.open(filename, std::ios::binary | std::ios::in);
  write_header_wav(f, this->rate_, static_cast<short>(this->bits_), static_cast<short>(this->stereo_mode_), nb_ech);
  f.close();
  free(buffer);
}

void alsa_control::force_period_size(int const &value) {
  this->period_size_ = static_cast<snd_pcm_uframes_t>(value);
}
