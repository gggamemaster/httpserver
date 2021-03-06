# NetServer

A C++ High Performance NetServer 



## Introduction  

本项目为C++11编写的基于epoll的多线程网络服务器框架，应用层实现了简单的HTTP服务器HttpServer和一个回显服务器EchoServer，其中HTTP服务器实现了HTTP的解析和Get方法请求，该框架不限于这两类服务器，用户可根据需要编写应用层服务。

## Origin and purpose of the project
* 项目起源：在看了陈硕老师的<<linux 多线程服务端编程:使用muduo c++ 网络库>>之后，借鉴了里面的一些思想，做了这个小项目
* 项目目的：学习C++知识、部分C++11的语法和编码规范、学习巩固网络编程、网络IO模型、多线程、TCP/IP、HTTP协议等知识

## Envoirment  
* OS: CentOS Linux release 7.0.1406 (# cat /etc/redhat-release)
* kernel: 3.10.0-123.el7.x86_64 (# uname -a)
* Complier: 4.8.5

## Build

	$ make
	$ make clean

## Run
	$ ./httpserver [port] [iothreadnum] [workerthreadnum]
	
	例：$ ./httpserver 80 4 2
	表示开启80端口，采用4个IO线程、2个工作线程的方式 
	一般情况下，业务处理简单的话，工作线程数设为0即可
    
## Tech
 * 基于epoll的IO复用机制实现Reactor模式，采用边缘触发（ET）模式，和非阻塞模式
 * 由于采用ET模式，read、write和accept的时候必须采用循环的方式，直到error==EAGAIN为止，防止漏读等清况，这样的效率会比LT模式高很多，减少了触发次数
 * 线程模型将划分为主线程、IO线程和worker线程，主线程接收客户端连接（accept），并通过Round-Robin策略分发给IO线程，IO线程负责连接管理（即事件监听和读写操作），worker线程负责业务计算任务（即对数据进行处理，应用层处理复杂的时候可以开启）
 * 支持优雅关闭连接
   * 通常情况下，由客户端主动发起FIN关闭连接
   * 客户端发送FIN关闭连接后，服务器把数据发完才close，而不是直接暴力close
   * 如果连接出错，则服务器可以直接close



