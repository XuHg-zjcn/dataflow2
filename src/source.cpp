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
#include "cjson/cJSON.h"
#include <cstdio>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "source.hpp"

template <typename T>
static size_t from_cJSON_array(const cJSON *json, vector<T> &v){
  int s = cJSON_GetArraySize(json);
  cJSON *item = NULL;
  cJSON_ArrayForEach(item, json){
    if(cJSON_IsNumber(item)){
      v.push_back(item->valueint);
    }
  }
  return s;
}

static AttrArray *getAttrArray(cJSON *item_array)
{
  int ndims;
  cJSON *item2;
  AttrArray *a = new AttrArray();
  a->type = AttrType_Array;
  item2 = cJSON_GetObjectItemCaseSensitive(item_array, "shape");
  if(cJSON_IsArray(item2)){
    a->ndim = from_cJSON_array(item2, a->shape);
  }
  item2 = cJSON_GetObjectItemCaseSensitive(item_array, "fr");
  if(cJSON_IsNumber(item2)){
    a->framerate = item2->valuedouble;
  }
  return a;
 err:
  delete a;
}

Source *Source::from_config_file(string path)
{
  struct stat sb;
  char *pBuff;
  int res, fd;
  if(path.length() < 5){
    return NULL;
  }
  //check vaild path and get file size
  stat(path.c_str(), &sb);
  if(!S_ISREG(sb.st_mode)) {
    return NULL;
  }

  //read full file to memory
  fd = open(path.c_str(), O_RDONLY);
  if(fd < 0){
    perror("open failed");
    return NULL;
  }
  pBuff = (char*)malloc(sb.st_size+1);
  if(pBuff == NULL){
    return NULL;
  }
  res = read(fd, pBuff, sb.st_size);
  if(res != sb.st_size){
    free(pBuff);
    fprintf(stderr, "read file size mistake: %ld -> %d\n", sb.st_size, res);
    return NULL;
  }
  close(fd);

  //parse json
  const cJSON *item=NULL;
  cJSON *json = cJSON_Parse(pBuff);
  free(pBuff);
  if(json == NULL){
    const char *errmsg = cJSON_GetErrorPtr();
    if(errmsg != NULL){
      fprintf(stderr, "Error before: %s\n", errmsg);
      return nullptr;
    }
  }

  Source *retObj = new Source;
  item = cJSON_GetObjectItemCaseSensitive(json, "name");
  if(cJSON_IsString(item) && (item->valuestring != NULL)){
    retObj->name = item->valuestring;
  }
  item = cJSON_GetObjectItemCaseSensitive(json, "streams");

  cJSON *item_stream = NULL;
  cJSON_ArrayForEach(item_stream, item){
    cJSON *item_attr;
    Stream *s = new Stream();
    item_attr = cJSON_GetObjectItemCaseSensitive(item_stream, "id");
    if(cJSON_IsNumber(item_attr)){
      s->id = item_attr->valueint;
    }
    item_attr = cJSON_GetObjectItemCaseSensitive(item_stream, "name");
    if(cJSON_IsString(item_attr)){
      s->name = item_attr->valuestring;
    }
    item_attr = cJSON_GetObjectItemCaseSensitive(item_stream, "short_name");
    if(cJSON_IsString(item_attr)){
      s->short_name = item_attr->valuestring;
    }
    item_attr = cJSON_GetObjectItemCaseSensitive(item_stream, "array");
    if(cJSON_IsObject(item_attr)){
      AttrArray *a = getAttrArray(item_attr);
      if(a != NULL){
	s->attrs.push_back(a);
      }
    }
    retObj->streams.push_back(s);
  }
 end:
  cJSON_Delete(json);
  return retObj;
}
