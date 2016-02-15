/**
 * @file TdbgNop.h
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

#ifndef __TDBGNOP_H__
#define __TDBGNOP_H__

/**
 * Simple assertion in compile time. It is mainly for use in headers for static
 * checks or in the sources out of functions scope. It can be used inside function,
 * too, but only where variables can be defines. For checks inside code use runtime
 * TASSERT(), instead.
 *
 * It is available for both debug and release build because there is no reason
 * to not check in compile time in both cases.
 */
#define TC_ASSERT(Expression)   \
    extern char _TDBG_C_ASSERT[(Expression) ? 1 : -1]

#if !defined(_TDBG_NOP)
// define nop unconditionally so all implementations can use it regardless
// of current _TDBG_LEVEL
#define _TDBG_NOP   ((void)0)
#endif // !defined(_TDBG_NOP)

#if !defined(_TDBG_LEVEL)
#define _TDBG_LEVEL     0
#endif  // _TDBG_LEVEL

#if _TDBG_LEVEL <= 1
// handle both level 1 and level 2 macros
#if _TDBG_LEVEL == 0
// declare level 1 macros to be NOPs at level 0
#define TDEBUG(x)
#define TASSERT(Expression)                         _TDBG_NOP
#define TASSERTF()                                  _TDBG_NOP
#define TASSERT_FORALL(ForCondition, Expression)    _TDBG_NOP
#define TASSERT_EXISTS(ForCondition, Expression)    _TDBG_NOP
// handle special embedded assertions
#define TASSERT_INTS_DISABLED()                     _TDBG_NOP
#define TASSERT_INTS_ENABLED()                      _TDBG_NOP

/**
 * @hideinitializer
 * Simple assertion, condition is always evaluated (in any build type).\ See
 * @ref assert_doc for further details
 */
#define TVERIFY(Expression)                         (Expression)

#endif  // _TDBG_LEVEL == 0

// declare level 2 macros to be NOPs at levels 0 and 1

// first, undefine all macros which can be defined
#undef TDBG_DECLARE_AREA
#undef TDBG_DEFINE_AREA
#undef TDBG_INIT
#undef TDBG_INIT_AREA
#undef TTRACE_SYNC
#undef TTRACE_IF
#undef TTRACE
#undef TTRACE_ENTER
#undef TTRACE_LEAVE
#undef TDUMP
#undef TDUMP_EX
#undef TTRACE0
#undef TTRACE1
#undef TTRACE2
#undef TTRACE3
#undef TTRACE4
#undef TTRACE5
#undef TTRACE6
#undef TTRACE7
#undef TTRACE8

#define TDBG_DECLARE_AREA(Area)
#define TDBG_DEFINE_AREA(Area)

#define TDBG_INIT(Param1, Param2)                   _TDBG_NOP
#define TDBG_INIT_AREA(Area, Param2)                _TDBG_NOP
#define TTRACE_SYNC(Area)                           _TDBG_NOP

#define TTRACE_IF(Area, Level, Action)                              _TDBG_NOP

#define TTRACE(Area, Level, Params)                                 _TDBG_NOP
#define TTRACE_ENTER(Area, Level, Params)                           _TDBG_NOP
#define TTRACE_LEAVE(Params)                                        _TDBG_NOP

#define TDUMP(Area, Level, Text, Data, Length)                      _TDBG_NOP
#define TDUMP_EX(Area, Level, Text, Data, Length)                   _TDBG_NOP

#define TTRACE0(Area, Level, Text)                                  _TDBG_NOP
#define TTRACE1(Area, Level, Text, P1)                              _TDBG_NOP
#define TTRACE2(Area, Level, Text, P1, P2)                          _TDBG_NOP
#define TTRACE3(Area, Level, Text, P1, P2, P3)                      _TDBG_NOP
#define TTRACE4(Area, Level, Text, P1, P2, P3, P4)                  _TDBG_NOP
#define TTRACE5(Area, Level, Text, P1, P2, P3, P4, P5)              _TDBG_NOP
#define TTRACE6(Area, Level, Text, P1, P2, P3, P4, P5, P6)          _TDBG_NOP
#define TTRACE7(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7)      _TDBG_NOP
#define TTRACE8(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7, P8)  _TDBG_NOP

#if defined(__cplusplus)
// C++ only macros
#undef TTRACE_BLOCK
#undef TTRACE0_BLOCK
#undef TTRACE1_BLOCK
#undef TTRACE2_BLOCK
#undef TTRACE3_BLOCK
#undef TTRACE4_BLOCK
#undef TTRACE5_BLOCK
#undef TTRACE6_BLOCK
#undef TTRACE7_BLOCK
#undef TTRACE8_BLOCK

#define TTRACE_BLOCK(Area, Level, Params)                                   _TDBG_NOP
#define TTRACE0_BLOCK(Area, Level, Text)                                    _TDBG_NOP
#define TTRACE1_BLOCK(Area, Level, Text, P1)                                _TDBG_NOP
#define TTRACE2_BLOCK(Area, Level, Text, P1, P2)                            _TDBG_NOP
#define TTRACE3_BLOCK(Area, Level, Text, P1, P2, P3)                        _TDBG_NOP
#define TTRACE4_BLOCK(Area, Level, Text, P1, P2, P3, P4)                    _TDBG_NOP
#define TTRACE5_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5)                _TDBG_NOP
#define TTRACE6_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5, P6)            _TDBG_NOP
#define TTRACE7_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7)        _TDBG_NOP
#define TTRACE8_BLOCK(Area, Level, Text, P1, P2, P3, P4, P5, P6, P7, P8)    _TDBG_NOP

#if !defined(_TDBG_NO_ERROR_H)
#undef TTRACE_ERROR
#undef TTRACE_ERROR_ALL
// Err parameter should be referenced to avoid compiler warning in common code
#define TTRACE_ERROR(Area, Level, Message, Err)                     ((void)Err)
#define TTRACE_ERROR_ALL(Area, Level, Message, Err)                 ((void)Err)
#endif  // !_TDBG_NO_ERROR_H

#endif  // !__cplusplus

#endif  // _TDBG_LEVEL <= 1

#endif  // __TDBGNOP_H__
