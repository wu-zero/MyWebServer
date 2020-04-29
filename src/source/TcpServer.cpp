/// \file TcpServer.cpp
///
/// TcpServer的具体实现, 负责管理Acceptor和Manager, 前者负责建立连接, 后者负责每个单独线程里的连接管理
/// 利用模板, 不同的Manager实现不同的功能
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


// 《c++primer》：模板与普通函数不同，进行实例化的时候，编译器必须能够访问定义模板的源代码
// 所以将类的声明和类的定义都写到TcpServer.h中


