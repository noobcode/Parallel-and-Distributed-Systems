#ifndef __FARM_UTILITY_H__
#define __FARM_UTILITY_H__

#include<cstddef>

class Task{
private:
  int data;
  size_t task_id; // TODO: if you want to use an ordered farm
  bool is_EOS;

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

#endif //__FARM_UTILITY_H__
