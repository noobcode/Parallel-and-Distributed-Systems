#ifndef __FARM_COLLECTOR_H__
#define __FARM_COLLECTOR_H__

#include "./farm_utility.h"
#include "./safe_queue.h"
#include <thread>
#include <chrono>
#include "./utimer.h"

class FarmCollector{
private:
  unsigned int max_nw;
  SafeQueue<Task*>* input_stream;
  SafeQueue<int>* output_stream;
  std::thread* collector_thread;

  std::chrono::microseconds elapsed_time;
  std::vector<std::chrono::microseconds> elapsed_time_history;

public:
  FarmCollector(unsigned int max_nw,
                SafeQueue<Task*>* input_stream,
                SafeQueue<int>* output_stream) : max_nw(max_nw),
                                                 input_stream(input_stream),
                                                 output_stream(output_stream) {};

  void body(){
    size_t count_EOS = 0;
    while(count_EOS < max_nw){
      {
        utimer timer(&elapsed_time);
        Task* t = input_stream->safePop();
        if(t->isEOS()){
          count_EOS++;
          continue;
        }
        else
          output_stream->safePush(t->getData());
      }
      elapsed_time_history.push_back(elapsed_time);
    }
  }

  void run(){
    collector_thread = new std::thread(&FarmCollector::body, this);
  }

  void join(){
    collector_thread->join();
  }

  std::vector<std::chrono::microseconds> getElapsedTimeHistory(){
    return elapsed_time_history;
  }

  void printCollector(){
    std::cout << "==== COLLECTOR ====" << std::endl;
    std::cout << "max_nw:" << max_nw << std::endl;

    std::cout << "(Workers->Collector) input_stream: address/" << input_stream
              << " - size/" << input_stream->safeSize()
              << " - max_size/" << input_stream->maxSize() << std::endl;

    std::cout << "(results) output_stream: address/" << output_stream
              << " - size/" << output_stream->safeSize()
              << " - max_size/" << output_stream->maxSize() << std::endl;
  }


};


#endif // __FARM_COLLECTOR_H__
