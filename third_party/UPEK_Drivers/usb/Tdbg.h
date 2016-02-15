/**
 * @file Tdbg.h
 * Generic header for all platform dependent TDBG implementations.
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */

#if defined(_MSC_VER) && (_MSC_VER > 1000)
#pragma once
#endif

#ifndef __TDBG_H__
#define __TDBG_H__

/**
 * @file Tdbg.h Generic implementation of tracing and assertion macros.\ See
 * @ref tdbg_doc for further documentation

 * @author Michal Vodicka <michal.vodicka@upek.com>
 * @author Pavel Holejsovsky <pavel.holejsovsky@upek.com>
 */


#if defined(DOXYGEN_PASS)
/**
 * @hideinitializer
 * Determines the level of debugging ability linked into the binary.  Different
 * values for this macro specify the debugging ability available in the
 * resulting binary. Available values are:
 * - @c 0   No debugging at all, smallest binaries.
 * - @c 1 Only asserts are available, but assert failure is only written to OS
 *   debug log.
 * - @c 2 Full asserts and traces are available, asserts also show up as
 *   message boxes.
 *
 * If the value is not directly specified, it is inferred automatically by the
 * definition of @c NDEBUG symbol. If @c NDEBUG is defined (i.e. release
 * builds), level 1 (only asserts) is set. Otherwise (debug builds) level 2 is
 * set.
 *
 * @note Setting of this define must be consistent (the same) for all sources
 * of single binary module, including @c Tdbg.cpp implementation
 * file. Therefore it is recommended to set it only from compiler's commandline
 * (e.g. @c -D_TDBG_LEVEL=x), which also allows building one module from the
 * same sources with different debugging ability included. Do not set @c
 * _TDBG_LEVEL using @c define directive in your sources at all.
 */
#define _TDBG_LEVEL 2

/**
 * Simple assertion.\ See @ref assert_doc for further details
 */
#define TASSERT(Expression)

/**
 * Always-fail assertion.\ See @ref assert_doc for further details.
 */
#define TASSERTF()

/**
 * Iteration assertion, condition must be fulfilled for all iterations.\ See
 * @ref assert_doc for further details.
 */
#define TASSERT_FORALL(ForCondition, Expression)

/**
 * Iteration assertion, condition must be fulfilled at least for one
 * interation.\ See @ref assert_doc for further details.
 */
#define TASSERT_EXISTS(ForCondition, Expression)

/**
 * Macro controlling whether debug area levels are dynamically updated from
 * registry during the run of the program.  When this macro is defined during
 * translation of @c Tdbg.cpp, resulting module will not have area-level
 * dynamic update functionality. Tracing level for the areas are read at the
 * start of the module only. This may be useful for programs which do not want
 * to consult registry every time the @c TTRACE macro is hit (performance
 * gain?), but is very inconvenient for programs like logon and system
 * services, which are usually not easy to stop (to reread the area level
 * configuration).
 *
 * When module is compiled with this macro defined, it automatically creates @c
 * info_nodynupdate value in its key in registry (see @ref tdbgconfig_doc), so
 * that user is aware that changes made in this key are not updated in the
 * program which is already running until the program is re-run.
 */
#define _TDBG_NODYNUPDATE

/**
 * Declares debug area defined in other translation unit.
 *
 * @param Name Name of the area, used in @c TTRACE macros, placed in trace
 * outputs and also identifying area for setting the trace level in registry.
 */
#define TDBG_DECLARE_AREA(Name)

/**
 * Defines area with given name. Name should be short, because it must be typed
 * in every TTRACE macro and also appears in the ouput, long names make trace
 * output more confusing. This macro must be used only in one .cpp file per
 * binary. Use @c TDBG_DECLARE_AREA to reference one created area in other .cpp
 * or .h files.
 *
 * @param Name Name of the area, used in @c TTRACE macros, placed in trace
 * outputs and also identifying area for setting the trace level in registry
 */
#define TDBG_DEFINE_AREA(Name)

