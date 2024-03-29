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

float libVLC_RTSP::GetVolume()
{
    if (m_pVlcMediaPlayer != nullptr)
    {
		// Get the volume level. Values range between 0 and 100
		float vol = (float)libvlc_audio_get_volume(m_pVlcMediaPlayer) / 100.0f;
        return vol;
	}
	return 0;
}

void libVLC_RTSP::SetPlaybackPosition(float pos)
{
    if (m_pVlcMediaPlayer != nullptr)
    {
		// Set the playback position as a percentage between 0.0 and 1.0
        libvlc_media_player_set_position(m_pVlcMediaPlayer, pos);
	}
}

float libVLC_RTSP::GetPlaybackPosition()
{
    if (m_pVlcMediaPlayer != nullptr)
    {
		// Get the playback position as a percentage between 0.0 and 1.0
		return libvlc_media_player_get_position(m_pVlcMediaPlayer);
	}

    return 0;
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
    
    //force range between 0 and 100
    if (intVol < 0) intVol = 0;
    if (intVol > 100) intVol = 100;

    if (m_pVlcMediaPlayer != nullptr)
    {
        LogMsg("Setting vol to %d, based on %.2f", intVol, vol);
        // Set the volume level. Values range between 0 and 100
        libvlc_audio_set_volume(m_pVlcMediaPlayer, intVol);
    }

    SendStatusUpdate(C_STATUS_SET_VOLUME, vol);

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



std::wstring StringToWString(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string WStringToString(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}


bool libVLC_RTSP::Init(const std::string& rtsp_url, int cachingMS, SurfaceAnim* pSurfaceToWriteTo, LibVlcStreamComponent * pStreamComp, int width, int height)
{
    Release();

    m_pSurface = pSurfaceToWriteTo;
    m_pStreamComp = pStreamComp;
    m_source = rtsp_url;
    m_cachingMS = cachingMS;
    m_timesChangedVideoResolution++;

   // putenv("VLC_VERBOSE=-1"); //also doesn't work

    //adding these parms are for forcing no hardware accelleration which fixes things for me
   //--avcodec-hw=none --avcodec-fast --no-avcodec-hurry-up

    const char* const vlc_args[] = {
      "-I", "dummy",
      "--no-video-title-show",
      "--quiet",
      "--no-xlib",
   //   "--avcodec-hw=none",
  //    "--avcodec-fast",
  //    "--no-avcodec-hurry-up",
      "--aout=directsound", //otherwise I can't set audio per stream, and even this seems to not really work right
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

    if (IsInStringCaseInsensitive(rtsp_url, "rtsp://"))
    {
        m_isStreaming = true;
        m_pVlcMedia = libvlc_media_new_location(m_pVlcInstance, rtsp_url.c_str());

        string caching = ":network-caching=" + toString(cachingMS);

        libvlc_media_add_option(m_pVlcMedia, caching.c_str());

    }
    else
    {
       

        //it's a file on our HD, let's play the file instead of trying to stream it

        //fix filename to work with umlauts and junk by converting from raw ascii to utf-8.  Sorry for the windows specific code here

        string filename = rtsp_url;
        std::wstring wideFilename = StringToWString(filename); // Step 1: Only if necessary
        std::string utf8Filename = WStringToString(wideFilename); // Step 2

        
        //note, the original filename (with an umlaut) works with fopen, but not with VLC

        /*
        if (FileExists(filename) == false)
        {
			LogMsg("File doesn't exist: %s", filename.c_str()); //this never gets called
			return false;
		}
        */      

        //but our converted utf-8 version will work with VLC

        m_pVlcMedia = libvlc_media_new_location(m_pVlcInstance, (string("file:///")+ utf8Filename).c_str());
    }
 
    if (m_pVlcMedia == nullptr) return false;


    libvlc_media_player_set_media(m_pVlcMediaPlayer, m_pVlcMedia);

    // Attach event handlers
    libvlc_event_manager_t* em = libvlc_media_player_event_manager(m_pVlcMediaPlayer);
    libvlc_event_attach(em, libvlc_MediaPlayerPlaying, handle_playing_event, this);
 
    // Set the video callbacks and format
    libvlc_video_set_callbacks(m_pVlcMediaPlayer, lock, unlock, display, this);
   
    // Initialize buffer for OpenGL texture data, assuming 1920x1080 resolution
   	m_video_width = width;
	m_video_height = height;
    InitVideoSurfaces();

    libvlc_media_player_play(m_pVlcMediaPlayer);
   
   return true;
}

//Unused as I figured out how to do it with libVLC

/*
std::tuple<int, int, int> libVLC_RTSP::GetRotationAndSizeOfVideoFile(const std::string& fileName)
{
    std::string command = "tools\\ffmpeg.exe -i \"" + fileName + "\" 2>&1";

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));

    // Create pipe for standard output redirection
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
    {
        std::cerr << "CreatePipe failed" << std::endl;
        return std::make_tuple(0, 0, 0);
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited
    if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0))
    {
        std::cerr << "Stdout SetHandleInformation failed" << std::endl;
        return std::make_tuple(0, 0, 0);
    }

    si.hStdError = hWritePipe;
    si.hStdOutput = hWritePipe;

    // Start the child process
    if (!CreateProcessA(NULL, &command[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ")" << std::endl;
        return std::make_tuple(0,0,0);
    }

    std::string output;
    DWORD bytesRead;
    CHAR buffer[4096];
    BOOL result;

    // Close the write end of the pipe before reading from the read end of the pipe
    CloseHandle(hWritePipe);

    // Read output from the child process
    do {
        result = ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesRead, NULL);
        if (bytesRead)
        {
            output.append(buffer, bytesRead);
        }
    } while (result && bytesRead);

    CloseHandle(hReadPipe);

    // Wait until child process exits
    WaitForSingleObject(pi.hProcess, INFINITE);
    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Extract rotation info
    std::string rotationKey = "rotate          :";
    int rotation = 0;
    auto pos = output.find(rotationKey);
    if (pos != std::string::npos)
    {
        pos += rotationKey.length();
        auto end = output.find('\n', pos);
        std::string rotationStr = output.substr(pos, end - pos);
        try {
            rotation = std::stoi(rotationStr);
        }
        catch (const std::exception&) 
        {
            rotation = 0;
        }
    }

    int width = 0;
    int height = 0; //todo

    return std::make_tuple(rotation, width, height);
}

*/

void libVLC_RTSP::SendStatusUpdate(eStatus status, float secondFloat)
{
    VariantList v;

    v.Get(0).Set(uint32(status));
    v.Get(1).Set(secondFloat);
    m_sig_update_status(&v);
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
               
                if (m_isStreaming || m_timesChangedVideoResolution < 2)
                {
                    if (width != m_video_width || height != m_video_height)
                    {

                        // Check for rotation
                        libvlc_media_t* p_media = libvlc_media_player_get_media(m_pVlcMediaPlayer);
                        libvlc_media_parse(p_media);
                        libvlc_media_track_t** tracks;
                        int num_tracks = libvlc_media_tracks_get(p_media, &tracks);

                        for (int i = 0; i < num_tracks; i++)
                        {
                            if (tracks[i]->i_type == libvlc_track_video && tracks[i]->video->i_orientation != libvlc_video_orient_top_left && tracks[i]->video->i_orientation != libvlc_video_orient_bottom_right)
                            {
                                // Video is rotated, swap width and height
                                swap(width, height);
                                break;
                            }
                        }

                        libvlc_media_tracks_release(tracks, num_tracks);

                        //I had problems trying to live-change things, so doing a complete reinit for this size change:
                      
                        m_video_width = width;
                        m_video_height = height;
                        Init(m_source, m_cachingMS, m_pSurface, m_pStreamComp, m_video_width, m_video_height);
                       
                        SendStatusUpdate(C_STATUS_INITTED);
                        SendStatusUpdate(C_STATUS_UNPAUSED);
                        return;
                    }
                }
               
                m_pSurface->UpdateSurfaceRect(rtRect(0, 0, m_video_width, m_video_height), m_pImageBuffer);
                
            }
        }
        else
        {
            // Handle the error: Could not get video size
            //  LogMsg("Can't get video buffer");  //yeah well, that's normal if it's an audio file
        }
    }
}

