/*************************************************************************
 *  Test program of dataflow2
 *  Copyright (C) 2024-2025  Xu Ruijun
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ************************************************************************/
#include <thread>
#include <cstdlib>
//#include <iostream>
#include "buffer.hpp"
#include "writer.hpp"
#include "sender.hpp"

using namespace std;

//多线程使用cout会出现消息混合现象，因此改用printf
void Pub(BuffHeadWrite *hw, string tag)
{
  for(int i=0;i<1000;i++){
    uint64_t data = i;
    hw->push_force(1, &data);
    //cout << "put data:" << data << endl;
    //printf("%s%ld\n", tag.c_str(), data);
    this_thread::sleep_for(chrono::milliseconds(100));
    //TODO: 注释掉延时函数会导致未全部发送就卡住问题，待修复
  }
}

void SubCopy(BuffHeadRead *hr, string tag)
{
  while(1){
    uint64_t data;
    hr->pop_copy_block(1, &data);
    //cout << tag << val << endl;
    printf("%s%ld\n", tag.c_str(), data);
  }
}

void SubNoCopy(BuffHeadRead *hr, string tag)
{
  while(1){
    framecount_t fc = hr->wait_frames_memcontine_timeout(4, 50);
    uint64_t *data = (uint64_t *)hr->get_ptr();
    printf("%s get %d:", tag.c_str(), fc);
    for(int i=0;i<fc;i++){
      printf("%ld,", data[i]);
    }
    printf("\n");
    hr->drop(fc);
  }
}

void Send(Buffer *buff)
{
  Sender s = Sender(buff);
  s.start_server();
}

void Rec(BuffHeadRead *hr, string pathfmt)
{
  Writer writer(hr, pathfmt, 10, 1024);
  writer.run();
}

int main()
{
  Buffer buff = Buffer(8, 100);
  thread pub(Pub, &buff.w_head, "pub : ");
  //thread sub1(SubCopy, buff.r_heads.new_head(), "sub_c: ");
  //thread sub2(SubNoCopy, buff.r_heads.new_head(), "subnc: ");
  thread rec(Rec, buff.r_heads.new_head(), "data/rec_%Y%m%d_%#.dat");
  thread send(Send, &buff);
  pub.join();
  //sub1.join();
  //sub2.join();
  rec.join();
  return 0;
}