/**
 * Helper macro which allows to trace the top level of the error using @c
 * TTRACE macro.
 *
 * @param Area debug area - see documentation of #TDBG_DEFINE_AREA
 * @param Level debug level - see documentation of #TdbgLevelType
 * @param Message text which prepends the trace of the string
 * @param Err error to trace
 */
#define TTRACE_ERROR(Area, Level, Message, Err)

/**
 * Tracing macro.\ See @ref tracing_doc for further details.
 *
 * @param Area debug area - see documentation of #TDBG_DEFINE_AREA
 * @param Level debug level - see documentation of #TdbgLevelType
 *
 * @param Params debug message and accompanied parameters, comma delimited.
 * The whole 'Params' parameter must be in parens.
 */
#define TTRACE(Area, Level, Params)

/**
 * Single-line memory dump macro.\ See @ref dumping_doc for further details.
 */
#define TDUMP(Area, Level, Label, Data, Length)

/**
 * Multiline memory dump macro.\ See @ref dumping_doc for further details.
 */
#define TDUMP_EX(Area, Level, Label, Data, Length)

/**
 * Tracing macros for automatic tracing of the enter/leave history of blocks,
 * mainly functions.  These macros are intended to be used on the very
 * beginning of the block. They have the same syntax as TTRACE macros and one
 * functional difference. When the block is entered, it will produce <tt>"==>
 * message"</tt>, and when the block leaved, it will produce <tt>"<==
 * message"</tt>, where message is the string defined by the arguments.  See
 * @ref tracing_doc for further details.
 */
#define TTRACE_BLOCK(Area, Level, Params)

/**
 * Available levels of tracing.  Declares tracing levels. Every trace message
 * declares its own tracing level and debug area, and every debug area has set
 * its own tracing level. If the tracing level of the debug area is higher than
 * tracing level of the message, the message is not emitted but silently
 * skipped
 */
enum TdbgLevelType
{
    /**
     * This level must not be used by applications, it is declared only to
     * allow completely turn off traces for given area.
     */
    TDBG_NONE,
    /**
     * %Error occured, something is wrong but the system can recover from this.
     */
    TDBG_ERROR,

    /**
     * No error, but some unexpected situation which may or may not be correct.
     */
    TDBG_WARN,

    /**
     * Normal flow of program, message with this classification should be
     * placed at important places (e.g.\ entry/exit points of the whole
     * module).
     */
    TDBG_DEBUG,

    /**
     * Detailed information about program flow.\ This level should be turned on
     * for the area only if some suspect behaviour happens, can produce <em>a
     * lot</em> of output for some areas.
     */
    TDBG_TRACE,

    /**
     * <em>Very</em> detailed information about program flow. This level should
     * be used for tracing very frequent events which shouldn't be normally
     * visible and should be enabled only if a problem occurs.
     */
    TDBG_LOUD
};

#else // defined(DOXYGEN_PASS)

// ---- platform defaults ------------------

// For every supported platform there has to be unique _TDBG_PLT_XXX variable
// and default debug levels for both release and debug compilation. Change with care.

