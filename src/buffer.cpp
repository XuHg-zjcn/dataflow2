#include "buffer.hpp"

buffindex_t BuffHead::framecount_to_buffindex(framecount_t fc)
{
  return fc%(buff->fc_cap);
}

void *BuffHead::get_ptr()
{
  return buff->p + framecount_to_buffindex(frame_i)*buff->f_size;
}

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
  memcpy(get_ptr(), p, fc*buff->f_size);
  frame_i += fc;
}

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
    //unique_lock<mutex> lock(buff->mtx_r);
    buff->cv_r.notify_all();
  }
  if(frame_i != buff->r_heads.get_firstkeep()){
    buff->full = false;
  }
}

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

int BuffHeadRead::drop(framecount_t fc)
{
  frame_i += fc;
}

void BuffHeadRead::pop_copy_block(framecount_t fc, void *p)
{
  while(fc > 0){
    if(frame_i == buff->w_head.getpos()){
      unique_lock<mutex> lock(buff->mtx_r);
      buff->cv_r.wait(lock);
    }
    framecount_t n = frames_avaible_memcontine();
    if(fc < n){
      n = fc;
    }
    memcpy(p, get_ptr(), n*buff->f_size);
    frame_i += n;
    p += n*buff->f_size;
    fc -= n;
    if(buff->full){
      buff->cv_w.notify_all();
    }
  }
}

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

BuffHeadRead *BuffHeadReads::new_head()
{
  BuffHeadRead *h = new BuffHeadRead(buff);
  heads.push_back(h);
  return h;
}

Buffer::Buffer(size_t f_size, framecount_t fc_cap):
  f_size(f_size), fc_cap(fc_cap), w_head(this), r_heads(this)
{
  p = malloc(f_size*fc_cap);
}
