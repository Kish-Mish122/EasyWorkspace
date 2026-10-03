#define MyAppName "EasyWorkspace"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "EasyWorkspace"

[Setup]
AppId={{BB13297B-77E0-4B97-8863-5AFA2C446D02}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=output
OutputBaseFilename=EasyWorkspace-Setup-{#MyAppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
ChangesAssociations=yes
PrivilegesRequiredOverridesAllowed=dialog
UninstallDisplayName={#MyAppName}

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "Полная установка"
Name: "custom"; Description: "Выборочная установка"; Flags: iscustom

[Components]
Name: "common"; Description: "Общие библиотеки (Qt)"; Types: full custom; Flags: fixed
Name: "write"; Description: "EasyWrite — текстовый редактор"; Types: full custom
Name: "sheets"; Description: "EasySheets — электронные таблицы"; Types: full custom
Name: "slides"; Description: "EasySlides — презентации"; Types: full custom

[Tasks]
Name: "desktopicon"; Description: "Создать значки на рабочем столе"; GroupDescription: "Дополнительно:"
Name: "assoc"; Description: "Связать файлы .ezw, .ezx, .ezp с программами"; GroupDescription: "Дополнительно:"

[Files]
Source: "E:\EasyOffice\build\powerpoint\EasySlides.exe"; DestDir: "{app}"; Components: slides; Flags: ignoreversion
Source: "E:\EasyOffice\build\word\EasyWrite.exe"; DestDir: "{app}"; Components: write; Flags: ignoreversion
Source: "E:\EasyOffice\build\excel\EasySheets.exe"; DestDir: "{app}"; Components: sheets; Flags: ignoreversion

[Icons]
Name: "{group}\EasyWrite"; Filename: "{app}\EasyWrite.exe"; Components: write
Name: "{group}\EasySheets"; Filename: "{app}\EasySheets.exe"; Components: sheets
Name: "{group}\EasySlides"; Filename: "{app}\EasySlides.exe"; Components: slides
Name: "{group}\Удалить EasyWorkspace"; Filename: "{uninstallexe}"
Name: "{autodesktop}\EasyWrite"; Filename: "{app}\EasyWrite.exe"; Components: write; Tasks: desktopicon
Name: "{autodesktop}\EasySheets"; Filename: "{app}\EasySheets.exe"; Components: sheets; Tasks: desktopicon
Name: "{autodesktop}\EasySlides"; Filename: "{app}\EasySlides.exe"; Components: slides; Tasks: desktopicon

[Registry]
; .ezw -> EasyWrite
Root: HKA; Subkey: "Software\Classes\.ezw"; ValueType: string; ValueData: "EasyWorkspace.ezw"; Flags: uninsdeletevalue; Components: write; Tasks: assoc
Root: HKA; Subkey: "Software\Classes\EasyWorkspace.ezw"; ValueType: string; ValueData: "Документ EasyWrite"; Flags: uninsdeletekey; Components: write; Tasks: assoc
Root: HKA; Subkey: "Software\Classes\EasyWorkspace.ezw\DefaultIcon"; ValueType: string; ValueData: "{app}\EasyWrite.exe,0"; Components: write; Tasks: assoc
Root: HKA; Subkey: "Software\Classes\EasyWorkspace.ezw\shell\open\command"; ValueType: string; ValueData: """{app}\EasyWrite.exe"" ""%1"""; Components: write; Tasks: assoc

; .ezx -> EasySheets
Root: HKA; Subkey: "Software\Classes\.ezx"; ValueType: string; ValueData: "EasyWorkspace.ezx"; Flags: uninsdeletevalue; Components: sheets; Tasks: assoc
Root: HKA; Subkey: "Software\Classes\EasyWorkspace.ezx"; ValueType: string; ValueData: "Таблица EasySheets"; Flags: uninsdeletekey; Components: sheets; Tasks: assoc
Root: HKA; Subkey: "Software\Classes\EasyWorkspace.ezx\DefaultIcon"; ValueType: string; ValueData: "{app}\EasySheets.exe,0"; Components: sheets; Tasks: assoc
Root: HKA; Subkey: "Software\Classes\EasyWorkspace.ezx\shell\open\command"; ValueType: string; ValueData: """{app}\EasySheets.exe"" ""%1"""; Components: sheets; Tasks: assoc

; .ezp -> EasySlides
Root: HKA; Subkey: "Software\Classes\.ezp"; ValueType: string; ValueData: "EasyWorkspace.ezp"; Flags: uninsdeletevalue; Components: slides; Tasks: assoc
Root: HKA; Subkey: "Software\Classes\EasyWorkspace.ezp"; ValueType: string; ValueData: "Презентация EasySlides"; Flags: uninsdeletekey; Components: slides; Tasks: assoc
Root: HKA; Subkey: "Software\Classes\EasyWorkspace.ezp\DefaultIcon"; ValueType: string; ValueData: "{app}\EasySlides.exe,0"; Components: slides; Tasks: assoc
Root: HKA; Subkey: "Software\Classes\EasyWorkspace.ezp\shell\open\command"; ValueType: string; ValueData: """{app}\EasySlides.exe"" ""%1"""; Components: slides; Tasks: assoc
