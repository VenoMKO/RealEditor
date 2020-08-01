#pragma once
#include "Core.h"


struct FScriptContainerElement
{
};

class FScriptArray
{
public:

	~FScriptArray()
	{
		if (Data)
		{
			free(Data);
			Data = nullptr;
		}
	}

	inline FScriptContainerElement* GetAllocation() const
	{
		return Data;
	}

	void ResizeAllocation(int32 PreviousNumElements, int32 NumElements, int32 NumBytesPerElement)
	{
		if (Data || NumElements)
		{
			Data = (FScriptContainerElement*)realloc(Data, NumElements * NumBytesPerElement);
		}
	}

	int32 CalculateSlack( int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
	{
		return DefaultCalculateSlack(NumElements, NumAllocatedElements, NumBytesPerElement);
	}

	int32 GetAllocatedSize(int32 NumAllocatedElements, int32 NumBytesPerElement) const
	{
		return NumAllocatedElements * NumBytesPerElement;
	}

	void* GetData()
	{
		return GetAllocation();
	}

	const void* GetData() const
	{
		return GetAllocation();
	}

	bool IsValidIndex(int32 i) const
	{
		return i >= 0 && i < ArrayNum;
	}

	inline int32 Num() const
	{
		return ArrayNum;
	}

	void InsertZeroed(int32 Index, int32 Count, int32 NumBytesPerElement)
	{
		Insert(Index, Count, NumBytesPerElement);
		memset(GetAllocation() + Index * NumBytesPerElement, 0, Count * NumBytesPerElement);
	}
	void Insert(int32 Index, int32 Count, int32 NumBytesPerElement)
	{
		const int32 OldNum = ArrayNum;
		if ((ArrayNum += Count) > ArrayMax)
		{
			ArrayMax = this->CalculateSlack(ArrayNum, ArrayMax, NumBytesPerElement);
			this->ResizeAllocation(OldNum, ArrayMax, NumBytesPerElement);
		}
		memmove(GetAllocation() + (Index + Count) * NumBytesPerElement, GetAllocation() + (Index)*NumBytesPerElement, (OldNum - Index) * NumBytesPerElement);
	}
	int32 Add(int32 Count, int32 NumBytesPerElement)
	{
		const int32 OldNum = ArrayNum;
		if ((ArrayNum += Count) > ArrayMax)
		{
			ArrayMax = this->CalculateSlack(ArrayNum, ArrayMax, NumBytesPerElement);
			this->ResizeAllocation(OldNum, ArrayMax, NumBytesPerElement);
		}

		return OldNum;
	}
	int32 AddZeroed(int32 Count, int32 NumBytesPerElement)
	{
		const int32 Index = Add(Count, NumBytesPerElement);
		memset(GetAllocation() + Index * NumBytesPerElement, 0, Count * NumBytesPerElement);
		return Index;
	}

	void Shrink(int32 NumBytesPerElement)
	{
		if (ArrayMax != ArrayNum)
		{
			ArrayMax = ArrayNum;
			this->ResizeAllocation(ArrayNum, ArrayMax, NumBytesPerElement);
		}
	}
	void Empty(int32 Slack, int32 NumBytesPerElement)
	{
		ArrayNum = 0;
		if (ArrayMax != Slack)
		{
			ArrayMax = Slack;
			this->ResizeAllocation(0, ArrayMax, NumBytesPerElement);
		}
	}
	void Swap(int32 A, int32 B, int32 NumBytesPerElement)
	{
		memswap(GetAllocation() + (NumBytesPerElement * A), GetAllocation() + (NumBytesPerElement * B), NumBytesPerElement);
	}

	int32 GetSlack() const
	{
		return ArrayMax - ArrayNum;
	}

	void Remove(int32 Index, int32 Count, int32 NumBytesPerElement)
	{
		// Skip memmove in the common case that there is nothing to move.
		int32 NumToMove = ArrayNum - Index - Count;
		if (NumToMove)
		{
			memmove(GetAllocation() + (Index)*NumBytesPerElement, GetAllocation() + (Index + Count) * NumBytesPerElement, NumToMove * NumBytesPerElement);
		}
		ArrayNum -= Count;

		const int32 NewArrayMax = CalculateSlack(ArrayNum, ArrayMax, NumBytesPerElement);
		if (NewArrayMax != ArrayMax)
		{
			ArrayMax = NewArrayMax;
			ResizeAllocation(ArrayNum, ArrayMax, NumBytesPerElement);
		}
	}

private:

	inline int32 DefaultCalculateSlack(int32 numElements, int32 numAllocatedElements, uint32 bytesPerElement) const
	{
		int32 retval = 0;
		if (numElements < numAllocatedElements)
		{
			const uint32 currentSlackElements = numAllocatedElements - numElements;
			const uint32 currentSlackBytes = (numAllocatedElements - numElements) * bytesPerElement;
			const bool bTooManySlackBytes = currentSlackBytes >= 16384;
			const bool bTooManySlackElements = 3 * numElements < 2 * numAllocatedElements;
			if ((bTooManySlackBytes || bTooManySlackElements) && (currentSlackElements > 64 || !numElements))
			{
				retval = numElements;
			}
			else
			{
				retval = numAllocatedElements;
			}
		}
		else if (numElements > 0)
		{
			const int32 firstAllocation = 4;
			if (!numAllocatedElements && numElements <= firstAllocation)
			{
				retval = firstAllocation;
			}
			else
			{
				retval = numElements + 3 * numElements / 8 + 16;
			}
		}
		else
		{
			retval = 0;
		}
		return retval;
	}

protected:

	FScriptArray(int32 InNum, int32 NumBytesPerElement)
		: ArrayNum(InNum)
		, ArrayMax(InNum)

	{
		this->ResizeAllocation(0, ArrayMax, NumBytesPerElement);
	}

	int32	  ArrayNum = 0;
	int32	  ArrayMax = 0;
	FScriptContainerElement* Data = nullptr;
};