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
typedef spsc_queue<shared_string, capacity<32> > ring_buffer;
}  // namespace shm

#include <iostream>
using namespace shm;
int cnt = 0;
int sec = 0;
int nsec = 0;
int main() {
  // create segment and corresponding allocator
  managed_shared_memory segment(open_or_create, "lag", 128 * 1048576);
  char_alloc char_alloc(segment.get_segment_manager());

  ring_buffer *queue = segment.find_or_construct<ring_buffer>("queue")();

  timespec x;
  while (true) {
    shared_string v(char_alloc);
    if (queue->pop(v)) {
      clock_gettime(CLOCK_REALTIME, &x);
      std::cout << v.length() << std::endl;
      if (v.length() > 10) {
        cnt++;
        sec += x.tv_sec - *(long *)(&v[0]);
        nsec += x.tv_nsec - *(long *)(&v[5]);
        std::cout << sec << " " << nsec << " " << cnt << " " << v.length()
                  << std::endl;
      }
    }
  }
}