/*
 * File:   alsa_control.cpp
 * Author: Maxime ROBIN
 *
 * Created on 17 février 201, 09:57
 */

#include <alsa_control.h>

alsa_control::alsa_control(unsigned int rate, unsigned long frames, int bits, unsigned int stereo_mode) :
        _rate(rate),
        _frames(frames),
        _bits(bits),
        _stereo_mode(stereo_mode) {

    this->open_pcm_device();
    snd_pcm_hw_params_alloca(&this->_params);
    this->set_parameters_ALSA();
}

alsa_control::~alsa_control() {
    snd_pcm_drain(this->_handle);
    snd_pcm_close(this->_handle);
}

void alsa_control::open_pcm_device() {
    int rc = snd_pcm_open(&this->_handle, "default", SND_PCM_STREAM_CAPTURE, 0);

    if (rc < 0) {
        cout << "ERROR :  unable to open pcm device: " << snd_strerror(rc) << endl;
        exit(1);
    }
}

void alsa_control::set_parameters_ALSA() {
    snd_pcm_hw_params_any(this->_handle, this->_params); // def values
    snd_pcm_hw_params_set_access(this->_handle, this->_params, SND_PCM_ACCESS_RW_NONINTERLEAVED); //non interleaved
    snd_pcm_hw_params_set_format(this->_handle, this->_params, SND_PCM_FORMAT_S16_LE); //16bits little-endian
    snd_pcm_hw_params_set_channels(this->_handle, this->_params, this->_stereo_mode); // stereo ou mono


    snd_pcm_hw_params_set_rate_near(this->_handle, this->_params, &this->_rate, NULL); // sample rate (freq echantillonage)
    snd_pcm_hw_params_set_period_size_near(this->_handle, this->_params, &this->_frames, NULL); //frames pour une période

    int rc = snd_pcm_hw_params(this->_handle, this->_params);
    if (rc < 0) {
        cout << "ERROR - unable to set hw parameters: " << snd_strerror(rc) << endl;
        exit(1);
    }

    snd_pcm_hw_params_get_period_size(this->_params, &this->_period_size, NULL);
    snd_pcm_hw_params_get_period_time(this->_params, &this->_time_period, NULL);
}

void alsa_control::record_to_file(std::string filename, int duration_in_us) {
    std::ofstream f;
    int rc;
    int nb_ech = 0;

    filename += ".wav";
    f.open(filename, std::ios::binary);
    write_header_wav(f, this->_rate, (short) this->_bits, (short) this->_stereo_mode, 10000); //10000 is an arbitrary constant because we don't know the size of the recording

    snd_pcm_uframes_t size = this->_period_size * 2; /* 2 bytes/sample, 1 channels */

    char *buffer = (char *) malloc(size);
    long loops = duration_in_us / this->_time_period;

    while (loops-- > 0) {
        rc = (int) snd_pcm_readi(this->_handle, buffer, this->_period_size);
        if (rc == -EPIPE) {
            cout << "ERROR - overrun occurred" << endl;
            snd_pcm_prepare(this->_handle);
        } else if (rc < 0) {
            cout << "ERROR - error from read: " << snd_strerror(rc) << endl;
        } else if (rc != (int) this->_period_size) {
            cout << "ERROR - short read, read " << rc << " frames" << endl;
        }

        f.write(buffer, rc * 2);
        nb_ech += rc;
    }

    f.close();
    f.open(filename, std::ios::binary | std::ios::in);
    write_header_wav(f, this->_rate, (short) this->_bits, (short) this->_stereo_mode, nb_ech);
    f.close();
    free(buffer);
}
