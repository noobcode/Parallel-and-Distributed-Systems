#ifndef __FARM_COLLECTOR_H__
#define __FARM_COLLECTOR_H__

#include "./task.h"
#include "./safe_queue.h"
#include <thread>

class FarmCollector{
private:
  unsigned int max_nw;
  SafeQueue<Task*>* input_stream;
  SafeQueue<int>* output_stream;
  std::thread* collector_thread;

public:
  FarmCollector(unsigned int max_nw,
                SafeQueue<Task*>* input_stream,
                SafeQueue<int>* output_stream) : max_nw(max_nw),
                                                 input_stream(input_stream),
                                                 output_stream(output_stream) {};

  static void body(unsigned int max_nw,
                SafeQueue<Task*>* input_stream,
                SafeQueue<int>* output_stream){

    size_t count_EOS = 0;
    while(count_EOS < max_nw){
      Task* t = input_stream->safePop();
      if(t->isEOS()){
        count_EOS++;
      }
      else{
        output_stream->safePush(t->getData());
      }
    }
  }

  void run(){
    collector_thread = new std::thread(body, max_nw, input_stream, output_stream);
  }

  void join(){
    collector_thread->join();
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
