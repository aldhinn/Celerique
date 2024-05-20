/*

File: ./include/celerique/logging.h
Author: Aldhinn Espinas
Description: This header file contains the logging functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_LOGGING_HEADER_FILE)
#define CELERIQUE_LOGGING_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/types.h>

#if defined(__cplusplus)
#include <string>
#endif

/// @brief The level of log message severity.
typedef uint8_t CeleriqueLogMessageSeverity;

/// @brief The null value for type `CeleriqueLogMessageSeverity`.
#define CELERIQUE_LOG_MESSAGE_SEVERITY_NULL                                                    0x00

/// @brief These are highly detailed messages offering insight
/// into the intricate execution flow within the application,
/// aiding developers in debugging and performance optimization. 
#define CELERIQUE_LOG_MESSAGE_SEVERITY_TRACE                                                   0x01
/// @brief These messages are typically very detailed and are used
/// during development and troubleshooting.
#define CELERIQUE_LOG_MESSAGE_SEVERITY_DEBUG                                                   0x02
/// @brief These messages are typically used to inform about important
/// events or milestones in the application.
#define CELERIQUE_LOG_MESSAGE_SEVERITY_INFO                                                    0x03
/// @brief These messages are used to alert developers or administrators
/// about conditions that might lead to problems if not addressed.
#define CELERIQUE_LOG_MESSAGE_SEVERITY_WARNING                                                 0x04
/// @brief These messages are typically used to log unexpected or exceptional
/// conditions that need to be investigated.
#define CELERIQUE_LOG_MESSAGE_SEVERITY_ERROR                                                   0x05
/// @brief These messages are used to log severe errors that prevent
/// the application from functioning properly.
#define CELERIQUE_LOG_MESSAGE_SEVERITY_FATAL                                                   0x06

#if defined(__cplusplus)
extern "C" {
#endif

/// @brief Base logging function.
/// @param severity The severity of the message.
/// @param message The message in C string.
/// @param lineNum The line number in the source file.
/// @param sourcePath The source path of interest.
CELERIQUE_SHARED_SYMBOL void __celeriqueLog(
    CeleriqueLogMessageSeverity severity, const char* message,
    unsigned long lineNum, const char* sourcePath
);

#if defined(__cplusplus)
}
#endif

#if !defined(celeriqueLog)
#if defined(__cplusplus)
/// @brief Log a message with the line number and source file where this
/// macro is being called for, automatically determined in compile time.
/// @param level The severity of the message.
/// @param message The message in ::std::string.
#define celeriqueLog(level, message) \
__celeriqueLog(level, ::std::string(message).c_str(), __LINE__, __FILE__)
#else
/// @brief Log a message with the line number and source file where this
/// macro is being called for, automatically determined in compile time.
/// @param level The severity of the message.
/// @param message The message in C string.
#define celeriqueLog(level, message) \
__celeriqueLog(level, message, __LINE__, __FILE__)
#endif
#endif

#if !defined(celeriqueLogTrace)
#if defined(CELERIQUE_DEBUG_MODE)
/// @brief Log a trace message.
/// @param message The message in C string.
#define celeriqueLogTrace(message) \
celeriqueLog(CELERIQUE_LOG_MESSAGE_SEVERITY_TRACE, message)
#else
/// @brief Log a trace message. (Expands to nothing).
/// @param message The message in C string.
#define celeriqueLogTrace(message)
#endif
#endif

#if !defined(celeriqueLogDebug)
#if defined(CELERIQUE_DEBUG_MODE)
/// @brief Log a debug message.
/// @param message The message in C string.
#define celeriqueLogDebug(message) \
celeriqueLog(CELERIQUE_LOG_MESSAGE_SEVERITY_DEBUG, message)
#else
/// @brief Log a debug message. (Expands to nothing).
/// @param message The message in C string.
#define celeriqueLogDebug(message)
#endif
#endif

#if !defined(celeriqueLogInfo)
/// @brief Log an info message.
/// @param message The message in C string.
#define celeriqueLogInfo(message) \
celeriqueLog(CELERIQUE_LOG_MESSAGE_SEVERITY_INFO, message)
#endif

#if !defined(celeriqueLogWarning)
/// @brief Log a warning message.
/// @param message The message in C string.
#define celeriqueLogWarning(message) \
celeriqueLog(CELERIQUE_LOG_MESSAGE_SEVERITY_WARNING, message)
#endif

#if !defined(celeriqueLogError)
/// @brief Log an error message.
/// @param message The message in C string.
#define celeriqueLogError(message) \
celeriqueLog(CELERIQUE_LOG_MESSAGE_SEVERITY_ERROR, message)
#endif

#if !defined(celeriqueLogFatal)
/// @brief Log a fatal message.
/// @param message The message in C string.
#define celeriqueLogFatal(message) \
celeriqueLog(CELERIQUE_LOG_MESSAGE_SEVERITY_FATAL, message)
#endif

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.