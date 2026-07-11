/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 1998-2005 CRI-MW
 *
 * Library  : CRI Middleware Library
 * Module   : CRI Common Header for Windows, Xbox and Xbox360
 * File     : cri_xpts_win.h
 * Date     : 2005-08-09
 * Version  : 1.43
 *
 ****************************************************************************/

#if !defined(CRI_XPTS_H)
#define CRI_XPTS_H

#if !defined(SEGA_XPT_H)

/*****************************************************************************
 * 定数マクロ
 *****************************************************************************/

/* NULL ポインタ型 */
#if !defined(NULL)
#ifdef __cplusplus
#define NULL	(0)
#else
#define NULL	((void *)0)
#endif
#endif

/* 論理定数１（偽、真） */
#if !defined(FALSE)
#define FALSE	(0)
#endif
#if !defined(TRUE)
#define TRUE	(1)
#endif

/* 論理定数２（スイッチ） */
#if !defined(OFF)
#define OFF		(0)
#endif
#if !defined(ON)
#define ON		(1)
#endif

/* 結果判定の定数（成功、失敗） */
#if !defined(OK)
#define OK		(0)			/* 成功 */
#endif
#if !defined(NG)
#define NG		(-1)		/* 失敗 */
#endif

/*****************************************************************************
 * 基本データ型宣言
 *****************************************************************************/

#if !defined(_TYPEDEF_Uint8)
#define _TYPEDEF_Uint8
typedef unsigned char			Uint8;		/* 符号なし１バイト整数 */
#endif

#if !defined(_TYPEDEF_Sint8)
#define _TYPEDEF_Sint8
typedef signed char				Sint8;		/* 符号つき１バイト整数 */
#endif

#if !defined(_TYPEDEF_Uint16)
#define _TYPEDEF_Uint16
typedef unsigned short			Uint16;		/* 符号なし２バイト整数 */
#endif

#if !defined(_TYPEDEF_Sint16)
#define _TYPEDEF_Sint16
typedef signed short			Sint16;		/* 符号つき２バイト整数 */
#endif

#if !defined(_TYPEDEF_Uint32)
#define _TYPEDEF_Uint32
typedef unsigned long			Uint32;		/* 符号なし４バイト整数 */
#endif

#if !defined(_TYPEDEF_Sint32)
#define _TYPEDEF_Sint32
typedef signed long				Sint32;		/* 符号つき４バイト整数 */
#endif

#if !defined(_TYPEDEF_Uint64)
#define _TYPEDEF_Uint64
typedef unsigned __int64		Uint64;		/* 符号なし８バイト整数 */
#endif

#if !defined(_TYPEDEF_Sint64)
#define _TYPEDEF_Sint64
typedef signed __int64			Sint64;		/* 符号つき８バイト整数 */
#endif

#if !defined(_TYPEDEF_Uint128)
#define _TYPEDEF_Uint128
typedef struct {							/* 符号なし16バイト整数 */
	Uint64	h;								/* 上位64ビット */
	Uint64	l;								/* 下位64ビット */
} Uint128;
#endif

#if !defined(_TYPEDEF_Sint128)
#define _TYPEDEF_Sint128
typedef struct {							/* 符号つき16バイト整数 */
	Sint64	h;								/* 上位64ビット */
	Uint64	l;								/* 下位64ビット */
} Sint128;
#endif

#if !defined(_TYPEDEF_Float16)
#define _TYPEDEF_Float16
typedef signed short			Float16;	/* ２バイト実数 */
#endif

#if !defined(_TYPEDEF_Float32)
#define _TYPEDEF_Float32
typedef float					Float32;	/* ４バイト実数 */
#endif

#if !defined(_TYPEDEF_Float64)
#define _TYPEDEF_Float64
typedef double					Float64;	/* ８バイト実数 */
#endif

#if !defined(_TYPEDEF_Fixed32)
#define _TYPEDEF_Fixed32
typedef signed long				Fixed32;	/* 固定小数点32ビット */
#endif

#if !defined(_TYPEDEF_Bool)
#define _TYPEDEF_Bool
typedef Sint32					Bool;		/* 論理型（論理定数を値にとる） */
#endif

#if !defined(_TYPEDEF_Char8)
#define _TYPEDEF_Char8
typedef char					Char8;		/* 文字型 */
#endif

/*****************************************************************************
 * ポインタ変数
 *****************************************************************************/

#if defined(_WIN64)

#if !defined(_TYPEDEF_SintPtr)
#define _TYPEDEF_SintPtr
typedef signed __int64			SintPtr;
#endif

#if !defined(_TYPEDEF_UintPtr)
#define _TYPEDEF_UintPtr
typedef unsigned __int64		UintPtr;
#endif

#else

#if defined(_MSC_VER) && (_MSC_VER >= 1300)

#if !defined(_TYPEDEF_SintPtr)
#define _TYPEDEF_SintPtr
typedef __w64 signed int		SintPtr;
#endif

#if !defined(_TYPEDEF_UintPtr)
#define _TYPEDEF_UintPtr
typedef __w64 unsigned int		UintPtr;
#endif

#else

#if !defined(_TYPEDEF_SintPtr)
#define _TYPEDEF_SintPtr
typedef Sint32					SintPtr;
#endif

#if !defined(_TYPEDEF_UintPtr)
#define _TYPEDEF_UintPtr
typedef Uint32					UintPtr;
#endif

#endif

#endif	/* end of _WIN64 */

/*****************************************************************************
 * 呼び出し規約
 *****************************************************************************/

#if !defined(CRIAPI)
#define CRIAPI	__cdecl
#endif

#endif

#endif	/* CRI_XPTS_H */

/* end of file */
