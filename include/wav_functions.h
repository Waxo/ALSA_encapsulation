#include <fstream>

void WriteHeaderWav(std::ofstream &f, const int &freq_ech, const short int &bits, const short int &stereo,
                      const long nb_ech);
