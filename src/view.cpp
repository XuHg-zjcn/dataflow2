/*************************************************************************
 *  view source information json program of dataflow2
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
#include <stdio.h>
#include <iostream>
#include "source.hpp"

using namespace std;

int main(int argc, char **argv)
{
  if(argc != 2){
    printf("must one parames\n");
    return 1;
  }

  Source *source = Source::from_config_file(argv[1]);
  cout << "source info" << endl;
  cout << "name:" << source->get_name() << endl;
  cout << endl;
  cout << "streams info" << endl;
  for(auto stream : source->get_streams()){
    cout << "name: " << stream->name << endl;
    cout << "desc: " << stream->descript << endl;
    cout << "attrs: " << endl;
    for(auto *a : stream->attrs){
      if(a->type == AttrType_Array){
	AttrArray *a2 = (AttrArray *)a;
	cout << "ndim:" << a2->ndim << endl;
	cout << "shape:";
	for(auto elems : a2->shape){
	  cout << elems << ",";
	}
	cout << endl;
	cout << "framerate:" << a2->framerate << endl;
      }
    }
    cout << endl;
  }
}
