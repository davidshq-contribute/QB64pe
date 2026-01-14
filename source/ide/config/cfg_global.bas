' All DIM SHARED declarations moved to qb64pe.bas

'===== Define and check settings location =====================================
ConfigFolder$ = "settings" 'relative config location inside the qb64pe main folder
ConfigFile$ = ConfigFolder$ + pathsep$ + "config.ini" 'main configuration (global with instance sections)
DebugFile$ = ConfigFolder$ + pathsep$ + "debug.ini" 'debug mode data (global with instance sections)
BookmarksFile$ = ConfigFolder$ + pathsep$ + "bookmarks.bin" 'set bokmarks (globally tied to source files)
RecentFile$ = ConfigFolder$ + pathsep$ + "recent.bin" 'recent file list (globally shared)
SearchedFile$ = ConfigFolder$ + pathsep$ + "searched.bin" 'search history (globally shared)
AutosaveFile$ = ConfigFolder$ + pathsep$ + "autosave" + tempfolderindexstr$ + ".bin" 'autosave flag (per instance)
UndoFile$ = ConfigFolder$ + pathsep$ + "undo" + tempfolderindexstr$ + ".bin" 'undo storage (per instance)
'---
askToCopyOther = _FALSE 'shall we ask the user to copy settings from another QB64-PE installation
IF NOT _DIREXISTS(ConfigFolder$) THEN MKDIR ConfigFolder$: askToCopyOther = _TRUE

'===== Define sections and standard behavior ==================================
'--- config.ini
windowSettingsSection$ = "IDE WINDOW" + STR$(tempfolderindex)
colorSettingsSection$ = "IDE COLOR SETTINGS" + STR$(tempfolderindex)
colorSchemesSection$ = "IDE COLOR SCHEMES"
customDictionarySection$ = "CUSTOM DICTIONARIES"
mouseSettingsSection$ = "MOUSE SETTINGS"
generalSettingsSection$ = "GENERAL SETTINGS"
displaySettingsSection$ = "IDE DISPLAY SETTINGS"
debugSettingsSection$ = "DEBUG SETTINGS"
compilerSettingsSection$ = "COMPILER SETTINGS"
loggingSettingsSection$ = "LOGGING SETTINGS"
'--- debug.ini
vwatchPanelSection$ = "VWATCH PANEL" + STR$(tempfolderindex)
'--- behavior
IniSetAddQuotes _FALSE
IniSetForceReload _TRUE
IniSetAllowBasicComments _TRUE
IniSetAutoCommit _TRUE

