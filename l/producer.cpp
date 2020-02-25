#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/lockfree/spsc_queue.hpp>

namespace shm {
using namespace boost::interprocess;
using boost::interprocess::allocator;
using namespace boost::lockfree;
using boost::interprocess::managed_shared_memory;
typedef allocator<char, managed_shared_memory::segment_manager> char_alloc;
typedef basic_string<char, std::char_traits<char>, char_alloc> shared_string;
typedef spsc_queue<shared_string, capacity<64> > ring_buffer;
}  // namespace shm

#include <unistd.h>

#include <ctime>
#include <iostream>
#include <string>
using namespace shm;
int main() {
  managed_shared_memory segment(open_or_create, "lag", 128 * 1048576);
  char_alloc char_alloc(segment.get_segment_manager());
  ring_buffer* queue = segment.find_or_construct<ring_buffer>("queue")();
  int timex = 0;
  unsigned long long avgs = 0, avgns = 0;
  timespec x;
  shared_string message("x", char_alloc);
  for (int i = 0; i < 10; i++) message += message;  // 1MB data
  while (timex < 100000) {
    if (queue->write_available()) {
      timex++;
      clock_gettime(CLOCK_REALTIME, &x);
      *((long*)&message[0]) = x.tv_sec;
      *((long*)&message[5]) = x.tv_nsec;
      queue->push(message);
      sleep(1);
    }
  }
}