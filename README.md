# dataflow2
我遇到很多实时数据流，如音视频、心电波形、大气温湿度、IMU惯性传感器数据等，每次都要为了这些数据编写一些专用程序进行储存、传输、显示等，很麻烦。
我想建立一个通用框架，来方便处理各种数据，我把这类数据称为array-like数据，其中一个axis为时间，允许不同元素有不同量纲，但必须数据类型相同，且能组成数组。
有些数据流可以拆分为多条array-like数据流，这样也可以适用此框架了。

| 名称 |进度| 描述 |                       |
|:---:|:---:|:-------                      |
|Buffer|可用，待完善| 一进多出队列容器，共享缓冲区|
|Writer|可用，待完善| 数据持久化写入器 |
|Sender|还未开始    | 数据发送器       |


## 目标功能
- 数据压缩(无损/有损)
- 数据带自述(想支持HDF格式，但过于复杂，或许采用JSON描述)
- 空间不足下自动删除已导出数据
- 空间不足且数据均未导出，可选择降低历史数据分辨率
- 移植到ESP32, STM32等MCU
- 配套可视化工具


## 版权和许可证
Copyright (C) 2024 徐瑞骏

程序部分以GNU通用公共许可证第三版或更新(GNU GPLv3 or later)授权

本READ采用以下许可证
```
Copyright (C) 2024 Xu Ruijun

Copying and distribution of this README file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.
```
