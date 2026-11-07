////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2006.
// -------------------------------------------------------------------------
//  File name:   BMPHelper.cpp
//  Version:     v1.00
//  Created:     28/11/2006 by AlexL
//  Compilers:   Visual Studio.NET
//  Description: BMPHelper
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <platform.h>
#include "BMPHelper.h"
#include "RichSaveGameTypes.h"

#if defined(NEED_ENDIAN_SWAP)
// big endian
static const unsigned short BF_TYPE = 0x424D;            /* "BM" */
#else
// little endian
static const unsigned short BF_TYPE = 0x4D42;            /* "MB" */
#endif

#define COMPRESSION_BI_RGB 0

namespace BMPHelper
{
	size_t CalcBMPSize(int width, int height, int depth)
	{
		if (width <= 0 || height <= 0 || (depth != 3 && depth != 4))
			return 0;

		const uint64 headerBytes = sizeof(CryBitmapFileHeader) + sizeof(CryBitmapInfoHeader);
		const uint64 rowBytes = uint64(width) * depth;
		const uint64 stride = (rowBytes + 3) & ~uint64(3);
		if (stride > (uint64(0xffffffffu) - headerBytes) / height)
			return 0;
		return static_cast<size_t>(headerBytes + stride * height);
	}

	// pByteData uses BGR or BGRA byte order.
	bool InternalSaveBMP(const char* filename, FILE* pFile, uint8* pByteData, int width, int height, int depth, bool inverseY)
	{
		const size_t totalSize = CalcBMPSize(width, height, depth);
		if (!totalSize || !pByteData)
			return false;

		CryBitmapFileHeader bitmapfileheader = {};
		CryBitmapInfoHeader bitmapinfoheader = {};
		const size_t headerBytes = sizeof(bitmapfileheader) + sizeof(bitmapinfoheader);
		const size_t stride = (totalSize - headerBytes) / height;
		const size_t rowBytes = size_t(width) * depth;
		const size_t padBytes = stride - rowBytes;
		const uint8 padding[3] = {};

		bitmapfileheader.bfType = BF_TYPE;
		bitmapfileheader.bfSize = static_cast<uint32>(totalSize);
		bitmapfileheader.bfOffBits = static_cast<uint32>(headerBytes);
		bitmapinfoheader.biSize = sizeof(CryBitmapInfoHeader);
		bitmapinfoheader.biWidth = width;
		bitmapinfoheader.biHeight = height;
		bitmapinfoheader.biPlanes = 1;
		bitmapinfoheader.biBitCount = depth * 8;
		bitmapinfoheader.biCompression = COMPRESSION_BI_RGB;

		ICryPak* pCryPak = gEnv->pCryPak;
		const bool ownsFile = pFile == NULL;
		if (ownsFile)
			pFile = pCryPak->FOpen(filename, "wb");
		if (!pFile)
			return false;

		bool written = pCryPak->FWrite(&bitmapfileheader, sizeof(bitmapfileheader), 1, pFile) == 1 &&
			pCryPak->FWrite(&bitmapinfoheader, sizeof(bitmapinfoheader), 1, pFile) == 1;
		for (int y = 0; written && y < height; ++y)
		{
			const int sourceRow = inverseY ? height - 1 - y : y;
			written = pCryPak->FWrite(pByteData + size_t(sourceRow) * rowBytes, 1, rowBytes, pFile) == rowBytes &&
				(!padBytes || pCryPak->FWrite(padding, 1, padBytes, pFile) == padBytes);
		}

		if (ownsFile && pCryPak->FClose(pFile) != 0)
			written = false;
		return written;
	}

