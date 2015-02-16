/*
 * File:   wav_functions.cpp
 * Author: Maxime ROBIN
 *
 * Created on 17 décembre 2014, 12:20
 */

#include "wav_functions.h"

template<typename T>
void write(std::ofstream &stream, const T &t) {
    stream.write((const char *) &t, sizeof(T));
}

void write_header_wav(std::ofstream &stream, const int &freq_ech, const short int &bits, const short int &stereo, const long nb_ech) {
    int file_size;

    file_size = (int) nb_ech * stereo * (bits / 8);
    stream.write("RIFF", 4);
    write<int>(stream, 36 + file_size);
    stream.write("WAVE", 4);
    stream.write("fmt ", 4);
    write<int>(stream, 16);                                         //Header Size
    write<short>(stream, 1);                                        // Format
    write<short>(stream, stereo);                                   // Channels
    write<int>(stream, freq_ech);                                   // Ech freq
    write<int>(stream, freq_ech * stereo * (bits / 8));             // Byterate
    write<short>(stream, (short) (bits / 8) * stereo);              // Frame size
    write<short>(stream, bits);                                     // Bits per sample
    stream.write("data", 4);

    write<int>(stream, file_size);
}

