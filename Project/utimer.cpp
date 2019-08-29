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
  std::string message;

  using usecs = std::chrono::microseconds;
  using msecs = std::chrono::milliseconds;

public:
  utimer(const std::string m, usecs* time_elapsed) : message(m)
                                                     time_elapsed(time_elapsed)
  {
    start = std::chrono::system_clock::now();
  }

  ~utimer() {
    stop = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = stop - start;
    *time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);

    std::cout << message << " computed in " << time_elapsed->count() << " usec " << std::endl;

  }
};
