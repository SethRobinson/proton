// libVLC_RTSP.cpp
#include "PlatformPrecomp.h"
#include "libVLC_RTSP.h"
#include <GL/gl.h> // or appropriate OpenGL header for your platform
#include "App.h"
#include "Entity/LibVlcStreamComponent.h"


libvlc_instance_t* m_pVlcInstance = NULL;


libVLC_RTSP::libVLC_RTSP()
{

}

libVLC_RTSP::~libVLC_RTSP()
{
    Release();
}

void libVLC_RTSP::SetVolume(float vol)
{
    // Map the volume from [0, 1] to [0.4, 1] - don't ask me why, but that's how it seems here.  It's not even close to right, like 0.5 is 35%vol, but better
    
    /*
    float minVal = 0.4f;
    float mappedVol = minVal + (vol * 0.6f);
    if (mappedVol <= minVal)
    {
        mappedVol = 0; //force 0 for muting
    }
    
    int intVol = (int)(mappedVol * 100.0f);
    */

    int intVol = (int)(vol * 100.0f);
    
    if (m_pVlcMediaPlayer != nullptr)
    {
        LogMsg("Setting vol to %d, based on %.2f", intVol, vol);
        // Set the volume level. Values range between 0 and 100
        libvlc_audio_set_volume(m_pVlcMediaPlayer, intVol);
    }
}

static void handle_playing_event(const libvlc_event_t* p_event, void* p_data)
{
    libVLC_RTSP* self = reinterpret_cast<libVLC_RTSP*>(p_data);
    // Query and set the video dimensions here
    // Similar to the code you already wrote for getting dimensions

    unsigned width = 0, height = 0;
    if (libvlc_video_get_size(self->GetMP(), 0, &width, &height) == 0) // 0 is the number of the video output
    {
        // Resize your buffer and OpenGL texture accordingly
        
       // LogMsg("Got size of %d, %d", width, height);
        /*
        if (buffer) delete[] buffer;
        buffer = new unsigned char[width * height * 4];

        // Update the video format
        libvlc_video_set_format(mp, "RGBA", width, height, width * 4);
        */
    }
    else
    {
        // Handle the error: Could not get video size
    }

}


void* libVLC_RTSP::lock(void* data, void** p_pixels)
{
    // Lock the buffer
    libVLC_RTSP* self = reinterpret_cast<libVLC_RTSP*>(data);
    *p_pixels = self->m_pImageBuffer;
    return nullptr;
}

void libVLC_RTSP::unlock(void* data, void* id, void* const* p_pixels)
{
    // Update the OpenGL texture here with self->buffer
}

void libVLC_RTSP::display(void* data, void* id)
{
    // Display function: Typically a no-op in this kind of setup
}

void log_handler(void* data, int level, const libvlc_log_t* ctx, const char* fmt, va_list args)
{
    // Assuming LogMsg can take a string with a length of up to 1024 characters.
    // Adjust the buffer size as needed.
    char buffer[1024];

    // Format the string
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    // Now call your LogMsg function
    LogMsg(buffer);
}

void  libVLC_RTSP::InitVideoSurfaces()
{
    //reinit video stuff in libvlc, being sure to properly handle locks and like
    // Set the video callbacks and format

    LogMsg("Setting size to %d, %d", m_video_width, m_video_height);
    m_pStreamComp->SetSurfaceSize(m_video_width, m_video_height);
    
    SAFE_DELETE_ARRAY(m_pImageBuffer);
    m_pImageBuffer = new unsigned char[m_video_width * m_video_height * 4]; // 4 bytes per pixel for RGBA
    libvlc_video_set_format(m_pVlcMediaPlayer, "RGBA", m_video_width, m_video_height, m_video_width * 4); // assuming 1920x1080 resolution and RGBA format

}


