; Inno Setup script for vram-task-manager.
; Invoked from CI as:
;   ISCC /DAppVersion=x.y.z /DStagedRoot=<dir> /DAppIconFile=<path-to-ico> installer.iss

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef StagedRoot
  #error "StagedRoot must be provided via /D"
#endif
#ifndef AppIconFile
  #error "AppIconFile must be provided via /D"
#endif

#define AppName        "VRAM Task Manager"
#define AppPublisher   "iteas"
#define AppPublisherURL "https://iteas.at"
#define AppExeName     "vram-task-manager.exe"
; Stable AppId — keep this constant across releases so upgrades replace the
; previous install instead of producing a second Apps & Features entry.
#define AppId          "{{8F2A7C42-3D9B-4F1E-A6C1-7E5B9D8E2A1C}"

[Setup]
AppId={#AppId}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppPublisherURL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
OutputBaseFilename=vram-task-manager-{#AppVersion}-windows-x64-setup
OutputDir=.
SetupIconFile={#AppIconFile}
UninstallDisplayIcon={app}\{#AppExeName}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
MinVersion=10.0.17763

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german";  MessagesFile: "compiler:Languages\German.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#StagedRoot}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\{#AppName}";                              Filename: "{app}\{#AppExeName}"
Name: "{group}\{cm:UninstallProgram,{#AppName}}";        Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}";                        Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent
