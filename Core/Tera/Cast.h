#pragma once

template<typename T>
T* Cast(UObject* obj)
{
  return obj && obj->IsA(T::StaticClassName()) ? (T*)obj : nullptr;
}

template< class T >
inline T* ExactCast(UObject* obj)
{
	return obj && (obj->GetStaticClassName() == T::StaticClassName()) ? (T*)obj : nullptr;
}

template< class T, class U >
T* CastChecked(U* obj)
{
#if _DEBUG
	if (!obj || !obj->IsA(T::StaticClassName()))
	{
		LogE("Cast of %s to %s failed", obj ? obj->GetObjectName().UTF8().c_str() : "NULL", T::StaticClassName());
	}
#endif
	return (T*)obj;
}

template <class T>
T* FindField(UStruct* owner, const FString& field)
{
	try
	{
		if (owner->GetPackage()->GetNameIndex(field) == INDEX_NONE)
		{
			return nullptr;
		}
	}
	catch (...)
	{
		return nullptr;
	}
	for (TFieldIterator<T>it(owner); it; ++it)
	{
		if (it->GetObjectName() == field)
		{
			return *it;
		}
	}
	// If we didn't find it, return no field
	return nullptr;
}