#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <new>

//#define _STATS
//#define _ADVANCED

#include "container.h"

static unsigned gs_nBufferSize = 0;
static char * gs_pBuffer = NULL;

//-------------------------------------------------------------------------
//
class CTestClass
{
	size_t			m_nAddr;
	unsigned		m_nAllocNum;
	static bool		ms_bAllowAdd;

public:
	CTestClass()
	{
		assert(ms_bAllowAdd);
		m_nAddr		= (size_t)this;
		m_nAllocNum = 0;
	}

	~CTestClass()
	{
		m_nAddr		= 0;
		m_nAllocNum = 0;
	}

	void SetAllocNum(unsigned nNumAlloc)
	{
		m_nAllocNum = nNumAlloc;
	}

	unsigned GetAllocNum() const
	{
		return m_nAllocNum;
	}

	size_t GetAddress() const
	{
		return m_nAddr;
	}

protected:
	static void AllowAdd(bool bAllowed)
	{
		ms_bAllowAdd = bAllowed;
	}

	friend void StressTest(CContainer<CTestClass> & oContainer, unsigned const nNumAllocs);
};

bool CTestClass::ms_bAllowAdd = false;

//-------------------------------------------------------------------------

inline bool IsInBuffer(void const * pAddr)
{
	return ((char const *)pAddr >= gs_pBuffer) && ((char const *)pAddr <= (gs_pBuffer + gs_nBufferSize - sizeof(CTestClass)));
}

//-------------------------------------------------------------------------

#ifdef _ADVANCED
int CompareTestClass(CTestClass const * pX, CTestClass const * pY)
{
	if (pX->GetAllocNum() < pY->GetAllocNum())
	{
		return -1;
	}
	else if (pX->GetAllocNum() > pY->GetAllocNum())
	{
		return 1;
	}

	return 0;
}
#endif

//-------------------------------------------------------------------------

void StressTest(CContainer<CTestClass> & oContainer, unsigned const nNumAllocs)
{
	int nSeed = 0;
	unsigned const nCapacity = oContainer.Capacity();

	unsigned nNballocs = 0;
	unsigned nNbFrees = 0;
#ifdef _STATS
	unsigned nNbEmpty = 0;
	unsigned nNbFull = 0;
	unsigned nMaxSize = 0;
#endif
	srand(nSeed);

	///======	pre-fill up 3/4 of the container.
	CTestClass::AllowAdd(true);
	for (unsigned i = 0; i < 3 * nCapacity / 4; ++i)
	{
		CTestClass * pObj = oContainer.Add();
		pObj->SetAllocNum(nNballocs);
		nNballocs++;
	}
	CTestClass::AllowAdd(false);

	///======	loop until we reach a certain number of allocations
	while (nNballocs < nNumAllocs)
	{
		///======	randomly add or remove object from the managed container
		bool bAdd = ((rand() & 0x1f) >= 16)  ? true : false;	

		if (bAdd && oContainer.IsFull())
		{
			bAdd = false;
		}
		else if (!bAdd && oContainer.IsEmpty())
		{
			bAdd = true;
		}

		if (bAdd)
		{
			CTestClass::AllowAdd(true);
			CTestClass * pObj = oContainer.Add();
			CTestClass::AllowAdd(false);

			assert(IsInBuffer(pObj));
			pObj->SetAllocNum(nNballocs);
			assert(pObj->GetAddress() == (size_t)pObj);	///======	sanity check
			nNballocs++;
		}
		else
		{
			int nIndex = (oContainer.Count() > 1) ? rand() % (oContainer.Count() - 1) : 0;

			CTestClass * pObj = oContainer[nIndex];
			assert(pObj->GetAddress() == (size_t)pObj);	///======	sanity check
			oContainer.Remove(pObj);
			assert(pObj->GetAddress() == 0);			///======	if this assert trips then you haven't called the destructor.
			nNbFrees++;
		}

#ifdef _STATS
		nMaxSize = oContainer.Count() > nMaxSize ? oContainer.Count() : nMaxSize;
		if (oContainer.IsEmpty())
		{
			++nNbEmpty;				
		}
		else if (oContainer.IsFull())
		{
			++nNbFull;
		}
#endif

		if ((nNballocs & 32767) == 0)
		{
			printf("Container => %d allocs, %d frees\r", nNballocs, nNbFrees);
		}
	}

#ifdef _STATS
	printf("\nMaxSize: %d (seed %d), Nb full containers: %d, Nb empty containers: %d\n", nMaxSize, nSeed, nNbFull, nNbEmpty);
#endif

#ifdef _ADVANCED
	oContainer.Sort(CompareTestClass);

	///======	sanity check after the sort
	for (unsigned i = 0; i < oContainer.Count() - 1; ++i)
	{
		assert(oContainer[i]->GetAllocNum() < oContainer[i + 1]->GetAllocNum());
	}
#endif

	///======	clean up
	while (oContainer.Count() > 0)
	{
		CTestClass * pObj = oContainer[0];
		assert(pObj->GetAddress() == (size_t)pObj);	///======	sanity check
		oContainer.Remove(pObj);
	}
	assert(oContainer.Count() == 0);
}

//-------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage:\nTest.exe buffersize (in KB)\n");
		return EXIT_FAILURE;
	}

	assert(sizeof(CContainer<CTestClass>) < 256);	///======	this is an arbitrary, but why should we need more?

	gs_nBufferSize = atoi(argv[1]) * 1024;
	gs_pBuffer = new char[gs_nBufferSize];

	CContainer<CTestClass> * pContainer = new CContainer<CTestClass>(gs_pBuffer, gs_nBufferSize);

	printf("Managed Container Capacity: %d\n", pContainer->Capacity());

	clock_t oStartTime = clock();
	StressTest(*pContainer, 20000000);
	clock_t oEndTime = clock();

	double fElapsedTime = (double)(oEndTime - oStartTime) / CLOCKS_PER_SEC;

	printf("\nTime elapsed: %f seconds\n", fElapsedTime);

	delete pContainer;
	pContainer = NULL;

	delete[] gs_pBuffer;

	return EXIT_SUCCESS;
}
