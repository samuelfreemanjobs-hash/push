// mpcforge: the command-line front end. Same engine as the app, but a real
// console program, so it prints progress on Windows too and can be scripted.
#include "CommandLine.h"
#include <iostream>

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit; // plugins need a message loop

    juce::StringArray args;
    for (int i = 1; i < argc; ++i)
        args.add (juce::CharPointer_UTF8 (argv[i]));

    if (args.isEmpty())
        args.add ("--help");
    if (! forge::CliRunner::wantsCli (args))
        args.insert (0, "--cli");

    int exitCode = 0;
    bool finished = false;
    forge::CliRunner runner;

    runner.start (args, [&] (int code)
    {
        exitCode = code;
        finished = true;
        juce::MessageManager::getInstance()->stopDispatchLoop();
    });

    if (! finished)
        juce::MessageManager::getInstance()->runDispatchLoop();

    return exitCode;
}
