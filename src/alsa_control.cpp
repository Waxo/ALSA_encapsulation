/*
 * File:   alsa_control.cpp
 * Author: Maxime ROBIN
 *
 * Created on 17 février 2015, 09:57
 */

#include <alsa_control.h>

alsa_control::alsa_control(unsigned int const &rate, unsigned long const &frames, int const &bits, unsigned int const &stereo_mode)
        :
        _rate(rate),
        _frames(frames),
        _bits(bits),
        _stereo_mode(stereo_mode) {

    this->_continue_listening.store(false, std::memory_order_relaxed);

    this->open_pcm_device();
    snd_pcm_hw_params_alloca(&this->_params);
    this->set_parameters_ALSA();
}

alsa_control::~alsa_control() {
    snd_pcm_drain(this->_handle);
    snd_pcm_close(this->_handle);

    if (this->_continue_listening.load(std::memory_order_relaxed)) {
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
    for (val = 0; val <= SND_PCM_FORMAT_LAST; val++)
        if (snd_pcm_format_name((snd_pcm_format_t) val) != NULL)
            std::cout << "  " << snd_pcm_format_name((snd_pcm_format_t) val) << " (" << snd_pcm_format_description((snd_pcm_format_t) val) << ")" << std::endl;

    std::cout << std::endl << "PCM subformats:" << std::endl;
    for (val = 0; val <= SND_PCM_SUBFORMAT_LAST; val++)
        std::cout << "  " << snd_pcm_subformat_name((snd_pcm_subformat_t) val) << " (" << snd_pcm_subformat_description((snd_pcm_subformat_t) val) << ")" << std::endl;

    std::cout << std::endl << "PCM states:" << std::endl;
    for (val = 0; val <= SND_PCM_STATE_LAST; val++)
        std::cout << " " << snd_pcm_state_name((snd_pcm_state_t) val) << std::endl;
}

void alsa_control::open_pcm_device() {
    int rc = snd_pcm_open(&this->_handle, "default", SND_PCM_STREAM_CAPTURE, 0);

    if (rc < 0) {
        std::cout << "ERROR :  unable to open pcm device: " << snd_strerror(rc) << std::endl;
        exit(1);
    }
}

void alsa_control::set_parameters_ALSA() {
    snd_pcm_hw_params_any(this->_handle, this->_params); // def values
    snd_pcm_hw_params_set_access(this->_handle, this->_params, SND_PCM_ACCESS_RW_INTERLEAVED); //non interleaved
    snd_pcm_hw_params_set_format(this->_handle, this->_params, SND_PCM_FORMAT_S16_LE); //16bits little-endian
    snd_pcm_hw_params_set_channels(this->_handle, this->_params, this->_stereo_mode); // stereo ou mono


    snd_pcm_hw_params_set_rate_near(this->_handle, this->_params, &this->_rate, NULL); // sample rate (freq echantillonage)
    snd_pcm_hw_params_set_period_size_near(this->_handle, this->_params, &this->_frames, NULL); //frames pour une période

    int rc = snd_pcm_hw_params(this->_handle, this->_params);
    if (rc < 0) {
        std::cout << "ERROR - unable to set hw parameters: " << snd_strerror(rc) << std::endl;
        exit(1);
    }

    snd_pcm_hw_params_get_period_size(this->_params, &this->_period_size, NULL);
    snd_pcm_hw_params_get_period_time(this->_params, &this->_time_period, NULL);
}

void alsa_control::listen() {
    if (!this->_continue_listening.load(std::memory_order_relaxed)) {
        this->_continue_listening.store(true, std::memory_order_relaxed);
        this->_thread = std::async(std::launch::async, &alsa_control::thread_listen, this, "");
    } else {
        std::cout << "ERROR - System is already listening/recording use stop()" << std::endl;
    }
}

void alsa_control::listen(std::string filename) {
    if (!this->_continue_listening.load(std::memory_order_relaxed)) {
        this->_continue_listening.store(true, std::memory_order_relaxed);
        this->_thread = std::async(std::launch::async, &alsa_control::thread_listen, this, filename);
    } else {
        std::cout << "ERROR - System is already listening/recording use stop()" << std::endl;
    }
}

void alsa_control::listen_with_callback(std::function<void(char *, int)> func) {
    if (!this->_continue_listening.load(std::memory_order_relaxed)) {
        this->_continue_listening.store(true, std::memory_order_relaxed);
        this->_thread = std::async(std::launch::async, &alsa_control::thread_listen_with_callback, this, func, "");
    } else {
        std::cout << "ERROR - System is already listening/recording use stop()" << std::endl;
    }
}

void alsa_control::listen_with_callback(std::function<void(char *, int)> func, std::string filename) {
    if (!this->_continue_listening.load(std::memory_order_relaxed)) {
        this->_continue_listening.store(true, std::memory_order_relaxed);
        this->_thread = std::async(std::launch::async, &alsa_control::thread_listen_with_callback, this, func, filename);
    } else {
        std::cout << "ERROR - System is already listening/recording use stop()" << std::endl;
    }
}

void alsa_control::record_to_file(std::string filename, int duration_in_us) {
    if (!this->_continue_listening.load(std::memory_order_relaxed)) {
        this->_continue_listening.store(true, std::memory_order_relaxed);
        this->_thread = std::async(std::launch::async, &alsa_control::thread_record_to_file, this, filename, duration_in_us);
        this->_thread.get();
        this->_continue_listening.store(false, std::memory_order_relaxed);
    } else {
        std::cout << std::endl << "ERROR - System is already listening/recording use stop()";
    }
}

void alsa_control::stop() {
    this->_continue_listening.store(false, std::memory_order_relaxed);
    this->_thread.get();
}

void alsa_control::thread_listen(std::string filename) {
    std::ofstream f;
    int rc;
    int nb_ech = 0;

    if (filename != "") {
        filename += ".wav";
        f.open(filename, std::ios::binary);
        write_header_wav(f, this->_rate, static_cast<short>(this->_bits), static_cast<short>(this->_stereo_mode), 10000); //10000 is an arbitrary constant because we don't know the size of the recording
    }

    snd_pcm_uframes_t size = this->_period_size * 2; /* 2 bytes/sample, 1 channels */
    char *buffer = (char *) malloc(size);


    while (this->_continue_listening.load(std::memory_order_relaxed)) {
        rc = (int) snd_pcm_readi(this->_handle, buffer, this->_period_size);
        if (rc == -EPIPE) {
            std::cout << std::endl << "ERROR - overrun occurred";
            snd_pcm_prepare(this->_handle);
        } else if (rc < 0) {
            std::cout << std::endl << "ERROR - error from read: " << snd_strerror(rc);
        } else if (rc != (int) this->_period_size) {
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
        write_header_wav(f, this->_rate, static_cast<short>(this->_bits), static_cast<short>(this->_stereo_mode), nb_ech);
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
        write_header_wav(f, this->_rate, static_cast<short>(this->_bits), static_cast<short>(this->_stereo_mode), 10000); //10000 is an arbitrary constant because we don't know the size of the recording
    }

    snd_pcm_uframes_t size = this->_period_size * 2; /* 2 bytes/sample, 1 channels */
    char *buffer = (char *) malloc(size);


    while (this->_continue_listening.load(std::memory_order_relaxed)) {
        rc = (int) snd_pcm_readi(this->_handle, buffer, this->_period_size);
        if (rc == -EPIPE) {
            std::cout << std::endl << "ERROR - overrun occurred";
            snd_pcm_prepare(this->_handle);
        } else if (rc < 0) {
            std::cout << std::endl << "ERROR - error from read: " << snd_strerror(rc);
        } else if (rc != (int) this->_period_size) {
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
        write_header_wav(f, this->_rate, static_cast<short>(this->_bits), static_cast<short>(this->_stereo_mode), nb_ech);
        f.close();
    }
}

void alsa_control::thread_record_to_file(std::string filename, int duration_in_us) {
    std::ofstream f;
    int rc;
    int nb_ech = 0;

    filename += ".wav";
    f.open(filename, std::ios::binary);
    write_header_wav(f, this->_rate, static_cast<short>(this->_bits), static_cast<short>(this->_stereo_mode), 10000); //10000 is an arbitrary constant because we don't know the size of the recording

    snd_pcm_uframes_t size = this->_period_size * 2; /* 2 bytes/sample, 1 channels */

    char *buffer = (char *) malloc(size);
    long loops = duration_in_us / this->_time_period;

    while (loops-- > 0) {
        rc = (int) snd_pcm_readi(this->_handle, buffer, this->_period_size);
        if (rc == -EPIPE) {
            std::cout << std::endl << "ERROR - overrun occurred";
            snd_pcm_prepare(this->_handle);
        } else if (rc < 0) {
            std::cout << std::endl << "ERROR - error from read: " << snd_strerror(rc);
        } else if (rc != (int) this->_period_size) {
            std::cout << std::endl << "ERROR - short read, read " << rc << " frames";
        }

        if (rc > 0) {
            f.write(buffer, rc * 2);
        }

        nb_ech += rc;
    }

    f.close();
    f.open(filename, std::ios::binary | std::ios::in);
    write_header_wav(f, this->_rate, static_cast<short>(this->_bits), static_cast<short>(this->_stereo_mode), nb_ech);
    f.close();
    free(buffer);
}

void alsa_control::force_period_size() {
    this->_period_size = this->_frames;
}