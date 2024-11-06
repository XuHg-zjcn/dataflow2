/*************************************************************************
 *  misc code of dataflow2
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
#include "misc.h"
#include <stdint.h>
#include <stddef.h>

static uint8_t buff[80];

char *itoa_x(int value, char *str, int radix, int length, char fill)
{
  if(radix < 2 || radix > 36){
    return NULL;
  }
  if(value < 0){
    //neg unsupported
    return NULL;
  }
  int n = 0;
  while(value != 0){
    buff[n] = value % radix;
    value /= radix;
    n++;
  }
  if(n > length){
    length = n;
  }
  for(int i=0;i<length-n;i++){
    str[i] = fill;
  }
  for(int i=length-n;i<length;i++){
    uint8_t x = buff[length-i-1];
    char ch = (x<10)?('0'+x):('a'+(10-x));
    str[i] = ch;
  }
  str[length] = '\0';
  return str;
}

