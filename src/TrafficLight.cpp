#include <iostream>
#include <random>
#include <chrono>
#include <stdexcept>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));   // debug
    // unique_lock as we pass it to the condition variable
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.back());   // front()
    _queue.pop_back();                  // pop.front()
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));   // debug
    std::lock_guard<std::mutex> lgLock(_mutex);
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = tlp::TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // repeatedly calls the receive function on the message queue. 
    while (true)
    {
        tlp::TrafficLightPhase message = _messages->receive();
        if (message == tlp::TrafficLightPhase::green) {
            break;
        }
    }
}

tlp::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // method „cycleThroughPhases“ is started in a thread when the public method „simulate“ is called
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // measures the time between two loop cycles, and toggles the current phase of the traffic light 
    // between red and green. Sends an update to the message queue using move semantics. 
    // The cycle duration is a random value between 4 and 6 seconds. 
    auto cycle_begin = std::chrono::high_resolution_clock::now();
    auto cycle_end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cycle_end - cycle_begin);
    unsigned int milliseconds_sum = 0; 
    std::mt19937_64 prng{std::random_device{}()};       // pseudo-random number generator
    std::uniform_int_distribution<> uDist{4, 6};        // uniform distribution
    int random_cycle_duration = uDist(prng)*1000;       // generates either 4000, 5000 or 6000 [ms]
    
    while (true)
    {
       duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cycle_end - cycle_begin);
       milliseconds_sum += duration_ms.count();
       cycle_begin = std::chrono::high_resolution_clock::now(); 
       std::this_thread::sleep_for(std::chrono::milliseconds(1));   // reduce CPU load
             
       if (milliseconds_sum >= random_cycle_duration) {
           if (_currentPhase == tlp::TrafficLightPhase::red) {
               _currentPhase = tlp::TrafficLightPhase::green;
           } 
           else if (_currentPhase == tlp::TrafficLightPhase::green) {
               _currentPhase = tlp::TrafficLightPhase::red;
           }
           _messages->send(std::move(_currentPhase));
           milliseconds_sum = 0;
           random_cycle_duration = uDist(prng)*1000;
       }       
            
       cycle_end = std::chrono::high_resolution_clock::now();
    }
}