#include <iostream>
#include <alsa_control.h>

using std::cout;
using std::endl;


class lambda_class {
public:
  void lambda_callback(void *c, int rc) {
    this->sample_count_ += rc;
    this->output_file_.write(static_cast<char *>(c), rc * 2);
  }

  lambda_class() {
    this->sample_count_ = 0;
    this->output_file_.open("sample_8_sec_with_callback_class_callback_record.wav", std::ios::binary);
    write_header_wav(this->output_file_, 16000, 16, MONO, 10000);
  }

  ~lambda_class() {
    this->output_file_.close();
    this->output_file_.open("sample_8_sec_with_callback_class_callback_record.wav",
                            std::ios::binary | std::ios::in);
    write_header_wav(this->output_file_, 16000, 16, MONO, this->sample_count_);
  }

private:
  int sample_count_;
  std::ofstream output_file_;
  lambda_class(const lambda_class &a) = delete;
};

int main() {
  alsa_control *ac = new alsa_control(16000, 2048, 16, MONO);
  lambda_class *class_to_call = new lambda_class();

  ac->show_ALSA_parameters();

  std::cout << std::endl << "Listen and record to file" << std::endl;
  ac->listen("sample_8_sec_with_listen");
  sleep(8);
  ac->stop();

  std::cout << std::endl << "Record in a file for 10 seconds" << std::endl;
  ac->record_to_file("sample_10_sec_with_record", 10000000);

  std::cout << std::endl << "Record inside and outside of the ALSA_control, outside is in a special class" << std::endl;
  ac->listen_with_callback(
      std::bind(&lambda_class::lambda_callback, class_to_call, std::placeholders::_1, std::placeholders::_2),
      "sample_8_sec_with_callback_alsa_encap_record");
  sleep(8);
  ac->stop();

  delete ac;
  delete class_to_call;

  return 0;
}
