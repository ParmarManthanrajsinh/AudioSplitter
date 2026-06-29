#include "Audio/AudioManager.h"
#include <cstring>
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "../ThirdParty/miniaudio.h"

static constexpr ma_uint32 SAMPLE_RATE = 48000;
static constexpr ma_uint32 CHANNELS = 2;
static constexpr ma_uint32 FRAME_SIZE = CHANNELS * sizeof(float);
static constexpr ma_uint32 TARGET_FRAMES = SAMPLE_RATE / 20; // 2400 = 50ms
static constexpr ma_uint32 BUFFER_CAPACITY = TARGET_FRAMES * 4; // 9600 = 200ms
static constexpr ma_uint32 HIGH_WATERMARK = TARGET_FRAMES * 3; // 7200 = 150ms

FAudioManager::FAudioManager()
{
    ma_context_config CtxCfg = ma_context_config_init();
    CtxCfg.threadPriority = ma_thread_priority_realtime;

    Context = new ma_context();
    if (ma_context_init(nullptr, 0, &CtxCfg, Context) != MA_SUCCESS)
    {
        std::cerr << "Failed to initialize miniaudio context.\n";
        delete Context;
        Context = nullptr;
    }
}

FAudioManager::~FAudioManager()
{
    Stop();
    if (Context)
    {
        ma_context_uninit(Context);
        delete Context;
    }
}

std::vector<FAudioDevice> FAudioManager::EnumerateDevices() const
{
    std::vector<FAudioDevice> Devices;
    if (!Context)
        return Devices;

    ma_device_info* pPlaybackDeviceInfos;
    ma_uint32 playbackDeviceCount;

    if (ma_context_get_devices(Context, &pPlaybackDeviceInfos, &playbackDeviceCount, nullptr, nullptr) != MA_SUCCESS)
    {
        std::cerr << "Failed to retrieve audio devices.\n";
        return Devices;
    }

    for (ma_uint32 i = 0; i < playbackDeviceCount; ++i)
    {
        bool bExists = false;
        for (const auto& D : Devices)
        {
            if (D.Id == pPlaybackDeviceInfos[i].name)
            {
                bExists = true;
                break;
            }
        }
        if (!bExists)
        {
            FAudioDevice Device;
            Device.Id = pPlaybackDeviceInfos[i].name;
            Device.Name = pPlaybackDeviceInfos[i].name;
            Devices.push_back(Device);
        }
    }

    return Devices;
}

[[maybe_unused]] static void CaptureDataCallback(
    ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pOutput;
    ((FAudioManager*)pDevice->pUserData)->OnCaptureData(pInput, frameCount);
}

[[maybe_unused]] static void PlaybackDataCallback1(
    ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pInput;
    ((FAudioManager*)pDevice->pUserData)->OnPlaybackData1(pOutput, frameCount);
}

[[maybe_unused]] static void PlaybackDataCallback2(
    ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pInput;
    ((FAudioManager*)pDevice->pUserData)->OnPlaybackData2(pOutput, frameCount);
}