// win32
#define _TDBG_PLT_W32                   1
#define _DEF_TDBG_LEVEL_REL_W32         2
#define _DEF_TDBG_LEVEL_DBG_W32         2
#define _DEF_TDBG_AREA_DEFAULT_REL_W32  0
#define _DEF_TDBG_AREA_DEFAULT_DBG_W32  2
#define _DEF_TDBG_WATCH_UPDATE_W32      1
#define _DEF_TDBG_NO_DYN_UPDATE_W32     0
// NT kernel
#define _TDBG_PLT_K32                   2
#define _DEF_TDBG_LEVEL_REL_K32         2
#define _DEF_TDBG_LEVEL_DBG_K32         2
#define _DEF_TDBG_AREA_DEFAULT_REL_K32  0
#define _DEF_TDBG_AREA_DEFAULT_DBG_K32  2
#define _DEF_TDBG_WATCH_UPDATE_K32      1
#define _DEF_TDBG_NO_DYN_UPDATE_K32     0
// ARC
#define _TDBG_PLT_ARC                   3
#define _DEF_TDBG_LEVEL_REL_ARC         0
#define _DEF_TDBG_LEVEL_DBG_ARC         1
#define _DEF_TDBG_AREA_DEFAULT_REL_ARC  2
#define _DEF_TDBG_AREA_DEFAULT_DBG_ARC  2
#define _DEF_TDBG_WATCH_UPDATE_ARC      1
#define _DEF_TDBG_NO_DYN_UPDATE_ARC     0
// ARM
#define _TDBG_PLT_ARM                   4
#define _DEF_TDBG_LEVEL_REL_ARM         0
#define _DEF_TDBG_LEVEL_DBG_ARM         0
#define _DEF_TDBG_AREA_DEFAULT_REL_ARM  2
#define _DEF_TDBG_AREA_DEFAULT_DBG_ARM  2
#define _DEF_TDBG_WATCH_UPDATE_ARM      1
#define _DEF_TDBG_NO_DYN_UPDATE_ARM     0
// WinCE
#define _TDBG_PLT_WCE                   5
#define _DEF_TDBG_LEVEL_REL_WCE         2
#define _DEF_TDBG_LEVEL_DBG_WCE         2
#define _DEF_TDBG_AREA_DEFAULT_REL_WCE  0
#define _DEF_TDBG_AREA_DEFAULT_DBG_WCE  2
#define _DEF_TDBG_WATCH_UPDATE_WCE      0
#define _DEF_TDBG_NO_DYN_UPDATE_WCE     1
// Linux
#define _TDBG_PLT_LNX                   6
#define _DEF_TDBG_LEVEL_REL_LNX         2
#define _DEF_TDBG_LEVEL_DBG_LNX         2
#define _DEF_TDBG_AREA_DEFAULT_REL_LNX  0
#define _DEF_TDBG_AREA_DEFAULT_DBG_LNX  2
#define _DEF_TDBG_WATCH_UPDATE_LNX      1
#define _DEF_TDBG_NO_DYN_UPDATE_LNX     0
// DOS
#define _TDBG_PLT_DOS                   7
#define _DEF_TDBG_LEVEL_REL_DOS         0
#define _DEF_TDBG_LEVEL_DBG_DOS         0
#define _DEF_TDBG_AREA_DEFAULT_REL_DOS  0
#define _DEF_TDBG_AREA_DEFAULT_DBG_DOS  2
#define _DEF_TDBG_WATCH_UPDATE_DOS      0
#define _DEF_TDBG_NO_DYN_UPDATE_DOS     1
// Symbian
#define _TDBG_PLT_EPOC                  8
#define _DEF_TDBG_LEVEL_REL_EPOC        0
#define _DEF_TDBG_LEVEL_DBG_EPOC        0
#define _DEF_TDBG_AREA_DEFAULT_REL_EPOC 0
#define _DEF_TDBG_AREA_DEFAULT_DBG_EPOC 2
#define _DEF_TDBG_WATCH_UPDATE_EPOC     0
#define _DEF_TDBG_NO_DYN_UPDATE_EPOC    1
// MacOS X
#define _TDBG_PLT_OSX                   9
#define _DEF_TDBG_LEVEL_REL_OSX         2
#define _DEF_TDBG_LEVEL_DBG_OSX         2
#define _DEF_TDBG_AREA_DEFAULT_REL_OSX  0
#define _DEF_TDBG_AREA_DEFAULT_DBG_OSX  2
#define _DEF_TDBG_WATCH_UPDATE_OSX      1
#define _DEF_TDBG_NO_DYN_UPDATE_OSX     0
// *BSD (FreeBSD, OpenBSD etc. probably can be shared)
#define _TDBG_PLT_BSD                   10
#define _DEF_TDBG_LEVEL_REL_BSD         2
#define _DEF_TDBG_LEVEL_DBG_BSD         2
#define _DEF_TDBG_AREA_DEFAULT_REL_BSD  0
#define _DEF_TDBG_AREA_DEFAULT_DBG_BSD  2
#define _DEF_TDBG_WATCH_UPDATE_BSD      1
#define _DEF_TDBG_NO_DYN_UPDATE_BSD     0


