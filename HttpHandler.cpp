/// \file HttpHandler.cpp
///
/// http处理相关, 解析request, 返回合适的response, 每个TcpConnection实例对应一个HttpHandler实例
/// 两者用weak_ptr互相绑定
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/3.


#include "HttpHandler.h"

#include <string>
#include <vector>
#include <iostream>
#include "sys/stat.h"
#include "fcntl.h"
#include "sys/mman.h"
#include "unistd.h"

std::vector<std::string> HttpHandler::kMethodVector = {"GET", "POST", "Unknown"};
std::vector<std::string> HttpHandler::kVersionVector = {"HTTP/1.0", "HTTP/1.1", "Unknown"};
const int HttpHandler::kKeepAliveTime = 120;

HttpHandler::HttpHandler()
        : mHttpState(kStart),
          mHttpMethod(kMethodUnknown),
          mHttpVersion(kVersionUnknown),
          mHttpContentType(kContentTypeUnknown),
          mIsKeepAlive(false)
{
}

HttpHandler::~HttpHandler()
{
}

void HttpHandler::setTcpConnection(const HttpHandler::WPtrConnection &wPtrConnection)
{
    mWPtrTcpConnection = wPtrConnection;
}

HttpHandler::WPtrConnection HttpHandler::getTcpConnection()
{

    return mWPtrTcpConnection;

}

bool HttpHandler::isKeepAlive()
{
    return mIsKeepAlive;
}

void HttpHandler::reset()
{
    mHttpState = kStart;
    mHttpMethod = kMethodUnknown;
    mHttpPath.clear();
    mHttpVersion = kVersionUnknown;
    mHeader.clear();
    mHttpContentType = kContentTypeUnknown;
    mHttpBody.clear();
    mIsKeepAlive = false;
}


void HttpHandler::handleHttpReq(std::string &buf)
{
    int beginPos=0, endPos=0;
    if(buf.empty()){
        mHttpState = kStart;
        return;
    }
    endPos = parseUrl(buf, beginPos);
    if(endPos < 0){
        mHttpState = kParseUrl;
        return;
    }
    beginPos = endPos;
    endPos = parseHeader(buf, beginPos);
    if(endPos < 0){
        mHttpState = kParseHeader;
        return;
    }
    beginPos = endPos;
    endPos = parseBody(buf, beginPos);
    if(endPos < 0){
        mHttpState = kParseBody;
        return;
    }
    mHttpState = kParseDone;
    return;
}

std::string HttpHandler::responseRequest()
{
    std::string filename("../websource/");
    std::string context;
    /// 解析中出错
    if (mHttpState != kParseDone)
    {
        return badRequest(400, "Bad request");
    }
    /// 解析url资源
    if (mHttpPath == "/")
    {
        mHttpContentType = kContentTypeText;
        filename += "index.html";

    } else if (mHttpPath == "/favicon.ico") {
        mHttpContentType = kContentTypeImage;
        filename += "favicon.ico";
    }
    else
    {
        return badRequest(404, "Not Found");
    }
    /// 返回页面
    struct stat st;
    if(::stat(filename.c_str(), &st)<0)
    {
        std::cout << "stat err"<< std::endl;
        return badRequest(404, "Not Found");
    }

    int fd = ::open(filename.c_str(), O_RDONLY);
    if(fd < 0)
    {
        badRequest(404, "Not Found");
        return badRequest(404, "Not Found");
    }

    void *mapFile = ::mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(mapFile == MAP_FAILED)
    {
        return badRequest(404, "Not Found");
    }

    char *pf = static_cast<char *>(mapFile);
    context = std::string(pf, pf + st.st_size);

    close(fd);
    munmap(mapFile, st.st_size);
    std::string head = Request(context);
    return head+context;
}


/// 解析请求行
HttpHandler::StringPos HttpHandler::parseUrl(std::string &buf, HttpHandler::StringPos beginPos)
{
    /// 1. 找到请求行
    StringPos endPos = buf.find("\r\n", beginPos);
    if (endPos == std::string::npos){return -1;}
    /// 2.1 解析请求方法
    StringPos space = buf.find(' ', beginPos);
    if (space == std::string::npos || space > endPos){return -1;}
    setMethod(buf.substr(beginPos, space - beginPos));
    if (mHttpMethod == kMethodUnknown){return -1;}
    /// 2.2 解析请求资源路径
    beginPos = space + 1;
    space = buf.find(' ', beginPos);
    if (space == std::string::npos || space > endPos){return -1;}
    setPath(buf.substr(beginPos, space - beginPos));
    if (mHttpPath.empty() || mHttpPath[0] != '/'){return -1;}
    /// 2.3 解析Http版本号
    beginPos = space + 1;
    setVersion(buf.substr(beginPos, endPos - beginPos));
    if (mHttpVersion == kVersionUnknown){return -1;}
    /// 3 根据HttpVersion设置是否长连接
    if (mHttpVersion == kVersion11)
    {
        mIsKeepAlive = true;
    }

//    std::cout << "method: " << kMethodVector[mHttpMethod] << std::ends
//              << "path: " << mHttpPath << std::ends
//              << "version: " << kVersionVector[mHttpVersion] << std::endl;
    return endPos + 2;
}

