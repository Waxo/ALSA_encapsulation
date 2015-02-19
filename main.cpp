#include <iostream>
#include <alsa/asoundlib.h>
#include <alsa_control.h>

using std::cout;
using std::endl;


int main() {

    alsa_control *ac = new alsa_control(16000, 2048, 16, MONO);

    ac->show_ALSA_parameters();

    ac->listen("aze");
    sleep(8);
    ac->stop();

    ac->record_to_file("azerty", 10000000);

    delete ac;

    return 0;
}