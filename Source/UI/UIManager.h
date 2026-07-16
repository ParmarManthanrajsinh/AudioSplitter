#pragma once

#include "Audio/AudioManager.h"

#include <RmlUi/Core/EventListener.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Rml
{
class Context;
class ElementDocument;
class Event;
} // namespace Rml

class FUIManager
{
public:
    ~FUIManager();

    bool Initialize(int WindowWidth, int WindowHeight);
    void SetDevices(const std::vector<FAudioDevice>& SourceDevices, const std::vector<FAudioDevice>& OutputDevices);
    void SetStatus(const std::string& StatusText);
    void Update();
    void Render();
    void Shutdown();
    Rml::Context* GetContext() const;
    std::string GetSelectedDevice(const char* ElementId) const;

    std::function<void()> OnStartClicked;
    std::function<void()> OnStopClicked;
    std::function<void()> OnRefreshClicked;

private:
    void BindButton(const char* ElementId, std::function<void()> Callback);

    std::vector<FAudioDevice> CachedDevices;
    std::vector<std::unique_ptr<Rml::EventListener>> EventListeners;
    std::string CurrentStatus = "Stopped";
    Rml::Context* Context = nullptr;
    Rml::ElementDocument* Document = nullptr;
    bool bInitialized = false;
};
