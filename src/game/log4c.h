/*
 * Copyright (c) 2001, Bit Farm, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Modified slightly for Race into Space project. */

/**
 * @file log4c.h
 * An easy-to-use, fast and flexible message logging architecture.
 *
 * This is loosely based on the Apache project's  Log4J, Log4CC,
 * etc. project. Because C is not object oriented, a lot had to change.
 *
 * The following documentation is a hacked version of the Log4J docs.
 *
 * OVERVIEW
 *
 * Log4C has 3 main concepts: category, priority and appender.
 * These three concepts work together to enable developers to log messages
 * according to message type and priority, and to control at runtime how these
 * messages are formatted and where they are reported.
 *
 * CATEGORY HIERARCHY
 *
 * The first and foremost advantage of any logging API over plain printf()
 * resides in its ability to disable certain log statements while allowing
 * others to print unhindered. This capability assumes that the logging space,
 * that is, the space of all possible logging statements, is categorized
 * according to some developer-chosen criteria.
 *
 * This observation led to choosing category as the central concept of the
 * system. Every category is declared by providing a name and an optional
 * parent. If no parent is explicitly named, the root category, LOG_ROOT_CAT is
 * the category's parent.
 *
 * A category is created by a macro call at the top level of a file.
 * A category can be created with any one of the following macros:
 *
 *      - LOG_NEW_CATEGORY(MyCat);
 *      - LOG_NEW_SUBCATEGORY(MyCat, ParentCat);
 *      - LOG_NEW_DEFAULT_CATEGORY(MyCat);
 *      - LOG_NEW_DEFAULT_SUBCATEGORY(MyCat, ParentCat);
 *
 * The parent cat can be defined in the same file or in another file, but each
 * category may have only one definition.
 *
 * Typically, there will be a Category for each module and sub-module, so you
 * can independently control logging for each module.
 *
 * PRIORITY
 *
 * A category may be assigned a threshold priorty. The set of priorites are
 * defined by symbolic constants: LP_TRACE, LP_DEBUG, LP_INFO,
 * LP_NOTICE, LP_WARNING , LP_ERROR, LP_CRITICAL, LP_ALERT, and LP_EMERGENCY.
 * The priorities are simply non-negative integers. [It is possible to use your
 * own set by supplying a custom appender.]
 *
 * If a given category is not assigned a threshold priority, then it inherits
 * one from its closest ancestor with an assigned threshold.
 *
 * To ensure that all categories can eventually inherit a threshold, the root
 * category always has an assigned threshold priority.
 *
 * Logging requests are made by invoking a logging macro on a category.
 * All of the macros have a printf-style format string followed by arguments.
 * Because most C compilers do not support vararg macros, there is a version of
 * the macro for any number of arguments from 0 to 6. The macro name ends with
 * the total number of arguments.
 *
 * Here is an example of the most basic type of macro:
 *
 * <code>
 *      CLOG5(MyCat, LP_WARN, "Values are: %d and '%s'", 5, "oops");
 * </code>
 *
 * This is a logging request with priority WARN.
 *
 * A logging request is said to be enabled if its priority is higher than or
 * equal to the threshold priority of its category. Otherwise, the request is
 * said to be disabled. A category without an assigned priority will inherit
 * one from the hierarchy.
 *
 * It is possible to use any non-negative integer as a priority. If, as in the
 * example, one of the standard priorites is used, then there is a convenience
 * macro that is typically used instead. For example, the above example is
 * equivalent to the shorter:
 *
 * <code>
 *      CWARN4(MyCat, "Values are: %d and '%s'", 5, "oops");
 * </code>
 *
 * DEFAULT CATEGORY
 *
 * If LOG_NEW_DEFAULT_SUBCATEGORY(MyCat, Parent) or
 * LOG_NEW_DEFAULT_CATEGORY(MyCat) is used to create the category, then the
 * even shorter form can be used:
 *
 * <code>
 *      WARN3("Values are: %d and '%s'", 5, "oops");
 * </code>
 *
 * Only one default category can be created per file, though multiple
 * non-defaults can be created and used.
 *
 * EXAMPLE
 *
 * Here is a more complete example:
 *
 * @code
 *      #include "log4c.h"
 *
 *      // create a category and a default subcategory
 *      LOG_NEW_CATEGORY(VSS);
 *      LOG_NEW_DEFAULT_SUBCATEGORY(SA, VSS);
 *
 *      main() {
 *             // Now set the parent's priority.
 *             // (the string would typcially be a runtime option)
 *             log_setControlString("SA.thresh=3");
 *
 *             // This request is enabled, because WARNING >= INFO.
 *             CWARN2(VSS, "Low fuel level.");
 *
 *             // This request is disabled, because DEBUG < INFO.
 *             CDEBUG2(VSS, "Starting search for nearest gas station.");
 *
 *             // The default category SA inherits its priority from VSS. Thus,
 *             // the following request is enabled because INFO >= INFO.
 *             INFO1("Located nearest gas station.");
 *
 *             // This request is disabled, because DEBUG < INFO.
 *             DEBUG1("Exiting gas station search");
 *       }
 * @endcode
 *
 * PERFORMANCE
 *
 * Clever design insures efficiency. Except for the first invocation, a
 * disabled logging request requires an a single comparison of a static
 * variable to a constant.
 *
 * There is also compile time constant, LOG_STATIC_THRESHOLD, which causes all
 * logging requests with a lower priority to be optimized to 0 cost by the
 * compiler. By setting it to LP_INFINITE, all logging requests are statically
 * disabled and cost nothing. Released executables might typically be compiled
 * with "-DLOG_STATIC_THRESHOLD=LP_INFO"
 *
 * APPENDERS
 *
 * Each category has an optional appender. An appender is a pointer to a
 * structure whcih starts with a pointer to a doAppend() function. DoAppend()
 * prints a message to a log.
 *
 * WHen a category is passed a message by one of the logging macros, the
 * category performs the following actions:
 *
 * 1. if the category has an appender, the message is passed to the appender's
 *    doAppend() function,
 *
 * 2. if 'willLogToParent' is true for the category,  the message is passed to
 * the category's parent.
 *
 * By default, all categories except root have no appender and
 * 'willLogToParent' is true. This situation causes all messages to be logged
 * by the root category's appender.
 *
 * Typically, you would only change the root category's appender when you
 * wanted, say, a different output format. Copying log_default.c would
 * be a good start.
 *
 * The default appender function currently prints to stderr.
 *
 * MISC
 *
 * Do not use any of the macros that start with '_'.
 *
 * CAVEATS
 *
 * Category names are global variables.
 */
