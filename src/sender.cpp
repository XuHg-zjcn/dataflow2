/*************************************************************************
 *  dataflow2 sender
 *  Copyright (C) 2025  Xu Ruijun
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
#include "sender.hpp"
#include "buffer.hpp"
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

Sender::Sender(Buffer *buff):
  buff(buff)
{
}

static void session(int fd, Buffer *buff)
{
  BuffHeadRead *hr = buff->r_heads.new_head();
  while(1){
    framecount_t fc = hr->wait_frames_memcontine_timeout(20, 100);
    send(fd, hr->get_ptr(), fc*buff->get_framesize(), 0);
    hr->drop(fc);
  }
}

void Sender::start_server()
{
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9871);
  addr.sin_addr.s_addr = htonl(0);
  socklen_t socklen = sizeof(addr);
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0){
    perror("create socket failed");
  }
  if(bind(sockfd, (struct sockaddr *)&addr, socklen) < 0){
    perror("bind failed");
  }
  if(listen(sockfd, 0) < 0){
    perror("listen failed");
  }
  printf("server readly\n");
  while(1){
    char iptext[INET_ADDRSTRLEN];
    int fd = accept(sockfd, (struct sockaddr *)&addr, &socklen);
    inet_ntop(AF_INET, &addr.sin_addr, iptext, INET_ADDRSTRLEN);
    printf("connect from %s:%d\n", iptext, ntohs(addr.sin_port));
    session(fd, this->buff);
  }
}