bool FAudioManager::Start(std::string_view SourceId, std::string_view Dest1Id, std::string_view Dest2Id)
{
    if (bIsRunning)
        Stop();
    if (SourceId.empty())
        return false;

    bool bUseDest1 = (Dest1Id != "none" && !Dest1Id.empty());
    bool bUseDest2 = (Dest2Id != "none" && !Dest2Id.empty());

    if (!bUseDest1 && !bUseDest2)
        return false;

    // Prevent feedback: destinations must differ from source
    if ((bUseDest1 && Dest1Id == SourceId) || (bUseDest2 && Dest2Id == SourceId))
    {
        std::cerr << "Output device cannot be the same as source device (feedback "
                     "loop).\n";
        return false;
    }

    ma_device_info* pInfos;
    ma_uint32 count;
    if (ma_context_get_devices(Context, &pInfos, &count, nullptr, nullptr) != MA_SUCCESS)
        return false;

    ma_device_id* pSourceDevId = nullptr;
    ma_device_id* pDest1DevId = nullptr;
    ma_device_id* pDest2DevId = nullptr;

    for (ma_uint32 i = 0; i < count; ++i)
    {
        if (pInfos[i].name == SourceId)
            pSourceDevId = &pInfos[i].id;
        if (bUseDest1 && pInfos[i].name == Dest1Id)
            pDest1DevId = &pInfos[i].id;
        if (bUseDest2 && pInfos[i].name == Dest2Id)
            pDest2DevId = &pInfos[i].id;
    }

    if (!pSourceDevId)
        return false;
    if (bUseDest1 && !pDest1DevId)
        return false;
    if (bUseDest2 && !pDest2DevId)
        return false;

    // Init ring buffers
    if (bUseDest1)
    {
        RingBuffer1 = new ma_pcm_rb;
        if (ma_pcm_rb_init(ma_format_f32, CHANNELS, BUFFER_CAPACITY, nullptr, nullptr, RingBuffer1) != MA_SUCCESS)
        {
            delete RingBuffer1;
            RingBuffer1 = nullptr;
            return false;
        }
    }

    if (bUseDest2)
    {
        RingBuffer2 = new ma_pcm_rb;
        if (ma_pcm_rb_init(ma_format_f32, CHANNELS, BUFFER_CAPACITY, nullptr, nullptr, RingBuffer2) != MA_SUCCESS)
        {
            if (RingBuffer1)
            {
                ma_pcm_rb_uninit(RingBuffer1);
                delete RingBuffer1;
                RingBuffer1 = nullptr;
            }
            delete RingBuffer2;
            RingBuffer2 = nullptr;
            return false;
        }
    }

    // Loopback capture from source device
    ma_device_config CaptureConfig = ma_device_config_init(ma_device_type_loopback);
    CaptureConfig.capture.pDeviceID = pSourceDevId;
    CaptureConfig.capture.format = ma_format_f32;
    CaptureConfig.capture.channels = CHANNELS;
    CaptureConfig.sampleRate = SAMPLE_RATE;
    CaptureConfig.dataCallback = CaptureDataCallback;
    CaptureConfig.pUserData = this;
    CaptureConfig.periodSizeInMilliseconds = 10;
    CaptureConfig.wasapi.usage = ma_wasapi_usage_pro_audio;

    CaptureDevice = new ma_device{};
    if (ma_device_init(Context, &CaptureConfig, CaptureDevice) != MA_SUCCESS)
    {
        std::cerr << "Failed to init loopback capture.\n";
        delete CaptureDevice;
        CaptureDevice = nullptr;
        Stop();
        return false;
    }

    if (bUseDest1)
    {
        ma_device_config PB1 = ma_device_config_init(ma_device_type_playback);
        PB1.playback.pDeviceID = pDest1DevId;
        PB1.playback.format = ma_format_f32;
        PB1.playback.channels = CHANNELS;
        PB1.sampleRate = SAMPLE_RATE;
        PB1.dataCallback = PlaybackDataCallback1;
        PB1.pUserData = this;
        PB1.periodSizeInMilliseconds = 10;
        PB1.wasapi.usage = ma_wasapi_usage_pro_audio;

        PlaybackDevice1 = new ma_device{};
        if (ma_device_init(Context, &PB1, PlaybackDevice1) != MA_SUCCESS)
        {
            std::cerr << "Failed to init playback device 1.\n";
            delete PlaybackDevice1;
            PlaybackDevice1 = nullptr;
            Stop();
            return false;
        }
        bIsBuffering1 = true;
    }

    if (bUseDest2)
    {
        ma_device_config PB2 = ma_device_config_init(ma_device_type_playback);
        PB2.playback.pDeviceID = pDest2DevId;
        PB2.playback.format = ma_format_f32;
        PB2.playback.channels = CHANNELS;
        PB2.sampleRate = SAMPLE_RATE;
        PB2.dataCallback = PlaybackDataCallback2;
        PB2.pUserData = this;
        PB2.periodSizeInMilliseconds = 10;
        PB2.wasapi.usage = ma_wasapi_usage_pro_audio;

        PlaybackDevice2 = new ma_device{};
        if (ma_device_init(Context, &PB2, PlaybackDevice2) != MA_SUCCESS)
        {
            std::cerr << "Failed to init playback device 2.\n";
            delete PlaybackDevice2;
            PlaybackDevice2 = nullptr;
            Stop();
            return false;
        }
        bIsBuffering2 = true;
    }

    ma_device_start(CaptureDevice);
    if (bUseDest1)
        ma_device_start(PlaybackDevice1);
    if (bUseDest2)
        ma_device_start(PlaybackDevice2);

    bIsRunning = true;
    return true;
}

void FAudioManager::Stop()
{
    if (CaptureDevice)
    {
        ma_device_stop(CaptureDevice);
    }
    if (PlaybackDevice1)
    {
        ma_device_stop(PlaybackDevice1);
    }
    if (PlaybackDevice2)
    {
        ma_device_stop(PlaybackDevice2);
    }

    if (CaptureDevice)
    {
        ma_device_uninit(CaptureDevice);
        delete CaptureDevice;
        CaptureDevice = nullptr;
    }
    if (PlaybackDevice1)
    {
        ma_device_uninit(PlaybackDevice1);
        delete PlaybackDevice1;
        PlaybackDevice1 = nullptr;
    }
    if (PlaybackDevice2)
    {
        ma_device_uninit(PlaybackDevice2);
        delete PlaybackDevice2;
        PlaybackDevice2 = nullptr;
    }

    if (RingBuffer1)
    {
        ma_pcm_rb_uninit(RingBuffer1);
        delete RingBuffer1;
        RingBuffer1 = nullptr;
    }
    if (RingBuffer2)
    {
        ma_pcm_rb_uninit(RingBuffer2);
        delete RingBuffer2;
        RingBuffer2 = nullptr;
    }

    bIsRunning = false;
}

bool FAudioManager::IsRunning() const
{
    return bIsRunning;
}

