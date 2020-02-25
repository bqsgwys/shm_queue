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

int main() {
  // create segment and corresponding allocator
  managed_shared_memory segment(open_or_create, "brandwidth", 64 * 1048576);
  char_alloc char_alloc(segment.get_segment_manager());

  ring_buffer *queue = segment.find_or_construct<ring_buffer>("queue")();
  while (true) {
    shm::shared_string v(char_alloc);
    queue->pop(v);
  }
}