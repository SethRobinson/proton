#include "PlatformPrecomp.h"
#include "libVLC_RTSP.h"
#include <GL/gl.h> // or appropriate OpenGL header for your platform
#include <algorithm> // for std::swap
#include <string> // for std::stoi
#include "App.h"
#include "Entity/LibVlcStreamComponent.h"
#include "Renderer/SoftSurface.h"

libvlc_instance_t* m_pVlcInstance = NULL;

libVLC_RTSP::libVLC_RTSP()
{

}

libVLC_RTSP::~libVLC_RTSP()
{
  
    Release();

    SAFE_DELETE_ARRAY(m_pCroppedBuffer);

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
        //LogMsg("Setting vol to %d, based on %.2f", intVol, vol);
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
    libVLC_RTSP* self = reinterpret_cast<libVLC_RTSP*>(data);
   
    
    *p_pixels = self->m_pImageBuffer;
    return nullptr;
}

void libVLC_RTSP::unlock(void* data, void* id, void* const* p_pixels)
{
    libVLC_RTSP* self = reinterpret_cast<libVLC_RTSP*>(data);

    if (!IsBaseAppInitted()) return;

   
    if (self->IsCropped())
    {
        const auto& crop = self->m_extraSettings.cropRect;

        // Ensure crop rectangle is valid
        if (crop.right > crop.left && crop.bottom > crop.top &&
            crop.right <= self->m_video_width && crop.bottom <= self->m_video_height)
        {
            int cropWidth = crop.right - crop.left;
            int cropHeight = crop.bottom - crop.top;

            // Copy only the cropped region to m_pCroppedBuffer
            for (int y = 0; y < cropHeight; y++)
            {
                for (int x = 0; x < cropWidth; x++)
                {
                    int srcX = x + crop.left;
                    int srcY = y + crop.top;

                    int srcIdx = (srcY * self->m_video_width + srcX) * 4;
                    int dstIdx = (y * cropWidth + x) * 4;

                    // Copy RGBA values
                    memcpy(&self->m_pCroppedBuffer[dstIdx],
                        &self->m_pImageBuffer[srcIdx], 4);
                }
            }
        }
    }
  
  
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
void libVLC_RTSP::InitVideoSurfaces()
{
    // Calculate dimensions based on crop if enabled
    int effectiveWidth = m_video_width;
    int effectiveHeight = m_video_height;

    if (IsCropped())
    {
        effectiveWidth = m_extraSettings.cropRect.right - m_extraSettings.cropRect.left;
        effectiveHeight = m_extraSettings.cropRect.bottom - m_extraSettings.cropRect.top;
    }

    LogMsg("Setting size to %d, %d (original: %d, %d)",
        effectiveWidth, effectiveHeight, m_video_width, m_video_height);

    m_pStreamComp->SetSurfaceSize(effectiveWidth, effectiveHeight);

    // Always allocate full size buffer for incoming frames
    SAFE_DELETE_ARRAY(m_pImageBuffer);
    m_pImageBuffer = new unsigned char[m_video_width * m_video_height * 4];

    // If we're cropping, we need a second buffer for the cropped output
    if (IsCropped())
    {
        SAFE_DELETE_ARRAY(m_pCroppedBuffer);
        m_pCroppedBuffer = new unsigned char[effectiveWidth * effectiveHeight * 4];
    }

    
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

bool libVLC_RTSP::InitWebcam(const std::string& deviceNameOrId, SurfaceAnim* pSurfaceToWriteTo,
    LibVlcStreamComponent* pStreamComp, int width, int height, VLC_ExtraSettings settings)
{
    m_isWebcam = true;
    std::string devicePath = GetWebcamDevicePath(deviceNameOrId);
    return Init(devicePath, 0, pSurfaceToWriteTo, pStreamComp, width, height, settings);
}

bool libVLC_RTSP::StringIsNumber(const std::string& s)
{
    return !s.empty() &&
        std::find_if(s.begin(), s.end(), [](unsigned char c) {
        return !std::isdigit(c);
            }) == s.end();
}


WebcamConfig ParseWebcamString(const std::string& input) {
    WebcamConfig config;

    // Look for REQUEST: format
    size_t requestPos = input.find(" REQUEST:");
    if (requestPos == std::string::npos) {
        // No resolution request, just return the device name
        config.deviceName = input;
        return config;
    }

    // Extract the base device name
    config.deviceName = input.substr(0, requestPos);

    // Parse the params
    std::string params = input.substr(requestPos + 9); // Skip " REQUEST:"
    std::vector<std::string> values;
    size_t pos = 0;
    std::string token;
    while ((pos = params.find(":")) != std::string::npos) {
        token = params.substr(0, pos);
        values.push_back(token);
        params.erase(0, pos + 1);
    }
    values.push_back(params); // Add the last value

    // Handle different parameter counts
    if (values.size() == 1) {
        // Single parameter - interpret as FPS
        try {
            config.fps = std::stof(values[0]);
            LogMsg("Parsed FPS request: %f", config.fps);
        }
        catch (...) {
            LogMsg("Failed to parse FPS value");
        }
    }
    else if (values.size() >= 2) {
        // Two or more parameters - first two are width/height
        try {
            config.width = std::stoi(values[0]);
            config.height = std::stoi(values[1]);
            LogMsg("Parsed resolution request: %dx%d", config.width, config.height);
        }
        catch (...) {
            LogMsg("Failed to parse resolution values");
        }

        // If there's a third parameter, it's FPS
        if (values.size() >= 3) {
            try {
                config.fps = std::stof(values[2]);
                LogMsg("Parsed FPS request: %f", config.fps);
            }
            catch (...) {
                LogMsg("Failed to parse FPS value");
            }
        }
    }

    return config;
}
bool libVLC_RTSP::Init(const std::string& rtsp_url, int cachingMS, SurfaceAnim* pSurfaceToWriteTo, LibVlcStreamComponent* pStreamComp, int width, int height, VLC_ExtraSettings settings)
{
    Release();

	m_extraSettings = settings;
    m_pSurface = pSurfaceToWriteTo;
    m_pStreamComp = pStreamComp;
    m_source = rtsp_url;
    m_cachingMS = cachingMS;
    m_timesChangedVideoResolution++;

    // Parse any REQUEST parameters from the URL
    std::string baseUrl = rtsp_url;
    WebcamConfig config;
    size_t requestPos = rtsp_url.find(" REQUEST:");
    if (requestPos != std::string::npos) {
        baseUrl = rtsp_url.substr(0, requestPos);
        config = ParseWebcamString(rtsp_url);

        // Override width/height if specified
        if (config.width > 0 && config.height > 0) {
            width = config.width;
            height = config.height;
            LogMsg("Using requested resolution: %dx%d", width, height);
        }
    }

    std::string deviceName = "default";

    if (baseUrl.find("dshow://vdev=") == 0) {
        size_t startPos = strlen("dshow://vdev=");
        deviceName = baseUrl.substr(startPos);
        if (deviceName.front() == '"') deviceName = deviceName.substr(1);
        if (deviceName.back() == '"') deviceName = deviceName.substr(0, deviceName.length() - 1);

        // Find the full device name if a partial name was provided
        std::string baseDeviceName = deviceName;
        if (!StringIsNumber(baseDeviceName)) {
            deviceName = FindFullDeviceName(baseDeviceName);
            LogMsg("Found full video device name: %s", deviceName.c_str());
        }

        // Find matching audio device using the base device name
        m_audioDeviceName = FindMatchingAudioDevice(baseDeviceName);
    }

    std::vector<const char*> vlc_args = {
        "-I", "dummy",
        "--no-video-title-show",
        "--quiet",
        "--no-xlib",
        "--aout=directsound"
    };

    if (baseUrl.find("dshow://") == 0) {
        LogMsg("Initializing webcam with device: %s", deviceName.c_str());

        std::string vdev = std::string("--dshow-vdev=") + deviceName;
        char* vdev_arg = new char[vdev.length() + 1];
        strcpy(vdev_arg, vdev.c_str());
        vlc_args.push_back(vdev_arg);

        vlc_args.push_back("--live-caching=0");
    }

    if (m_pVlcInstance == NULL)
    {
        LogMsg("Creating VLC instance with %d arguments", (int)vlc_args.size());
        for (size_t i = 0; i < vlc_args.size(); i++) {
            LogMsg("VLC arg %d: %s", (int)i, vlc_args[i]);
        }

        m_pVlcInstance = libvlc_new(vlc_args.size(), vlc_args.data());
        if (m_pVlcInstance == nullptr) {
            LogMsg("Failed to create VLC instance");
            if (baseUrl.find("dshow://") == 0) {
                delete[] vlc_args[vlc_args.size() - 2]; // vdev
            }
            return false;
        }
    }

    if (baseUrl.find("dshow://") == 0) {
        delete[] vlc_args[vlc_args.size() - 2]; // vdev
    }

    m_pVlcMediaPlayer = libvlc_media_player_new(m_pVlcInstance);
    if (m_pVlcMediaPlayer == nullptr) return false;

    if (IsInStringCaseInsensitive(baseUrl, "rtsp://"))
    {
        m_isStreaming = true;
        m_pVlcMedia = libvlc_media_new_location(m_pVlcInstance, baseUrl.c_str());
        string caching = ":network-caching=" + toString(cachingMS);
        libvlc_media_add_option(m_pVlcMedia, caching.c_str());

        // Add resolution options if specified for RTSP
        if (width > 0 && height > 0) {
            libvlc_media_add_option(m_pVlcMedia, (":rtsp-size=" + std::to_string(width) + "x" + std::to_string(height)).c_str());
            if (config.fps > 0) {
                libvlc_media_add_option(m_pVlcMedia, (":rtsp-fps=" + std::to_string(config.fps)).c_str());
            }
        }
    }
    else if (baseUrl.find("dshow://") == 0)
    {
        m_isStreaming = true;
        LogMsg("Creating media for URL: %s", baseUrl.c_str());
        m_pVlcMedia = libvlc_media_new_location(m_pVlcInstance, baseUrl.c_str());

        // Add webcam-specific options
        libvlc_media_add_option(m_pVlcMedia, ":live-caching=0");

        // Add resolution options BEFORE setting the device
        if (width > 0 && height > 0) {
            std::string res = ":dshow-vdev-width=" + std::to_string(width) +
                " :dshow-vdev-height=" + std::to_string(height);
            libvlc_media_add_option(m_pVlcMedia, res.c_str());

            // Also try adding these additional options
            libvlc_media_add_option(m_pVlcMedia, (":dshow-size=" + std::to_string(width) + "x" + std::to_string(height)).c_str());

            LogMsg("Setting resolution options: %s", res.c_str());
        }

        if (!m_audioDeviceName.empty()) {
            std::string audioOpt = ":dshow-adev=" + m_audioDeviceName;
            LogMsg("Adding audio device: %s", audioOpt.c_str());
            libvlc_media_add_option(m_pVlcMedia, audioOpt.c_str());
        }
    }
    else
    {
        string filename = baseUrl;
        std::wstring wideFilename = StringToWString(filename);
        std::string utf8Filename = WStringToString(wideFilename);
        m_pVlcMedia = libvlc_media_new_location(m_pVlcInstance, (string("file:///") + utf8Filename).c_str());
    }

    if (m_pVlcMedia == nullptr) return false;

    libvlc_media_player_set_media(m_pVlcMediaPlayer, m_pVlcMedia);

    libvlc_event_manager_t* em = libvlc_media_player_event_manager(m_pVlcMediaPlayer);
    libvlc_event_attach(em, libvlc_MediaPlayerPlaying, handle_playing_event, this);

    libvlc_video_set_callbacks(m_pVlcMediaPlayer, lock, unlock, display, this);

    m_video_width = width;
    m_video_height = height;
    
    libvlc_video_set_format(m_pVlcMediaPlayer, "RGBA", m_video_width, m_video_height, m_video_width * 4);

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

bool libVLC_RTSP::IsCropped()
{
	return m_extraSettings.cropRect.left != 0 || m_extraSettings.cropRect.top != 0 || m_extraSettings.cropRect.right != 0 || m_extraSettings.cropRect.bottom != 0;
}

int libVLC_RTSP::GetVideoRotation()
{
    if (m_pVlcMediaPlayer == nullptr) return 0;
    
    // Try to get rotation from libVLC metadata
    libvlc_media_t* media = libvlc_media_player_get_media(m_pVlcMediaPlayer);
    if (media)
    {
        // Try different metadata keys that might contain rotation info
        char* rotationMeta = libvlc_media_get_meta(media, libvlc_meta_Setting);
        if (rotationMeta)
        {
            // Parse rotation metadata - common values are "90", "180", "270"
            std::string rotStr(rotationMeta);
            libvlc_free(rotationMeta);
            
            try {
                int rotation = std::stoi(rotStr);
                if (rotation == 90 || rotation == 180 || rotation == 270) {
                    return rotation;
                }
            } catch (...) {
                // Not a valid rotation value, continue trying other methods
            }
        }
        
        // Try alternative metadata approach - some videos store rotation in Description
        char* descMeta = libvlc_media_get_meta(media, libvlc_meta_Description);
        if (descMeta)
        {
            std::string desc(descMeta);
            libvlc_free(descMeta);
            
            // Look for rotation patterns in description
            if (desc.find("rotate=90") != std::string::npos || desc.find("rotation=90") != std::string::npos) return 90;
            if (desc.find("rotate=180") != std::string::npos || desc.find("rotation=180") != std::string::npos) return 180;
            if (desc.find("rotate=270") != std::string::npos || desc.find("rotation=270") != std::string::npos) return 270;
        }
    }
    
    return 0; // No rotation detected
}

void libVLC_RTSP::UpdateFrame()
{
    if (m_pVlcMediaPlayer != nullptr)
    {
        unsigned width = 0, height = 0;
        if (libvlc_video_get_size(GetMP(), 0, &width, &height) == 0)
        {
            if (width != 0)
            {
                if (m_isStreaming || m_timesChangedVideoResolution < 2)
                {
                    if (width != m_video_width || height != m_video_height)
                    {
                        // Check for rotation metadata and swap dimensions if needed
                        int rotation = GetVideoRotation();
                        
                        // If video is rotated 90° or 270°, swap dimensions for correct aspect ratio
                        if (rotation == 90 || rotation == 270) 
                        {
                            std::swap(width, height);
                            LogMsg("Detected %d° rotation, swapping dimensions to %dx%d", rotation, width, height);
                        }
                        else if (rotation != 0)
                        {
                            LogMsg("Detected %d° rotation (no dimension swap needed)", rotation);
                        }

                        m_video_width = width;
                        m_video_height = height;
                        m_rotationAngle = rotation; // Store rotation for potential future use
                        
                        Init(m_source, m_cachingMS, m_pSurface, m_pStreamComp,
                            m_video_width, m_video_height, m_extraSettings); // Pass settings to maintain crop

                        SendStatusUpdate(C_STATUS_INITTED);
                        SendStatusUpdate(C_STATUS_UNPAUSED);
                        return;
                    }
                }

                // Update surface with appropriate buffer and dimensions
                if (IsCropped())
                {
                    int cropWidth = m_extraSettings.cropRect.right - m_extraSettings.cropRect.left;
                    int cropHeight = m_extraSettings.cropRect.bottom - m_extraSettings.cropRect.top;
                    
                    /*
                    //for debugging, create a SoftSurface of the same size that's pure red
					
                    SoftSurface surface;
                    surface.Init(cropWidth, cropHeight, SoftSurface::eSurfaceType::SURFACE_RGBA, false);
					surface.FillColor(glColorBytes(255, 0, 0, 255));
					m_pSurface->UpdateSurfaceRect(rtRect(0, 0, cropWidth, cropHeight), surface.GetPixelData());
                    */
                    
                    m_pSurface->UpdateSurfaceRect(rtRect(0, 0, cropWidth, cropHeight), m_pCroppedBuffer);
                }
                else
                {
                    m_pSurface->UpdateSurfaceRect(rtRect(0, 0, m_video_width, m_video_height), m_pImageBuffer);
                }
            }
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



// Add this before including Windows headers to prevent byte conflict
#ifdef _DEBUG
#define byte win_byte_override  // Temporarily rename byte
#endif
#include <dshow.h>
#ifdef _DEBUG
#undef byte                     // Restore original byte definition
#endif

#pragma comment(lib, "strmiids")

// Add this helper function to find full device name
std::string libVLC_RTSP::FindFullDeviceName(const std::string& partialName)
{
    ICreateDevEnum* pDevEnum = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, (void**)&pDevEnum);

    if (SUCCEEDED(hr))
    {
        IEnumMoniker* pClassEnum = NULL;
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);

        if (hr == S_OK)
        {
            IMoniker* pMoniker = NULL;
            while (pClassEnum->Next(1, &pMoniker, NULL) == S_OK)
            {
                IPropertyBag* pPropBag;
                hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&pPropBag);

                if (SUCCEEDED(hr))
                {
                    VARIANT varName;
                    VariantInit(&varName);
                    hr = pPropBag->Read(L"FriendlyName", &varName, NULL);
                    if (SUCCEEDED(hr))
                    {
                        char friendlyName[256];
                        WideCharToMultiByte(CP_UTF8, 0, varName.bstrVal, -1,
                            friendlyName, sizeof(friendlyName), NULL, NULL);

                        std::string deviceName = friendlyName;
                        if (deviceName.find(partialName) != std::string::npos)
                        {
                            VariantClear(&varName);
                            pPropBag->Release();
                            pMoniker->Release();
                            pClassEnum->Release();
                            pDevEnum->Release();
                            return deviceName;
                        }
                    }
                    VariantClear(&varName);
                    pPropBag->Release();
                }
                pMoniker->Release();
            }
            pClassEnum->Release();
        }
        pDevEnum->Release();
    }

    // If no match found, return the original partial name
    return partialName;
}


void ListWebcamDevices()
{
    // List Video Devices
    {
        ICreateDevEnum* pDevEnum = NULL;
        HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
            IID_ICreateDevEnum, (void**)&pDevEnum);

        if (SUCCEEDED(hr))
        {
            IEnumMoniker* pClassEnum = NULL;
            hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);

            if (hr == S_OK)
            {
                LogMsg("Available Video Devices:");
                IMoniker* pMoniker = NULL;
                int deviceIndex = 0;
                while (pClassEnum->Next(1, &pMoniker, NULL) == S_OK)
                {
                    IPropertyBag* pPropBag;
                    hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&pPropBag);

                    if (SUCCEEDED(hr))
                    {
                        VARIANT varName;
                        VariantInit(&varName);
                        hr = pPropBag->Read(L"FriendlyName", &varName, NULL);
                        if (SUCCEEDED(hr))
                        {
                            char friendlyName[256];
                            WideCharToMultiByte(CP_UTF8, 0, varName.bstrVal, -1,
                                friendlyName, sizeof(friendlyName), NULL, NULL);
                            LogMsg("Video Device %d: %s", deviceIndex, friendlyName);
                        }
                        VariantClear(&varName);
                        pPropBag->Release();
                    }
                    pMoniker->Release();
                    deviceIndex++;
                }
                pClassEnum->Release();
            }
            pDevEnum->Release();
        }
    }

    // List Audio Devices
    {
        ICreateDevEnum* pDevEnum = NULL;
        HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
            IID_ICreateDevEnum, (void**)&pDevEnum);

        if (SUCCEEDED(hr))
        {
            IEnumMoniker* pClassEnum = NULL;
            hr = pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pClassEnum, 0);

            if (hr == S_OK)
            {
                LogMsg("\nAvailable Audio Devices:");
                IMoniker* pMoniker = NULL;
                int deviceIndex = 0;
                while (pClassEnum->Next(1, &pMoniker, NULL) == S_OK)
                {
                    IPropertyBag* pPropBag;
                    hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&pPropBag);

                    if (SUCCEEDED(hr))
                    {
                        VARIANT varName;
                        VariantInit(&varName);
                        hr = pPropBag->Read(L"FriendlyName", &varName, NULL);
                        if (SUCCEEDED(hr))
                        {
                            char friendlyName[256];
                            WideCharToMultiByte(CP_UTF8, 0, varName.bstrVal, -1,
                                friendlyName, sizeof(friendlyName), NULL, NULL);
                            LogMsg("Audio Device %d: %s", deviceIndex, friendlyName);
                        }
                        VariantClear(&varName);
                        pPropBag->Release();
                    }
                    pMoniker->Release();
                    deviceIndex++;
                }
                pClassEnum->Release();
            }
            pDevEnum->Release();
        }
    }
}