void libVLC_RTSP::Update()
{
    if (m_pVlcMediaPlayer != nullptr)
    {
        //if video isn't playing, restart it
          // Get the current state of the media player
        libvlc_state_t state = libvlc_media_player_get_state(m_pVlcMediaPlayer);

        if (state == libvlc_Ended)
        {
            if (m_bLoopVideo)
            {
                // The media has ended, restart it
                LogMsg("Restarting...");
                float volTemp = GetVolume();
                libvlc_media_player_stop(m_pVlcMediaPlayer); // Stop the player
                libvlc_media_player_play(m_pVlcMediaPlayer); // Start playback again
                SendStatusUpdate(C_STATUS_INITTED);
                SendStatusUpdate(C_STATUS_UNPAUSED);
                SetVolume(volTemp); 

            }
        }
        else if (libvlc_media_player_is_playing(m_pVlcMediaPlayer))
        {
            // The media is playing
        }
        else
        {
            // The media is paused
        }
    }

    UpdateFrame();
}

bool libVLC_RTSP::GetPause()
{
    if (m_pVlcMediaPlayer != nullptr)
    {
        // Get the current state of the media player
        libvlc_state_t state = libvlc_media_player_get_state(m_pVlcMediaPlayer);
       
        if (state == libvlc_Paused)
        {
            return true;
        }
    }

    return false;
}

