#include <iostream>
#include <random>
#include <functional> // required fot std::bind
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  std::unique_lock<std::mutex> uLock(_mutex);
  _condition.wait(uLock, [this] { return !_queue.empty(); });
  
  // remove last element from the queue
  T message = std::move(_queue.back());
  _queue.pop_back();
  
  return message;
}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  // perform queue modification under the lock
  std::lock_guard<std::mutex> uLock(_mutex);
  
  // add item to queue
  _queue.push_back(std::move(msg));
  _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
  	_type = ObjectType::objectTrafficLight;
  	_messageQueue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  
  while (true)
  {
    
    TrafficLightPhase trafficLightPhase = _messageQueue->receive();
    if (trafficLightPhase == TrafficLightPhase::green) {
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
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  double minTrafficLightDuration = 4000; // milliseconds
  double maxTrafficLightDuration = 6000;
  double trafficLightDuration = getRandomValueInRange(minTrafficLightDuration, maxTrafficLightDuration);
  
  std::chrono::time_point<std::chrono::system_clock> startTime;
  // initialize stop watch
  
  startTime = std::chrono::system_clock::now();
  
  while(true) 
  {
    long timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count();
    
    if (timeSinceStart >= trafficLightDuration) 
    {
      toggleCurrentPhase();
      TrafficLightPhase currentPhase = getCurrentPhase();
      _messageQueue->send(std::move(currentPhase));
      
      // wait for 1 millisecond between two cycles
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      startTime = std::chrono::system_clock::now();
      trafficLightDuration = getRandomValueInRange(minTrafficLightDuration, maxTrafficLightDuration);
    }
  }
  
  
  
}


double TrafficLight::getRandomValueInRange(double low, double high) 
{
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  auto rand = std::bind(std::uniform_real_distribution<double>{low, high},
                        std::mt19937(seed));
  return rand();
}

void TrafficLight::toggleCurrentPhase()
{
  if (_currentPhase == TrafficLightPhase::red) {
    _currentPhase = TrafficLightPhase::green;
  } else {
    _currentPhase = TrafficLightPhase::red;
  }
}