#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "../ThirdParty/miniaudio.h"

struct FAudioDevice
{
    std::string Id;
    std::string Name;
    bool bIsMonitor = false; // PulseAudio monitor source (Linux loopback equivalent)
};

class FAudioManager
{
public:
    FAudioManager();
    ~FAudioManager();

    std::vector<FAudioDevice> EnumerateDevices() const;
    bool Start(std::string_view SourceId, std::string_view Dest1Id, std::string_view Dest2Id);
    void Stop();
    bool IsRunning() const;

    void OnCaptureData(const void* pInput, unsigned int frameCount);
    void OnPlaybackData1(void* pOutput, unsigned int frameCount);
    void OnPlaybackData2(void* pOutput, unsigned int frameCount);

private:
    bool bIsRunning = false;
    bool bIsBuffering1 = true;
    bool bIsBuffering2 = true;
    ma_context* Context = nullptr;

    ma_device* CaptureDevice = nullptr;
    ma_device* PlaybackDevice1 = nullptr;
    ma_device* PlaybackDevice2 = nullptr;

    ma_pcm_rb* RingBuffer1 = nullptr;
    ma_pcm_rb* RingBuffer2 = nullptr;
};
