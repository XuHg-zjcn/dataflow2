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
#include <string>
#include <fstream>
#include "buffer.hpp"

using namespace std;

//TODO: load config file *.ini
//TODO: auto-flush setting
class Writer{
private:
  ofstream ofs;
  BuffHeadRead *hr;
  framecount_t copy_block;
  size_t maxfilesize;
  string pathfmt;
  void open_newfile();
public:
  Writer(BuffHeadRead *hr, string pathfmt, framecount_t copy_block, size_t maxfilesize);
  void run();
};
