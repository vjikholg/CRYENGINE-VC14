#ifndef __CRYDXPSAPTR__
#define __CRYDXPSAPTR__

#ifdef WIN32
	#pragma warning( disable : 4355) 
#endif

template<class T>
class CCryRefAndWeak
{
private:
	void*													m_pThis;
	unsigned	int									m_RefCount;
	unsigned	int									m_WeakCount;
public:

	CCryRefAndWeak(void* pThis) :m_pThis(pThis),
		m_RefCount(1), m_WeakCount(0)
	{
	}

	unsigned long IncRef()
	{
		return ++m_RefCount;
	}

	unsigned long DecRef()
	{
		if(!--m_RefCount)
		{
			reinterpret_cast<T*>(m_pThis)->ReleaseResources();
			if(!m_WeakCount)
				CRY_DXPS_RELEASE(reinterpret_cast<T*>(m_pThis));
		}
		return m_RefCount;
	}

	void AddWeak()
	{
		++m_WeakCount;
	}
	void ReleaseWeak()
	{
		if(!--m_WeakCount && !m_RefCount)
			CRY_DXPS_RELEASE(reinterpret_cast<T*>(m_pThis));
	}
	uint32 Valid()	const
	{
		return m_RefCount;
	}
};

template<class T>
class CCryAPtrRefCnt
{
	T*									m_pData;
public:
	CCryAPtrRefCnt():
	m_pData(0)
	{
	}
	CCryAPtrRefCnt(T* pData):
	m_pData(pData)
	{
		pData->AddRef();
	}
	CCryAPtrRefCnt(const CCryAPtrRefCnt<T>& rData):
	m_pData(rData->m_pData)
	{
		rData->IncRef();
	}
	~CCryAPtrRefCnt()
	{
		if(m_pData)
			m_pData->DecRef();
	}

	CCryAPtrRefCnt<T>&	operator=(T* pData)
	{
		if(pData)
			pData->IncRef();
		if(m_pData)
			m_pData->DecRef();
		m_pData	=	pData;
		return *this;
	}

	CCryAPtrRefCnt<T>&	operator=(const CCryAPtrRefCnt<T>& rData)
	{
		if(rData->m_pData)
			rData->m_pData->IncRef();
		if(m_pData)
			m_pData->DecRef();
		m_pData	=	rData->m_pData;
		return *this;
	}

	T* operator->()
	{
		return m_pData;
	}

	operator T*()
	{
		return m_pData;
	}
};

template<class T>
class CCryAPtrWeakCnt
{
	T*									m_pData;
public:
	CCryAPtrWeakCnt():
	m_pData(0)
	{
	}
	CCryAPtrWeakCnt(T* pData):
	m_pData(pData)
	{
		pData->AddWeak();
	}
	CCryAPtrWeakCnt(const CCryAPtrWeakCnt<T>& rData):
	m_pData(rData->m_pData)
	{
		rData->AddWeak();
	}
	~CCryAPtrWeakCnt()
	{
		if(m_pData)
			m_pData->ReleaseWeak();
	}

	CCryAPtrWeakCnt<T>&	operator=(T* pData)
	{
		if(pData)
			pData->AddWeak();
		if(m_pData)
			m_pData->ReleaseWeak();
		m_pData	=	pData;
		return *this;
	}

	CCryAPtrWeakCnt<T>&	operator=(const CCryAPtrWeakCnt<T>& rData)
	{
		if(rData->m_pData)
			rData->m_pData->AddWeak();
		if(m_pData)
			m_pData->ReleaseWeak();
		m_pData	=	rData->m_pData;
		return *this;
	}

	T* operator->()
	{
		return m_pData && m_pData->Valid()?m_pData:0;
	}

	operator T*()
	{
		return m_pData && m_pData->Valid()?m_pData:0;
	}
};

template<class T,bool ARRAY=true>
class CCryAPtrScope
{
	T*									m_pData;

	CCryAPtrScope(const CCryAPtrScope<T>& rData){}
	void operator=(const CCryAPtrScope<T>& rData){}

public:
	CCryAPtrScope():
	m_pData(0)
	{
	}
	CCryAPtrScope(T* pData):
	m_pData(pData)
	{
	}
	~CCryAPtrScope()
	{
		if(ARRAY)
			delete[] m_pData;
		else
			delete m_pData;
	}

	void Release()
	{
		if(ARRAY)
			delete[] m_pData;
		else
			delete m_pData;
		m_pData	=	0;
	}

	T* operator->()
	{
		return m_pData;
	}

	operator T*()
	{
		return m_pData;
	}
};

template<class T>
class CCryAPtrBoundCheck
{
	T*									m_pData;
	uint32				m_Size;
 
	CCryAPtrBoundCheck(const CCryAPtrBoundCheck<T>& rData)
	{
	}

	void operator=(const CCryAPtrBoundCheck<T>& rData)
	{
	}

public:
	CCryAPtrBoundCheck(T* pData,uint32 Size):
	m_pData(pData),
	m_Size(Size)
	{
	}
	~CCryAPtrBoundCheck()
	{
			delete m_pData;
	}

	T& operator[](uint32 Pt)
	{
		assert(Pt<m_Size);
		return m_pData[Pt];
	}
	const T& operator[](uint32 Pt)const
	{
		assert(Pt<m_Size);
		return m_pData[Pt];
	}
};

#endif

