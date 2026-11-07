#ifndef __IANIMATIONSTATENODE_H__
#define __IANIMATIONSTATENODE_H__

#pragma once

#include "IAnimationGraph.h"
#include "IAnimationGraphSystem.h"

struct AG_FILE
{
	FILE* file = NULL;
	DynArray<uint8> m_buffer;
	size_t m_curpos = 0;
	DynArray<char> m_strBuffer;
	bool m_readFailed = false;

	bool SetFile(bool read, FILE* f);
};

// Helpers instantiated by nested serializers share the read failure on AG_FILE.
struct FileSerializationHelper
{
	bool reading = false;
	bool swappingEndian = false;
	AG_FILE* ag_file = NULL;
	FILE* file = NULL;
	ICryPak* pak = NULL;

	FileSerializationHelper() = default;

	FileSerializationHelper(bool read, AG_FILE* f)
	{
		Init(read, f);
	}

	void Init(bool read, AG_FILE* f);

	bool Good() const { return ag_file && !ag_file->m_readFailed; }

	bool Fail()
	{
		if (ag_file)
			ag_file->m_readFailed = true;
		return false;
	}

	bool CheckCount(int count, size_t elementBytes)
	{
		if (!Good() || count < 0 || elementBytes == 0 || ag_file->m_curpos > size_t(ag_file->m_buffer.size()))
			return Fail();
		const size_t remaining = size_t(ag_file->m_buffer.size()) - ag_file->m_curpos;
		if (size_t(count) > remaining / elementBytes)
			return Fail();
		return true;
	}

	template<class T>
	bool ReadFromBuffer(T* data, int elements = 1)
	{
		if (!CheckCount(elements, sizeof(T)))
			return false;
		if (elements == 0)
			return true;

		const size_t bytes = size_t(elements) * sizeof(T);
		memcpy(data, ag_file->m_buffer.begin() + ag_file->m_curpos, bytes);
		SwapEndian(data, elements);
		ag_file->m_curpos += bytes;
		return true;
	}

	bool StringValue(string* s);

	template<class T>
	bool Value(T* data, int elements = 1)
	{
		if (!file)
			return false;
		if (reading)
			return ReadFromBuffer(data, elements);
		if (elements)
			pak->FWrite(data, elements, file);
		return true;
	}

	bool Value(CCryName* data, int elements = 1)
	{
		if (!file)
			return false;
		for (int i = 0; i < elements; ++i)
		{
			string temp = data[i].c_str();
			if (!StringValue(&temp))
				return false;
			if (reading)
				data[i] = temp;
		}
		return true;
	}

	bool Value(uint64*, int = 1)
	{
		assert(!"Binary serialization of size_t data types is not supported!");
		return Fail();
	}
};

struct IAnimationStateNodeFactory;

enum EHasEnteredState
{
	eHES_Entered,
	eHES_Waiting,
	eHES_Instant
};

enum EAnimationStateNodeFlags
{
	eASNF_Update = 1,
};

struct SAnimationMovement
{
	SAnimationMovement() : translation(ZERO), rotation(IDENTITY), duration(0.0f) {}
	SAnimationMovement( const Vec3& t, const Quat& r, CTimeValue d) : translation(t), rotation(r), duration(d) {}
	Vec3 translation;
	Quat rotation;
	CTimeValue duration;

	SAnimationMovement& operator+=( const SAnimationMovement& rhs )
	{
		translation += rotation * rhs.translation;
		rotation *= rhs.rotation;
		duration += rhs.duration;
		return *this;
	}
};

struct SAnimationDesc
{
	const SAnimationSelectionProperties* properties;
	SAnimationMovement movement;
	//CTimeValue duration;
	QuatT startLocation;
	bool initialized;

	SAnimationDesc() : initialized(false), properties(NULL), startLocation(IDENTITY) {}
};

struct IAnimationStateNode
{
	IAnimationStateNode( uint32 f = 0 ) : flags(f) {}

	// Description:
	//   Called after instantiation, the state should perform initialization based on data
	virtual void EnterState( SAnimationStateData& data, bool dueToRollback ) = 0;
	// Description:
	//   Called after EnterState... determines when the state has really entered
	virtual EHasEnteredState HasEnteredState( SAnimationStateData& data ) = 0;
	// Description:
	//   Called after EnterState, when at least one state node has returned "Entered"
	virtual void EnteredState( SAnimationStateData& data ) = 0;
	// Description:
	//   Verify that this state can actually leave yet
	virtual bool CanLeaveState( SAnimationStateData& data ) = 0;
	// Description:
	//   Called when the state has completed, this function should cleanup anything in data
	virtual void LeaveState( SAnimationStateData& data ) = 0;
	// Description:
	//   Called after some state has had EnteredState called on it - this state node is then no longer needed (call delete this; and cleanup)
	virtual void LeftState( SAnimationStateData& data, bool wasEntered ) = 0;

	// Description:
	//   If we have eASNF_Update in flags, this will be called every frame
	virtual void Update( SAnimationStateData& data ) {assert(false);}
	// Description:
	//   What is the first time that this node will allow transitioning to another state (or 0)
	virtual void GetCompletionTimes( SAnimationStateData& data, CTimeValue stateStartTime, CTimeValue& mustRunToTime, CTimeValue& requeryTime ) = 0;

	virtual void DebugDraw( SAnimationStateData& data, IRenderer * pRenderer, int x, int& y, int yIncrement ) = 0;

	virtual IAnimationStateNodeFactory * GetFactory() = 0;

	virtual void GetStateMemoryStatistics( ICrySizer * s ) = 0;

	uint32 flags;

protected:

	void SerializeAsFile_NodeBase(bool reading, AG_FILE *file)
	{
		FileSerializationHelper h(reading, file);
		h.Value(&flags);
	}

};

struct IAnimationStateNodeFactory
{
	typedef IAnimationGraphStateFactoryIterator::SStateFactoryParameter Params;

	virtual bool Init( const XmlNodeRef& node, IAnimationGraphPtr pGraph ) = 0;
	virtual IAnimationStateNode * Create() = 0;
	virtual const char * GetCategory() = 0;
	virtual void Release() = 0;
	virtual const char * GetName() = 0;
	virtual const Params * GetParameters() = 0;
	virtual void SerializeAsFile(bool reading, AG_FILE *file) = 0;

	virtual void GetFactoryMemoryStatistics( ICrySizer * s ) = 0;

	virtual bool IsLessThan( IAnimationStateNodeFactory * pFactory ) = 0;
	virtual bool GetForceReentering() const { return false; }
};

#define AG_LT_BEGIN(cls) cls& r = *static_cast<cls*>(pFactory); if (false) 
#define AG_LT_BEGIN_FACTORY(cls) cls& r = *static_cast<cls*>(pFactory); if (flags < r.flags) return true; else if (flags > r.flags) return false
#define AG_LT_BEGIN_PARENT(cls,par) cls& r = *static_cast<cls*>(pFactory); if (par::IsLessThan(pFactory)) return true
#define AG_LT_ELEM(mem) else if (mem < r.mem) return true; else if (mem > r.mem) return false
#define AG_LT_END() return false

#endif
