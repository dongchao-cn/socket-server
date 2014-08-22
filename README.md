socket-server
=============

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

环境
----

* ubuntu 12.04
* g++