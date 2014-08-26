socket-server
=============

[![Build Status](https://travis-ci.org/dccrazyboy/socket-server.svg?branch=master)](https://travis-ci.org/dccrazyboy/socket-server)

说明
----

这是一个关于各种IO模型的socket server实现与测试。

这里采用多种模型实现了一个TCP ECHO服务器，将client发送的内容echo回去。

client每秒会发送一次请求，server保持并发连接并进行相应。

现已实现的模型：

1. base: 最原始的echo服务器
2. thread: 多线程模型
3. select: select模型
4. poll: poll模型
5. epoll: epoll模型

参数
----

1. 修改`src/clients.sh` 可以生成指定数量的client
2. 修改`src/base.cpp` 中的`MAX_CLIENT_NUM`参数调整server最大服务数
3. 修改`src/base.cpp` 中的`CLIENT_INTERVAL`参数调整client发包间隔
4. 修改`Makefile` 中的`DEBUG`参数确定是否输出调试信息

使用方法
-------

修改`Makefile`中的`DEBUG`变量可以控制是否输出包内容。

`make`，bin目录会生成可执行文件。

* `client`: 请求端，每隔一秒发送一次请求。
* `clients.sh`: 一次启动多个`client`
* `base_server`: 最原始的server，同时只能服务一个`client`
* `thread_server`: 多线程版server，每当一个新用户到来时启动一个新的线程进行服务
* `select_server`: select版server，通过select管理连接
* `poll_server`: poll版server，通过poll管理连接
* `epoll_server`: epoll版server，通过epoll管理连接

测试结果
--------

CPU占用率

| 并发数\模型        | thread   |  select  | poll | epoll
| --------   | :-----:  | :----:  | :-----:  | :----:  |
| 1k    | 2-3%    | 10-20% | 10-20% | 1-2% |
| 2k    | 4-7%    | X      | 43-52% | 2-4% |
| 4k    | 11-12%  | X      | 84-88% | 7-8% |
| 8k    | 17-18%  | X      | 72-74% | 11-12% |

1. select最大到1024，再进行连接会崩溃。
2. poll在8k的时候cpu占用率反而下降了，情况不明。。。
3. epoll确实是最优的选择，虽然cpu上和thread差不多，但内存差距特别大。
4. thread是最简单的实现，一个连接一个thread，用thread poll效果应该会好的多。

环境
----

https://registry.hub.docker.com/u/dccrazyboy/socket-server/
