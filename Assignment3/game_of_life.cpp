#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "./CImg.h"
#include "./safe_queue.h"
using namespace std;
using namespace cimg_library;

std::atomic_int barrier = {0};

// barrier condition variable and lock
std::mutex d_mutex;
std::unique_lock<std::mutex> barrier_lock(d_mutex);
std::condition_variable barrier_condition;
typedef struct Rect{int x1, y1, x2, y2; } Rect;

// rectangle queue
SafeQueue<Rect> rectangles;

// notable constants and types
using board_t = cimg_library::CImg<unsigned char>;
const unsigned int alive = 000;
const unsigned int dead  = 255;

// used to compute neighbors position in circular mesh
long mod(long a, long b){return (a%b+b)%b;}

// thread subroutine
int neighbors_alive(board_t* board, int n, int i, int j){
  int alive_counter = 0;
  for(int y = i-1; y <= i+1; y++)
    for(int x = j-1; x <= j+1; x++)
      if (y==i & x==j)
        continue;
      else
        alive_counter += (*board)(mod(y,n),mod(x,n),0,0) == alive ? 1:0;

  return alive_counter;
}

// thread body function
void rule(board_t* board, board_t* next_board, Rect r, int n){
  for(int i = r.y1; i < r.y2; i++){
    for(int j = r.x1; j < r.x2; j++){
      int count = neighbors_alive(board, n, i, j);
      if((*next_board)(i,j,0,0) == alive){
        if(count < 2 || count > 3)
          (*next_board)(i,j,0,0) = dead;
      } else {
        if(count == 3 && (*next_board)(i,j,0,0) == dead)
          (*next_board)(i,j,0,0) = alive;
      }
    }
  }
}

void worker_task(board_t* board, board_t* new_board, int n){
  while(true){
    Rect r = rectangles.safe_pop();
    if(r.x1 == -1) break;
    rule(board, new_board, r, n);
    barrier--;
    barrier_condition.notify_one();
  }
}

void fill_queue(SafeQueue<Rect>* rectangles, int n, int nw){
  int size = n / nw;
  for(int i = 0; i < nw; ++i){
    Rect r;
    r.y1 = 0;
    r.y2 = n-1;
    r.x1 = i == n-1 ? n-1 : i * size;
    r.x2 = i == 0 ? size-1 : ((i+1)*size) - 1;
    rectangles->safe_push(r);
  }
}

int main(int argc, char * argv[]) {

  if(argc < 5) {
    cout << "Usage is: " << argv[0] << " n seed iter nw" << endl;
    return(0);
  }
  // get matrix dimensions from the command line
  int n = atoi(argv[1]);
  int seed = atoi(argv[2]);
  int iter = atoi(argv[3]);
  int nw = atoi(argv[4]);

  cout << "Using " << n << "x" << n << " board " << endl;

  // workers queue
  std::vector<std::thread> workers(nw);

  // create an empty boards
  board_t board(n,n,1,1,0);
  board_t new_board(n,n,1,1,0);

  // initialize it randomly
  srand(seed);
  for(int i=0; i<n; i++)
    for(int j=0; j<n; j++)
      board(i,j,0,0) = (rand() % 32 == 0 ? alive : dead);

  CImgDisplay main_displ(board,"Init");
  sleep(2);  // show for two seconds

  // start thread pool
  for(int i = 0; i < nw; i++)
    workers[i] = std::thread(worker_task, &board, &new_board, n);

  for(int i = 0; i < iter; i++){
    std::cout << i << '\n';
    // create tasks
    fill_queue(&rectangles, n, nw);
    barrier = rectangles.safe_size();

    // wait end of iteration
    while(barrier > 0){
      barrier_condition.wait(barrier_lock);
    }

    // swap boards
    board = new_board;

    // display
    main_displ.display(board);
    sleep(1);
  }

  // send EOS
  for(int i=0; i < nw; i++){
    Rect r = {-1, -1, -1, -1};
    rectangles.safe_push(r);
  }

  for(int i =0; i < nw; i++)
    workers[i].join();

  return 0;

}
