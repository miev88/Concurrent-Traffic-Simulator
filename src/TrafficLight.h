#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <memory>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

template <class T>
class MessageQueue
{
public:
    void send(T &&msg);
    T receive();

private:
    std::condition_variable _condition;
    std::mutex _mutex;    
    std::deque<T> _queue;
};

namespace tlp
{
   enum TrafficLightPhase { red, green };
}

class TrafficLight : public TrafficObject
{
public:
    // constructor / destructor
    TrafficLight();

    // getters / setters
    tlp::TrafficLightPhase getCurrentPhase();

    // typical behaviour methods
    void waitForGreen();
    void simulate();

private:
    // typical behaviour methods
    void cycleThroughPhases();

    std::condition_variable _condition;
    std::mutex _mutex;
    tlp::TrafficLightPhase _currentPhase;
    std::shared_ptr<MessageQueue<tlp::TrafficLightPhase>> _messages{new MessageQueue<tlp::TrafficLightPhase>};  // monitor object
};

#endif