std::string libVLC_RTSP::GetWebcamDevicePath(const std::string& deviceNameOrId)
{
    HRESULT hr = 0;

    // If it's a number, treat it as a device index
    try {
        int deviceId = std::stoi(deviceNameOrId);
        // Now we need to enumerate devices to get the actual name

        std::string deviceName;
        ICreateDevEnum* pDevEnum = NULL;
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
            IID_ICreateDevEnum, (void**)&pDevEnum);

        if (SUCCEEDED(hr))
        {
            IEnumMoniker* pClassEnum = NULL;
            hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);

            if (hr == S_OK)
            {
                IMoniker* pMoniker = NULL;
                int currentIndex = 0;
                while (pClassEnum->Next(1, &pMoniker, NULL) == S_OK)
                {
                    if (currentIndex == deviceId) {
                        IPropertyBag* pPropBag;
                        hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&pPropBag);
                        if (SUCCEEDED(hr))
                        {
                            VARIANT varName;
                            VariantInit(&varName);
                            hr = pPropBag->Read(L"FriendlyName", &varName, NULL);
                            if (SUCCEEDED(hr))
                            {
                                char friendlyName[256];
                                WideCharToMultiByte(CP_UTF8, 0, varName.bstrVal, -1,
                                    friendlyName, sizeof(friendlyName), NULL, NULL);
                                deviceName = friendlyName;
                            }
                            VariantClear(&varName);
                            pPropBag->Release();
                        }
                    }
                    pMoniker->Release();
                    currentIndex++;
                }
                pClassEnum->Release();
            }
            pDevEnum->Release();
        }


        if (!deviceName.empty()) {
            return "dshow://vdev=\"" + deviceName + "\"";
        }
    }
    catch (...) {
        // Not a number, treat as name
    }

    // If it's not a number or we couldn't find the device by index,
    // use the name directly with quotes
    return "dshow://vdev=\"" + deviceNameOrId + "\"";
}