/// 解析Header
HttpHandler::StringPos HttpHandler::parseHeader(std::string &buf, HttpHandler::StringPos beginPos)
{
    StringPos endPos = beginPos;
    while ((endPos = buf.find("\r\n", beginPos)) != beginPos)
    {
        /// 1 找到：, :左边是key, 右边是value
        StringPos separatePos = buf.find(':', beginPos);
        if (beginPos == std::string::npos || separatePos > endPos) { return -1; }
        /// 2 找到key
        beginPos = buf.find_first_not_of(' ', beginPos);
        std::string key(buf.substr(beginPos, separatePos - beginPos));
        /// 3 找到value
        separatePos += 1;
        separatePos = buf.find_first_not_of(' ', separatePos);
        std::string value(buf.substr(separatePos, endPos - separatePos));
        /// 4 设置header
        setHeader(key, value);
        beginPos = endPos + 2;
    }
    /// keepAlive处理
    if(mHeader.count("Connection")){
//        std::cout << mHeader["Connection"] <<std::endl;
        if(mHeader["Connection"] == "keep-alive" || mHeader["Connection"] == "Keep-Alive"){
            mIsKeepAlive = true;
        }
        else if(mHeader["Connection"] == "close" || mHeader["Connection"] == "Close"){
            mIsKeepAlive = false;
        }
    }

//    for (auto &p: mHeader)
//    {
//        std::cout << p.first << p.second << std::endl;//<< ": "
//    }
    return 0;
}

/// 解析Body
HttpHandler::StringPos HttpHandler::parseBody(std::string &buf, HttpHandler::StringPos beginPos)
{
    /// 1 判断Method是Post
    if (mHttpMethod != kMethodPost) { return 0; }
    /// 2 找到Body长度
    if (!mHeader.count("Content-length")) { return -1; }
    int bodyLen = stoi(mHeader["Content-length"]);
    if (bodyLen < static_cast<int>(buf.size()) - beginPos) { return -1; }
    /// 3 设置Body
    mHttpBody = buf.substr(beginPos);
    //std::cout << mHttpBody << std::endl;
}

void HttpHandler::setMethod(const std::string &method)
{
    mHttpMethod = kMethodUnknown;
    for (int i = 0; i < kMethodVector.size(); ++i)
    {
        if (method == kMethodVector[i])
        {
            mHttpMethod = static_cast<HttpMethod>(i);
            return;
        }
    }
}

void HttpHandler::setVersion(const std::string &version)
{
    mHttpVersion = kVersionUnknown;
    for (int i = 0; i < kVersionVector.size(); ++i)
    {
        if (version == kVersionVector[i])
        {
            mHttpVersion = static_cast<HttpVersion>(i);
            return;
        }
    }
}

void HttpHandler::setPath(const std::string &path)
{
    mHttpPath = path;
}

void HttpHandler::setHeader(const std::string &key, const std::string &value)
{
    mHeader[key] = value;
}

/// 应答错误请求
std::string HttpHandler::badRequest(int num, const std::string &note)
{
    std::string header;
    std::string body;
    header += "HTTP/1.1" + std::to_string(num) + " " + note + "\r\n";
    header += "Content-Type: text/html\r\n";
    if (!mIsKeepAlive)
    {
        header += "Connection: close\r\n";
    } else
    {
        header += "Connection: Keep-Alive\r\n";
    }

    body += "<html><title>呀~出错了</title>";
    body += "<body>" + std::to_string(num) + " " + note;
    body += "</body></html>";

    header += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    header += "Server: WYW's WebServer\r\n\r\n";
    return header+body;
}

/// 应答正常请求
std::string HttpHandler::Request(const std::string &body)
{
    std::string header;

    header += "HTTP/1.1 200 OK\r\n";
    if(mHttpContentType == kContentTypeText){
        header += "Content-Type: text/html\r\n";
    }
    else if(mHttpContentType == kContentTypeImage){
        header += "Content-Type: image/x-icon\r\n";
    }

    if (!mIsKeepAlive)
    {
        header += "Connection: close\r\n";
    } else
    {
        header += "Connection: Keep-Alive\r\n";
        header += "Keep-Alive: timeout=" + std::to_string(kKeepAliveTime) + "\r\n";
    }
    header += "Content-Length: " + std::to_string(body.size())+"\r\n";
    header += "Server: WYW's WebServer\r\n\r\n";
    return header;
}

