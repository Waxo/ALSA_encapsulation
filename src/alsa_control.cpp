/*
 * File:   alsa_control.cpp
 * Author: Maxime ROBIN
 *
 * Created on 17 février 201, 09:57
 */

#include <alsa_control.h>

alsa_control::alsa_control(unsigned int rate, unsigned long frames, int bits, unsigned int stereo_mode)
        : __rate(rate),
          __stereo_mode(stereo_mode),
          __bits(bits),
          __frames(frames) {

    this->open_pcm_device();
    snd_pcm_hw_params_alloca(&this->__params);
    this->set_parameters_ALSA();
}

alsa_control::~alsa_control() {
    snd_pcm_drain(this->__handle);
    snd_pcm_close(this->__handle);
}

void alsa_control::open_pcm_device() {
    int rc = snd_pcm_open(&this->__handle, "default", SND_PCM_STREAM_CAPTURE, 0);

    if (rc < 0) {
        cout << "ERROR :  unable to open pcm device: " << snd_strerror(rc) << endl;
        exit(1);
    }
}

void alsa_control::set_parameters_ALSA() {
    snd_pcm_hw_params_any(this->__handle, this->__params); // def values
    snd_pcm_hw_params_set_access(this->__handle, this->__params, SND_PCM_ACCESS_RW_NONINTERLEAVED); //non interleaved
    snd_pcm_hw_params_set_format(this->__handle, this->__params, SND_PCM_FORMAT_S16_LE); //16bits little-endian
    snd_pcm_hw_params_set_channels(this->__handle, this->__params, this->__stereo_mode); // stereo ou mono


    snd_pcm_hw_params_set_rate_near(this->__handle, this->__params, &this->__rate, NULL); // sample rate (freq echantillonage)
    auto ret = snd_pcm_hw_params_set_period_size_near(this->__handle, this->__params, &this->__frames, NULL); //frames pour une période

    int rc = snd_pcm_hw_params(this->__handle, this->__params);
    if (rc < 0) {
        cout << "ERROR - unable to set hw parameters: " << snd_strerror(rc) << endl;
        exit(1);
    }
}

void alsa_control::record_to_file(std::string filename, int duration_in_us) {
    std::ofstream f;
    int rc;
    int nb_ech = 0;
    snd_pcm_uframes_t period_size;
    unsigned int time_period;


    filename += ".wav";
    f.open(filename, std::ios::binary);
    write_header_wav(f, this->__rate, (short) this->__bits, (short) this->__stereo_mode, 10000); //10000 is a constant because we don't know the size of the recording

    snd_pcm_hw_params_get_period_size(this->__params, &period_size, NULL);
    period_size = 2048;
    snd_pcm_uframes_t size = period_size * 2; /* 2 bytes/sample, 1 channels */
    char *buffer = (char *) malloc(size);


    snd_pcm_hw_params_get_period_time(this->__params, &time_period, NULL);
    time_period = 128000;
    long loops = duration_in_us / time_period;

    while (loops-- > 0) {
        rc = (int) snd_pcm_readi(this->__handle, buffer, period_size);
        if (rc == -EPIPE) {
            cout << "ERROR - overrun occurred" << endl;
            snd_pcm_prepare(this->__handle);
        } else if (rc < 0) {
            cout << "ERROR - error from read: " << snd_strerror(rc) << endl;
        } else if (rc != (int) period_size) {
            cout << "ERROR - short read, read " << rc << " frames" << endl;
        }

        if (!(loops % 10))cout << loops << endl;

        f.write(buffer, rc * 2);
        nb_ech += rc;
    }

    f.close();
    f.open(filename, std::ios::binary | std::ios::in);
    write_header_wav(f, this->__rate, (short) this->__bits, (short) this->__stereo_mode, nb_ech);
    f.close();
    free(buffer);
}