// Helper to find audio device name that matches our video device
std::string libVLC_RTSP::FindMatchingAudioDevice(const std::string& videoDeviceName)
{
    ICreateDevEnum* pDevEnum = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, (void**)&pDevEnum);

    if (SUCCEEDED(hr))
    {
        IEnumMoniker* pClassEnum = NULL;
        hr = pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pClassEnum, 0);

        if (hr == S_OK)
        {
            IMoniker* pMoniker = NULL;
            while (pClassEnum->Next(1, &pMoniker, NULL) == S_OK)
            {
                IPropertyBag* pPropBag;
                hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&pPropBag);

                if (SUCCEEDED(hr))
                {
                    VARIANT varName;
                    VariantInit(&varName);
                    hr = pPropBag->Read(L"FriendlyName", &varName, NULL);
                    if (SUCCEEDED(hr))
                    {
                        char friendlyName[256];
                        WideCharToMultiByte(CP_UTF8, 0, varName.bstrVal, -1,
                            friendlyName, sizeof(friendlyName), NULL, NULL);

                        std::string audioDeviceName = friendlyName;
                        // Look for any part of video device name in the audio device name
                        if (audioDeviceName.find(videoDeviceName) != std::string::npos)
                        {
                            VariantClear(&varName);
                            pPropBag->Release();
                            pMoniker->Release();
                            pClassEnum->Release();
                            pDevEnum->Release();
                            LogMsg("Found matching audio device: %s", audioDeviceName.c_str());
                            return audioDeviceName;
                        }
                    }
                    VariantClear(&varName);
                    pPropBag->Release();
                }
                pMoniker->Release();
            }
            pClassEnum->Release();
        }
        pDevEnum->Release();
    }

    return ""; // Return empty string if no match found
}