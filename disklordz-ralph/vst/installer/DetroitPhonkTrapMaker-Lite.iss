; Detroit Phonk Trap Maker — Lite VST3 installer (Inno Setup 6)
; Build on Windows after Release VST3 + optional code signing (see README.md).
; Compile: "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" DetroitPhonkTrapMaker-Lite.iss

#define MyAppName "Detroit Phonk Trap Maker Lite"
#define MyAppPublisher "DiskLordz"
#define MyAppURL "https://disklordz.example"
#define MyAppVersion "1.0.0"
; Relative to this .iss file — override via /DBuildRoot= on command line
#ifndef BuildRoot
  #define BuildRoot "C:\Dev\DetroitPhonkTrapVST3\build\DetroitPhonkTrapMaker_artefacts\Release\VST3"
#endif

[Setup]
AppId={{A7C9E2B1-4F3D-4B8A-9E1C-2D5F6A8B0C4E}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppPublisher}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=
PrivilegesRequired=admin
OutputDir=..\..\..\Assets\Releases\installers
OutputBaseFilename=DetroitPhonkTrapMaker-Lite-{#MyAppVersion}-win64
SetupIconFile=
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; Sign OutputBaseFilename.exe with signtool after compile (see README)

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut to the install folder"; GroupDescription: "Additional icons:"; Flags: unchecked

[Files]
; Entire .vst3 bundle — FL Studio scans Common Files\VST3
Source: "{#BuildRoot}\Detroit Phonk Trap Maker.vst3\*"; DestDir: "{commoncf64}\VST3\Detroit Phonk Trap Maker.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
; Optional: factory .blob for Lite single kit (Core installer adds expansions)
; Source: "..\..\Assets\factory\lite-kit.blob"; DestDir: "{userappdata}\DiskLordz\DetroitPhonkTrap\factory"; Flags: ignoreversion

[Dirs]
Name: "{userappdata}\DiskLordz\DetroitPhonkTrap"; Permissions: users-modify

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{winexplorer}"; Parameters: "{commoncf64}\VST3\Detroit Phonk Trap Maker.vst3"; Comment: "Open VST3 bundle folder"
Name: "{autodesktop}\{#MyAppName} folder"; Filename: "{commoncf64}\VST3"; Tasks: desktopicon

[Run]
Filename: "{winexplorer}"; Parameters: "{commoncf64}\VST3\Detroit Phonk Trap Maker.vst3"; Description: "Open VST3 install location"; Flags: postinstall shellexec skipifsilent

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
  if not FileExists(ExpandConstant('{#BuildRoot}\Detroit Phonk Trap Maker.vst3\x86_64-win\Detroit Phonk Trap Maker.vst3')) then
  begin
    MsgBox('Build the Release VST3 first, or pass /DBuildRoot= to ISCC.' + #13#10 +
      'Expected under: {#BuildRoot}', mbError, MB_OK);
    Result := False;
  end;
end;