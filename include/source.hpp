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
#ifndef SOURCE_HPP
#define SOURCE_HPP

#include <vector>
#include "buffer.hpp"

using namespace std;

class Stream;
class ArrStream;
class Source;

typedef enum AttrType{
  AttrType_End = -1,
  AttrType_Empty = 0,
  AttrType_Array,
};

typedef struct AttrAbs{
  AttrType type;
};

typedef struct AttrArray : AttrAbs{
  int ndim;
  vector<uint32_t> shape;
  double framerate;
};

//一般数据流
typedef struct Stream{
  uint64_t id;
  string name;
  string short_name;
  string descript;
  vector<AttrAbs *> attrs;
};

//一个数据源，能包含多个数据流
class Source{
private:
  string name;
  string uuid;
  vector<Stream *> streams;
public:
  static Source *from_config_file(string path);
  static Source *create();
  bool isLocalRelaser(); //本节点是发布节点吗
  void connect();        //连接到源，用于获取数据
  Buffer *get_buff();
  string get_name(){return name;};
  auto get_streams(){return streams;};
};

#endif
