#include <iostream>
#include <alsa_control.h>

using std::cout;
using std::endl;


class lambda_class {
public:
    void lambda_callback(char *c, int rc) {
        this->_nb_ech += rc;
        this->_f.write(c, rc * 2);
    }

    lambda_class() {
        this->_nb_ech = 0;
        this->_f.open("aaaa.wav", std::ios::binary);
        write_header_wav(this->_f, 16000, 16, MONO, 10000);
    }

    ~lambda_class() {
        this->_f.close();
        this->_f.open("aaaa.wav", std::ios::binary | std::ios::in);
        write_header_wav(this->_f, 16000, 16, MONO, this->_nb_ech);
        //cout << this->_nb_ech << endl;
    }

private:
    int _nb_ech;
    std::ofstream _f;
    lambda_class(const lambda_class &a) = delete;
};

int main() {

    alsa_control *ac = new alsa_control(16000, 2048, 16, MONO);
    lambda_class *cb = new lambda_class();

    /*ac->show_ALSA_parameters();

    //listen and record to file
    ac->listen("aze");
    sleep(8);
    ac->stop();

    //record in a file for 10 seconds
    ac->record_to_file("azerty", 10000000);*/

    //record outside of the ALSA_control and inside, outside is in a special class
    ac->listen_with_callback(std::bind(&lambda_class::lambda_callback, cb, std::placeholders::_1, std::placeholders::_2), "qsd");
    sleep(8);
    ac->stop();

    delete ac;
    delete cb;

    return 0;
}