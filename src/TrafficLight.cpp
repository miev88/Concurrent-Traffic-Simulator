#include <iostream>
#include <random>
#include <chrono>
#include <stdexcept>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::this_thread::sleep_for(std::chrono::milliseconds(100));   // for debugging
    // It must be a unique_lock as we pass it to the condition variable - gives unique 
    // ownership in both states: locked and unlocked.
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.back());   // front()
    _queue.pop_back();                  // pop.front()
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));   // for debugging
    std::lock_guard<std::mutex> lgLock(_mutex);
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    //_currentPhase = TrafficLightPhase::red;
    _currentPhase = tlp::TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        //TrafficLightPhase message = _messages->receive();
        tlp::TrafficLightPhase message = _messages->receive();
        //if (message == TrafficLightPhase::green) {
        if (message == tlp::TrafficLightPhase::green) {
            break;
        }
    }
}

//TrafficLightPhase TrafficLight::getCurrentPhase()
tlp::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : The private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green. Moreover, it sends an update 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    auto cycle_begin = std::chrono::high_resolution_clock::now();
    auto cycle_end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cycle_end - cycle_begin);
    unsigned int milliseconds_sum = 0; 
    std::mt19937_64 prng{std::random_device{}()};       // pseudo-random number generator
    std::uniform_int_distribution<> uDist{4, 6};        // uniform distribution
    int random_cycle_duration = uDist(prng)*1000;       // generates either 4000, 5000 or 6000 [ms]
    
    while (true)
    {
       // https://en.cppreference.com/w/cpp/chrono/duration/duration_cast
       duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cycle_end - cycle_begin);
       milliseconds_sum += duration_ms.count();
       cycle_begin = std::chrono::high_resolution_clock::now(); 
       std::this_thread::sleep_for(std::chrono::milliseconds(1));
             
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