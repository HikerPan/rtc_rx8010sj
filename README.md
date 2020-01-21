# rtc_8010

## 1、介绍

rtc_8010是基于RT-Thread实现的rx8010sj软件包，时钟/Alarm的API接口。

### 1.1 特性

1. 代码简洁易懂，充分使用RT-Thread提供的API
2. 详细注释
3. 线程安全
4. 断言保护
5. API操作简单

### 1.2 目录结构

| 名称     | 说明             |
| -------- | ---------------- |
| examples | 例子目录         |
| docs     | rx8010sj芯片手册 |
| inc      | 头文件           |
| src      | 源文件           |

### 1.3 许可证

agile_led package 遵循 LGPLv2.1 许可，详见 `LICENSE` 文件。

### 1.4 依赖

- RT-Thread 3.0+
- RT-Thread 4.0+

## 2、如何打开 rtc_rx8010sj

使用 agile_led package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    miscellaneous packets --->
        [*] rx8010sj driver for RT-Thread
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 rtc_rx8010sj

在打开 rtc_rx8010 package 后，当进行 bsp 编译时，它会被加入到 bsp 工程中进行编译。

### 3.1、API说明

1. 初始化RX8010SJ

```C
rt_err_t rx8010_init(char const *devicename);
```

| 参数         | 注释                          |
| ------------ | ----------------------------- |
| devicename   | 使用的i2c名称                 |


| 返回      | 注释              |
| --------- | ----------------- |
| RT_EOK    | 初始化成功 		|
| !=RT_OK | 异常 |


2. 设置时间

```C
rt_err_t  rx8010_set_time(struct tm *dt);
```

| 参数 | 注释 |
| ---- | ---- |
|struct tm *dt   | 时间|

|  返回  |   注释   |
| ------ |   ----   |
| RT_EOK |   成功   |
| !=RT_OK | 异常 |

3. 读取时间

```C
rt_err_t rx8010_get_time(struct tm *dt);
```

| 参数 | 注释 |
| ---- | ---- |
| 无  | 无 |

| 返回    | 注释 |
| ------- | ---- |
|struct tm *dt   | 获取到的时间 |
| RT_EOK  | 成功 |
| !=RT_OK | 异常 |

4. 停止led对象

```C
rt_err_t rx8010_set_alarm(rx8010_alarm_time_t *t);
```

| 参数 | 注释 |
| ---- | ---- |
|rx8010_alarm_time_t *t | Alarm时间|

|  返回      |  注释  |
| - ------ - |  ----  |
| RT_EOK  | 成功 |
| !=RT_OK | 异常 |



### 3.2、示例

使用示例在 [examples](./examples) 下


## 4、联系方式 & 感谢

* 维护：Alex.Pan
* 主页：<https://github.com/HikerPan/rtc_rx8010sj>
* 邮箱：<94034822@qq.com>