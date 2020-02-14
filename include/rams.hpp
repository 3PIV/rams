#include <string>
#include <unordered_map>

namespace rams {

enum LifeTime { NONE = -1, TEMP = 0, SEMI = 1, PERM = 2 };

template <typename Type> struct Tracker {
  std::uint64_t mCount;
  Type *mData;
  LifeTime mLifeTime;
};

/**
 * @brief A Resource Manager for Class of Template Type
 * No public access to most member functions
 * @tparam Type
 */
template <typename Type> class ResourceHandler {
public:
  static ResourceHandler *gInstance;

  /**
   * @brief Get the Instance object for the Singleton
   *
   * @return ResourceHandler* pointer to the Singleton Resource Handler of Type
   */
  static ResourceHandler *getInstance() {
    if (!gInstance) {
      gInstance = new ResourceHandler();
    }
    return gInstance;
  }

  /**
   * @brief acquire allocates or increments a reference to memory and attributes
   * it a life time
   *
   * @param anName std::string handle which will be the dictionary key to the
   * memory value
   * @param anLife LifeTime resource longevity, defaults to TEMP (destroy on 0
   * remaining refs)
   * @return Type* newly allocated memory or pointer to previously created
   * memory
   */
  Type *acquire(const std::string &anName, LifeTime anLife = TEMP) {
    /* STEPS:
     * 1) Iterate the map, see if the passed key is in it
     * 2) If key is not in, make a new tracker with a single reference
     *      count and allocate memory for a new object of Type
     * 3) If key is in, increment the reference count to that memory,
     *      update the lifetime if a greater lifetime is passed
     * 4) Return new memory or existing memory
     */
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
   * @brief object that acquired memory tracked by handler is releasing its
   * reference to memory
   *
   * @param anName std::string key of the memory which was being tracked
   */
  void release(const std::string &anName) {
    /* STEPS:
     * 1) Search for key in map
     * 2) If key not in map, return
     * 3) If key is in, decrement reference count
     * 4) If reference count is 0 and lifetime is TEMP, remove from handler
     */
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

    // Decrement the reference count, delete on 0 references if TEMP
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
  /**
   * @brief Construct a new Resource Handler object
   *
   * Private Due to Singleton Pattern
   */
  ResourceHandler() {}
  /**
   * @brief Construct a new Resource Handler object
   *
   * Private Due to Singleton Pattern
   */
  ResourceHandler(const ResourceHandler &) {}
  /**
   * @brief Assign a value from an rvalue ResourceHandler
   *
   * Private Due to Singleton Pattern
   * @return ResourceHandler&
   */
  ResourceHandler &operator=(const ResourceHandler) {}

  /**
   * @brief Default load method for new allocation, can be specialized
   *
   * By default this shall only generate new memory,
   * this can be specialized the following way:
   *
   * template<> Type* rams::ResourceHandler<Type>::load(const std::string&){
   *    Type* mine = new Type("Do Specialized Construction");
   *    return mine;
   * }
   * @return Type* newly allocated memory
   */
  virtual Type *load(const std::string &) { return new Type; }

  std::string mHandleID;
  std::unordered_map<std::string, Tracker<Type> > mResourceMap;
};

// Set Global Pointer of the ResourceHandler for Type to null as default
template <typename Type>
ResourceHandler<Type> *ResourceHandler<Type>::gInstance = nullptr;

/**
 * @brief Class which contains pointer of Type and is LifeTime managed by
 * ResourceHandler<Type>
 *
 * @tparam Type
 */
template <typename Type> class Resource {
public:
  /**
   * @brief Construct a new Resource object
   *
   * Constructs a new resource object, acquiring memory related to the
   *    key, anName, from ResourceHandler<Type>
   * @param anName std::string key value to be associated to memory
   * @param anLife LifeTime expected life time of memory being acquired
   */
  explicit Resource(const std::string &anName, LifeTime anLife = TEMP)
      : mID(anName) {
    std::cout << "New resource: " << anName << std::endl;
    ResourceHandler<Type> *gHandler = ResourceHandler<Type>::getInstance();
    mData = gHandler->acquire(mID, anLife);
  }

  /**
   * @brief Destroy the Resource object
   *
   * Destroy the Resource object and make the ResourceHandler aware of
   *    the memory reference being released upon Resource Destruction.
   */
  ~Resource() {
    std::cout << "Resource Instance destroyed: " << mID << std::endl;
    ResourceHandler<Type> *gHandler = ResourceHandler<Type>::getInstance();
    gHandler->release(mID);
  }

  /**
   * @brief Overloaded () operator which retrieves the underlying memory
   *
   * @return const Type&
   */
  const Type &operator()() const { return *mData; }

  /**
   * @brief Retrieves the underlying memory
   *
   * @return const Type&
   */
  const Type &data() const { return *mData; }

  /**
   * @brief Set value of underlying data by deleting memory and pointing to new
   * memory
   *
   * @param newData New memory to set pointer to.
   */
  void set(Type *newData) {
    delete mData;
    mData = newData;
  }

  /**
   * @brief Set value of underlying data by assigning a new value to what the
   * unerlying pointer refers to
   *
   * @param newData New value for memory to reference
   */
  void set(Type newData) { *mData = newData; }

private:
  const std::string mID;
  Type *mData;
};
} // namespace rams