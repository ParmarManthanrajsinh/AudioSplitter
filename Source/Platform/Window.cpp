#include "Platform/Window.h"

#include <RmlUi_Backend.h>

bool FWindow::Initialize()
{
    if (bInitialized)
    {
        return true;
    }

    if (!Backend::Initialize("Audio Splitter", Width, Height, false))
    {
        return false;
    }

    bInitialized = true;
    bShouldClose = false;
    return true;
}

bool FWindow::ShouldClose() const
{
    return !bInitialized || bShouldClose;
}

void FWindow::PollEvents(Rml::Context* Context)
{
    if (!bInitialized || Context == nullptr)
    {
        bShouldClose = true;
        return;
    }

    bShouldClose = !Backend::ProcessEvents(Context, nullptr, true);
}

void FWindow::PresentFrame()
{
    if (!bInitialized)
    {
        return;
    }

    Backend::PresentFrame();
}

void FWindow::Shutdown()
{
    if (!bInitialized)
    {
        return;
    }

    Backend::Shutdown();
    bInitialized = false;
    bShouldClose = true;
}

int FWindow::GetWidth() const
{
    return Width;
}

int FWindow::GetHeight() const
{
    return Height;
}
