#include <iostream>
#include <cstdlib>  // rand
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.

     std::unique_lock<std::mutex> unique_lock(_mutex);
    _condition.wait(unique_lock, [this]{
        return !_queue.empty();
    });

    T message = std::move(_queue.back());
    _queue.pop_back();

    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> lock_guard(_mutex);
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();

}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (_messages.receive() == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method âcycleThroughPhasesâ should be started in a thread when the public method âsimulateâ is called. To do this, use the thread queue in the base class. 
    std::cout << "TrafficLight::simulate() called\n";
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}



// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    // random number generation with inlcusize max, min
    const int min = 4, max = 6;
    const int cycle_duration  = rand() % (max - min + 1) + min;

    // start time (current clock time)
    auto start_time = std::chrono::high_resolution_clock::now();

    // infinite loop
    while(true) {
        // wait one ms between cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // elapsed time since start
        std::chrono::duration<double> elapsed_time = std::chrono::high_resolution_clock::now() - start_time;

        // check if elapsed time has passed the set cycle duration. If yes, send message to queue and reset cycle start time
        if (elapsed_time.count() >= cycle_duration) {
            switch (_currentPhase) {
                case red : _currentPhase = green; break;
                case green : _currentPhase = red; break;
            };

            TrafficLightPhase message = TrafficLight::getCurrentPhase();
            _messages.send(std::move(message));

            start_time = std::chrono::high_resolution_clock::now();
        }
    };
}

