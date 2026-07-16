#include "App/Application.h"

#include <chrono>
#include <thread>

bool FApplication::Initialize()
{
    if (bInitialized)
    {
        return true;
    }

    if (!Window.Initialize())
    {
        return false;
    }

    if (!UIManager.Initialize(Window.GetWidth(), Window.GetHeight()))
    {
        Window.Shutdown();
        return false;
    }

    {
        auto AllDevices = AudioManager.EnumerateDevices();
        std::vector<FAudioDevice> SourceDevices;
        std::vector<FAudioDevice> OutputDevices;
        for (const auto& D : AllDevices)
        {
            if (D.bIsMonitor)
                SourceDevices.push_back(D);
            else
                OutputDevices.push_back(D);
        }
        // On Windows (no monitor sources), show all devices as source options
        if (SourceDevices.empty())
            SourceDevices = AllDevices;
        UIManager.SetDevices(SourceDevices, OutputDevices);
    }
    UIManager.SetStatus("Stopped");

    UIManager.OnStartClicked = [this]()
    {
        std::string Source = UIManager.GetSelectedDevice("device-source");
        std::string Dev1 = UIManager.GetSelectedDevice("device-one");
        std::string Dev2 = UIManager.GetSelectedDevice("device-two");

        if (AudioManager.Start(Source, Dev1, Dev2))
        {
            UIManager.SetStatus("Running");
        }
        else
        {
            UIManager.SetStatus("Failed to start");
        }
    };

    UIManager.OnStopClicked = [this]()
    {
        AudioManager.Stop();
        UIManager.SetStatus("Stopped");
    };

    UIManager.OnRefreshClicked = [this]()
    {
        AudioManager.Stop();
        auto AllDevices = AudioManager.EnumerateDevices();
        std::vector<FAudioDevice> SourceDevices;
        std::vector<FAudioDevice> OutputDevices;
        for (const auto& D : AllDevices)
        {
            if (D.bIsMonitor)
                SourceDevices.push_back(D);
            else
                OutputDevices.push_back(D);
        }
        if (SourceDevices.empty())
            SourceDevices = AllDevices;
        UIManager.SetDevices(SourceDevices, OutputDevices);
        UIManager.SetStatus("Device list refreshed");
    };

    bInitialized = true;
    return true;
}

int FApplication::Run()
{
    if (!bInitialized)
    {
        return 1;
    }

    while (!Window.ShouldClose())
    {
        Window.PollEvents(UIManager.GetContext());
        UIManager.Update();
        UIManager.Render();
        Window.PresentFrame();
    }

    return 0;
}

void FApplication::Shutdown()
{
    if (!bInitialized)
    {
        return;
    }

    AudioManager.Stop();
    UIManager.Shutdown();
    Window.Shutdown();

    bInitialized = false;
}
