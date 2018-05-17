# ALSA_encapsulation
Encapsulation of the standard ALSA library
This library is multi-threaded.

# Example Usage
###### Allocation or instantiation
```cpp
AlsaControl *ac = new AlsaControl(16000, 2048, 16, MONO); //rate : 16000, frames: 2048, bits: 16
/* do something */
delete ac;
```

###### Recording in a file for a specific time (in micro-seconds)
```cpp
ac->RecordToFile("filename", 10000000); //recording to filename.wav for 10 seconds
```

###### Listening and recording to a file
```cpp
ac->Listen("filename");
/* do something */
ac->Stop();
```

###### Listening and using it inside an other class
```cpp
ac->ListenWithCallback(std::bind(&LambdaClass::LambdaCallback, class_to_call, std::placeholders::_1, std::placeholders::_2), "filename");
sleep(8);
ac->Stop();
```

The `LambdaCallback` must have the signature `void LambdaCallback(char* buffer, int rc)`

# Methods
#### Listen
`Listen()` and `Listen(std::string filename)`<br>
Start listening the audio input. If a filename is sent record the output in a file.<br>
**You must call Stop() before deleting the object**<br>
*`Listen()` is not really usefull*<br>

#### Listen with callback
`ListenWithCallback(std::function<void(char *, int)> func)` and `ListenWithCallback(std::function<void(char *, int)> func, std::string filename)`<br>
Start listening the audio input and send it to a callback. If a filename is sent record the output in a file.<br>
`char*` is the buffer with the part of the recording<br>
**You must call Stop() before deleting the object**<br>

#### Stop (listening)
`Stop()`<br>
Stop listening. Must be called after each listen* function
- `Listen()`
- `Listen(std::string filename)`
- `ListenWithCallback(std::function<void(char *, int)> func)`
- `ListenWithCallback(std::function<void(char *, int)> func, std::string filename)`

#### Record to file
`RecordToFile(std::string filename, int const &duration_in_us)`<br>
Start recording the audio input to the file.

#### ALSA parameters
`ShowALSAParameters()`<br>
Show all the parameters of the current sound card

#### Force period size
`ForcePeriodSize(int const &value)`<br>
Sometimes the period size gets wrong value (from `snd_pcm_hw_params_get_period_size`), this method force the period size to the value passed
