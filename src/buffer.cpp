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
#include "buffer.hpp"
#include "config.hpp"


//member functions of BuffHead
buffindex_t BuffHead::framecount_to_buffindex(framecount_t fc)
{
  return fc%(buff->fc_cap);
}

void *BuffHead::get_ptr()
{
  return buff->p + framecount_to_buffindex(frame_i)*buff->f_size;
}


//member functions of BuffHeadWrite
framecount_t BuffHeadWrite::fcap_remain()
{
  return buff->fc_cap - (frame_i - buff->r_heads.get_firstkeep());
}

framecount_t BuffHeadWrite::fcap_remain_memcontine()
{
  framecount_t fKeep = buff->r_heads.get_firstkeep();
  framecount_t fWrite = frame_i;
  framecount_t fcapr = buff->fc_cap - (fWrite - fKeep);
  framecount_t fright = buff->fc_cap - framecount_to_buffindex(fWrite);
  if(fcapr > fright){
    return fright;
  }else{
    return fcapr;
  }
}

int BuffHeadWrite::push_noblock(framecount_t fc, void *p)
{
  if(fc > fcap_remain()){
    return -1;
  }
  //TODO: fix 填充超过缓冲区尾部
  memcpy(get_ptr(), p, fc*buff->f_size);
  frame_i += fc;
  return fc;
}

#if !WHEAD_FORCE_ONLY
int BuffHeadWrite::push_block(framecount_t fc, void *p)
{
  while(fc > 0){
    if(frame_i - buff->r_heads.get_firstkeep() == buff->fc_cap){
      buff->full = true;
      unique_lock<mutex> lock(buff->mtx_w);
      buff->cv_w.wait(lock);
    }
    framecount_t n = fcap_remain_memcontine();
    if(fc < n){
      n = fc;
    }
    memcpy(get_ptr(), p, n*buff->f_size);
    frame_i += n;
    p += buff->f_size*n;
    fc -= n;
#if RHEAD_BLOCK_EN
    if(frame_i >= buff->min_until){
      printf("push block: cv_r notify\n");
      buff->cv_r.notify_all();
    }
#endif
  }
  if(frame_i != buff->r_heads.get_firstkeep()){
    buff->full = false;
  }
  return fc;
}
#endif

#if ALLOW_OVERWRITE
int BuffHeadWrite::push_force(framecount_t fc, void *p)
{
  memcpy(get_ptr(), p, fc*buff->f_size);
  frame_i += fc;
#if RHEAD_BLOCK_EN
  if(frame_i >= buff->min_until){
    printf("push force: cv_r notify\n");
    buff->cv_r.notify_all();
  }
#endif
  return frame_i;
}
#endif


//member functions of BuffHeadRead
framecount_t BuffHeadRead::frames_avaible()
{
  return buff->w_head.getpos() - frame_i;
}

framecount_t BuffHeadRead::frames_avaible_memcontine()
{
  framecount_t fRead = frame_i;
  framecount_t fWrite = buff->w_head.getpos();
  framecount_t favaib = fWrite - fRead;
  framecount_t fright = buff->fc_cap - framecount_to_buffindex(fRead);
  if(favaib > fright){
    return fright;
  }else{
    return favaib;
  }
}

#if ALLOW_OVERWRITE
bool BuffHeadRead::hasOverwrite(){
  return buff->w_head.getpos() - frame_i > buff->fc_cap;
}
#endif

int BuffHeadRead::drop(framecount_t fc)
{
  frame_i += fc;
  return fc;
}

void BuffHeadRead::set_to_newest()
{
  frame_i = buff->w_head.getpos();
}

framecount_t BuffHeadRead::pop_copy_noblock_base(framecount_t fc, void *p)
{
  framecount_t bi = framecount_to_buffindex(frame_i);
  if(bi + fc > buff->fc_cap){
    framecount_t f1 = buff->fc_cap - bi;
    memcpy(p, get_ptr(), f1*buff->f_size);
    frame_i += f1;
    p += f1*buff->f_size;
    framecount_t f2 = fc-f1;
    memcpy(p, get_ptr(), f2*buff->f_size);
    frame_i += f2;
    p += f2*buff->f_size;
  }else{
    memcpy(p, get_ptr(), fc*buff->f_size);
    frame_i += fc;
    p += fc*buff->f_size;
  }
#if !WHEAD_FORCE_ONLY
    if(buff->full){
      buff->cv_w.notify_all();
    }
#endif
  return fc;
}

