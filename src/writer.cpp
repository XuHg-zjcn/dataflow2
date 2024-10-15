/*************************************************************************
 *  dataflow2 writer
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
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "writer.hpp"
#include "misc.h"

using namespace std;

Writer::Writer(BuffHeadRead *hr,
	       string pathfmt,
	       framecount_t copy_block,
	       size_t maxfilesize):
  hr(hr),copy_block(copy_block),maxfilesize(maxfilesize),pathfmt(pathfmt)
{
  open_newfile();
}

static int path_num(const string &path)
{
  string dirpath, filename;
  int num = 0;
  int x = path.rfind('/');
  if(x != string::npos){
    dirpath = path.substr(0, x);
    filename = path.substr(x+1);
  }else{
    dirpath = ".";
    filename = path;
  }
  struct stat sb;
  DIR *dir;
  if(stat(dirpath.c_str(), &sb) == 0 &&
     S_ISDIR(sb.st_mode) &&
     (dir=opendir(dirpath.c_str()))!=nullptr){
    while(1){
      struct dirent *dire = readdir(dir);
      if(dire == NULL){
	break;
      }
      if(strlen(dire->d_name)>=filename.length() &&
	 strncmp(dire->d_name, filename.c_str(), filename.length()) == 0){
	int n_ = atoi(dire->d_name + filename.length());
	if(n_ > num){
	  num = n_;
	}
      }
    }
  }
  num += 1;
  return num;
}

void Writer::open_newfile()
{
  bool has_time = false;
  bool has_num = false;
  size_t len = pathfmt.length();
  struct tm *tm;
  int x = -1;
  while(1){
    x = pathfmt.find('%', x+1);
    if(x == string::npos){
      break;
    }
    if(x+1 >= len){
      break;
    }
    char ch = pathfmt[x+1];
    if(string("YmdHMS").find(ch) != string::npos){
      has_time = true;
    }
    if(ch == '#'){
      has_num = true;
    }
  }
  if(has_time){
    time_t t;
    time(&t);
    tm = localtime(&t);
  }
  string path;
  for(int i=0;i<pathfmt.length();i++){
    if(pathfmt[i] != '%'){
      path.push_back(pathfmt[i]);
    }else if(i+1<pathfmt.length()){
      char buff[16];
      switch(pathfmt[i+1]){
      case 'Y':
	path.append(itoa_x(1900+tm->tm_year, buff, 10, 4, '0'));
	break;
      case 'm':
	path.append(itoa_x(tm->tm_mon, buff, 10, 2, '0'));
	break;
      case 'd':
	path.append(itoa_x(tm->tm_mday, buff, 10, 2, '0'));
	break;
      case 'H':
	path.append(itoa_x(tm->tm_hour, buff, 10, 2, '0'));
	break;
      case 'M':
	path.append(itoa_x(tm->tm_min, buff, 10, 2, '0'));
	break;
      case 'S':
	path.append(itoa_x(tm->tm_sec, buff, 10, 2, '0'));
	break;
      case '#':
	path.append(itoa_x(path_num(path), buff, 10, 0, '0'));
	break;
      default:
	path.push_back('%');
	path.push_back(pathfmt[i+1]);
	break;
      }
      i++;
    }
  }
  //TODO: fix: 目录不存在时自动递归创建
  cout << "Open " << path << endl;
  if(ofs.is_open()){
    ofs.close();
  }
  ofs.open(path, ios::out|ios::binary);
}

void Writer::run()
{
  while(1){
    if(ofs.tellp() >= maxfilesize){
      open_newfile();
    }
    void *p = hr->get_ptr();
    framecount_t fc = hr->wait_frames_memcontine(copy_block);
    ofs.write((const char *)p, fc*hr->get_buff()->get_framesize());
    hr->drop(fc);
  }
}
