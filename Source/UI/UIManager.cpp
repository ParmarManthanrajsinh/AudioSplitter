#include "UI/UIManager.h"

#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControlSelect.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi_Backend.h>

#include <filesystem>
#include <iostream>
#include <utility>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace
{
std::filesystem::path GetExecutableDirectory()
{
#if defined(_WIN32)
    char Buffer[MAX_PATH] = {};
    const DWORD Length = GetModuleFileNameA(nullptr, Buffer, MAX_PATH);
    if (Length > 0)
    {
        return std::filesystem::path(Buffer).parent_path();
    }
#endif

    return std::filesystem::current_path();
}

class FButtonListener final : public Rml::EventListener
{
public:
    FButtonListener(std::function<void()> InCallback) : Callback(std::move(InCallback)) {}

    void ProcessEvent(Rml::Event& Event) override
    {
        Event.StopPropagation();
        if (Callback)
        {
            Callback();
        }
    }

private:
    std::function<void()> Callback;
};
} // namespace

FUIManager::~FUIManager() = default;

bool FUIManager::Initialize(int WindowWidth, int WindowHeight)
{
    if (bInitialized)
    {
        return true;
    }

    Rml::SetSystemInterface(Backend::GetSystemInterface());
    Rml::SetRenderInterface(Backend::GetRenderInterface());

    Rml::Initialise();

    Context = Rml::CreateContext("main", Rml::Vector2i(WindowWidth, WindowHeight));
    if (Context == nullptr)
    {
        Rml::Shutdown();
        return false;
    }

    const std::filesystem::path AssetRoot = GetExecutableDirectory() / "Assets";
    const std::filesystem::path FontPath = AssetRoot / "Fonts" / "LatoLatin-Regular.ttf";
    const std::filesystem::path DocumentPath = AssetRoot / "UI" / "Main.rml";

    if (!Rml::LoadFontFace(FontPath.string()))
    {
        std::cerr << "Failed to load font: " << FontPath.string() << '\n';
    }

    Document = Context->LoadDocument(DocumentPath.string());
    if (Document == nullptr)
    {
        std::cerr << "Failed to load UI document: " << DocumentPath.string() << '\n';
        Rml::Shutdown();
        Context = nullptr;
        return false;
    }

    BindButton("refresh-button",
        [this]()
        {
            if (OnRefreshClicked)
                OnRefreshClicked();
        });
    BindButton("start-button",
        [this]()
        {
            if (OnStartClicked)
                OnStartClicked();
        });
    BindButton("stop-button",
        [this]()
        {
            if (OnStopClicked)
                OnStopClicked();
        });

    Document->Show();
    bInitialized = true;
    return true;
}

void FUIManager::SetDevices(const std::vector<FAudioDevice>& SourceDevices, const std::vector<FAudioDevice>& OutputDevices)
{
    CachedDevices = SourceDevices;

    if (Document == nullptr)
    {
        return;
    }

    auto PopulateSelect = [&](const char* ElementId, const std::vector<FAudioDevice>& Items, bool bAddNone)
    {
        if (auto* Select = rmlui_dynamic_cast<Rml::ElementFormControlSelect*>(Document->GetElementById(ElementId)))
        {
            Select->RemoveAll();
            if (Items.empty())
            {
                Select->Add("No devices found", "");
            }
            else
            {
                if (bAddNone)
                    Select->Add("None", "none");
                for (const FAudioDevice& Device : Items)
                {
                    Select->Add(Device.Name, Device.Id);
                }
            }
        }
    };

    PopulateSelect("device-source", SourceDevices, false);
    PopulateSelect("device-one", OutputDevices, true);
    PopulateSelect("device-two", OutputDevices, true);
}

void FUIManager::SetStatus(const std::string& StatusText)
{
    CurrentStatus = StatusText;

    if (Document != nullptr)
    {
        if (Rml::Element* StatusElement = Document->GetElementById("status-text"))
        {
            StatusElement->SetInnerRML(CurrentStatus);
        }

        if (Rml::Element* DotElement = Document->GetElementById("status-dot"))
        {
            DotElement->SetClassNames("");
            if (StatusText == "Running")
            {
                DotElement->SetClass("running", true);
            }
            else if (StatusText == "Stopped")
            {
                DotElement->SetClass("stopped", true);
            }
            else if (StatusText == "Failed to start")
            {
                DotElement->SetClass("error", true);
            }
            else
            {
                DotElement->SetClass("info", true);
            }
        }
    }
}

void FUIManager::Update()
{
    if (Context != nullptr)
    {
        Context->Update();
    }
}

void FUIManager::Render()
{
    if (Context == nullptr)
    {
        return;
    }

    Backend::BeginFrame();
    Context->Render();
}

void FUIManager::Shutdown()
{
    if (!bInitialized)
    {
        return;
    }

    Document = nullptr;
    Context = nullptr;
    Rml::Shutdown();
    EventListeners.clear();

    CachedDevices.clear();
    CurrentStatus = "Stopped";
    bInitialized = false;
}

Rml::Context* FUIManager::GetContext() const
{
    return Context;
}

std::string FUIManager::GetSelectedDevice(const char* ElementId) const
{
    if (Document != nullptr)
    {
        if (Rml::Element* SelectElement = Document->GetElementById(ElementId))
        {
            if (auto* FormControl = rmlui_dynamic_cast<Rml::ElementFormControlSelect*>(SelectElement))
            {
                return FormControl->GetValue();
            }

            const Rml::Variant* ValueVariant = SelectElement->GetAttribute("value");
            if (ValueVariant != nullptr)
            {
                return ValueVariant->Get<std::string>();
            }
        }
    }
    return "";
}

void FUIManager::BindButton(const char* ElementId, std::function<void()> Callback)
{
    if (Document == nullptr)
    {
        return;
    }

    Rml::Element* Button = Document->GetElementById(ElementId);
    if (Button == nullptr)
    {
        return;
    }

    auto Listener = std::make_unique<FButtonListener>(std::move(Callback));
    Button->AddEventListener("click", Listener.get());
    EventListeners.push_back(std::move(Listener));
}
