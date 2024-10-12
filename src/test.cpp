#include <thread>
#include <cstdlib>
//#include <iostream>
#include "buffer.hpp"

using namespace std;

//多线程使用cout会出现消息混合现象，因此改用printf
void Pub(BuffHeadWrite *hw, string tag)
{
  for(int i=0;i<10;i++){
    uint64_t data = i;
    hw->push_block(1, &data);
    //cout << "put data:" << data << endl;
    printf("%s%ld\n", tag.c_str(), data);
    this_thread::sleep_for(chrono::seconds(1));
  }
}

void Sub(BuffHeadRead *hr, string tag)
{
  while(1){
    uint64_t data;
    hr->pop_copy_block(1, &data);
    //cout << tag << val << endl;
    printf("%s%ld\n", tag.c_str(), data);
  }
}

int main()
{
  Buffer buff = Buffer(8, 5);
  thread pub(Pub, &buff.w_head, "pub : ");
  thread sub1(Sub, buff.r_heads.new_head(), "sub1: ");
  thread sub2(Sub, buff.r_heads.new_head(), "sub2: ");
  pub.join();
  sub1.join();
  sub2.join();
  return 0;
}