// ---- crazy preprocessor stuff -----------

// convert Symbol to its string value
#define _TDBG_GET_STR_VALUE(Symbol) \
    #Symbol

// get string from an Expression
#define _TDBG_MAKE_STRING(Expression) \
    _TDBG_GET_STR_VALUE(Expression)

// concatenate Prefix and Suffix to symbol name
#define _TDBG_BUILD_SYMBOL(Prefix, Suffix)  \
    Prefix##Suffix

// add Prefix to preprocessor Symbol
#define _TDBG_PREFIX_SYMBOL(Prefix, Symbol) \
    _TDBG_BUILD_SYMBOL(Prefix, Symbol)

#if defined(__arm)
// handle silly ARM compiler which isn't able to use above statement
#define _TDBG_PLT_VAR(Prefix)   \
    Prefix##ARM
#elif defined(_MSDOS)
#define _TDBG_PLT_VAR(Prefix)   \
    Prefix##DOS
#else
// create platform dependent preprocessor variable name (add current platform as a suffix)
#define _TDBG_PLT_VAR(Prefix)   \
    _TDBG_PREFIX_SYMBOL(Prefix, _TDBG_PLATFORM)
#endif

// create a string from StrPrefix and current platform value
#define _TDBG_PLT_STR(StrPrefix)    \
    StrPrefix ##_TDBG_GET_STR_VALUE(_TDBG_PLATFORM)

// ---- platform selection -----------------

#if !defined(_TDBG_PLATFORM)
// autoselect
    #if defined(_ARC)
        #define _TDBG_PLATFORM      ARC
    #elif defined(__SYMBIAN32__)
        #define _TDBG_PLATFORM      EPOC
    #elif defined(__arm)
        #define _TDBG_PLATFORM      ARM
    #elif defined(DEVL)
    // defined by DDK environment when debug info is generated
        #define _TDBG_PLATFORM      K32
    #elif defined(UNDER_CE)
    // defined by Platform Builder environment
        #define _TDBG_PLATFORM      WCE
    #elif defined(__linux__)
        #define _TDBG_PLATFORM      LNX
    #elif defined(__APPLE__) && defined(__MACH__)
        #define _TDBG_PLATFORM      OSX
    #elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	#define _TDBG_PLATFORM      BSD
    #elif defined(_MSDOS)
        #define _TDBG_PLATFORM      DOS
    #else
    // nothing of above, use win32
        #define _TDBG_PLATFORM      W32
    #endif
#else   // if defined(_TDBG_PLATFORM)
// check if defined platform is supported
    #if !_TDBG_PLT_VAR(_TDBG_PLT_)
    // don't know how to expand value within #error so use message before
        #pragma message("Defined platform: " _TDBG_PLT_STR(""))
        #error Platform not supported (use W32, K32, ARC, ARM, WCE, LNX, BSD, OSX, DOS or EPOC)
    #endif
#endif  // defined(_TDBG_PLATFORM)

// #pragma message("Defined platform: " _TDBG_PLT_STR(""))

// ---- debug level selection --------------

// Find out the correct definition for _TDBG_LEVEL, if not already defined.
#if !defined(_TDBG_LEVEL)
    #if defined(NDEBUG)
        #define _TDBG_LEVEL _TDBG_PLT_VAR(_DEF_TDBG_LEVEL_REL_)
    #else   // if !defined(NDEBUG)
        #define _TDBG_LEVEL _TDBG_PLT_VAR(_DEF_TDBG_LEVEL_DBG_)
    #endif    // !defined (NDEBUG)
#endif  // !defined(_TDBG_LEVEL)

