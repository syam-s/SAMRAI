/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and COPYING.LESSER.
 *
 * Copyright:     (c) 1997-2011 Lawrence Livermore National Security, LLC
 * Description:   Utility class for logging.
 *
 ************************************************************************/

#ifndef included_tbox_Logger
#define included_tbox_Logger

#include "SAMRAI/tbox/Pointer.h"

#include <string>

namespace SAMRAI {
namespace tbox {

/*!
 * Logger is a Singleton class for logging error, debug, and warning messages.
 *
 * The logAbort, logWarning, and logDebug methods are used to log a
 * message coming from a file and line.
 *
 * Warning and Debug messages can be turned on/off using the
 * setWarning and setDebug methods.  Abort messages may not be turned
 * off.
 *
 * Applications may override the logging mechanism.The basic idea is
 * the singleton Logger directs log messages to the appropriate
 * Appender which is responsible for writing the log message.  The
 * Appender is the destination for the log message.  Users may supply
 * implementors of the Appender interface to log the messages to
 * different I/O streams or elsewhere.  In this way applications can
 * redirect SAMRAI generated messages to that applications logging
 * mechanism.  Appenders may be provided for abort, warning, and/or
 * debug messages.  The same Appender may be used for all messages.
 *
 *
 * For example to log to a stream simply inherit from LoggerAppender and
 * implement the logMesage method:
 * \code
 *
 *   class StreamAppender : public tbox::Logger::Appender {
 *
 *   public:
 *
 *      StreamAppender(ostream *stream) {
 *         d_stream = stream;
 *      }
 *
 *      void logMessage(const std::string &message,
 *                 const std::string &filename,
 *                 const int line)
 *      {
 *         (*d_stream) << "At :" << filename << " line :" << line
 *                     << " message: " << message << std::endl;
 *      }
 *
 *   private:
 *      ostream *d_stream;
 *   };
 * \endcode
 *
 *
 * This Appender could be use to log warning message using:
 * tbox::Pointer<tbox::Logger::Appender> appender = new ConsoleAppender()
 * tbox::Logger.getInstance() -> setWarningAppender(appender);
 *
 * Normally this would be done at the start of an application.
 *
 */
class Logger
{

public:
   /*!
    * Interface for class that does the logging for the Logger.
    *
    *
    */
   class Appender
   {

public:
      /*!
       * Log a message with file and location information.
       */
      virtual void
      logMessage(
         const std::string& message,
         const std::string& filename,
         const int line) = 0;

      virtual ~Appender() {
      }
   };

   /*!
    * Gets the instance of the singleton logger.
    */
   static Logger *
   getInstance();

   /*!
    * Logs an abort message with file & location
    */
   void
   logAbort(
      const std::string& message,
      const std::string& filename,
      const int line);

   /*!
    * Logs warning message with file & location.
    */
   void
   logWarning(
      const std::string& message,
      const std::string& filename,
      const int line);

   /*!
    * Logs debug message with file & location.
    */
   void
   logDebug(
      const std::string& message,
      const std::string& filename,
      const int line);

   /*!
    * Set the Appender for logging abort messages to an
    * application specific class.
    *
    * Default is to log to perr.
    */
   void
   setAbortAppender(
      tbox::Pointer<Appender> appender);

   /*!
    * Set the Appender for logging warning messages to an
    * application specific class.
    *
    * Default is to log to plog.
    */
   void
   setWarningAppender(
      tbox::Pointer<Appender> appender);

   /*!
    * Set the Appender for logging debug messages to an
    * application specific class.
    *
    * Default is to log to plog.
    */
   void
   setDebugAppender(
      tbox::Pointer<Appender> appender);

   /*!
    * Turn logging of warning messages on or off.
    *
    * Default is on.
    */
   void
   setWarning(
      bool onoff);

   /*!
    * Turn logging of debug messages on or off.
    *
    * Default is off.
    */
   void
   setDebug(
      bool onoff);

private:
   /*
    * Private constructor to avoid construction of the singleton
    * outside this class.
    */
   Logger();

   /*
    * Private destructor to avoid destruction of the singleton
    * outside this class.
    */
   ~Logger();

   /*!
    * Frees instance of the singleton logger.
    *
    * NOTE: should be called by StartupShutdownManager only.
    */
   static void
   finalizeCallback();

   /*
    * Instance of the singleton.
    */
   static Logger* s_instance;

   /*
    * Appenders for each type of logging.
    */
   tbox::Pointer<Appender> d_abort_appender;
   tbox::Pointer<Appender> d_warning_appender;
   tbox::Pointer<Appender> d_debug_appender;

   /*
    * Logging state (on or off)
    */
   bool d_log_warning;
   bool d_log_debug;

   static StartupShutdownManager::Handler
      s_finalize_handler;

};

}
}

#endif
