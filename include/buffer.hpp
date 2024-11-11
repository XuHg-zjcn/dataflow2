/*************************************************************************
 *  dataflow2 buffer
 *  Copyright (C) 2024  Xu Ruijun
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
#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstdint>
#include <cstring>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "config.hpp"

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
  inline Buffer *get_buff(){return buff;};
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
#if !WHEAD_FORCE_ONLY
  int push_block(framecount_t fc, void *p);
#endif
#if ALLOW_OVERWRITE
  int push_force(framecount_t fc, void *p);
#endif
};

class BuffHeadRead : public BuffHead{
private:
  framecount_t pop_copy_noblock_base(framecount_t fc, void *p);
#if RHEAD_BLOCK_EN
  framecount_t wait_until;
#endif
public:
  BuffHeadRead(Buffer *buff, framecount_t frame_i):BuffHead(buff, frame_i),wait_until(FRAMECOUNT_MAX){};
  BuffHeadRead(Buffer *buff):BuffHead(buff){};
  framecount_t frames_avaible(); //可用帧
  framecount_t frames_avaible_memcontine(); //
#if ALLOW_OVERWRITE
  bool hasOverwrite();
#else
  inline bool hasOverwrite(){return false;};
#endif
  int drop(framecount_t fc);
  void set_to_newest();
  framecount_t pop_copy_noblock_fail(framecount_t fc, void *p);
  framecount_t pop_copy_noblock_redu(framecount_t fc, void *p);
#if RHEAD_BLOCK_EN
  framecount_t pop_copy_block(framecount_t fc, void *p);
  void wait_frameid(framecount_t fid);
  void wait_frames(framecount_t fc);
  framecount_t wait_frames_memcontine(framecount_t fc);
  friend class BuffHeadReads;
#endif
};

class BuffHeadReads{
private:
  Buffer *buff;
  vector<BuffHeadRead *> heads;
public:
  BuffHeadReads(Buffer *buff):buff(buff){};
  framecount_t get_firstkeep();
  framecount_t get_firstwaituntil();
  BuffHeadRead *new_head();
};

class Buffer{
private:
  void *p;
  size_t f_size;
  framecount_t fc_cap;
#if !WHEAD_FORCE_ONLY
  mutex mtx_w;
  condition_variable cv_w;
  bool full; //未读数据满了，写头被阻塞
#endif
#if RHEAD_BLOCK_EN
  mutex mtx_r;
  condition_variable cv_r;
  framecount_t min_until; //直到该id才noitfy,修改需加锁
#endif
public:
  Buffer(size_t f_size, framecount_t fc_cap);
  BuffHeadWrite w_head;
  BuffHeadReads r_heads;
  friend class BuffHead;
  friend class BuffHeadRead;
  friend class BuffHeadWrite;
  inline size_t get_framesize(){return f_size;};
};

#endif
