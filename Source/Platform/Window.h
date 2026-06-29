#pragma once

namespace Rml
{
class Context;
}

class FWindow
{
public:
    bool Initialize();
    bool ShouldClose() const;
    void PollEvents(Rml::Context* Context);
    void PresentFrame();
    void Shutdown();
    int GetWidth() const;
    int GetHeight() const;

private:
    int Width = 420;
    int Height = 500;
    bool bInitialized = false;
    bool bShouldClose = true;
};
