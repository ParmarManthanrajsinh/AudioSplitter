#pragma once

#include "Audio/AudioManager.h"
#include "Platform/Window.h"
#include "UI/UIManager.h"

class FApplication
{
public:
    bool Initialize();
    int Run();
    void Shutdown();

private:
    FWindow Window;
    FUIManager UIManager;
    FAudioManager AudioManager;

    bool bInitialized = false;
};
