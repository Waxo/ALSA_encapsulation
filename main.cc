#include <iostream>
#include "AlsaControl.h"

using std::cout;
using std::endl;


class LambdaClass {
public:
  void LambdaCallback(void *c, int rc) {
    this->sample_count_ += rc;
    this->output_file_.write(static_cast<char *>(c), rc * 2);
  }

  LambdaClass() {
    this->sample_count_ = 0;
    this->output_file_.open("sample_8_sec_with_callback_class_callback_record.wav", std::ios::binary);
    WriteHeaderWav(this->output_file_, 16000, 16, MONO, 10000);
  }

  virtual ~LambdaClass() {
    this->output_file_.close();
    this->output_file_.open("sample_8_sec_with_callback_class_callback_record.wav",
                            std::ios::binary | std::ios::in);
    WriteHeaderWav(this->output_file_, 16000, 16, MONO, this->sample_count_);
  }

private:
  int sample_count_;
  std::ofstream output_file_;
  LambdaClass(const LambdaClass &a) = delete;
};

int main() {
  AlsaControl *ac = new AlsaControl(16000, 2048, 16, MONO);
  LambdaClass *class_to_call = new LambdaClass();

  ac->ShowALSAParameters();

  std::cout << std::endl << "Listen and record to file" << std::endl;
  ac->Listen("sample_8_sec_with_listen");
  sleep(8);
  ac->Stop();

  std::cout << std::endl << "Record in a file for 10 seconds" << std::endl;
  ac->RecordToFile("sample_10_sec_with_record", 10000000);

  std::cout << std::endl << "Record inside and outside of the ALSA_control, outside is in a special class" << std::endl;
  ac->ListenWithCallback(
      std::bind(&LambdaClass::LambdaCallback, class_to_call,
          std::placeholders::_1, std::placeholders::_2
      ),
      "sample_8_sec_with_callback_alsa_encap_record");
  sleep(8);
  ac->Stop();

  delete ac;
  delete class_to_call;

  return 0;
}
