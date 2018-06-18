#pragma once

#include <memory>
#include <vector>

namespace s2j {

template<typename EventListenerType>
class EventProvider {
public:
    void addEventListener(std::shared_ptr<EventListenerType> eventListener) {
        if (eventListener != nullptr) {
            eventListeners_.push_back(std::move(eventListener));
        }
    }

protected:
    std::vector<std::shared_ptr<EventListenerType>> eventListeners_;
};

}
