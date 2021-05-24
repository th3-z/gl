#ifndef ResourceCache_H
#define ResourceCache_H

#include <unordered_map>
#include <string>
#include <memory>
#ifndef NDEBUG
#include <iostream>
#endif

template <class T> class ResourceCache {
public:
    ResourceCache();
    ~ResourceCache();

    std::shared_ptr<T> loadResource(const std::string &resourceName);
    void sweep();
#ifndef NDEBUG
    void dbg();
#endif

private:
    std::unordered_map<std::string, std::shared_ptr<T>> resources;
};

template <class T> ResourceCache<T>::ResourceCache() = default;
template <class T> ResourceCache<T>::~ResourceCache() = default;

template <class T> std::shared_ptr<T> ResourceCache<T>::loadResource(const std::string &resourceName) {
    auto found = this->resources.find(resourceName);

    if (found != this->resources.end())
        return found->second;

#ifndef NDEBUG
    std::cout << "Resource cache miss: " << resourceName << std::endl;
#endif
    this->resources.emplace(resourceName, std::make_shared<T>(resourceName));

    return this->loadResource(resourceName);
}

template <class T> void ResourceCache<T>::sweep() {
    for (auto resourceIter = this->resources.cbegin(); resourceIter != this->resources.cend();) {
        if (resourceIter->second.use_count() < 2)
            resourceIter = this->resources.erase(resourceIter);
        else
            resourceIter++;
    }
}

#ifndef NDEBUG
template <class T> void ResourceCache<T>::dbg() {
    std::cout << "Resource cache contents:" << std::endl;
    if (!this->resources.empty())
        for (auto &resource : this->resources) {
            std::cout << "\t" << resource.first << ": " << resource.second.use_count() << std::endl;
        }
    else
        std::cout << "\tEmpty" << std::endl;
}
#endif

#endif // ResourceCache_H