#ifndef _LOG4C_H_
#define _LOG4C_H_

#include <cstdarg>

#ifdef ALTERED_STRUCTURE_PACKING
#error "log4c breaks when you include it late"
#endif

/**
 * @name Priority Values
 *
 * These are the same names (except TRACE is new) as used by Unix syslog(), but
 * the numerical values are different.
 *@{
 */

#define LP_NONE           0
#define LP_TRACE          1
#define LP_DEBUG          2
#define LP_INFO           3
#define LP_NOTICE         4
#define LP_WARNING        5
#define LP_ERROR          6
#define LP_CRITICAL       7
#define LP_ALERT          8
#define LP_EMERGENCY      9

#define LP_UNINITIALIZED  -1    ///< for internal use only

#ifndef LOG_STATIC_THRESHOLD
/**
 * All logging with priority < LOG_STATIC_THRESHOLD is disabled at compile
 * time, i.e., compiled out.
 */
#define LOG_STATIC_THRESHOLD LP_NONE
#endif

/*@}*/

/** Transforms a category name to a global variable name. */
#define _LOGV(cat)   _LOG_CONCAT(_log_c, cat)
#define _LOG_CONCAT(x,y) x ## _ ## y

/** The root of the category hierarchy. */
#ifndef LOG_ROOT_CAT
#define LOG_ROOT_CAT   root
#endif

/** Defines a new subcategory of the parent. */
#define LOG_NEW_SUBCATEGORY(catName, parent)    \
    extern struct LogCategory _LOGV(parent);    \
    struct LogCategory _LOGV(catName) = {       \
        &_LOGV(parent), 0, 0,                   \
        #catName, LP_UNINITIALIZED, 1,          \
        0, 1                                    \
    };

/**
 * Creates a new subcategory of the root category.
 */
#define LOG_NEW_CATEGORY(catName)  LOG_NEW_SUBCATEGORY(catName, LOG_ROOT_CAT)

/**
 * Exports category (use this one in header files)
 */
#define LOG_EXPORT_CATEGORY(catName)  extern struct LogCategory _LOGV(catName)

/**
 * Creates a new subcategory of the root category.
 */

