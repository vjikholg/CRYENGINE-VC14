/***************************************************************************
 *
 *  CRI Middleware
 *
 *  Copyright (c) 2001-2007 CRI-MW
 *
 *  Library : ADX Library (CRI ADX for PC)
 *  Module  : Functions for PC
 *  File    : adx_pc.h
 *
 ***************************************************************************/
/* Prevention of redefinition */
#ifndef _ADXPC_H_INCLUDED
#define _ADXPC_H_INCLUDED

/***************************************************************************
 *       バージョン情報
 *       Version
 ***************************************************************************/
#define ADXPC_VER					"1.46"

/***************************************************************************
 *      インクルードファイル
 *      Include file
 ***************************************************************************/

/* Disable VC++ warning 
 *   In some cases, including windows.h, dsound.h causes the following warning:
 *       rpcasync.h(45) : warning C4115: '_RPC_ASYNC_STATE' : ...
 */
#if defined(_MSC_VER) && ((_MSC_VER >= 1200) && (_MSC_VER < 1300))
	#pragma warning(disable : 4115)
#endif

#include <windows.h>
#include <dsound.h>
#include "cri_xpt.h"
#include "cri_adxt.h"

/***************************************************************************
 *      定数マクロ
 *      MACRO CONSTANT
 ***************************************************************************/
/* デフォルトプライオリティ */
/* Default thread priority */
#define ADXM_PRIO_VSYNC				(THREAD_PRIORITY_HIGHEST)		/* Vsyncスレッド */
#define ADXM_PRIO_FS				(THREAD_PRIORITY_ABOVE_NORMAL)	/* ファイル処理スレッド */
#define ADXM_PRIO_MWIDLE			(THREAD_PRIORITY_IDLE)			/* アイドルスレッド */

/* スタックサイズ */
/* Thread stack size */
#define	ADXM_STACK_SIZE_VSYNC		(1024 * 1024)
#define	ADXM_STACK_SIZE_FS			(1024 * 1024)
#define	ADXM_STACK_SIZE_MWIDLE		(1024 * 1024)

/* ディレクトリ探索モード		*/
/* Mode of directory look-up	*/
#define ADXFIC_MODE_ALLDIR			(-1)
#define ADXFIC_MODE_CURRENT			(0)

/* ADXFICハンドルステータス	 */
/* Status of ADXFIC handle */
#define ADXFIC_STAT_STOP			(0)
#define ADXFIC_STAT_CACHING			(1)
#define ADXFIC_STAT_CACHEEND		(2)
#define ADXFIC_STAT_ERROR			(3)

/* デフォルト同期周波数x100					*/
/* Default of virtual vsync frequency x100	*/
#define ADXPC_DEF_VHZ100			(6000)

/***************************************************************************
 *      処理マクロ
 *      MACRO FUNCTIONS
 ***************************************************************************/
/* ワーク領域最大サイズ計算マクロ */
/* Macro that calculates maximum work area size */
#define ADXFIC_CALC_WORK_SIZE(nfiles, max_flen) \
	((nfiles + 1) * (16 + maxflen) + 32)

/***************************************************************************
 *      データ型宣言
 *      Data type declaration
 ***************************************************************************/
/*--------------------------------------------------------------------------
 * フレームワーク種別（PC）
 * Type of framework
 *-------------------------------------------------------------------------*/
typedef enum {
	ADXM_FRAMEWORK_DEFAULT			= 0,
	ADXM_FRAMEWORK_PC_SINGLE_THREAD	= 1,
	ADXM_FRAMEWORK_PC_MULTI_THREAD	= 2,
} AdxmFramework;

/*--------------------------------------------------------------------------
 * スレッドセットアップパラメータ（PC）
 * Thread setup parameter for PC
 *-------------------------------------------------------------------------*/
typedef struct {
	/* プライオリティ */
	Sint32			priority_vsync;		/* Vsyncスレッド */
	Sint32 			priority_fs;		/* ファイル処理スレッド */
	Sint32 			priority_mwidle;	/* アイドルスレッド */
} AdxmThreadSprm;