// check if level value is correct
#if !defined(_TDBG_LEVEL) || (_TDBG_LEVEL != 0 && _TDBG_LEVEL != 1 && _TDBG_LEVEL != 2)
    #pragma message("_TDBG_LEVEL: " _TDBG_MAKE_STRING(_TDBG_LEVEL))
    #error _TDBG_LEVEL macro has an unknown value. Terminating the compilation.
#endif

#if !defined(_TDBG_NOP)
#define _TDBG_NOP   ((void)0)
#endif // !defined(_TDBG_NOP)

// ---- trace levels -----------------------

#if _TDBG_LEVEL > 1
// define trace levels before including platform specific headers

enum TdbgLevelType
{
    TDBG_NONE,
    TDBG_ERROR,
    TDBG_WARN,
    TDBG_DEBUG,
    TDBG_TRACE,
    TDBG_LOUD
};

#if !defined(_TDBG_AREA_DEFAULT)
// set default level used for area initialization
    #if defined(NDEBUG)
        #define _TDBG_AREA_DEFAULT _TDBG_PLT_VAR(_DEF_TDBG_AREA_DEFAULT_REL_)
    #else   // if !defined(NDEBUG)
        #define _TDBG_AREA_DEFAULT _TDBG_PLT_VAR(_DEF_TDBG_AREA_DEFAULT_DBG_)
    #endif    // !defined (NDEBUG)
#else   // if _TDBG_AREA_DEFAULT
// check if default area level is valid
    #if (_TDBG_AREA_DEFAULT > 5)
        #pragma message("_TDBG_AREA_DEFAULT: " _TDBG_MAKE_STRING(_TDBG_AREA_DEFAULT))
        #error Invalid _TDBG_AREA_DEFAULT value.
    #endif  // _TDBG_AREA_DEFAULT is valid
#endif  // _TDBG_AREA_DEFAULT
#define _TDBG_AREA_DEFAULT_VALUE  ((enum TdbgLevelType)_TDBG_AREA_DEFAULT)

#if !defined(_TDBG_WATCH_UPDATE)
// set default watch update value
    #define _TDBG_WATCH_UPDATE_VALUE        _TDBG_PLT_VAR(_DEF_TDBG_WATCH_UPDATE_)
#else   // if _TDBG_WATCH_UPDATE
// set correct value
    #if _TDBG_WATCH_UPDATE
        #define _TDBG_WATCH_UPDATE_VALUE    1
    #else
        #define _TDBG_WATCH_UPDATE_VALUE    0
    #endif  // _TDBG_WATCH_UPDATE
#endif  // _TDBG_WATCH_UPDATE

#if !defined(_TDBG_NO_DYN_UPDATE)
// set default watch update value
    #define _TDBG_NO_DYN_UPDATE_VALUE       _TDBG_PLT_VAR(_DEF_TDBG_NO_DYN_UPDATE_)
#else   // if _TDBG_NO_DYN_UPDATE
// set correct value
    #if _TDBG_NO_DYN_UPDATE
        #define _TDBG_NO_DYN_UPDATE_VALUE   1
    #else
        #define _TDBG_NO_DYN_UPDATE_VALUE   0
    #endif  // _TDBG_NO_DYN_UPDATE
#endif  // _TDBG_NO_DYN_UPDATE

#if !defined(__cplusplus)
// define types for C-only implementations
typedef enum TdbgLevelType TDBG_LEVEL, *PTDBG_LEVEL;
#endif  // !__cplusplus

#endif  // _TDBG_LEVEL > 1

#if _TDBG_LEVEL > 0
// ---- _TDBG_LEVEL > 0 --------------------

// Include platform specific headers.
//
// The sad thing is we can't use variable to include platform specific name
// because preprocessor doesn't (probably) support it
#if _TDBG_PLT_VAR(_TDBG_PLT_) == _TDBG_PLT_W32
    #include "platform/win/Tdbg.h"
#elif _TDBG_PLT_VAR(_TDBG_PLT_) == _TDBG_PLT_K32
    #include "platform/k32/tdbg.h"
#elif _TDBG_PLT_VAR(_TDBG_PLT_) == _TDBG_PLT_ARC
    #include "platform/arc/tdbg.h"
