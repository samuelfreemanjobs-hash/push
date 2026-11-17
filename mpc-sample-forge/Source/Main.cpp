#include "MainComponent.h"
#include "CommandLine.h"
#include "Diagnostics.h"

class MPCSampleForgeApp final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override          { return true; }

    void initialise (const juce::String&) override
    {
        auto args = getCommandLineParameterArray();
        forge::Log::init (getApplicationName() + " " + getApplicationVersion());

        if (forge::CliRunner::wantsCli (args))
        {
            cli = std::make_unique<forge::CliRunner>();
            cli->start (args, [this] (int code)
            {
                setApplicationReturnValue (code);
                juce::MessageManager::callAsync ([this] { quit(); });
            });
            return;
        }

        window = std::make_unique<MainWindow> (getApplicationName());
    }

    void shutdown() override
    {
        window.reset();
        cli.reset();
        forge::Log::shutdown();
    }

    void systemRequestedQuit() override { quit(); }

private:
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (const juce::String& name)
            : DocumentWindow (name, juce::Colour (0xff16181d), DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new forge::MainComponent(), true);
            setResizable (true, true);
            setResizeLimits (860, 640, 4000, 3000);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }
    };

    std::unique_ptr<MainWindow> window;
    std::unique_ptr<forge::CliRunner> cli;
};

START_JUCE_APPLICATION (MPCSampleForgeApp)