/* スレッドのセットアップを行う際の注意点									*/
/* (a) セットアップパラメータは、フレームワークとして						*/
/* 　　ADXM_FRAMEWORK_Pc_MULTI_THREADを選んだ場合のみ有効です。				*/
/* (b) セットアップパラメータは、ADXM_SetupFramework関数に引数prmと			*/
/* 　　して渡します。NULLを指定するとデフォルト設定が使用されます。			*/
/* Notes when setup of thread												*/
/* (a) If you need to apply custom framework setting, set the setup			*/
/*     parameter to ADXM_SetupFramework function as argument "prm".			*/
/*     When NULL is passed, the default setting is used.	 				*/
/* (b) The setup parameter is effective only when you chose 				*/
/*     ADXM_FRAMEWORK_PC_MULTI_THREAD as framework.							*/

/*--------------------------------------------------------------------------
 * ファイルシステムのセットアップパラメータ構造体
 * Parameter structure for file system setup function
 *-------------------------------------------------------------------------*/
typedef struct {
	Char8	*rtdir;				/* ルートディレクトリ	*/
								/* Root directory		*/
} AdxpcSprmFs;

/*--------------------------------------------------------------------------
 * ファイル情報キャッシュハンドル（ADXFICハンドル）
 * File information cache handle (ADXFIC handle)
 *-------------------------------------------------------------------------*/
typedef void *ADXFIC;

/*--------------------------------------------------------------------------
 * バッファ作成時の配置位置
 * ハードウェアアクセラレーションの有無に関係
 * Locate mode of the DirectSoundBuffer
 * Influences the use of hardware acceleration
 *-------------------------------------------------------------------------*/
typedef enum {
	ADXPC_LOCMODE_SOFTWAREONLY = 0x00,	
		/* Softwareバッファ／ミキシングのみ使用（デフォルト）	*/
		/* Only software buffer/mixing will be used(Default)	*/
	ADXPC_LOCMODE_HARDWAREONLY = 0x01,
		/* Hardwareバッファ／ミキシングのみ使用					*/
		/* Only hardware buffer/mixing will be used				*/
	ADXPC_LOCMODE_HARDWARE = 0x02,
		/* 可能ならHardwareを使用								*/
		/* Hardware buffer/mixing will be used if available		*/
	ADXPC_LOCMODE_DEFER = 0x03,
		/* バッファ確保を再生時まで遅延、可能ならHardwareを使用	*/
		/* The buffer can be assigned to a hardware or software	*/
		/* resource at play time								*/
} ADXPC_LOCMODE;

/*--------------------------------------------------------------------------
 * V同期システム用オブジェクトの種類
 * Kind of the object for Vsync system
 *-------------------------------------------------------------------------*/
typedef enum {
	ADXPC_VIDEO_NONE = 0x00,
	ADXPC_VIDEO_DDRAW7 = 0x01,		// DirectDraw7
	ADXPC_VIDEO_D3D8 = 0x02,		// Direct3D8
} ADXPC_VIDEO;

/***************************************************************************
 *      変数宣言
 *      Prototype Function 
 ***************************************************************************/

/***************************************************************************
 *      関数宣言
 *      Prototype Function 
 ***************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

/***
*			フレームワーク設定関数
*			Framework setting function
***/

/* CRIミドルウェアのフレームワークセットアップ */
/* Setup the framework */
Bool CRIAPI ADXM_SetupFramework(AdxmFramework framework, void *prm);

/* CRIミドルウェアのフレームワークの終了 */
/* Shutdown the framework */
Bool CRIAPI ADXM_ShutdownFramework(void);

/***
 *          スレッド関係
 *          Thread
 ***/
 
/* ADXサーバ関数(非スレッド用)							*/
/* ADX server function(for using ADX without thread)	*/
void CRIAPI ADXPC_ExecServer(void);

