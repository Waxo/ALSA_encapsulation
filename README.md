#ALSA_encapsulation
Encapsulation of the standard ALSA library
This library is multi-threaded.

#Example Usage
######Allocation or instantiation
```cpp
alsa_control *ac = new alsa_control(16000, 2048, 16, MONO); //rate : 16000, frames: 2048, bits: 16
/* do something */
delete ac;
```

######Recording in a file for a specific time (in micro-seconds)
```cpp
ac->record_to_file("filename", 10000000); //recording to filename.wav for 10 seconds
```

######Listening and recording to a file
```cpp
ac->listen("filename");
/* do something */
ac->stop();
```

######Listening and using it inside an other class
```cpp
ac->listen_with_callback(std::bind(&lambda_class::lambda_callback, cb, std::placeholders::_1, std::placeholders::_2), "qsd");
sleep(8);
ac->stop();
```

The `lambda_callback` must have the signature `void lambda_callback(char* buffer, int rc)`

#Methods
####Listen
`listen()` and `listen(std::string filename)`<br>
Start listening the audio input. If a filename is sent record the output in a file.<br>
**You must call stop() before deleting the object**<br>
*`listen()` is not really usefull*<br>

####Listen with callback
`listen_with_callback(std::function<void(char *, int)> func)` and `listen_with_callback(std::function<void(char *, int)> func, std::string filename)`<br>
Start listening the audio input and send it to a callback. If a filename is sent record the output in a file.<br>
`char*` is the buffer with the part of the recording<br>
**You must call stop() before deleting the object**<br>

####Stop (listening)
`stop()`<br>
Stop listening. Must be called after each listen* function
- `listen()`
- `listen(std::string filename)`
- `listen_with_callback(std::function<void(char *, int)> func)`
- `listen_with_callback(std::function<void(char *, int)> func, std::string filename)`

####Record to file
`record_to_file(std::string filename, int duration_in_us)`<br>
Start recording the audio input to the file.

####ALSA parameters
`show_ALSA_parameters()`<br>
Show all the parameters of the current sound card