// ---- Capture: push to both ring buffers ----
void FAudioManager::OnCaptureData(const void* pInput, unsigned int frameCount)
{
    const ma_uint8* pSrc = static_cast<const ma_uint8*>(pInput);

    // Ring buffer 1
    if (RingBuffer1)
    {
        ma_uint32 remaining = frameCount;
        ma_uint32 offset = 0;
        while (remaining > 0)
        {
            ma_uint32 toWrite = remaining;
            void* pDst;

            if (ma_pcm_rb_acquire_write(RingBuffer1, &toWrite, &pDst) != MA_SUCCESS || toWrite == 0)
                break;

            if (pSrc)
                memcpy(pDst, pSrc + offset * FRAME_SIZE, toWrite * FRAME_SIZE);
            else
                memset(pDst, 0, toWrite * FRAME_SIZE);

            ma_pcm_rb_commit_write(RingBuffer1, toWrite);
            remaining -= toWrite;
            offset += toWrite;
        }
    }

    // Ring buffer 2
    if (RingBuffer2)
    {
        ma_uint32 remaining = frameCount;
        ma_uint32 offset = 0;
        while (remaining > 0)
        {
            ma_uint32 toWrite = remaining;
            void* pDst;

            if (ma_pcm_rb_acquire_write(RingBuffer2, &toWrite, &pDst) != MA_SUCCESS || toWrite == 0)
                break;
            if (pSrc)
                memcpy(pDst, pSrc + offset * FRAME_SIZE, toWrite * FRAME_SIZE);
            else
                memset(pDst, 0, toWrite * FRAME_SIZE);

            ma_pcm_rb_commit_write(RingBuffer2, toWrite);
            remaining -= toWrite;
            offset += toWrite;
        }
    }
}

// ---- Playback 1 ----
void FAudioManager::OnPlaybackData1(void* pOutput, unsigned int frameCount)
{
    if (!RingBuffer1)
    {
        memset(pOutput, 0, frameCount * FRAME_SIZE);
        return;
    }

    // Drift correction: if buffer too full, skip old data (Consumer thread safely
    // moves read pointer)
    ma_int32 dist = ma_pcm_rb_pointer_distance(RingBuffer1);
    if (dist > (ma_int32)HIGH_WATERMARK)
    {
        ma_pcm_rb_seek_read(RingBuffer1, (ma_uint32)(dist - (ma_int32)TARGET_FRAMES));
    }

    if (bIsBuffering1)
    {
        if (ma_pcm_rb_available_read(RingBuffer1) >= TARGET_FRAMES)
            bIsBuffering1 = false;
        else
        {
            memset(pOutput, 0, frameCount * FRAME_SIZE);
            return;
        }
    }

    ma_uint8* pDst = static_cast<ma_uint8*>(pOutput);
    ma_uint32 remaining = frameCount;
    ma_uint32 offset = 0;

    while (remaining > 0)
    {
        ma_uint32 toRead = remaining;
        void* pSrc;

        if (ma_pcm_rb_acquire_read(RingBuffer1, &toRead, &pSrc) != MA_SUCCESS || toRead == 0)
        {
            break;
        }

        memcpy(pDst + offset * FRAME_SIZE, pSrc, toRead * FRAME_SIZE);
        ma_pcm_rb_commit_read(RingBuffer1, toRead);

        remaining -= toRead;
        offset += toRead;
    }

    if (remaining > 0)
    {
        memset(pDst + offset * FRAME_SIZE, 0, remaining * FRAME_SIZE);
        bIsBuffering1 = true;
    }
}

// ---- Playback 2 ----
void FAudioManager::OnPlaybackData2(void* pOutput, unsigned int frameCount)
{
    if (!RingBuffer2)
    {
        memset(pOutput, 0, frameCount * FRAME_SIZE);
        return;
    }

    ma_int32 dist = ma_pcm_rb_pointer_distance(RingBuffer2);
    if (dist > (ma_int32)HIGH_WATERMARK)
    {
        ma_pcm_rb_seek_read(RingBuffer2, (ma_uint32)(dist - (ma_int32)TARGET_FRAMES));
    }

    if (bIsBuffering2)
    {
        if (ma_pcm_rb_available_read(RingBuffer2) >= TARGET_FRAMES)
            bIsBuffering2 = false;
        else
        {
            memset(pOutput, 0, frameCount * FRAME_SIZE);
            return;
        }
    }

    ma_uint8* pDst = static_cast<ma_uint8*>(pOutput);
    ma_uint32 remaining = frameCount;
    ma_uint32 offset = 0;

    while (remaining > 0)
    {
        ma_uint32 toRead = remaining;
        void* pSrc;
        if (ma_pcm_rb_acquire_read(RingBuffer2, &toRead, &pSrc) != MA_SUCCESS || toRead == 0)
            break;
        memcpy(pDst + offset * FRAME_SIZE, pSrc, toRead * FRAME_SIZE);
        ma_pcm_rb_commit_read(RingBuffer2, toRead);
        remaining -= toRead;
        offset += toRead;
    }

    if (remaining > 0)
    {
        memset(pDst + offset * FRAME_SIZE, 0, remaining * FRAME_SIZE);
        bIsBuffering2 = true;
    }
}