/* ADXサーバ関数(スレッド用)						*/
/* ADX server function(for using ADX with thread)	*/
Bool CRIAPI ADXPC_ExecServerEx(void *obj);

/***
 *          ファイル処理関係
 *          File proseccing
 ***/

/* ファイルシステムのセットアップ	*/
/* Set up the file system			*/
void CRIAPI ADXPC_SetupFileSystem(AdxpcSprmFs *sprmd);

/* ファイルシステムのシャットダウン	*/
/* Shut down the file system		*/
void CRIAPI ADXPC_ShutdownFileSystem(void);

/***
 *          サウンド関係
 *          Sound
 ***/

/* サウンドのセットアップ	*/
/* Set up the sound	system	*/
void CRIAPI ADXPC_SetupSound(LPDIRECTSOUND8 ds);

/* サウンドのシャットダウン		*/
/* Shut down the sound system	*/
void CRIAPI ADXPC_ShutdownSound(void);

/* DirectSoundBuffer8オブジェクトの取得	*/
/* Get the DirectSoundBuffer8 object	*/
LPDIRECTSOUNDBUFFER8 CRIAPI ADXPC_GetDirectSoundBuffer8(ADXT adxt);

/* 強制ステレオ出力スイッチの設定 */
/* Enable force stereo output mode */
void CRIAPI ADXPC_SetOutputStereo(Bool flag);

/* DirectSoundBufferの配置位置の設定			*/
/* Set the locate mode of the DirectSoundBuffer	*/
void CRIAPI ADXPC_SetDsbCapsLocMode(Sint32 loc_mode);

/* DirectSoundBufferの配置位置の取得			*/
/* Get the locate mode of the DirectSoundBuffer	*/
Sint32 CRIAPI ADXPC_GetDsbCapsLocMode(void);

/* グローバルフォーカスを持つDirectSoundBufferを作成するかどうかを設定	*/
/* Set the DirectSoundBuffer to have global focus						*/
void CRIAPI ADXPC_SetDsbCapsGlobalFocus(Bool flag);

/* グローバルフォーカスを持つDirectSoundBufferを作成するかどうかを取得	*/
/* Get whether the DirectSoundBuffer is having global focus				*/
Bool CRIAPI ADXPC_GetDsbCapsGlobalFocus(void);

/* 3Dコントロール機能を持つDirectSoundBufferを作成するかどうかを設定	*/
/* Set the DirectSoundBuffer to support 3D control						*/
void CRIAPI ADXPC_SetDsbCapsCtrl3D(Bool flag);

/* 3Dコントロール機能を持つDirectSoundBufferを作成するかどうかを取得	*/
/* Get whether the DirectSoundBuffer is supporting 3D control			*/
Bool CRIAPI ADXPC_GetDsbCapsCtrl3D(void);

/* エフェクト機能を持つDirectSoundBufferを作成するかどうかを設定		*/
/* Set the DirectSoundBuffer to support effects processing				*/
void CRIAPI ADXPC_SetDsbCapsCtrlFx(Bool flag);

/* エフェクト機能を持つDirectSoundBufferを作成するかどうかを取得		*/
/* Get whether the DirectSoundBuffer is supporting effects processing	*/
Bool CRIAPI ADXPC_GetDsbCapsCtrlFx(void);

/* エフェクトを設定する			*/
/* Enable effects on a buffer	*/
Bool CRIAPI ADXPC_SetDsFx(
	ADXT adxt, Sint32 count, LPDSEFFECTDESC effect, Uint32 *result);

/***
 *          V同期関係
 *          V-Sync system
 ***/

/* V同期システムをセットアップ	*/
/* Set up the V-Sync system		*/
Sint32 CRIAPI ADXPC_SetupVsync(void *video_obj, Sint32 mode);

/* V同期システムをシャットダウン	*/
/* Shut down the V-Sync system		*/
Bool CRIAPI ADXPC_ShutdownVsync(void);

