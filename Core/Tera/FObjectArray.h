#pragma once
#include <vector>

// A vector wrapper to hold objects and there indices. Same idea behind the DECL_UREF.
// If we failed to load an object we will still have it's index.
template <typename T>
struct FObjectArray {
  FObjectArray() = default;

  FObjectArray(size_t size)
    : Objects(size, nullptr)
    , Indices(size, INDEX_NONE)
  {
  }

  FObjectArray(const std::vector<UObject*>& vec)
  {
    Objects.resize(vec.size());
    Indices.resize(vec.size());
    for (int32 idx = 0; idx < vec.size(); ++idx)
    {
      Objects[idx] = (T)vec[idx];
      Indices[idx] = vec[idx] ? vec[idx]->GetExportObject()->ObjectIndex : INDEX_NONE;
    }
  }

  T& front()
  {
    return Objects.front();
  }

  T& back()
  {
    return Objects.back();
  }

  const T& front() const
  {
    return Objects.front();
  }

  const T& back() const
  {
    return Objects.back();
  }

  typename std::vector<T>::iterator begin()
  {
    return Objects.begin();
  }

  typename std::vector<T>::iterator end()
  {
    return Objects.end();
  }

  T const& operator[](size_t idx) const
  {
    return Objects[idx];
  }

  T& operator[](size_t idx)
  {
    return Objects[idx];
  }

  const T& at(size_t idx) const
  {
    return Objects.at(idx);
  }

  size_t size() const
  {
    return Objects.size();
  }

  T& emplace_back(T&& obj)
  {
    return Objects.emplace_back(obj);
  }

  void clear()
  {
    Objects.clear();
  }

  void resize(size_t idx)
  {
    Objects.resize(idx);
  }

  void push_back(T&& obj)
  {
    Objects.push_back(obj);
  }

  std::vector<T>& GetObjects()
  {
    return Objects;
  }

  std::vector<UObject*> GetUObjects() const
  {
    std::vector<UObject*> result(size());
    std::memcpy(result.data(), Objects.data(), result.size() * sizeof(UObject*));
    return result;
  }

  std::vector<PACKAGE_INDEX>& GetIndices()
  {
    return Indices;
  }

  friend FStream& operator<<(FStream& s, FObjectArray<T>& arr)
  {
    return s.SerializeObjectArray((std::vector<UObject*>&)arr.GetObjects(), arr.GetIndices());
  }

protected:
  std::vector<PACKAGE_INDEX> Indices;
  std::vector<T> Objects;
};