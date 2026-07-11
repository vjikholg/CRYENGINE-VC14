/*****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2006 CRI-MW
 *
 * Library  : CRI Middleware Library
 * Module   : CRI Common Header / Post-Process
 * File     : cri_xpt_post.h
 * Date     : 2006-09-09
 * Version  : 1.01
 *
 *****************************************************************************/
#ifndef CRI_XPT_POST_H
#define CRI_XPT_POST_H
#ifndef SEGA_XPT_H


/*****************************************************************************
 *      マクロ定義
 *      Macro definition 
 *****************************************************************************/

/* コンパイル時の静的アサート
 *   C++ BoostのSTATIC_ASSERTと同等の機能をC言語で実装する。
 *   条件が真であることを検証する。偽の場合はコンパイルエラーを起こして、コン
 *   パイルを停止させる。
 * Static Asart when compiling
 *   This function is equal to STATIC_ASSERT of C++ Boost, and is imprementated
 *   by C language. If the condition is false, a compiler error is caused and
 *   it stop compiling.
 */
#define XPT_STATIC_ASSERT(cond) extern int xpt_static_assert_array[(cond)?1:-1]


/*****************************************************************************
 *      コンパイルオプションの検証
 *      Compiler option test
 *****************************************************************************/

/* enum型の検証
 * 　enum型がint型に整合することを検証する。
 * 　適切なエラーメッセージを表示したいので、可能ならコンパイルオプションによる
 * 　チェックを行う。不可能な場合、汎用的な静的アサートによるチェックが働く。
 * Enum type test
 *   It is verified that the enum type adjusts to the int type. 
 *   To display an appropriate error message, it checks the compiler option 
 *   if possible. General static asart test works if it is impossible. 
 */
#if defined(__MWERKS__) /* CodeWarrior */
    //#if !__option(enumsalwaysint)
    //    #error ERROR cri_xpt_post.h : Compiler option 'Enums Always Int' should be ON.
    //#endif
#elif defined(__GNUC__)
    //#if Compiler option -fshort-enums is used
    //    #error ERROR cri_xpt_post.h : Compiler option '-fshort-enums' should not be used.
    //#endif
#endif
//typedef enum {XPT_TEST_ENUM_0, XPT_TEST_ENUM_1, XPT_TEST_ENUM_2} XptTestEnum;
//XPT_STATIC_ASSERT(sizeof(XptTestEnum) == sizeof(Uint32));	/* enum should be 4byte. */


#endif
#endif	/* CRI_XPT_POST_H */

/* end of file */
