#ifndef __SAFE_QUEUE_H__
#define __SAFE_QUEUE_H__

#include <limits>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>

template <class T> class SafeQueue{
    private:
        std::mutex d_mutex;
        std::condition_variable p_condition;        //producer
        std::condition_variable c_condition;        //consumer
        std::queue<T> queue;  //initialize a local queue
        const size_t max_size;

    public:
        SafeQueue(size_t my_size) : max_size(my_size) {};
        SafeQueue() : max_size(std::numeric_limits<unsigned int>::max()) {};

        void safePush(T item){  //max size
            std::unique_lock<std::mutex> lock(d_mutex);
            this->p_condition.wait(lock, [=]{return this->queue.size() < this->max_size;});   //if holds, it goes through
            this->queue.push(item);
            c_condition.notify_one();       //wake a consumer since 1 element has been pushed in
        }

        bool trySafePush(T item){
            if(d_mutex.try_lock()){
                this->queue.push(item);
                c_condition.notify_one();
                return true;
            }
            return false;
        }

        size_t safeSize(){
             std::unique_lock<std::mutex> lock(d_mutex);
             return this->queue.size();
        }

        T safePop(){
            std::unique_lock<std::mutex> lock(d_mutex);
            this->c_condition.wait(lock, [=]{return !this->queue.empty();});   //if holds, it goes through
            T popped = this->queue.front();
            this->queue.pop();
            p_condition.notify_one();      //wake a producer since 1 element has been popped from
            return popped;
        }

        bool isEmpty(){
            std::unique_lock<std::mutex> lock(d_mutex);
            return this->queue.empty();
        }

        size_t maxSize(){
          return this->max_size;
        }

        void empty_and_print(){
            while(!this->queue.empty()){
                T popped = this->queue.front();
                this->queue.pop();
                std::cout << popped << std::endl;
            }
        }

};

#endif // __SAFE_QUEUE_H__