#elif _TDBG_PLT_VAR(_TDBG_PLT_) == _TDBG_PLT_ARM
    #include "platform/arm/tdbg.h"
#elif _TDBG_PLT_VAR(_TDBG_PLT_) == _TDBG_PLT_WCE
    #include "platform/wce/Tdbg.h"
#elif _TDBG_PLT_VAR(_TDBG_PLT_) == _TDBG_PLT_LNX  ||  \
      _TDBG_PLT_VAR(_TDBG_PLT_) == _TDBG_PLT_BSD  ||  \
      _TDBG_PLT_VAR(_TDBG_PLT_) == _TDBG_PLT_OSX
    #include "platform/posix/Tdbg.h"
#elif _TDBG_PLT_VAR(_TDBG_DOS_) == _TDBG_PLT_DOS_
    #include "platform/dos/Tdbg.h"
#elif _TDBG_PLT_VAR(_TDBG_EPOC_) == _TDBG_PLT_EPOC_
    #include "platform/epoc/Tdbg.h"
#else
    #error _TDBG_PLT_XXX value has to be defined for current platform.
#endif

#ifndef TDEBUG
/**
 * @hideinitializer
 * Macro used to hide parameter which is used only during debug build.  Its
 * purpose is to avoid compiler warning for release builds, when @c _TDBG_LEVEL
 * is not defined or is 0
 */
#define TDEBUG(Param)           Param
#endif

// check level 1 functionality
#if !defined(TASSERT)
    #error Incomplete TDBG implementation. TASSERT has to be defined.
#endif  // !TASSERT

// define undefined macros
#if !defined(TVERIFY)
#define TVERIFY(Expression) \
    TASSERT(Expression)
#endif  // TVERIFY

#if !defined(TASSERTF)
#define TASSERTF()  \
    TASSERT(FALSE)
#endif  // TASSERTF

#if defined(_lint)
// special lint support
#if defined(__cplusplus)
bool __assert(bool Expression) throw();         // prototype for lint
bool __assert(int Expression) throw();          // to be able to TVERIFY() win32 functions
#else   // if !__cplusplus
int __assert(int Expression);
#endif  // !__cplusplus

#undef TASSERT
#undef TASSERTF

#define TASSERT(Expression) __assert(Expression)

#if defined(__cplusplus)
#define TASSERTF()          __assert(false)
#else   // if !__cplusplus
#define TASSERTF()          __assert(FALSE)
#endif  // !__cplusplus

//lint -esym(534, __assert)

#endif  // _lint

#endif  // _TDBG_LEVEL > 0

#if _TDBG_LEVEL > 1
// ---- _TDBG_LEVEL > 1 --------------------
// full functionality required

#if !defined(TDBG_DECLARE_AREA) || !defined(TDBG_DEFINE_AREA)
    #error Incomplete TDBG implementation. Area macros have to be defined.
#endif

#if !defined(TTRACE) || !defined(TDUMP_EX) || !defined(TTRACE_ENTER) || !defined(TTRACE_LEAVE)
    #error Incomplete TDBG implementation. Some basic trace macro(s) is missing.
#endif

#ifdef _lint
// redefine traces for lint; force parameter check
#undef TTRACE
#undef TTRACE_ENTER
#undef TTRACE_LEAVE

// fake prototype to avoid dependency on stdio.h
void __printf(const char *, ...);
//lint -printf(1, __printf)

#define TTRACE(Area, Level, Params) \
    __printf Params

#define TTRACE_ENTER(Area, Level, Params)   \
    __printf Params

#define TTRACE_LEAVE(Params)    \
    __printf Params
#endif  // _lint

// define missing numbered trace macros using generic trace
#ifndef TTRACE0
#define TTRACE0(Area, Level, Text)  \
    TTRACE(Area, Level, Text)
#endif  // TTRACE0

#ifndef TTRACE1
#define TTRACE1(Area, Level, Text, P1)  \
    TTRACE(Area, Level, Text, P1)
