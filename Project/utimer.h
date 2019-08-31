#ifndef __UTIMER_H__
#define __UTIMER_H__

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>


class utimer {
private:
  std::chrono::system_clock::time_point start;
  std::chrono::system_clock::time_point stop;
  std::chrono::microseconds* time_elapsed;

  using usecs = std::chrono::microseconds;
  using msecs = std::chrono::milliseconds;

public:
  utimer(usecs* time_elapsed) : time_elapsed(time_elapsed)
  {
    start = std::chrono::system_clock::now();
  }

  ~utimer() {
    stop = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = stop - start;
    *time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
  }
};

#endif // __UTIMER_H__