/* V同期システムを再設定	*/
/* Reset the V-Sync system	*/
Sint32 CRIAPI ADXPC_ResetVsync(void);

/* ADXサーバー処理をVBlankからどれだけずらすか設定	*/
/* Set ADX server offset from VBlank				*/
void CRIAPI ADXPC_SetSvrOffset(Sint32 offset_msec);

/* 仮想V周波数の設定（1/100Hz単位、60Hzなら6000）				*/
/* Set virtual Vsync frequency	(in unit of 1/100Hz, 60Hz=6000)	*/
void CRIAPI ADXPC_SetVirtualVhz(Uint32 vhz100);

/* 仮想V周波数の取得			*/
/* Get virtual Vsync frequency	*/
Uint32 CRIAPI ADXPC_GetVirtualVhz(void);

/* Presentタイミングの通知 */
void CRIAPI ADXPC_NotifyPresentTiming(void);

/* Presentタイミングマージンの設定 [%] */
void CRIAPI ADXPC_SetPresentTimingMargin(Sint32 margin);

/***
 *          ファイル情報キャッシング用関数
 *          Functions for file information cache
 ***/

/* FICの初期化処理	*/
/* Initialize FIC	*/
void CRIAPI ADXFIC_Init(void);

/* 終了処理		*/
/* Finish FIC	*/
void CRIAPI ADXFIC_Finish(void);

/* 作業領域サイズの取得		*/
/* Get size of work area	*/
Sint32 CRIAPI ADXFIC_GetWorkSize(const Char8 *dname, Sint32 mode);

/* ADXFICハンドルの作成	*/
/* Create ADXFIC handle	*/
ADXFIC CRIAPI ADXFIC_Create(const Char8 *dname, Sint32 mode, void *work, Sint32 wksize);

/* ADXFICハンドルの消去		*/
/* Destroy ADXFIC handle	*/
void CRIAPI ADXFIC_Destroy(ADXFIC fic);

/* キャッシングされたファイル数の取得	*/
/* Get number of cached files			*/
Sint32 CRIAPI ADXFIC_GetNumFiles(ADXFIC fic);

/* 利用されている作業領域サイズの取得	*/
/* Get size of used work area			*/
Sint32 CRIAPI ADXFIC_GetUsedWkSize(ADXFIC fic);

/* ファイル探索を行ったディレクトリ名の取得 */
/* Get directory name for look-up			*/
Char8 * CRIAPI ADXFIC_GetDirName(ADXFIC fic);

/* キャッシングされたファイルのサイズを取得（セクタ単位）	*/
/* Get size of cached file (sector)							*/
Sint32 CRIAPI ADXFIC_GetFsizeSct(ADXFIC fic, const Char8 *fname);

/* キャッシングされたファイルのサイズを取得（バイト単位）	*/
/* Get size of cached file (Byte)							*/
Sint64 CRIAPI ADXFIC_GetFileSize(ADXFIC fic, const Char8 *fname);

/* ファイルがキャッシュされているかどうかのチェック */
/* Check whether the file is cached					*/
Bool CRIAPI ADXFIC_IsFileCached(const Char8 *fname);

/* 指定ファイルのキャッシュ無効化		*/
/* Disable caching of specified file	*/
void CRIAPI ADXFIC_DisableFile(ADXFIC fic, const Char8 *fname);

/* 指定ファイルのキャッシュ有効化		*/
/* Enable chaching of specified file	*/
void CRIAPI ADXFIC_EnableFile(ADXFIC fic, const Char8 *fname);

/* ステータスの取得	*/
/* Get state		*/
Sint32 CRIAPI ADXFIC_GetStat(ADXFIC fic);


/***
 *          その他
 *          Others
 ***/

/* バージョン文字列の取得	*/
/* Get version string		*/
Char8 * CRIAPI ADXPC_GetVersion(void);


/***
 *          デバッグ用
 *          For debugging
 ***/

#ifdef _DEBUG

