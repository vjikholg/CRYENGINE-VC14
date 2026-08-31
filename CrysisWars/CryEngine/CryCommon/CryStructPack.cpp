//////////////////////////////////////////////////////////////////////////
//
#include <CryStructPack.h>
#include <CryCheckedArithmetic.h>

#if NEED_STRUCT_PACK
uint32 StructSize(const CTypeInfo& typeInfo, uint32 limit)
{
	size_t size = 0;
	const size_t sizeLimit = limit;

	for AllSubVars (pVar, typeInfo)
	{
		if (pVar->Offset >= sizeLimit)
			break;

		size_t variableSize = 0;
		if (pVar->Type.IsType<void*>())
		{
			if (!CryTryMultiplySize(4, pVar->ArrayDim, variableSize))
				return limit;
		}
		else if (pVar->Type.HasSubVars())
		{
			const uint32 typeSize = StructSize(pVar->Type);
			if (!CryTryMultiplySize(pVar->ArrayDim, typeSize, variableSize))
				return limit;
		}
		else
		{
			variableSize = pVar->GetSize();
		}

		if (!CryIsRangeWithinSize(sizeLimit, pVar->Offset, variableSize) ||
			!CryIsRangeWithinSize(sizeLimit, size, variableSize))
		{
			return limit;
		}
		size += variableSize;
	}
	return static_cast<uint32>(size);
}

uint32 StructUnpack(const CTypeInfo& typeInfo, uint8* ptr, const uint8* buffer,
	uint32 limit, bool expandPointers)
{
	size_t offset = 0;
	const size_t sizeLimit = limit;
	for AllSubVars (pVar, typeInfo)
	{
		if (pVar->Offset >= sizeLimit)
			break;
		if (pVar->Type.IsType<void*>())
		{
			size_t packedPointerBytes = 0;
			if (!CryTryMultiplySize(4, pVar->ArrayDim, packedPointerBytes) ||
				!CryIsRangeWithinSize(sizeLimit, offset, packedPointerBytes) ||
				!CryIsRangeWithinSize(sizeLimit, pVar->Offset, pVar->GetSize()))
			{
				return static_cast<uint32>(offset);
			}
			if (expandPointers)
			{
				for (size_t j = 0; j < pVar->ArrayDim; ++j)
				{
					INT_PTR v = buffer[offset] | (buffer[offset + 1] << 8)
						| (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
					memcpy(
						(uint8*)ptr + pVar->Offset + j * sizeof(void*),
						&v,
						sizeof(void*));
					offset += 4;
				}
			}
			else
			{
				memset((uint8*)ptr + pVar->Offset, 0, pVar->GetSize());
				offset += packedPointerBytes;
			}
		}
		else if (pVar->Type.HasSubVars())
		{
			size_t varOffset = pVar->Offset;
			for (size_t j = 0; j < pVar->ArrayDim; ++j)
			{
				if (varOffset >= sizeLimit)
					break;
				const uint32 nestedLimit = static_cast<uint32>(sizeLimit - varOffset);
				offset += StructUnpack(pVar->Type, ptr + varOffset, buffer + offset,
					nestedLimit, expandPointers);
				if (!CryTryAddSize(varOffset, pVar->Type.Size, varOffset) || offset > sizeLimit)
					return static_cast<uint32>((std::min)(offset, sizeLimit));
			}
		}
		else
		{
			const size_t varSize = pVar->GetSize();
			if (!CryIsRangeWithinSize(sizeLimit, pVar->Offset, varSize) ||
				!CryIsRangeWithinSize(sizeLimit, offset, varSize))
			{
				return static_cast<uint32>(offset);
			}
			memcpy((uint8*)ptr + pVar->Offset, buffer + offset, varSize);
#ifdef NEED_ENDIAN_SWAP
			SwapEndian((uint8*)ptr + pVar->Offset,
				pVar->ArrayDim, pVar->Type, pVar->Type.Size);
#endif
			offset += varSize;
		}
	}
	return static_cast<uint32>(offset);
}

uint32 StructPack(const CTypeInfo& typeInfo, const uint8* ptr, uint8* buffer,
	uint32 limit, bool packPointers)
{
	size_t offset = 0;
	const size_t sizeLimit = limit;
	for AllSubVars (pVar, typeInfo)
	{
		if (pVar->Offset >= sizeLimit)
			break;
		if (pVar->Type.IsType<void*>())
		{
			size_t packedPointerBytes = 0;
			if (!CryTryMultiplySize(4, pVar->ArrayDim, packedPointerBytes) ||
				!CryIsRangeWithinSize(sizeLimit, offset, packedPointerBytes) ||
				!CryIsRangeWithinSize(sizeLimit, pVar->Offset, pVar->GetSize()))
			{
				return static_cast<uint32>(offset);
			}
			if (packPointers)
			{
				// This legacy format stores pointer fields as 32-bit values.
				for (size_t i = 0; i < pVar->ArrayDim; ++i)
				{
					INT_PTR v = 0;
					memcpy(
						&v,
						(const uint8*)ptr + pVar->Offset + i * sizeof(void*),
						sizeof(void*));
					buffer[offset] = (uint8)(v & 0xff);
					buffer[offset + 1] = (uint8)((v >> 8) & 0xff);
					buffer[offset + 2] = (uint8)((v >> 16) & 0xff);
					buffer[offset + 3] = (uint8)((v >> 24) & 0xff);
					offset += 4;
				}
			}
			else
			{
				memset(buffer + offset, 0, packedPointerBytes);
				offset += packedPointerBytes;
			}
		}
		else if (pVar->Type.HasSubVars())
		{
			size_t varOffset = pVar->Offset;
			for (size_t j = 0; j < pVar->ArrayDim; ++j)
			{
				if (varOffset >= sizeLimit)
					break;
				const uint32 nestedLimit = static_cast<uint32>(sizeLimit - varOffset);
				offset += StructPack(pVar->Type, ptr + varOffset, buffer + offset,
					nestedLimit, packPointers);
				if (!CryTryAddSize(varOffset, pVar->Type.Size, varOffset) || offset > sizeLimit)
					return static_cast<uint32>((std::min)(offset, sizeLimit));
			}
		}
		else
		{
			const size_t varSize = pVar->GetSize();
			if (!CryIsRangeWithinSize(sizeLimit, pVar->Offset, varSize) ||
				!CryIsRangeWithinSize(sizeLimit, offset, varSize))
			{
				return static_cast<uint32>(offset);
			}
			memcpy(buffer + offset, (const uint8*)ptr + pVar->Offset, varSize);
#ifdef NEED_ENDIAN_SWAP
			SwapEndian(buffer + offset,
				pVar->ArrayDim, pVar->Type, pVar->Type.Size);
#endif
			offset += varSize;
		}
	}
	return static_cast<uint32>(offset);
}
#endif