void libVLC_RTSP::SetPause(bool bPause)
{
    if (m_pVlcMediaPlayer != nullptr)
    {
        // Get the current state of the media player
        libvlc_state_t state = libvlc_media_player_get_state(m_pVlcMediaPlayer);

        if (state == libvlc_Ended)
        {
            if (!bPause)
            {
                // The media has ended, restart it
                LogMsg("Restarting...");
                libvlc_media_player_stop(m_pVlcMediaPlayer); // Stop the player
                libvlc_media_player_play(m_pVlcMediaPlayer); // Start playback again
                SendStatusUpdate(C_STATUS_INITTED);
                SendStatusUpdate(C_STATUS_UNPAUSED);

            }
        }
        else if (libvlc_media_player_is_playing(m_pVlcMediaPlayer))
        {
            if (bPause)
            {
                // The media is playing, pause it
                LogMsg("Pausing...");
                libvlc_media_player_pause(m_pVlcMediaPlayer);
                SendStatusUpdate(C_STATUS_PAUSED);

            }
        }
        else
        {
            // The media is paused, unpause it
            if (!bPause)
            {
                LogMsg("Unpausing...");
                libvlc_media_player_set_pause(m_pVlcMediaPlayer, 0);
                libvlc_media_player_play(m_pVlcMediaPlayer);
                SendStatusUpdate(C_STATUS_UNPAUSED);

            }
        }
	}
}

void libVLC_RTSP::TogglePause()
{
    SetPause(!GetPause());
}

void libVLC_RTSP::SetLooping(bool bLoop)
{
    m_bLoopVideo = bLoop;

    //this method didn't work I guess, I'LL DO IT LIVE!
     
    /*
    // Ensure the media player is initialized
    if (m_pVlcMediaPlayer == nullptr)
        return;

    // Create a media list and add the current media to it
    libvlc_media_list_t* pMediaList = libvlc_media_list_new(m_pVlcInstance);
    libvlc_media_t* pCurrentMedia = libvlc_media_player_get_media(m_pVlcMediaPlayer);
    if (pCurrentMedia == nullptr)
    {
        // Handle error: No media is currently loaded in the media player
        return;
    }
    libvlc_media_list_add_media(pMediaList, pCurrentMedia);

    // Create a media list player and associate it with the media player
    libvlc_media_list_player_t* pMediaListPlayer = libvlc_media_list_player_new(m_pVlcInstance);
    libvlc_media_list_player_set_media_player(pMediaListPlayer, m_pVlcMediaPlayer);
    libvlc_media_list_player_set_media_list(pMediaListPlayer, pMediaList);

    // Set the looping mode
    if (bLoop)
        libvlc_media_list_player_set_playback_mode(pMediaListPlayer, libvlc_playback_mode_loop);
    else
        libvlc_media_list_player_set_playback_mode(pMediaListPlayer, libvlc_playback_mode_default);

    // Release references to the media list and media list player
    libvlc_media_release(pCurrentMedia);
    libvlc_media_list_release(pMediaList);
    libvlc_media_list_player_release(pMediaListPlayer);

    */


}

void libVLC_RTSP::Release()
{
    if (m_pVlcMediaPlayer != nullptr)
    {

        libvlc_event_manager_t* em = libvlc_media_player_event_manager(m_pVlcMediaPlayer);
        if (em)
            libvlc_event_detach(em, libvlc_MediaPlayerPlaying, handle_playing_event, this);


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