/* ファイル名のデバッグ出力	*/
/* Print cached file names	*/
void CRIAPI ADXFIC_DebugPrintFlist(ADXFIC fic);

/* 全てのファイル名のデバッグ出力 */
/* Print all cached file names	*/
void CRIAPI ADXFIC_DebugPrintFlistAll(void);

#endif	// #ifdef _DEBUG


#ifdef __cplusplus
}
#endif	// __cplusplus

/***************************************************************************
 *      旧バージョンとの互換用
 *      For compatibility with old version
 ***************************************************************************/
#define ADXPC_SetupThrd(tprm)			ADXM_SetupThrd(tprm)
#define ADXPC_SetupThread(tprm)			ADXM_SetupThrd(tprm)
#define ADXPC_ShutdownThread()			ADXM_ShutdownThrd()
#define ADXPC_Setup3DSound()			ADXPC_SetDsbCapsCtrl3D(TRUE)
#define ADXPC_Shutdown3DSound()			ADXPC_SetDsbCapsCtrl3D(FALSE)
#define ADXPC_GetDsb(adxt, dsb)	\
	(*dsb) = (void *)ADXPC_GetDirectSoundBuffer8(adxt);
#define ADXPC_GetDirectSoundBuffer(adxt, dsb)	\
	(*dsb) = (void *)ADXPC_GetDirectSoundBuffer8(adxt);
#define ADXPC_SetSndBufGlobalFocus(mode)	\
	ADXPC_SetDsbCapsFocusMode(mode)
#define ADXPC_GetSndBufGlobalFocus()	ADXPC_GetDsbCapsFocusMode()
#define ADXPC_SetLocMode(mode)			ADXPC_SetDsbCapsLocMode(mode)
#define ADXPC_GetLocMode()				ADXPC_GetDsbCapsLocMode()
#define ADXPC_SPRM_DVD					AdxpcSprmFs
#define ADXPC_SPRM_WINFS				AdxpcSprmFs
#define ADXPC_SPRM_ANSI					AdxpcSprmFs
#define ADXPC_SetupWinFs(prm)			ADXPC_SetupFileSystem(prm)
#define ADXPC_SetupDvdFs(prm)			ADXPC_SetupFileSystem(prm)
#define ADXPC_SPRM_WINFS				AdxpcSprmFs
#define ADXPC_SPRM_DVD					AdxpcSprmFs
#define ADXPC_SPRM_ANSI					AdxpcSprmFs
#define ADXPC_SetupWinFs(prm)			ADXPC_SetupFileSystem(prm)
#define ADXPC_SetupDvdFs(prm)			ADXPC_SetupFileSystem(prm)
#define ADXPC_SetupAnsiFs(prm)			ADXPC_SetupFileSystem(prm)
#define ADXPC_ShutdownWinFs()			ADXPC_ShutdownFileSystem()
#define ADXPC_ShutdownDvdFs()			ADXPC_ShutdownFileSystem()
#define ADXPC_ShutdownAnsiFs()			ADXPC_ShutdownFileSystem()
#define ADXPC_SPEAKER_FRONT_LEFT		ADXT_SPEAKER_FRONT_LEFT
#define ADXPC_SPEAKER_FRONT_RIGHT		ADXT_SPEAKER_FRONT_RIGHT
#define ADXPC_SPEAKER_FRONT_CENTER		ADXT_SPEAKER_FRONT_CENTER
#define ADXPC_SPEAKER_LOW_FREQUENCY		ADXT_SPEAKER_LOW_FREQUENCY
#define ADXPC_SPEAKER_BACK_LEFT 		ADXT_SPEAKER_BACK_LEFT
#define ADXPC_SPEAKER_BACK_RIGHT		ADXT_SPEAKER_BACK_RIGHT
#define ADXPC_GetSendSpeakerLevel		ADXT_GetSendSpeakerLevel
#define ADXPC_SetSendSpeakerLevel		ADXT_SetSendSpeakerLevel

#endif	// #ifndef _ADXPC_H_INCLUDED

/* end of file */
