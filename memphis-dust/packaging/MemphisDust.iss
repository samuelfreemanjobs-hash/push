; Inno Setup script for the Windows installer (built by CI, or locally with ISCC.exe).
;   ISCC.exe /DAppVersion=1.0.0 /DBuildDir=..\build\MemphisDust_artefacts\Release packaging\MemphisDust.iss
#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif
#ifndef BuildDir
  #define BuildDir "..\build\MemphisDust_artefacts\Release"
#endif

[Setup]
AppId={{6B8F2D51-6A1E-4C2B-9D8E-4D5A1C0F9E21}
AppName=Memphis Dust
AppVersion={#AppVersion}
AppPublisher=Sam Freeman
DefaultDirName={autopf}\Memphis Dust
DefaultGroupName=Memphis Dust
DisableProgramGroupPage=yes
OutputBaseFilename=MemphisDust-{#AppVersion}-Windows-Setup
OutputDir=..\dist
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
UninstallDisplayName=Memphis Dust

[Types]
Name: "full"; Description: "VST3 plugin + Standalone app"
Name: "custom"; Description: "Custom"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 plugin (C:\Program Files\Common Files\VST3)"; Types: full custom; Flags: fixed
Name: "standalone"; Description: "Standalone application"; Types: full

[Files]
Source: "{#BuildDir}\VST3\Memphis Dust.vst3\*"; DestDir: "{commoncf64}\VST3\Memphis Dust.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BuildDir}\Standalone\Memphis Dust.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion isreadme

[Icons]
Name: "{group}\Memphis Dust (Standalone)"; Filename: "{app}\Memphis Dust.exe"; Components: standalone
Name: "{group}\Uninstall Memphis Dust"; Filename: "{uninstallexe}"
