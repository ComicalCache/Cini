#ifndef INSTANCE_TRACKER_HPP_
#define INSTANCE_TRACKER_HPP_

#include <atomic>
#include <cstddef>

/// Tracks live instances of structs used for debugging.
template<typename T>
struct InstanceTracker {
    friend T;

public:
    inline static std::atomic<std::size_t> instances_{0};

private:
    InstanceTracker() { InstanceTracker<T>::instances_++; }
    InstanceTracker(const InstanceTracker& _) { InstanceTracker<T>::instances_++; }
    ~InstanceTracker() { InstanceTracker<T>::instances_--; }
};

#endif
