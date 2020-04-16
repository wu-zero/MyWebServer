/// \file HttpHandler.h
///
/// http处理相关, 解析request, 返回合适的response, 每个TcpConnection实例对应一个HttpHandler实例
/// 两者用weak_ptr互相绑定
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/3.


#ifndef MYWEBSERVER_HTTPHANDLE_H
#define MYWEBSERVER_HTTPHANDLE_H


#include <string>
#include <vector>
#include <map>

#include "IHolder.h"
#include "TcpConnection.h"

class HttpHandler: public IHolder, std::enable_shared_from_this<HttpHandler>

{
public:
    enum HttpVersion{kVersion10, kVersion11, kVersionUnknown};
    enum HttpMethod{kMethodGet, kMethodPost, kMethodUnknown};
    enum HttpState{kStart, kParseUrl,kParseHeader, kParseBody, kParseDone, kResponse};
    enum HttpContentType{kContentTypeText,kContentTypeImage, kContentTypeUnknown};
    static const int kKeepAliveTime;
private:
    static std::vector<std::string> kMethodVector; // Method枚举值对应的string
    static std::vector<std::string> kVersionVector; // Version枚举值对应的string

    using SPtrConnection = std::shared_ptr<TcpConnection>;
    using WPtrConnection  = std::weak_ptr<TcpConnection>;
    using StringPos = std::string::size_type;
private:
    HttpState mHttpState; // 解析和响应状态

    HttpMethod mHttpMethod; // Http方法
    HttpVersion mHttpVersion; // Http版本
    std::string mHttpPath; // Http虚拟目录部分
    std::map<std::string, std::string> mHeader; // Http的header
    HttpContentType mHttpContentType; // header中的"Content-Type"
    std::string mHttpBody; //Http的Body

    bool mIsKeepAlive; // 是否长连接
    std::weak_ptr<TcpConnection> mWPtrTcpConnection; // 关联的TcpConnection
public:
    HttpHandler();
    ~HttpHandler();
    /// 利用weak_ptr关联TcpConnection
    void setTcpConnection(const WPtrConnection& wPtrConnection);
    WPtrConnection getTcpConnection();
    /// 是否长连接
    bool isKeepAlive();
    /// 长连接中的重置
    void reset();
    /// 解析请求、返回响应
    void handleHttpReq(std::string &buf);
    std::string responseRequest();
private:
    /// 解析HttpRequest
    StringPos parseUrl(std::string &buf, StringPos beginPos);
    StringPos parseHeader(std::string &buf, StringPos beginPos);
    StringPos parseBody(std::string &buf, StringPos beginPos);
    /// 错误响应、正常响应
    std::string badRequest(int num, const std::string &note);
    std::string Request(const std::string &body);


    void setMethod(const std::string &method);
    void setPath(const std::string &path);
    void setVersion(const std::string &version);
    void setHeader(const std::string &key, const std::string &value);

};


#endif //MYWEBSERVER_HTTPHANDLE_H
