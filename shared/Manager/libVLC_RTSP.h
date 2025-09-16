// libVLC_RTSP.h
#pragma once
#include <string>
#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif
#include <vlc/vlc.h>
#include <mutex>

class LibVlcStreamComponent;

struct WebcamConfig
{
    std::string deviceName;
    int width = 0;
    int height = 0;
    float fps = 0;
};

struct VLC_ExtraSettings
{
	rtRect cropRect = rtRect(0,0,0,0); //all 0's if unused
};

class libVLC_RTSP
{
public:
    libVLC_RTSP();
    virtual ~libVLC_RTSP();

    void InitVideoSurfaces();
    bool Init(const std::string& rtsp_url, int cachingMS, SurfaceAnim* pSurfaceToWriteTo, LibVlcStreamComponent* pStreamComp, 
        int width, int height, VLC_ExtraSettings = VLC_ExtraSettings());
   
    bool InitWebcam(const std::string& deviceNameOrId, SurfaceAnim* pSurfaceToWriteTo,
        LibVlcStreamComponent* pStreamComp, int width, int height, VLC_ExtraSettings = VLC_ExtraSettings());

    std::tuple<int, int, int> GetRotationAndSizeOfVideoFile(const std::string& fileName);
    void GetMetaData();
    void Update();

    void SetPause(bool bPause);
    bool GetPause();
    void TogglePause();

    void SetLooping(bool bLoop);

     float GetVolume();
    void SetPlaybackPosition(float pos);
    float GetPlaybackPosition();
    void SetVolume(float vol);  // Set the volume level. Values range between 0 and 1

    libvlc_media_player_t* GetMP() { return m_pVlcMediaPlayer; }
    void Release();

    std::string FindFullDeviceName(const std::string& partialName);
    bool StringIsNumber(const std::string& s);
    std::string FindMatchingAudioDevice(const std::string& videoDeviceName);

    enum eStatus
    {
        C_STATUS_INITTED,
        C_STATUS_PAUSED,
        C_STATUS_UNPAUSED,
        C_STATUS_SET_VOLUME
    };

    boost::signals2::signal<void(VariantList*)> m_sig_update_status; //a way to get a callback when something important changes

    bool IsCropped();

protected:
    
    static void* lock(void* data, void** p_pixels);
    static void unlock(void* data, void* id, void* const* p_pixels);
    static void display(void* data, void* id);


    unsigned char* m_pCroppedBuffer = nullptr; // Add this member

    VLC_ExtraSettings m_extraSettings;

    std::string m_audioDeviceName;  // Store the found audio device name
    bool m_isWebcam = false;
    std::string GetWebcamDevicePath(const std::string& deviceNameOrId);
  
    void SendStatusUpdate(eStatus status, float secondFloat = 0.0f);
    int GetVideoRotation(); // Get rotation metadata from video

    void UpdateFrame();

    unsigned char* m_pImageBuffer = NULL; // Buffer for OpenGL texture data

    int m_video_width = 320;
    int m_video_height = 200;

    libvlc_media_player_t* m_pVlcMediaPlayer = NULL;
    libvlc_media_t* m_pVlcMedia = NULL;

    SurfaceAnim *m_pSurface = NULL;
    LibVlcStreamComponent* m_pStreamComp = NULL;
    string m_source;
    int m_cachingMS;

    int m_rotationAngle =0;
    bool m_isStreaming = false;
    int m_timesChangedVideoResolution = 0;
    std::mutex m_bufferMutex;
    bool m_bLoopVideo = false;
};

void OneTimeReleaseOnAppClose(); //you should probably call this when you exit the app, to release the main VLC instance

void ListWebcamDevices(); //Easy way to list all webcam devices, not connected to VLC, but needs dshow stuff and coinit to have been done