#define LOG_DEFAULT_CATEGORY(cname) \
    static struct LogCategory* _log_defaultCategory = &_LOGV(cname);

/**
 * Creates a new subcategory of the root category and makes it the default
 * (used by macros that don't explicitly specify a category).
 */
#define LOG_NEW_DEFAULT_CATEGORY(cname)         \
    LOG_NEW_CATEGORY(cname);                    \
    LOG_DEFAULT_CATEGORY(cname);

/**
 * Creates a new subcategory of the parent category and makes it the default
 * (used by macros that don't explicitly specify a category).
 */
#define LOG_NEW_DEFAULT_SUBCATEGORY(cname, parent)      \
    LOG_NEW_SUBCATEGORY(cname, parent);                 \
    LOG_DEFAULT_CATEGORY(cname);

// Functions you may call

#if 0
/**
 * Typically passed a command-line argument. The string has the syntax:
 *
 *      ( <category> "." <keyword> "=" value (" ")... )...
 *
 * where <category> is one the category names and keyword is one of the
 * following:
 *
 *      thresh      value is an integer priority level. Sets the category's
 *                        threshold priority.
 *
 * @warning
 * This routine may only be called once and that must be before any other
 * logging command! Typically, this is done from main().
 *
 * @todo should allow symbolic priority values.
 */
extern const char *log_setControlString(const char *cs);
#endif

// Forward declarations
struct LogAppender;
struct LogEvent;

/**
 * Do NOT access any members of this structure directly.
 */
struct LogCategory {
    struct LogCategory *parent;
    struct LogCategory *firstChild, *nextSibling;
    const char *name;
    int thresholdPriority;
    int isThreshInherited;
    struct LogAppender *appender;
    int willLogToParent;
    // TODO: Formats?
};

struct LogAppender {
    void (*doAppend)(struct LogAppender *thisLogAppender,
                     struct LogEvent *event);
};

struct LogEvent {
    struct LogCategory *cat;
    int priority;
    const char *fileName;
    const char *functionName;
    int lineNum;
    const char *fmt;
    va_list ap;
};

/**
 * Programatically alters a category's threshold priority.
 */
extern void log_setThreshold(struct LogCategory *cat, int thresholdPriority);

/**
 * Programatically alter a category's parent.
 */
extern void log_setParent(struct LogCategory *cat, struct LogCategory *parent);

/**
 * Sets the category's appender.
 */
extern void log_setAppender(struct LogCategory *cat, struct LogAppender *app);

// Functions that you shouldn't call.
extern void _log_logEvent(struct LogCategory *category,
                          struct LogEvent *ev, ...);
extern int _log_initCat(int priority, struct LogCategory *category);

extern struct LogCategory _LOGV(LOG_ROOT_CAT);
extern struct LogAppender *log_defaultLogAppender;

/**
 * Returns true if the given priority is enabled for the category.
 * If you have expensive expressions that are computed outside of the log
 * command and used only within it, you should make its evaluation conditional
 * using this macro.
 */
#define LOG_ISENABLED(catName, priority) \
            _LOG_ISENABLEDV(_LOGV(catName), priority)

/**
 * Helper function that implements LOG_ISENABLED.
 *
 * NOTES
 * First part is a compile-time constant.
 * Call to _log_initCat only happens once.
 */
#define _LOG_ISENABLEDV(catv, priority)                         \
       (priority >= LOG_STATIC_THRESHOLD                        \
        && priority >= (catv).thresholdPriority                 \
        && ((catv).thresholdPriority != LP_UNINITIALIZED        \
            || _log_initCat(priority, &(catv))) )

/**
 * @name Internal Macros
 * Some kludge macros to ease maintenance. See how they're used below.
 *
 * IMPLEMENTATION NOTE: To reduce the parameter passing overhead of an enabled
 * message, the many parameters passed to the logging function are packed in a
 * structure. Since these values will be usually be passed to at least 3
 * functions, this is a win.
 * It also allows adding new values (such as a timestamp) without breaking
 * code.
 * Setting the LogEvent's valist member is done inside _log_logEvent.
 * UPDATE: we set va_list to 0 to shut up the compiler.
 * UPDATE2: we don't set va_list at all to shut up the compiler :)
 */
