#ifndef __FARM_UTILITY_H__
#define __FARM_UTILITY_H__

#include<cstddef>

class Task{
private:
  int data;
  size_t task_id; // TODO: if you want to use an ordered farm
  bool is_EOS;    // TODO: eliminare e semplicemente restituire un puntatore nullo

public:
  Task(int data) : data(data), task_id(-1), is_EOS(false) {};
  Task(int data, size_t task_id): data(data), task_id(task_id), is_EOS(false) {};
  static Task* EOS(){
    return new Task(true);
  }

  // getters
  int getData(){ return data; }
  size_t getTaskId(){ return task_id; }
  bool isEOS(){ return is_EOS; }

private:
  Task(bool is_EOS): is_EOS(is_EOS) {};
};

/*
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    std::chrono::system_clock::time_point tic, toc;

    tic = std::chrono::system_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    toc = std::chrono::system_clock::now();

    std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic);

    std::cout << elapsed_time.count() << std::endl;

    std::chrono::milliseconds eos(-1);
    std::cout << eos.count() << std::endl;

    std::chrono::milliseconds* eos_ptr = new std::chrono::milliseconds(-1);
    std::cout << eos_ptr->count() << std::endl;

}

 */

#endif //__FARM_UTILITY_H__