framecount_t BuffHeadRead::pop_copy_noblock_fail(framecount_t fc, void *p)
{
#if ALLOW_OVERWRITE
  if(hasOverwrite()){
    return -1;
  }
#endif
  if(frames_avaible() < fc){
    return -1;
  }
  return pop_copy_noblock_base(fc, p);
}

framecount_t BuffHeadRead::pop_copy_noblock_redu(framecount_t fc, void *p)
{
#if ALLOW_OVERWRITE
  if(hasOverwrite()){
    return -1;
  }
#endif
  framecount_t fa = frames_avaible();
  if(fa < fc){
    fc = fa;
  }
  return pop_copy_noblock_base(fc, p);
}

#if RHEAD_BLOCK_EN
framecount_t BuffHeadRead::pop_copy_block(framecount_t fc, void *p)
{
#if ALLOW_OVERWRITE
  if(hasOverwrite()){
    return -1;
  }
#endif
  framecount_t fc_ = fc;
  bool hasWait = false;
  while(fc_ > 0){
    if(frame_i == buff->w_head.getpos()){
      hasWait = true;
      wait_until = 0;
      unique_lock<mutex> lock(buff->mtx_r);
      buff->min_until = 0;
      buff->cv_r.wait(lock);
    }
    framecount_t n = frames_avaible_memcontine();
    if(fc_ < n){
      n = fc_;
    }
    memcpy(p, get_ptr(), n*buff->f_size);
    frame_i += n;
    p += n*buff->f_size;
    fc_ -= n;
#if !WHEAD_FORCE_ONLY
    if(buff->full){
      buff->cv_w.notify_all();
    }
#endif
  }
  if(hasWait){
    wait_until = FRAMECOUNT_MAX;
    unique_lock<mutex> lock(buff->mtx_r);
    buff->min_until = buff->r_heads.get_firstwaituntil();
  }
  return fc;
}

void BuffHeadRead::wait_frameid(framecount_t fid)
{
  if(buff->w_head.getpos() >= fid){
    return;
  }
  wait_until = fid;
  while(1){
    unique_lock<mutex> lock(buff->mtx_r);
    if(fid < buff->min_until){
      buff->min_until = fid;
    }
    buff->cv_r.wait(lock);
    if(buff->w_head.getpos() >= wait_until){
      wait_until = FRAMECOUNT_MAX;
      buff->min_until = buff->r_heads.get_firstwaituntil();
      break;
    }
  }
}

void BuffHeadRead::wait_frames(framecount_t fc)
{
  wait_frameid(getpos() + fc);
}

framecount_t BuffHeadRead::wait_frames_memcontine(framecount_t fc)
{
  framecount_t fright = buff->fc_cap - framecount_to_buffindex(frame_i);
  if(fright < fc){
    fc = fright;
  }
  wait_frames(fc);
  return fc;
}
#endif


//member functions of BuffHeadReads
framecount_t BuffHeadReads::get_firstkeep()
{
  framecount_t m = heads[0]->getpos();
  for(auto h: heads){
    if(h->getpos() < m){
      m = h->getpos();
    }
  }
  return m;
}

framecount_t BuffHeadReads::get_firstwaituntil()
{
  framecount_t m = FRAMECOUNT_MAX;
  for(auto h: heads){
    if(h->wait_until < m){
      m = h->wait_until;
    }
    if(m <= 0){
      return 0;
    }
  }
  return m;
}

BuffHeadRead *BuffHeadReads::new_head()
{
  BuffHeadRead *h = new BuffHeadRead(buff);
  heads.push_back(h);
  return h;
}

Buffer::Buffer(size_t f_size, framecount_t fc_cap):
  f_size(f_size), fc_cap(fc_cap),
#if RHEAD_BLOCK_EN
  min_until(FRAMECOUNT_MAX),
#endif
  w_head(this), r_heads(this)
{
  p = malloc(f_size*fc_cap);
}
