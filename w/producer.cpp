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

#include <unistd.h>

#include <ctime>
#include <iostream>
#include <string>
using namespace shm;
int main() {
  managed_shared_memory segment(open_or_create, "brandwidth", 64 * 1048576);
  char_alloc char_alloc(segment.get_segment_manager());
  ring_buffer* queue = segment.find_or_construct<ring_buffer>("queue")();

  std::string message = "1";
  for (int i = 0; i < 20; i++) message += message;  // 1MB data
  auto data = shared_string(message.c_str(), char_alloc);
  time_t clk = clock();
  int timex = 0, sz = 0;
  std::cout << CLOCKS_PER_SEC << " " << clk << std::endl;
  while (1) {
    if (queue->write_available()) {
      queue->push(data);
      timex++;
      if (clock() - clk > CLOCKS_PER_SEC) break;
    }
  }
  std::cout << timex;
}