	// fills data with BGR or BGRA 
	// always top->bottom
	bool InternalLoadBMP(const char* filename, FILE* pFile, uint8* pByteData, int& width, int& height, int& depth, bool bForceInverseY)
	{
		// todo: cleanup for pFile
		struct Cleanup
		{
			Cleanup() : m_pFile(0), m_bCloseFile(false), m_bRestoreCur(false), m_offset(0) {}
			~Cleanup()
			{
				if (m_pFile)
				{
					if (m_bRestoreCur)
						gEnv->pCryPak->FSeek(m_pFile, m_offset, SEEK_SET);
					if (m_bCloseFile)
						gEnv->pCryPak->FClose(m_pFile);
				}
			}
			void SetFile(FILE* pFile, bool bClose, bool bRestoreCur)
			{
				m_pFile = pFile;
				m_bCloseFile = bClose;
				m_bRestoreCur = bRestoreCur;
				if (m_pFile && m_bRestoreCur)
					m_offset = gEnv->pCryPak->FTell(m_pFile);
			}
		protected:
			FILE* m_pFile;
			bool  m_bCloseFile;
			bool  m_bRestoreCur;
			long  m_offset;
		};

		Cleanup cleanup; // potentially close the file on return (if pFile was given, do nothing)
		CryBitmapFileHeader bitmapfileheader;
		CryBitmapInfoHeader bitmapinfoheader;

		memset(&bitmapfileheader, 0, sizeof(CryBitmapFileHeader));
		memset(&bitmapinfoheader, 0, sizeof(CryBitmapInfoHeader));

		ICryPak* pCryPak = gEnv->pCryPak;
		const bool bFileGiven = pFile != 0;
		if (bFileGiven == false)
			pFile = pCryPak->FOpen(filename, "rb");
		if (pFile == 0)
			return false;

		cleanup.SetFile(pFile, !bFileGiven, bFileGiven);

		size_t els = pCryPak->FRead(&bitmapfileheader, 1, pFile);
		if (els != 1)
			return false;

		els = pCryPak->FRead(&bitmapinfoheader, 1, pFile);
		if (els != 1)
			return false;

		if (bitmapfileheader.bfType != BF_TYPE)
			return false;

		if (bitmapinfoheader.biCompression != COMPRESSION_BI_RGB)
			return false;

		if (bitmapinfoheader.biBitCount != 24 && bitmapinfoheader.biBitCount != 32)
			return false;

		if (bitmapinfoheader.biPlanes != 1)
			return false;

		int imgWidth = bitmapinfoheader.biWidth;
		int imgHeight = bitmapinfoheader.biHeight;
		int imgDepth = bitmapinfoheader.biBitCount == 24 ? 3 : 4;

		int imgBytesPerLine = imgWidth * imgDepth;
		int fileBytesPerLine = imgBytesPerLine;
		if (fileBytesPerLine & 0x0003)
		{
			fileBytesPerLine |= 0x0003;
			++fileBytesPerLine;
		}
		int padBytes = fileBytesPerLine - imgBytesPerLine; // how many pad-bytes

		size_t prologSize = sizeof(CryBitmapFileHeader) + sizeof(CryBitmapInfoHeader);
		size_t bufSize = bitmapfileheader.bfSize-prologSize;

		// some BMPs from Adobe PhotoShop seem 2 bytes longer 
		if (bufSize != fileBytesPerLine * imgHeight && bufSize != (fileBytesPerLine * imgHeight)+2)
			return false;

		bool bFlipY = true;
		width = imgWidth;
		height = imgHeight;
		if (height < 0)   // height > 0: bottom->top, height < 0: top->bottom, no flip necessary
		{
			height = -height;
			bFlipY = false;
		}
		depth = imgDepth;

		if (pByteData == 0) // only requested dimensions
			return true;

		uint8* pTempImage = new uint8[bufSize];
		if (pTempImage == 0)
			return false;

		els = pCryPak->FReadRaw(pTempImage, 1, bufSize, pFile);
		if (els != bufSize)
		{
			delete[] pTempImage;
			return false;
		}

		if (bForceInverseY)
			bFlipY = !bFlipY;

		// read all rows
		uint8* pSrcLine = pTempImage;
		for (int y=0; y<imgHeight; ++y)
		{
			int dstY=y;
			if(bFlipY)
				dstY=imgHeight-y-1;

			uint8* pDstLine = pByteData + dstY*imgBytesPerLine;
			memcpy(pDstLine, pSrcLine, imgBytesPerLine);
			pSrcLine+=fileBytesPerLine;
		}

		delete[] pTempImage;

		// Success
		return true;
	}

	bool LoadBMP(const char* filename, uint8* pByteData, int& width, int& height, int& depth, bool bForceInverseY)
	{
		return InternalLoadBMP(filename, 0, pByteData, width, height, depth, bForceInverseY);
	}

	bool LoadBMP(FILE* pFile, uint8* pByteData, int& width, int& height, int& depth, bool bForceInverseY)
	{
		return InternalLoadBMP("", pFile, pByteData, width, height, depth, bForceInverseY);
	}


	bool SaveBMP(const char* filename, uint8* pByteData, int width, int height, int depth, bool flipY)
	{
		return InternalSaveBMP(filename, 0, pByteData, width, height, depth, flipY);
	}

	bool SaveBMP(FILE* pFile, uint8* pByteData, int width, int height, int depth, bool flipY)
	{
		return InternalSaveBMP("", pFile, pByteData, width, height, depth, flipY);
	}
};
