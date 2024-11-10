/*************************************************************************
 *  Config file for dataflow2
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
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <stdint.h>
#ifdef EXTERN_CONFIG_FILE
#include "config_dataflow2.hpp"
#endif


typedef int32_t framecount_t;
typedef int32_t buffindex_t;

#ifndef THREAD_SYNC_EN
#define THREAD_SYNC_EN    1 //使用线程同步
#endif

#ifndef ALLOW_OVERWRITE
#if defined(WHEAD_FORCE_ONLY) && (WHEAD_FORCE_ONLY==1)
#define ALLOW_OVERWRITE   1
#else
#define ALLOW_OVERWRITE   0 //允许覆盖
#endif
#endif

#ifndef WHEAD_FORCE_ONLY
#define WHEAD_FORCE_ONLY  0 //写头总是强制
#endif

#ifndef REHAD_BLOCK_EN
#define RHEAD_BLOCK_EN    1 //读头等待开启
#endif

#if (WHEAD_FORCE_ONLY && !ALLOW_OVERWRITE)
#error "enable WHEAD_FORCE_ONLY must ALLOW_OVERWRITE"
#endif

#if (!THREAD_SYNC_EN && !WHEAD_FORCE_ONLY)
#error "w_head not force-only mode must THREAD_SYNC_EN"
#endif

#if (!THREAD_SYNC_EN && RHEAD_BLOCK_EN)
#error "enable r_head blocking must THREAD_SYNC_EN"
#endif

#endif
