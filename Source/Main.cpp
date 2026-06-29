#include "App/Application.h"

#include <iostream>

int main()
{
    FApplication Application;

    if (!Application.Initialize())
    {
        std::cerr << "AudioSplitter failed to initialize.\n";
        Application.Shutdown();
        return 1;
    }

    const int ExitCode = Application.Run();
    Application.Shutdown();

    return ExitCode;
}
