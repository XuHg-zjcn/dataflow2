#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstdint>
#include <cstring>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <config.hpp>

using namespace std;

typedef struct{
  uint32_t ndim;
  uint32_t *shape;
  uint32_t elembits;
}frameshape;

class Buffer;

class BuffHead{
protected:
  Buffer *buff;
  framecount_t frame_i;//待写入读取的frame号
public:
  BuffHead(Buffer *buff, framecount_t frame_i):buff(buff), frame_i(frame_i){};
  BuffHead(Buffer *buff):buff(buff), frame_i(0){};
  inline framecount_t getpos(){return frame_i;};
  buffindex_t framecount_to_buffindex(framecount_t fc);
  void *get_ptr();
};

class BuffHeadWrite : public BuffHead{
public:
  BuffHeadWrite(Buffer *buff, framecount_t frame_i):BuffHead(buff, frame_i){};
  BuffHeadWrite(Buffer *buff):BuffHead(buff){};
  framecount_t fcap_remain();
  framecount_t fcap_remain_memcontine();
  int push_noblock(framecount_t fc, void *p);
  int push_block(framecount_t fc, void *p);
};

class BuffHeadRead : public BuffHead{
public:
  
  BuffHeadRead(Buffer *buff, framecount_t frame_i):BuffHead(buff, frame_i){};
  BuffHeadRead(Buffer *buff):BuffHead(buff){};
  framecount_t frames_avaible(); //可用帧
  framecount_t frames_avaible_memcontine(); //
  int drop(framecount_t fc);
  framecount_t set_to_newest();
  void pop_copy_block(framecount_t fc, void *p);
  void pop_copy_noblock(framecount_t fc, void *p);
};

class BuffHeadReads{
private:
  Buffer *buff;
  vector<BuffHeadRead *> heads;
public:
  BuffHeadReads(Buffer *buff):buff(buff){};
  framecount_t get_firstkeep();
  BuffHeadRead *new_head();
};

class Buffer{
private:
  void *p;
  size_t f_size;
  framecount_t fc_cap;
  mutex mtx_w;
  condition_variable cv_w;
  mutex mtx_r;
  condition_variable cv_r;
  bool full;
  bool hasEmpty;
public:
  Buffer(size_t f_size, framecount_t fc_cap);
  BuffHeadWrite w_head;
  BuffHeadReads r_heads;
  friend class BuffHead;
  friend class BuffHeadRead;
  friend class BuffHeadWrite;
};

#endif
