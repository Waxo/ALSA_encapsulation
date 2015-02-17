#include <iostream>
#include <fstream>
#include <alsa/asoundlib.h>
#include "wav_functions.h"

using std::cout;
using std::endl;

void show_ALSA_parameters() {
    int val;

    cout << "ALSA library version: " << SND_LIB_VERSION_STR << endl;

    cout << endl << "PCM stream types:" << endl;
    for (val = 0; val <= SND_PCM_STREAM_LAST; val++)
        cout << " " << snd_pcm_stream_name((snd_pcm_stream_t) val) << endl;

    cout << endl << "PCM access types:" << endl;
    for (val = 0; val <= SND_PCM_ACCESS_LAST; val++)
        cout << " " << snd_pcm_access_name((snd_pcm_access_t) val) << endl;

    cout << endl << "PCM formats:" << endl;
    for (val = 0; val <= SND_PCM_FORMAT_LAST; val++)
        if (snd_pcm_format_name((snd_pcm_format_t) val) != NULL)
            cout << "  " << snd_pcm_format_name((snd_pcm_format_t) val) << " (" << snd_pcm_format_description((snd_pcm_format_t) val) << ")" << endl;

    cout << endl << "PCM subformats:" << endl;
    for (val = 0; val <= SND_PCM_SUBFORMAT_LAST; val++)
        cout << "  " << snd_pcm_subformat_name((snd_pcm_subformat_t) val) << " (" << snd_pcm_subformat_description((snd_pcm_subformat_t) val) << ")" << endl;

    cout << endl << "PCM states:" << endl;
    for (val = 0; val <= SND_PCM_STATE_LAST; val++)
        cout << " " << snd_pcm_state_name((snd_pcm_state_t) val) << endl;
}

void set_param_ALSA(snd_pcm_t *handle, snd_pcm_hw_params_t *params, unsigned int &rate, snd_pcm_uframes_t &frames) { //}, unsigned int &freq, snd_pcm_uframes_t &frames) {
    snd_pcm_hw_params_any(handle, params); // def values
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_NONINTERLEAVED); //non interleaved
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE); //16bits little-endian
    snd_pcm_hw_params_set_channels(handle, params, 1); // stereo ou mono


    snd_pcm_hw_params_set_rate_near(handle, params, &rate, NULL); // sample rate (freq echantillonage)
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, NULL); //frames pour une période

    int rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        cout << "ERROR - unable to set hw parameters: " << snd_strerror(rc) << endl;
        exit(1);
    }
}

void record_in_file(std::string filename, snd_pcm_t *handle, snd_pcm_hw_params_t *params, const unsigned int &freq, snd_pcm_uframes_t &frames) {

    std::ofstream f;

    int rc;
    int nb_ech = 0;
    unsigned int time_period;

    filename += ".wav";

    f.open(filename, std::ios::binary);

    write_header_wav(f, freq, 16, 1, 10000);


    snd_pcm_hw_params_get_period_size(params, &frames, NULL);
    snd_pcm_uframes_t size = frames * 2; /* 2 bytes/sample, 1 channels */
    char *buffer = (char *) malloc(size);


    snd_pcm_hw_params_get_period_time(params, &time_period, NULL);
    long loops = 10000000 / time_period; //10 seconds loop

    while (loops-- > 0) {
        rc = (int) snd_pcm_readi(handle, buffer, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            cout << "ERROR - overrun occurred" << endl;
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            cout << "ERROR - error from read: " << snd_strerror(rc) << endl;
        } else if (rc != (int) frames) {
            cout << "ERROR - short read, read " << rc << " frames" << endl;
        }

        if (!(loops % 10))cout << loops << endl;

        f.write(buffer, rc * 2);
        nb_ech += rc;
    }

    f.close();
    f.open(filename, std::ios::binary | std::ios::in);
    write_header_wav(f, freq, 16, 1, nb_ech);
    f.close();
    free(buffer);
}

void record() {
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames = 2048;
    unsigned int rate = 16000;

    int rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);

    if (rc < 0) {
        cout << "ERROR :  unable to open pcm device: " << snd_strerror(rc) << endl;
        exit(1);
    }

    snd_pcm_hw_params_alloca(&params);

    set_param_ALSA(handle, params, rate, frames); //, freq, frames);

    record_in_file("azerty", handle, params, rate, frames);


    snd_pcm_drain(handle);
    snd_pcm_close(handle);
}


int main() {
    show_ALSA_parameters();
    record();

    return 0;
}