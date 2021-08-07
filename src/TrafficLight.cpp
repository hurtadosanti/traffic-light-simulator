#include <iostream>
#include <random>
#include "TrafficLight.h"


/* Implementation of class "MessageQueue" */


template<typename T>
T MessageQueue<T>::receive() {

    std::unique_lock<std::mutex> uniqueLock(_mutex);
    // pass unique lock to condition variable
    _conditionVariable.wait(uniqueLock, [this] { return !_messages.empty(); });
    // remove last vector element from queue
    T msg = std::move(_messages.back());
    _messages.pop_back();

    return msg;
}

template<typename T>
void MessageQueue<T>::send(T &&msg) {
    std::lock_guard<std::mutex> uniqueLock(_mutex);
    // add vector to queue
    _messages.push_back(std::move(msg));
    _conditionVariable.notify_one(); // notify client after pushing new Vehicle into vector
}

TrafficLight::TrafficLight() {
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen() {
    // infinite while-loop
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true){
        auto msg =_messages.receive();
        if(msg==TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase() {
    return _currentPhase;
}

void TrafficLight::simulate() {
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
[[noreturn]] void TrafficLight::cycleThroughPhases() {
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    std::random_device randomDevice;
    std::uniform_int_distribution<int> uniformIntDistribution(4000, 6000);
    auto duration = uniformIntDistribution(randomDevice);
    auto cycleTime = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> lastUpdated;
    double elapsed;
    while (true) {
        lastUpdated = std::chrono::system_clock::now();
        elapsed = std::chrono::duration<double, std::milli>(lastUpdated - cycleTime).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (elapsed >= duration) {
            if (_currentPhase == TrafficLightPhase::red) {
                _currentPhase = TrafficLightPhase::green;
            } else {
                _currentPhase = TrafficLightPhase::red;
            }
            _messages.send(std::move(_currentPhase));
            cycleTime = std::chrono::system_clock::now();
        }

    }
}