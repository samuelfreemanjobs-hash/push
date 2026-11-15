; DrumSynth Autoresearch — preview VST3 (VoiceDSP + license UI)
; Compile after: cmake --build build --config Release --target DrumSynthAutoresearch_VST3

#define MyAppName "DrumSynth Autoresearch"
#define MyAppPublisher "DiskLordz"
#define MyAppURL "https://disklordz.example"
#define MyAppVersion "1.0.0"
#ifndef BuildRoot
  #define BuildRoot "C:\Dev\push\disklordz-ralph\vst\starter-cpp\build\autoresearch-cpp\DrumSynthAutoresearch_artefacts\Release\VST3"
#endif

[Setup]
AppId={{B8D4F1C2-6E3A-4D9B-8F2C-1A5E7D9C3B6F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppPublisher}\{#MyAppName}
DisableProgramGroupPage=yes
PrivilegesRequired=admin
OutputDir=..\..\..\Assets\Releases\installers
OutputBaseFilename=DrumSynth-Autoresearch-{#MyAppVersion}-win64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#BuildRoot}\DrumSynth Autoresearch.vst3\*"; DestDir: "{commoncf64}\VST3\DrumSynth Autoresearch.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Dirs]
Name: "{userappdata}\DiskLordz"; Permissions: users-modify

[Run]
Filename: "{winexplorer}"; Parameters: "{commoncf64}\VST3\DrumSynth Autoresearch.vst3"; Description: "Open VST3 install location"; Flags: postinstall shellexec skipifsilent

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
  if not DirExists(ExpandConstant('{#BuildRoot}\DrumSynth Autoresearch.vst3')) then
  begin
    MsgBox('Build DrumSynthAutoresearch_VST3 first or pass /DBuildRoot= to ISCC.', mbError, MB_OK);
    Result := False;
  end;
end;