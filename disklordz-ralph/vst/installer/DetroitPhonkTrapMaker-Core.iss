; Core tier: VST3 + unlocked 24-bit WAV bonus folder (no Moonbase until BIZ gate — see LICENSING.md)
#ifndef BuildRoot
  #define BuildRoot "C:\Dev\DetroitPhonkTrapVST3\build\DetroitPhonkTrapMaker_artefacts\Release\VST3"
#endif
#ifndef WavPackRoot
  #define WavPackRoot "C:\Dev\push\Assets\Releases\LouisianaCoreWavs"
#endif

#define MyAppName "Detroit Phonk Trap Maker Core"
#define MyAppPublisher "DiskLordz"
#define MyAppURL "https://disklordz.example"
#define MyAppVersion "1.0.0"

[Setup]
AppId={{B8D0F3C2-5E4A-4C9B-8F2D-3E607B9C1D5F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppPublisher}\{#MyAppName}
DisableProgramGroupPage=yes
PrivilegesRequired=admin
OutputDir=..\..\..\Assets\Releases\installers
OutputBaseFilename=DetroitPhonkTrapMaker-Core-{#MyAppVersion}-win64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#BuildRoot}\Detroit Phonk Trap Maker.vst3\*"; DestDir: "{commoncf64}\VST3\Detroit Phonk Trap Maker.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#WavPackRoot}\*"; DestDir: "{userdocs}\DiskLordz\Louisiana Core WAV Pack"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: DirExists(ExpandConstant('{#WavPackRoot}'))

[Dirs]
Name: "{userdocs}\DiskLordz\Louisiana Core WAV Pack"; Permissions: users-modify
Name: "{userappdata}\DiskLordz\DetroitPhonkTrap\expansions"; Permissions: users-modify

[Run]
Filename: "{winexplorer}"; Parameters: "{userdocs}\DiskLordz\Louisiana Core WAV Pack"; Description: "Open unlocked WAV pack"; Flags: postinstall shellexec skipifsilent unchecked