/*

File: ./core/src/logging.cpp
Author: Aldhinn Espinas
Description: This file contains implementations of the logging functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/logging.h>

#include <mutex>
#include <sstream>
#include <iomanip>
#include <string>
#include <thread>
#include <filesystem>
#include <iostream>

/// @brief The synchronization mechanism for critical sections.
static ::std::mutex celeriqueLoggingMutex;

/// @brief Base logging function.
/// @param severity The severity of the message.
/// @param message The message in C string.
/// @param lineNum The line number in the source file.
/// @param sourcePath The source path of interest.
void __celeriqueLog(
    CeleriqueLogMessageSeverity severity, const char* message,
    unsigned long lineNum, const char* sourcePath
) {
    // Extract the time from the system.
    const ::std::chrono::time_point execTime = ::std::chrono::system_clock::now();

    // Container for the log message severity text.
    ::std::string severityText;
    // The container for the log message colour code.
    ::std::string colourCode;
    // Colour code and severity text setting based on the severity value provided.
    switch (severity) {
    case CELERIQUE_LOG_MESSAGE_SEVERITY_TRACE:
        colourCode = "\033[0;94m"; // Darker Blue.
        severityText = "TRACE";
        break;
    case CELERIQUE_LOG_MESSAGE_SEVERITY_DEBUG:
        colourCode = "\033[0;96m"; // Lighter Blue.
        severityText = "DEBUG";
        break;
    case CELERIQUE_LOG_MESSAGE_SEVERITY_INFO:
        colourCode = "\033[0;92m"; // Green.
        severityText = "INFO";
        break;
    case CELERIQUE_LOG_MESSAGE_SEVERITY_WARNING:
        colourCode = "\033[0;93m"; // Yellow.
        severityText = "WARNING";
        break;
    case CELERIQUE_LOG_MESSAGE_SEVERITY_ERROR:
        colourCode = "\033[0;95m"; // Pink.
        severityText = "ERROR";
        break;
    case CELERIQUE_LOG_MESSAGE_SEVERITY_FATAL:
        colourCode = "\033[0;91m"; // Red.
        severityText = "FATAL";
        break;
    default:
        // Stop execution from here on as the log level is invalid.
        return;
    }

    // Convert it to time_t.
    ::std::time_t execTimeInTimeType = ::std::chrono::system_clock::to_time_t(execTime);
    // Get the time information in the localtime.
    std::tm timeInfo = *std::localtime(&execTimeInTimeType);
    // The byte stream for strings used for formatting and extracting datetime values in string.
    ::std::stringstream formatterStringStream;
    // Format in Timezone YYYY-MM-DD hh:mm:ss AM/PM.
    formatterStringStream << std::put_time(&timeInfo, "%Z %Y-%b-%d %I:%M:%S %p");
    // Extract time string value.
    const ::std::string execTimeStr = formatterStringStream.str();

    // Reset string stream buffer.
    formatterStringStream.str("");

    // parsing thread id.
    formatterStringStream << ::std::this_thread::get_id();
    const ::std::string threadStr = formatterStringStream.str();

    // Parse the path of the source relative to the root of the repository.
    const ::std::string filename = ::std::filesystem::relative(
        sourcePath, CELERIQUE_REPO_ROOT_DIR
    ).string();
    // The code that resets the log message colour back to the original (mostly white).
    const ::std::string colourReset = "\033[0m";

    // Construct the ::std::string message to be printed.
    const ::std::string constructedLogMessage = colourCode + "[" + threadStr + "] " +
        colourReset + execTimeStr + colourCode + " [" + severityText + "] " +
        colourReset + message + " " + colourCode + filename + ":" +
        ::std::to_string(lineNum) + colourReset + "\n";

    // Executing one thread at a time from this point on.
    ::std::lock_guard<::std::mutex> writeLock(celeriqueLoggingMutex);

    if (severity < CELERIQUE_LOG_MESSAGE_SEVERITY_ERROR) {
        ::std::cout << constructedLogMessage;
    } else {
        ::std::cerr << constructedLogMessage;
    }
}