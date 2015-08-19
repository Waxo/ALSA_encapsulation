#include <wav_functions.h>

template<typename T>
void write(std::ofstream &stream, const T &t) {
  stream.write((const char *) &t, sizeof(T));
}

/**
* Write the header of a wav file
* @param stream the file stream
* @param sample_rate sample rate of the wav file
* @param bits number of bits by sample
* @param stereo 1 is mono 2 is stereo
* @param sample_nb the number of samples
*/
void write_header_wav(std::ofstream &stream, const int &sample_rate, const short int &bits, const short int &stereo,
                      const long sample_nb) {
  int file_size;
  stream.seekp(0, std::ios_base::beg);
  file_size = (int) sample_nb * stereo * (bits / 8);
  stream.write("RIFF", 4);
  write<int>(stream, 36 + file_size);
  stream.write("WAVE", 4);
  stream.write("fmt ", 4);
  write<int>(stream, 16);                                         //Header Size
  write<short>(stream, 1);                                        // Format
  write<short>(stream, stereo);                                   // Channels
  write<int>(stream, sample_rate);                                // Ech freq
  write<int>(stream, sample_rate * stereo * (bits / 8));          // Byterate
  write<short>(stream, (short) (bits / 8) * stereo);              // Frame size
  write<short>(stream, bits);                                     // Bits per sample
  stream.write("data", 4);

  write<int>(stream, file_size);
}
