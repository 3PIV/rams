#include <string>
#include <unordered_map>

namespace rams {

enum LifeTime { NONE = -1, TEMP = 0, SEMI = 1, PERM = 2 };

template <typename Type> struct Tracker {
  std::uint64_t mCount;
  Type *mData;
  LifeTime mLifeTime;
};

template <typename Type> class ResourceHandler {
public:
  static ResourceHandler *gInstance;

  static ResourceHandler *getInstance() {
    if (!gInstance) {
      gInstance = new ResourceHandler();
    }
    return gInstance;
  }

  /**
   *
   */
  Type *acquire(const std::string &anName, LifeTime anLife = TEMP) {
    Type *theReturnData = nullptr;

    typename std::unordered_map<std::string, Tracker<Type> >::iterator
        searchResult = mResourceMap.find(anName);
    if (searchResult == mResourceMap.end()) {
      std::cout << "Allocating for: " << anName << std::endl;
      theReturnData = load(anName);
      Tracker<Type> newTracker = {1, theReturnData, TEMP};
      mResourceMap.insert(std::make_pair(anName, newTracker));
    } else {
      if (anLife > searchResult->second.mLifeTime) {
        searchResult->second.mLifeTime = anLife;
      }
      theReturnData = searchResult->second.mData;
      ++searchResult->second.mCount;
    }

    return theReturnData;
  }

  /**
   *
   */
  void release(const std::string &anName) {
    typename std::unordered_map<std::string, Tracker<Type> >::iterator
        searchResult = mResourceMap.find(anName);

    /// Make sure an element is not double deleted
    if (searchResult == mResourceMap.end()) {
      std::cerr << "Release on non-existant resource " + anName +
                       " for handler of type " +
                       std::string(typeid(Type).name())
                << std::endl;
      return;
    }

    /// Decrement the reference count, delete on 0 references
    if (searchResult->second.mCount > 0) {
      --searchResult->second.mCount;
    }
    if (searchResult->second.mCount == 0 &&
        searchResult->second.mLifeTime == TEMP) {
      std::cout << "Removing memory of: " << anName << std::endl; 
      delete searchResult->second.mData;
      mResourceMap.erase(searchResult);
    }
  }

private:
  ResourceHandler() {}
  ResourceHandler(const ResourceHandler &) {}
  ResourceHandler &operator=(const ResourceHandler) {}

  /**
   *
   */
  virtual Type *load(const std::string &) { return new Type; }

  std::string mHandleID;
  std::unordered_map<std::string, Tracker<Type> > mResourceMap;
};

template <typename Type>
ResourceHandler<Type> *ResourceHandler<Type>::gInstance = nullptr;

template <typename Type> class Resource {

public:
  explicit Resource(const std::string &anName, LifeTime anLife = TEMP)
      : mID(anName) {
    std::cout << "New resource: " << anName << std::endl;
    if (!gHandler) {
      gHandler = ResourceHandler<Type>::getInstance();
    }
    mData = gHandler->acquire(mID, anLife);
  }

  ~Resource() {
    std::cout << "Resource Instance destroyed: " << mID << std::endl;
    gHandler->release(mID);
  }

  const Type &operator()() const { return *mData; }

  const Type &data() const { return *mData; }

  void set(Type* newData) {
    delete mData;
    mData = newData;
  }

  void set(Type newData) {
    *mData = newData;
  }

private:
  const std::string mID;
  Type * mData;
  static ResourceHandler<Type> *gHandler;
};

template <typename Type>
ResourceHandler<Type> *Resource<Type>::gHandler = nullptr;
} // namespace rams