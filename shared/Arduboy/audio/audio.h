#ifndef ArduboyAudio_h
#define ArduboyAudio_h

class ArduboyAudio
{
public:
  void static begin();
  void static on();
  void static off();
  void static saveOnOff();
  bool static enabled();


  bool static audio_enabled;
};

#endif