//@{
#define _LOG_PRE(catv, prio, format) do {                                  \
     if (_LOG_ISENABLEDV(catv, prio)) {                                 \
         struct LogEvent _log_ev;                                       \
         _log_ev.cat = &(catv);                                         \
         _log_ev.priority = (prio);                                     \
         _log_ev.fileName = __FILE__;                                   \
         _log_ev.functionName = (const char *)__func__;                 \
         _log_ev.lineNum = __LINE__;                                    \
         _log_ev.fmt = (format);                                           \
         _log_logEvent(&(catv), &_log_ev

#define _LOG_POST                               \
                        );                      \
     } } while(0)
//@}

/** @name Logging Macros */
//@{
// Thank God for Emacs...
// Many repetitions follow...
#define CLOG3(c, p, f) _LOG_PRE(_LOGV(c),p,f) _LOG_POST
#define CTRACE2(c, f) CLOG3(c, LP_TRACE, f)
#define CDEBUG2(c, f) CLOG3(c, LP_DEBUG, f)
#define CINFO2(c, f) CLOG3(c, LP_INFO, f)
#define CNOTICE2(c, f) CLOG3(c, LP_NOTICE, f)
#define CWARNING2(c, f) CLOG3(c, LP_WARNING, f)
#define CERROR2(c, f) CLOG3(c, LP_ERROR, f)
#define CCRITICAL2(c, f) CLOG3(c, LP_CRITICAL, f)
#define CALERT2(c, f) CLOG3(c, LP_ALERT, f)
#define CEMERGENCY2(c, f) CLOG3(c, LP_EMERGENCY, f)
#define LOG2(p, f)    _LOG_PRE(*_log_defaultCategory, p, f) _LOG_POST
#define INFO1(f)      _LOG_PRE(*_log_defaultCategory, LP_INFO, f) _LOG_POST
#define NOTICE1(f)    _LOG_PRE(*_log_defaultCategory, LP_NOTICE, f) _LOG_POST
#define TRACE1(f)     _LOG_PRE(*_log_defaultCategory, LP_TRACE, f) _LOG_POST
#define DEBUG1(f)     _LOG_PRE(*_log_defaultCategory, LP_DEBUG, f) _LOG_POST
#define WARNING1(f)   _LOG_PRE(*_log_defaultCategory, LP_WARNING, f) _LOG_POST
#define ERROR1(f)     _LOG_PRE(*_log_defaultCategory, LP_ERROR, f) _LOG_POST
#define CRITICAL1(f)  _LOG_PRE(*_log_defaultCategory, LP_CRITICAL, f) _LOG_POST
#define ALERT1(f)     _LOG_PRE(*_log_defaultCategory, LP_ALERT, f)  _LOG_POST
#define EMERGENCY1(f) _LOG_PRE(*_log_defaultCategory, LP_EMERGENCY, f)  _LOG_POST
// Many repetitions follow...
#define CLOG4(c, p, f,a1) _LOG_PRE(_LOGV(c),p,f) ,a1 _LOG_POST
#define CTRACE3(c, f,a1) CLOG4(c, LP_TRACE, f,a1)
#define CDEBUG3(c, f,a1) CLOG4(c, LP_DEBUG, f,a1)
#define CINFO3(c, f,a1) CLOG4(c, LP_INFO, f,a1)
#define CNOTICE3(c, f,a1) CLOG4(c, LP_NOTICE, f,a1)
#define CWARNING3(c, f,a1) CLOG4(c, LP_WARNING, f,a1)
#define CERROR3(c, f,a1) CLOG4(c, LP_ERROR, f,a1)
#define CCRITICAL3(c, f,a1) CLOG4(c, LP_CRITICAL, f,a1)
#define CALERT3(c, f,a1) CLOG4(c, LP_ALERT, f,a1)
#define CEMERGENCY3(c, f,a1) CLOG4(c, LP_EMERGENCY, f,a1)
#define LOG3(p, f,a1)    _LOG_PRE(*_log_defaultCategory, p, f) ,a1 _LOG_POST
#define INFO2(f,a1)      _LOG_PRE(*_log_defaultCategory, LP_INFO, f) ,a1 _LOG_POST
#define NOTICE2(f,a1)    _LOG_PRE(*_log_defaultCategory, LP_NOTICE, f) ,a1 _LOG_POST
#define TRACE2(f,a1)     _LOG_PRE(*_log_defaultCategory, LP_TRACE, f) ,a1 _LOG_POST
#define DEBUG2(f,a1)     _LOG_PRE(*_log_defaultCategory, LP_DEBUG, f) ,a1 _LOG_POST
#define WARNING2(f,a1)   _LOG_PRE(*_log_defaultCategory, LP_WARNING, f) ,a1 _LOG_POST
#define ERROR2(f,a1)     _LOG_PRE(*_log_defaultCategory, LP_ERROR, f) ,a1 _LOG_POST
#define CRITICAL2(f,a1)  _LOG_PRE(*_log_defaultCategory, LP_CRITICAL, f) ,a1 _LOG_POST
#define ALERT2(f,a1)     _LOG_PRE(*_log_defaultCategory, LP_ALERT, f) ,a1 _LOG_POST
#define EMERGENCY2(f,a1) _LOG_PRE(*_log_defaultCategory, LP_EMERGENCY, f) ,a1  _LOG_POST
// Many repetitions follow...
#define CLOG5(c, p, f,a1,a2) _LOG_PRE(_LOGV(c),p,f) ,a1,a2 _LOG_POST
#define CTRACE4(c, f,a1,a2) CLOG5(c, LP_TRACE, f,a1,a2)
#define CDEBUG4(c, f,a1,a2) CLOG5(c, LP_DEBUG, f,a1,a2)
#define CINFO4(c, f,a1,a2) CLOG5(c, LP_INFO, f,a1,a2)
#define CNOTICE4(c, f,a1,a2) CLOG5(c, LP_NOTICE, f,a1,a2)
#define CWARNING4(c, f,a1,a2) CLOG5(c, LP_WARNING, f,a1,a2)
#define CERROR4(c, f,a1,a2) CLOG5(c, LP_ERROR, f,a1,a2)
#define CCRITICAL4(c, f,a1,a2) CLOG5(c, LP_CRITICAL, f,a1,a2)
#define CALERT4(c, f,a1,a2) CLOG5(c, LP_ALERT, f,a1,a2)
#define CEMERGENCY4(c, f,a1,a2) CLOG5(c, LP_EMERGENCY, f,a1,a2)
#define LOG4(p, f,a1,a2)    _LOG_PRE(*_log_defaultCategory, p, f) ,a1,a2 _LOG_POST
#define INFO3(f,a1,a2)      _LOG_PRE(*_log_defaultCategory, LP_INFO, f) ,a1,a2 _LOG_POST
#define NOTICE3(f,a1,a2)    _LOG_PRE(*_log_defaultCategory, LP_NOTICE, f) ,a1,a2 _LOG_POST
#define TRACE3(f,a1,a2)     _LOG_PRE(*_log_defaultCategory, LP_TRACE, f) ,a1,a2 _LOG_POST
#define DEBUG3(f,a1,a2)     _LOG_PRE(*_log_defaultCategory, LP_DEBUG, f) ,a1,a2 _LOG_POST
#define WARNING3(f,a1,a2)   _LOG_PRE(*_log_defaultCategory, LP_WARNING, f) ,a1,a2 _LOG_POST
#define ERROR3(f,a1,a2)     _LOG_PRE(*_log_defaultCategory, LP_ERROR, f) ,a1,a2 _LOG_POST
#define CRITICAL3(f,a1,a2)  _LOG_PRE(*_log_defaultCategory, LP_CRITICAL, f) ,a1,a2 _LOG_POST
#define ALERT3(f,a1,a2)     _LOG_PRE(*_log_defaultCategory, LP_ALERT, f) ,a1,a2 _LOG_POST
#define EMERGENCY3(f,a1,a2) _LOG_PRE(*_log_defaultCategory, LP_EMERGENCY, f) ,a1,a2  _LOG_POST
// Many repetitions follow...
#define CLOG6(c, p, f,a1,a2,a3) _LOG_PRE(_LOGV(c),p,f) ,a1,a2,a3 _LOG_POST
#define CTRACE5(c, f,a1,a2,a3) CLOG6(c, LP_TRACE, f,a1,a2,a3)
#define CDEBUG5(c, f,a1,a2,a3) CLOG6(c, LP_DEBUG, f,a1,a2,a3)
#define CINFO5(c, f,a1,a2,a3) CLOG6(c, LP_INFO, f,a1,a2,a3)
#define CNOTICE5(c, f,a1,a2,a3) CLOG6(c, LP_NOTICE, f,a1,a2,a3)
#define CWARNING5(c, f,a1,a2,a3) CLOG6(c, LP_WARNING, f,a1,a2,a3)
#define CERROR5(c, f,a1,a2,a3) CLOG6(c, LP_ERROR, f,a1,a2,a3)
#define CCRITICAL5(c, f,a1,a2,a3) CLOG6(c, LP_CRITICAL, f,a1,a2,a3)
#define CALERT5(c, f,a1,a2,a3) CLOG6(c, LP_ALERT, f,a1,a2,a3)
#define CEMERGENCY5(c, f,a1,a2,a3) CLOG6(c, LP_EMERGENCY, f,a1,a2,a3)
#define LOG5(p, f,a1,a2,a3)    _LOG_PRE(*_log_defaultCategory, p, f) ,a1,a2,a3 _LOG_POST
#define INFO4(f,a1,a2,a3)      _LOG_PRE(*_log_defaultCategory, LP_INFO, f) ,a1,a2,a3 _LOG_POST
#define NOTICE4(f,a1,a2,a3)    _LOG_PRE(*_log_defaultCategory, LP_NOTICE, f) ,a1,a2,a3 _LOG_POST
#define TRACE4(f,a1,a2,a3)     _LOG_PRE(*_log_defaultCategory, LP_TRACE, f) ,a1,a2,a3 _LOG_POST
#define DEBUG4(f,a1,a2,a3)     _LOG_PRE(*_log_defaultCategory, LP_DEBUG, f) ,a1,a2,a3 _LOG_POST
#define WARNING4(f,a1,a2,a3)   _LOG_PRE(*_log_defaultCategory, LP_WARNING, f) ,a1,a2,a3 _LOG_POST
#define ERROR4(f,a1,a2,a3)     _LOG_PRE(*_log_defaultCategory, LP_ERROR, f) ,a1,a2,a3 _LOG_POST
#define CRITICAL4(f,a1,a2,a3)  _LOG_PRE(*_log_defaultCategory, LP_CRITICAL, f) ,a1,a2,a3 _LOG_POST
#define ALERT4(f,a1,a2,a3)     _LOG_PRE(*_log_defaultCategory, LP_ALERT, f) ,a1,a2,a3 _LOG_POST
#define EMERGENCY4(f,a1,a2,a3) _LOG_PRE(*_log_defaultCategory, LP_EMERGENCY, f) ,a1,a2,a3  _LOG_POST
// Many repetitions follow...
#define CLOG7(c, p, f,a1,a2,a3,a4) _LOG_PRE(_LOGV(c),p,f) ,a1,a2,a3,a4 _LOG_POST
#define CTRACE6(c, f,a1,a2,a3,a4) CLOG7(c, LP_TRACE, f,a1,a2,a3,a4)
#define CDEBUG6(c, f,a1,a2,a3,a4) CLOG7(c, LP_DEBUG, f,a1,a2,a3,a4)
#define CINFO6(c, f,a1,a2,a3,a4) CLOG7(c, LP_INFO, f,a1,a2,a3,a4)
#define CNOTICE6(c, f,a1,a2,a3,a4) CLOG7(c, LP_NOTICE, f,a1,a2,a3,a4)
#define CWARNING6(c, f,a1,a2,a3,a4) CLOG7(c, LP_WARNING, f,a1,a2,a3,a4)
#define CERROR6(c, f,a1,a2,a3,a4) CLOG7(c, LP_ERROR, f,a1,a2,a3,a4)
#define CCRITICAL6(c, f,a1,a2,a3,a4) CLOG7(c, LP_CRITICAL, f,a1,a2,a3,a4)
#define CALERT6(c, f,a1,a2,a3,a4) CLOG7(c, LP_ALERT, f,a1,a2,a3,a4)
#define CEMERGENCY6(c, f,a1,a2,a3,a4) CLOG7(c, LP_EMERGENCY, f,a1,a2,a3,a4)
#define LOG6(p, f,a1,a2,a3,a4)    _LOG_PRE(*_log_defaultCategory, p, f) ,a1,a2,a3,a4 _LOG_POST
#define INFO5(f,a1,a2,a3,a4)      _LOG_PRE(*_log_defaultCategory, LP_INFO, f) ,a1,a2,a3,a4 _LOG_POST
#define NOTICE5(f,a1,a2,a3,a4)    _LOG_PRE(*_log_defaultCategory, LP_NOTICE, f) ,a1,a2,a3,a4 _LOG_POST
#define TRACE5(f,a1,a2,a3,a4)     _LOG_PRE(*_log_defaultCategory, LP_TRACE, f) ,a1,a2,a3,a4 _LOG_POST
#define DEBUG5(f,a1,a2,a3,a4)     _LOG_PRE(*_log_defaultCategory, LP_DEBUG, f) ,a1,a2,a3,a4 _LOG_POST
#define WARNING5(f,a1,a2,a3,a4)   _LOG_PRE(*_log_defaultCategory, LP_WARNING, f) ,a1,a2,a3,a4 _LOG_POST
#define ERROR5(f,a1,a2,a3,a4)     _LOG_PRE(*_log_defaultCategory, LP_ERROR, f) ,a1,a2,a3,a4 _LOG_POST
#define CRITICAL5(f,a1,a2,a3,a4)  _LOG_PRE(*_log_defaultCategory, LP_CRITICAL, f) ,a1,a2,a3,a4 _LOG_POST
#define ALERT5(f,a1,a2,a3,a4)     _LOG_PRE(*_log_defaultCategory, LP_ALERT, f) ,a1,a2,a3,a4 _LOG_POST
#define EMERGENCY5(f,a1,a2,a3,a4) _LOG_PRE(*_log_defaultCategory, LP_EMERGENCY, f) ,a1,a2,a3,a4  _LOG_POST
// Many repetitions follow...
#define CLOG8(c, p, f,a1,a2,a3,a4,a5) _LOG_PRE(_LOGV(c),p,f) ,a1,a2,a3,a4,a5 _LOG_POST
#define CTRACE7(c, f,a1,a2,a3,a4,a5) CLOG8(c, LP_TRACE, f,a1,a2,a3,a4,a5)
#define CDEBUG7(c, f,a1,a2,a3,a4,a5) CLOG8(c, LP_DEBUG, f,a1,a2,a3,a4,a5)
#define CINFO7(c, f,a1,a2,a3,a4,a5) CLOG8(c, LP_INFO, f,a1,a2,a3,a4,a5)
#define CNOTICE7(c, f,a1,a2,a3,a4,a5) CLOG8(c, LP_NOTICE, f,a1,a2,a3,a4,a5)
#define CWARNING7(c, f,a1,a2,a3,a4,a5) CLOG8(c, LP_WARNING, f,a1,a2,a3,a4,a5)
#define CERROR7(c, f,a1,a2,a3,a4,a5) CLOG8(c, LP_ERROR, f,a1,a2,a3,a4,a5)
#define CCRITICAL7(c, f,a1,a2,a3,a4,a5) CLOG8(c, LP_CRITICAL, f,a1,a2,a3,a4,a5)
#define CALERT7(c, f,a1,a2,a3,a4,a5) CLOG8(c, LP_ALERT, f,a1,a2,a3,a4,a5)
#define CEMERGENCY7(c, f,a1,a2,a3,a4,a5) CLOG8(c, LP_EMERGENCY, f,a1,a2,a3,a4,a5)
#define LOG7(p, f,a1,a2,a3,a4,a5)    _LOG_PRE(*_log_defaultCategory, p, f) ,a1,a2,a3,a4,a5 _LOG_POST
#define INFO6(f,a1,a2,a3,a4,a5)      _LOG_PRE(*_log_defaultCategory, LP_INFO, f) ,a1,a2,a3,a4,a5 _LOG_POST
#define NOTICE6(f,a1,a2,a3,a4,a5)    _LOG_PRE(*_log_defaultCategory, LP_NOTICE, f) ,a1,a2,a3,a4,a5 _LOG_POST
#define TRACE6(f,a1,a2,a3,a4,a5)     _LOG_PRE(*_log_defaultCategory, LP_TRACE, f) ,a1,a2,a3,a4,a5 _LOG_POST
#define DEBUG6(f,a1,a2,a3,a4,a5)     _LOG_PRE(*_log_defaultCategory, LP_DEBUG, f) ,a1,a2,a3,a4,a5 _LOG_POST
#define WARNING6(f,a1,a2,a3,a4,a5)   _LOG_PRE(*_log_defaultCategory, LP_WARNING, f) ,a1,a2,a3,a4,a5 _LOG_POST
#define ERROR6(f,a1,a2,a3,a4,a5)     _LOG_PRE(*_log_defaultCategory, LP_ERROR, f) ,a1,a2,a3,a4,a5 _LOG_POST
#define CRITICAL6(f,a1,a2,a3,a4,a5)  _LOG_PRE(*_log_defaultCategory, LP_CRITICAL, f) ,a1,a2,a3,a4,a5 _LOG_POST
#define ALERT6(f,a1,a2,a3,a4,a5)     _LOG_PRE(*_log_defaultCategory, LP_ALERT, f) ,a1,a2,a3,a4,a5 _LOG_POST
#define EMERGENCY6(f,a1,a2,a3,a4,a5) _LOG_PRE(*_log_defaultCategory, LP_EMERGENCY, f) ,a1,a2,a3,a4,a5  _LOG_POST
// Many repetitions follow...
#define CLOG9(c, p, f,a1,a2,a3,a4,a5,a6) _LOG_PRE(_LOGV(c),p,f) ,a1,a2,a3,a4,a5,a6 _LOG_POST
#define CTRACE8(c, f,a1,a2,a3,a4,a5,a6) CLOG9(c, LP_TRACE, f,a1,a2,a3,a4,a5,a6)
#define CDEBUG8(c, f,a1,a2,a3,a4,a5,a6) CLOG9(c, LP_DEBUG, f,a1,a2,a3,a4,a5,a6)
#define CINFO8(c, f,a1,a2,a3,a4,a5,a6) CLOG9(c, LP_INFO, f,a1,a2,a3,a4,a5,a6)
#define CNOTICE8(c, f,a1,a2,a3,a4,a5,a6) CLOG9(c, LP_NOTICE, f,a1,a2,a3,a4,a5,a6)
#define CWARNING8(c, f,a1,a2,a3,a4,a5,a6) CLOG9(c, LP_WARNING, f,a1,a2,a3,a4,a5,a6)
#define CERROR8(c, f,a1,a2,a3,a4,a5,a6) CLOG9(c, LP_ERROR, f,a1,a2,a3,a4,a5,a6)
#define CCRITICAL8(c, f,a1,a2,a3,a4,a5,a6) CLOG9(c, LP_CRITICAL, f,a1,a2,a3,a4,a5,a6)
#define CALERT8(c, f,a1,a2,a3,a4,a5,a6) CLOG9(c, LP_ALERT, f,a1,a2,a3,a4,a5,a6)
#define CEMERGENCY8(c, f,a1,a2,a3,a4,a5,a6) CLOG9(c, LP_EMERGENCY, f,a1,a2,a3,a4,a5,a6)
#define LOG8(p, f,a1,a2,a3,a4,a5,a6)    _LOG_PRE(*_log_defaultCategory, p, f) ,a1,a2,a3,a4,a5,a6 _LOG_POST
#define INFO7(f,a1,a2,a3,a4,a5,a6)      _LOG_PRE(*_log_defaultCategory, LP_INFO, f) ,a1,a2,a3,a4,a5,a6 _LOG_POST
#define NOTICE7(f,a1,a2,a3,a4,a5,a6)    _LOG_PRE(*_log_defaultCategory, LP_NOTICE, f) ,a1,a2,a3,a4,a5,a6 _LOG_POST
#define TRACE7(f,a1,a2,a3,a4,a5,a6)     _LOG_PRE(*_log_defaultCategory, LP_TRACE, f) ,a1,a2,a3,a4,a5,a6 _LOG_POST
#define DEBUG7(f,a1,a2,a3,a4,a5,a6)     _LOG_PRE(*_log_defaultCategory, LP_DEBUG, f) ,a1,a2,a3,a4,a5,a6 _LOG_POST
#define WARNING7(f,a1,a2,a3,a4,a5,a6)   _LOG_PRE(*_log_defaultCategory, LP_WARNING, f) ,a1,a2,a3,a4,a5,a6 _LOG_POST
#define ERROR7(f,a1,a2,a3,a4,a5,a6)     _LOG_PRE(*_log_defaultCategory, LP_ERROR, f) ,a1,a2,a3,a4,a5,a6 _LOG_POST
#define CRITICAL7(f,a1,a2,a3,a4,a5,a6)  _LOG_PRE(*_log_defaultCategory, LP_CRITICAL, f) ,a1,a2,a3,a4,a5,a6 _LOG_POST
#define ALERT7(f,a1,a2,a3,a4,a5,a6)     _LOG_PRE(*_log_defaultCategory, LP_ALERT, f) ,a1,a2,a3,a4,a5,a6 _LOG_POST
#define EMERGENCY7(f,a1,a2,a3,a4,a5,a6) _LOG_PRE(*_log_defaultCategory, LP_EMERGENCY, f) ,a1,a2,a3,a4,a5,a6  _LOG_POST
//@}
#endif // _LOG4C_H_