bool libVLC_RTSP::Init(const std::string& rtsp_url, int cachingMS, SurfaceAnim* pSurfaceToWriteTo, LibVlcStreamComponent * pStreamComp, int width, int height)
{
    m_pSurface = pSurfaceToWriteTo;
    m_pStreamComp = pStreamComp;
    m_source = rtsp_url;
    m_cachingMS = cachingMS;

   // putenv("VLC_VERBOSE=-1"); //also doesn't work


    //adding these parms are for forcing no hardware accelleration which fixes things for me
   //--avcodec-hw=none --avcodec-fast --no-avcodec-hurry-up

    const char* const vlc_args[] = {
      "-I", "dummy",
      "--no-video-title-show",
      "--quiet",
      "--no-xlib",
      "--avcodec-hw=none",
      "--avcodec-fast",
      "--no-avcodec-hurry-up",
      "--aout=directsound", //otherwise I can't set audio per stream, it would affect all streams.  
     //, "--verbose=-1" // explicitly set verbosity level, 'cept it doesn't work
    };

    if (m_pVlcInstance == NULL)
    {
        m_pVlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);

        //libvlc_log_set(m_pVlcInstance, log_handler, NULL);

        if (m_pVlcInstance == nullptr) return false;
    }

    m_pVlcMediaPlayer = libvlc_media_player_new(m_pVlcInstance);
    if (m_pVlcMediaPlayer == nullptr) return false;

    m_pVlcMedia = libvlc_media_new_location(m_pVlcInstance, rtsp_url.c_str());
    if (m_pVlcMedia == nullptr) return false;

    // Reduce network caching 

    string caching = ":network-caching=" + toString(cachingMS);

    libvlc_media_add_option(m_pVlcMedia, caching.c_str());

    libvlc_media_player_set_media(m_pVlcMediaPlayer, m_pVlcMedia);

    // Attach event handlers
    libvlc_event_manager_t* em = libvlc_media_player_event_manager(m_pVlcMediaPlayer);
    libvlc_event_attach(em, libvlc_MediaPlayerPlaying, handle_playing_event, this);

  
    // Set the video callbacks and format
    libvlc_video_set_callbacks(m_pVlcMediaPlayer, lock, unlock, display, this);
   
    // Initialize buffer for OpenGL texture data, assuming 1920x1080 resolution
    
    InitVideoSurfaces();
    libvlc_media_player_play(m_pVlcMediaPlayer);

    return true;
}

void libVLC_RTSP::UpdateFrame()
{
    if (m_pVlcMediaPlayer != nullptr)
    {
        // logic for updating frame if needed
        // this depends on how you intend to interact with OpenGL

        unsigned width = 0, height = 0;
        if (libvlc_video_get_size(GetMP(), 0, &width, &height) == 0) // 0 is the number of the video output
        {
            // Resize your buffer and OpenGL texture accordingly

            //LogMsg("Frame update: Got size of %d, %d", width, height);
            if (width != 0)
            {
                //valid size.  But does it match what we've got?
               
               if (width != m_video_width || height != m_video_height)
                {
                   //I had problems trying to live-change things, so doing a complete reinit for this size change:
                   Release();

                    m_video_width = width;
                    m_video_height = height;
                    Init( m_source, m_cachingMS, m_pSurface, m_pStreamComp, m_video_width, m_video_height);
                    return;
               }
               
                m_pSurface->UpdateSurfaceRect(rtRect(0, 0, m_video_width, m_video_height), m_pImageBuffer);
            }
          
        }
        else
        {
            // Handle the error: Could not get video size
        }

  
    }
}

void libVLC_RTSP::Update()
{
    if (m_pVlcMediaPlayer != nullptr)
    {
        libvlc_media_player_play(m_pVlcMediaPlayer);
    }

    UpdateFrame();
}

void libVLC_RTSP::Release()
{
    if (m_pVlcMediaPlayer != nullptr)
    {
        libvlc_media_player_stop(m_pVlcMediaPlayer);
        libvlc_media_player_release(m_pVlcMediaPlayer);

        m_pVlcMediaPlayer = NULL;

    }

    if (m_pVlcMedia != nullptr)
    {
        libvlc_media_release(m_pVlcMedia);
        m_pVlcMedia = NULL;
    }


    SAFE_DELETE_ARRAY(m_pImageBuffer);

   // SAFE_DELETE(m_pSurface);  No, the OverlayRenderComponent will delete it by default
}

void OneTimeReleaseOnAppClose()
{
    if (m_pVlcInstance != nullptr)
    {
        libvlc_release(m_pVlcInstance);
        m_pVlcInstance = NULL;
    }
}