#endif  // TTRACE1

#ifndef TTRACE2
#define TTRACE2(Area, Level, Text, P1, P2)  \
    TTRACE(Area, Level, Text, P1, P2)
#endif  // TTRACE2

#ifndef TTRACE3
#define TTRACE3(Area, Level, Text, P1, P2, P3)  \
    TTRACE(Area, Level, Text, P1, P2, P3)
#endif  // TTRACE3

#ifndef TTRACE4
#define TTRACE4(Area, Level, Text, P1, P2, P3, P4)  \
    TTRACE(Area, Level, Text, P1, P2, P3, P4)
#endif  // TTRACE4

#ifndef TTRACE5
#define TTRACE5(Area, Level, Text, P1, P2, P3, P4, P5)  \
    TTRACE(Area, Level, Text, P1, P2, P3, P4, P5)
#endif  // TTRACE5

#ifndef TTRACE6
#define TTRACE6(Area, Level, Text, P1, P2, P3, P4, P5, P6)  \
    TTRACE(Area, Level, Text, P1, P2, P3, P4, P5, P6)
#endif  // TTRACE6

#ifndef TTRACE7
#define TTRACE7(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7)  \
    TTRACE(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7)
#endif  // TTRACE7

#ifndef TTRACE8
#define TTRACE8(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7, P8)  \
    TTRACE(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7, P8)
#endif  // TTRACE8

#ifndef TDUMP
#define TDUMP(Area, Level, Text, Data, Length)  \
    TDUMP_EX(Area, Level, Text, Data, Length)
#endif  // TDUMP

#if defined(__cplusplus)
// check C++ specific code
#if !defined(TTRACE_BLOCK)
    #error Incomplete C++ TDBG implementation.
#endif  // !TTRACE_BLOCK

// define missing numbered trace block macros using generic trace block
#ifndef TTRACE0_BLOCK
#define TTRACE0_BLOCK(Area, Level, Text)  \
    TTRACE_BLOCK(Area, Level, Text)
#endif  // TTRACE0_BLOCK

#ifndef TTRACE1_BLOCK
#define TTRACE1_BLOCK(Area, Level, Text, P1)  \
    TTRACE_BLOCK(Area, Level, Text, P1)
#endif  // TTRACE1_BLOCK

#ifndef TTRACE2_BLOCK
#define TTRACE2_BLOCK(Area, Level, Text, P1, P2)  \
    TTRACE_BLOCK(Area, Level, Text, P1, P2)
#endif  // TTRACE2_BLOCK

#ifndef TTRACE3_BLOCK
#define TTRACE3_BLOCK(Area, Level, Text, P1, P2, P3)  \
    TTRACE_BLOCK(Area, Level, Text, P1, P2, P3)
#endif  // TTRACE3_BLOCK

#ifndef TTRACE4_BLOCK
#define TTRACE4_BLOCK(Area, Level, Text, P1, P2, P3, P4)  \
    TTRACE_BLOCK(Area, Level, Text, P1, P2, P3, P4)
#endif  // TTRACE4_BLOCK

#ifndef TTRACE5_BLOCK
#define TTRACE5_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5)  \
    TTRACE_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5)
#endif  // TTRACE5_BLOCK

#ifndef TTRACE6_BLOCK
#define TTRACE6_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5, P6)  \
    TTRACE_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5, P6)
#endif  // TTRACE6_BLOCK

#ifndef TTRACE7_BLOCK
#define TTRACE7_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7)  \
    TTRACE_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7)
#endif  // TTRACE7_BLOCK

#ifndef TTRACE8_BLOCK
#define TTRACE8_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7, P8)  \
    TTRACE_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7, P8)
#endif  // TTRACE8_BLOCK
#endif  // !__cplusplus

#endif  // _TDBG_LEVEL > 1

// ---- NOP macros -------------------------

// include empty implementation to define the rest of macros when necessary
#include "TdbgNop.h"

#endif // def DOXYGEN_PASS

#endif  // __TDBG_H__
