# 目录
- [目录](#目录)
- [简介](#简介)
- [记录](#记录)
  - [目前进展](#目前进展)
    - [2020.03.12](#20200312)
    - [2020.03.29](#20200329)
    - [2020.04.16](#20200416)
    - [2020.04.28](#20200428)
    - [2020.05.03](#20200503)


# 简介
出于学习的目的，用C++11编写的Web服务器。
* 利用主从Reactor模式作为服务器整体框架，主Reactor负责监听连接并派发给从Reactor，从Reactor负责管理时间描述符（timerfd用于定时任务）、事件描述符（eventfd用于唤醒IO线程）和 派发过来的 socket 文件描述符
* 采用Epoll边沿触发模式作为IO多路复用技术
* 使用多线程充分利用多核CPU，并使用线程池避免线程频繁创建销毁的开销
* 用状态机的思想解析了http请求，支持http长连接
* 利用timerfd结合priority_queue实现底层高效定时器，利用升序的双向链表记录活动连接，超时释放连接资源
* 利用WireShark抓包分析Chrome和Edge浏览器长短连接策略，利用WebBench分析服务器并发性能
# 记录
## 目前进展
### 2020.03.12
写了很长时间，终于把第一个版本写出来了，采用了epoll作为IO复用技术。只是实现了server和client之间简单的数据收发，未添加http协议的内容。  
**学到了：**  
* 熟悉了`socket(sys/socket.h)`的创建、绑定、监听、连接、读写等；对`epoll`的机制更加理解(linux内核的select、poll、epoll机制也太奇妙了)，同时也更熟悉`epoll(sys/epoll.h)`的创建、事件的注册、等待事件处理回调等。  
* 另外学到的就是Channel类，自己也实现了一下，确实好用。每一个Channel对象对应一个Socket文件描述符，设置好要监听的事件以及对应的回调函数，根据监听到的事件类型执行相应的回调函数。  

### 2020.03.29
第二个版本，添加了IUser接口便于实现底层功能和顶层功能的分离，添加了Buffer类(功能见下)，EventLoop里通过Eventfd机制实现异步处理事件的能力。尝试分析了每个资源(类对象、socket资源等)的生命周期和资源删除时的相关操作(关闭fd、epoll里删除等)，感觉还是有bug。  
**学到了:**
* 通过IUser接口，将底层网络功能和顶层用户功能相关的内容分开，以后顶层功能是HTTP相关内容，现在只是简单的数据接收显示和发送。  
* 学到了Buffer类，既可以用于发送缓冲，也可以用于接收缓冲。接收缓冲：TCP是一个无边界的字节流协议，用户更方便读想要的数据；发送缓冲：顶层用户通过TcpConnection::send()来告诉底层网络库它要发的数据，网络库没发送的数据由发送缓冲保存，并开始关注可写事件，以发送剩余的数据。  
* TcpServer类负责管理Acceptor和Connection，前者负责建立连接，后者负责每个连接，Connection关闭时通过回调让TcpServer类删除与自己有关的内容。  
* 处理socket对端关闭，还没处理socket相关异常。  

### 2020.04.16
第三个版本，添加了Http协议的相关内容，为了实现对每个连接维护一个状态(虽然HTTP是一种无状态的协议，我们的server对其处理时是有状态的)，删除了原有的IUser类，添加了IHolder类，此外添加了Timer类来实现应用层的长连接。  
**学到了:**
* 原来的IUser不能对每个连接维护一个状态，而HTTP相关功能实现时需要为每个连接维护一个状态，所以改用IHolder类，每个TcpConnection与一个继承了IHolder类的HttpHandler类通过weak_ptr互相绑定。HttpHandler实现request的解析和reponse的返回，Tcpconnection创建时与HttpHandler互相绑定，同时设置Tcpconnection回调，实现相应功能。  
* Edge和Chrome浏览器如何处理短链接和长连接，用WireShark进行分析，具体见[浏览器长短连接实验](https://github.com/wu-zero/MyWebServer/blob/master/doc/%E6%B5%8F%E8%A7%88%E5%99%A8%E9%95%BF%E7%9F%AD%E8%BF%9E%E6%8E%A5%E5%AE%9E%E9%AA%8C.md)，基于此优化了Tcp的socket的关闭。
* 分两层实现了Http应用层的长连接，底层利用timefd相关内容，结合priority_queue实现了只用一个timefd可以提供多个定时任务(包括单次任务和循环任务)的定时器；顶层为每一个HttpConnection设定一个关闭时间，有request时延长关闭时间，定时根据关闭时间关闭相关httpConnection和对应的TcpConnection。  

### 2020.04.28
第四个版本，实现了如下图所示的多线程Reactor并发模型版本，每个Reactor都属于独立的线程，其中MainReactor负责accept，并将连接分发给SubReactor，该连接之后的所有操作都由该SubReactor负责。  
多线程下为了便于线程安全的实现，将TcpServer更改为模板，模板的类型参数为Manager类，这样，Manager类实现不同的功能，相应TcpServer类就实现不同的功能，每个SubReactor里有一个Manager实例，该实例负责该SubReactor(即该线程)中Connection的管理。  
  
<div align=center><img width="500"  src="https://github.com/wu-zero/MyWebServer/raw/master/doc/pics/%E5%B9%B6%E5%8F%91%E6%A8%A1%E5%9E%8B%E7%A4%BA%E6%84%8F%E5%9B%BE.jpg"/></div>  

**学到了:**  
* 利用std::thread、std::mutex、std::condition_variable等实现多线程编程，线程池的实现以及维护线程间共享数据的线程安全。  
* TcpServer利用类模板实现Http相关功能，模板的类型参数为Manager类或者Manager的派生类，TcpServer类内线程池的每个线程都对应一个Manager示例。最初设计考虑传入ManagerCreate函数实现工厂模式，感觉不优雅，最后选择用类模板实现。  

### 2020.05.03
进行了压力测试，更改了一个测试发现的bug，具体测试结果见[测试结果](https://github.com/wu-zero/MyWebServer/blob/master/doc/%E5%8E%8B%E5%8A%9B%E6%B5%8B%E8%AF%95.md)。