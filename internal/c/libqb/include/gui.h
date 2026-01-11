//-----------------------------------------------------------------------------------------------------
//    ___  ____   __   _  _   ____  _____    ____ _   _ ___   _     _ _
//   / _ \| __ ) / /_ | || | |  _ \| ____|  / ___| | | |_ _| | |   (_) |__  _ __ __ _ _ __ _   _
//  | | | |  _ \| '_ \| || |_| |_) |  _|   | |  _| | | || |  | |   | | '_ \| '__/ _` | '__| | | |
//  | |_| | |_) | (_) |__   _|  __/| |___  | |_| | |_| || |  | |___| | |_) | | | (_| | |  | |_| |
//   \__\_\____/ \___/   |_| |_|   |_____|  \____|\___/|___| |_____|_|_.__/|_|  \__,_|_|   \__, |
//                                                                                         |___/
//  QB64-PE GUI Library
//  Powered by tinyfiledialogs (http://tinyfiledialogs.sourceforge.net)
//
//-----------------------------------------------------------------------------------------------------

#pragma once

#include <stdarg.h>
#include <stdint.h>

struct qbs;

/**
 * @brief Shows a notification popup (QB64 _GUINOTIFYPOPUP statement)
 * @param qbsTitle qbs string containing the popup title
 * @param qbsMessage qbs string containing the message
 * @param qbsIconType qbs string containing the icon type
 * @param passed Number of parameters provided
 * @note Displays a non-blocking notification popup to the user
 */
void sub__guiNotifyPopup(qbs *qbsTitle, qbs *qbsMessage, qbs *qbsIconType, int32_t passed);

/**
 * @brief Shows a message box and returns the button pressed (QB64 _GUIMESSAGEBOX function)
 * @param qbsTitle qbs string containing the dialog title
 * @param qbsMessage qbs string containing the message
 * @param qbsDialogType qbs string containing the dialog type
 * @param qbsIconType qbs string containing the icon type
 * @param nDefaultButton Default button number
 * @param passed Number of parameters provided
 * @return Button number that was pressed
 */
int32_t func__guiMessageBox(qbs *qbsTitle, qbs *qbsMessage, qbs *qbsDialogType, qbs *qbsIconType, int32_t nDefaultButton, int32_t passed);

/**
 * @brief Shows a message box (QB64 _GUIMESSAGEBOX statement)
 * @param qbsTitle qbs string containing the dialog title
 * @param qbsMessage qbs string containing the message
 * @param qbsIconType qbs string containing the icon type
 * @param passed Number of parameters provided
 * @note Displays a blocking message box dialog
 */
void sub__guiMessageBox(qbs *qbsTitle, qbs *qbsMessage, qbs *qbsIconType, int32_t passed);

/**
 * @brief Shows an input box (QB64 _GUIINPUTBOX$ function)
 * @param qbsTitle qbs string containing the dialog title
 * @param qbsMessage qbs string containing the prompt message
 * @param qbsDefaultInput qbs string containing the default input value
 * @param passed Number of parameters provided
 * @return qbs string containing user input, or NULL if cancelled
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__guiInputBox(qbs *qbsTitle, qbs *qbsMessage, qbs *qbsDefaultInput, int32_t passed);

/**
 * @brief Shows a folder selection dialog (QB64 _GUISELECTFOLDERDIALOG$ function)
 * @param qbsTitle qbs string containing the dialog title
 * @param qbsDefaultPath qbs string containing the default path
 * @param passed Number of parameters provided
 * @return qbs string containing the selected folder path, or NULL if cancelled
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__guiSelectFolderDialog(qbs *qbsTitle, qbs *qbsDefaultPath, int32_t passed);

/**
 * @brief Shows a color chooser dialog (QB64 _GUICOLORCHOOSERDIALOG function)
 * @param qbsTitle qbs string containing the dialog title
 * @param nDefaultRGB Default RGB color value
 * @param passed Number of parameters provided
 * @return Selected RGB color value, or 0 if cancelled
 */
uint32_t func__guiColorChooserDialog(qbs *qbsTitle, uint32_t nDefaultRGB, int32_t passed);

/**
 * @brief Shows an open file dialog (QB64 _GUIOpenFileDialog$ function)
 * @param qbsTitle qbs string containing the dialog title
 * @param qbsDefaultPathAndFile qbs string containing the default path and filename
 * @param qbsFilterPatterns qbs string containing file filter patterns
 * @param qbsSingleFilterDescription qbs string containing filter description
 * @param nAllowMultipleSelects Flag to allow multiple file selection
 * @param passed Number of parameters provided
 * @return qbs string containing selected file path(s), or NULL if cancelled
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__guiOpenFileDialog(qbs *qbsTitle, qbs *qbsDefaultPathAndFile, qbs *qbsFilterPatterns, qbs *qbsSingleFilterDescription, int32_t nAllowMultipleSelects,
                             int32_t passed);

/**
 * @brief Shows a save file dialog (QB64 _GUISAVEFILEDIALOG$ function)
 * @param qbsTitle qbs string containing the dialog title
 * @param qbsDefaultPathAndFile qbs string containing the default path and filename
 * @param qbsFilterPatterns qbs string containing file filter patterns
 * @param qbsSingleFilterDescription qbs string containing filter description
 * @param passed Number of parameters provided
 * @return qbs string containing the selected file path, or NULL if cancelled
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__guiSaveFileDialog(qbs *qbsTitle, qbs *qbsDefaultPathAndFile, qbs *qbsFilterPatterns, qbs *qbsSingleFilterDescription, int32_t passed);

/**
 * @name Internal GUI Alert Functions
 * @brief Internal functions for displaying alerts
 */
///@{
/**
 * @brief Shows an alert dialog (internal use)
 * @param message Alert message
 * @param title Dialog title
 * @param type Alert type
 * @return Button pressed (implementation-dependent)
 */
int gui_alert(const char *message, const char *title, const char *type);

/**
 * @brief Shows an alert dialog with formatted message (internal use)
 * @param fmt Format string (printf-style)
 * @param ... Format arguments
 * @return true if user confirmed, false otherwise
 */
bool gui_alert(const char *fmt, ...);
///@}