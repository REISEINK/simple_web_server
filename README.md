# simple_web_server
参照《Linux高性能服务器编程》实现的简易的C/C++服务器
## 已实现
* 使用线程池 + 非阻塞I/O + Epoll边沿触发的IO多路复用技术 + 模拟Proactor事件处理 的并发模型
* 使用状态机解析HTTP请求报文，支持解析GET请求
## 计划实现
* 异步日志系统
* 定时器
* Reactor模式
* 智能指针封装
* 解析POST请求
