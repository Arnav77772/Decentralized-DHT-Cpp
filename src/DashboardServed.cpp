#include "DashboardServer.h"
#include "DHT.h"
#include "Hashing.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_len = int;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
using socket_len = socklen_t;
#endif

namespace {
#ifdef _WIN32
void closeSock(int s) { if (s != -1) closesocket(s); }
#else
void closeSock(int s) { if (s != -1) close(s); }
#endif
std::string jsonEscape(const std::string& in) {
    std::string out;
    for (unsigned char c : in) {
        switch(c) { case '"': out += "\\\""; break; case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break; case '\r': out += "\\r"; break; case '\t': out += "\\t"; break;
        default: if(c >= 32) out += static_cast<char>(c); }
    }
    return out;
}
int hexval(char c) { if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return 0; }
std::string decode(const std::string& s) {
    std::string o;
    for(size_t i=0;i<s.size();++i) { if(s[i]=='+') o+=' '; else if(s[i]=='%'&&i+2<s.size()){o+=char(hexval(s[i+1])*16+hexval(s[i+2]));i+=2;} else o+=s[i]; }
    return o;
}
std::string field(const std::string& body,const std::string& name) {
    std::string needle=name+"="; size_t p=body.find(needle); if(p==std::string::npos)return "";
    if(p>0 && body[p-1]!='&') { p=body.find("&"+needle); if(p==std::string::npos)return ""; ++p; }
    p+=needle.size(); size_t e=body.find('&',p); return decode(body.substr(p,e==std::string::npos?e:e-p));
}
std::string response(const std::string& body, const std::string& status="200 OK") {
    std::ostringstream o; o<<"HTTP/1.1 "<<status<<"\r\nContent-Type: application/json; charset=utf-8\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: GET, POST, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type\r\nConnection: close\r\nContent-Length: "<<body.size()<<"\r\n\r\n"<<body; return o.str();
}
}
DashboardServer::DashboardServer(DHT& dht):dht_(dht){}
DashboardServer::~DashboardServer(){stop();}
bool DashboardServer::start(unsigned short port) {
#ifdef _WIN32
    WSADATA wsa; if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)return false;
#endif
    listenSocket_=static_cast<int>(socket(AF_INET,SOCK_STREAM,0)); if(listenSocket_==-1)return false;
    int yes=1;
#ifdef _WIN32
    setsockopt(listenSocket_,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&yes),sizeof(yes));
#else
    setsockopt(listenSocket_,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
#endif
    sockaddr_in addr{}; addr.sin_family=AF_INET; addr.sin_addr.s_addr=htonl(INADDR_ANY); addr.sin_port=htons(port);
    if(bind(listenSocket_,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))<0 || listen(listenSocket_,8)<0){closeSock(listenSocket_);listenSocket_=-1;
#ifdef _WIN32
        WSACleanup();
#endif
        return false;}
    running_=true; worker_=std::thread(&DashboardServer::run,this); return true;
}
void DashboardServer::stop(){
    if(!running_.exchange(false))return;
#ifdef _WIN32
    shutdown(listenSocket_,SD_BOTH);
#else
    shutdown(listenSocket_,SHUT_RDWR);
#endif
    closeSock(listenSocket_); listenSocket_=-1; if(worker_.joinable())worker_.join();
#ifdef _WIN32
    WSACleanup();
#endif
}
void DashboardServer::run(){
    while(running_){ sockaddr_in client{}; socket_len len=sizeof(client); int s=static_cast<int>(accept(listenSocket_,reinterpret_cast<sockaddr*>(&client),&len)); if(s<0){if(!running_)break;continue;}
        std::string req; char buf[4096]; int n=0; do { n=recv(s,buf,sizeof(buf),0); if(n>0)req.append(buf,n); } while(n==(int)sizeof(buf)&&req.find("\r\n\r\n") == std::string::npos);
        size_t sep=req.find("\r\n\r\n"); std::string head=sep==std::string::npos?req:req.substr(0,sep); std::string body=sep==std::string::npos?"":req.substr(sep+4);
        std::istringstream hs(head); std::string method,path,version; hs>>method>>path>>version; std::string out="{\"ok\":false,\"message\":\"Unknown endpoint\"}";
        if(method=="OPTIONS") out="{}";
        else if(path.find("/api/status")==0) out="{\"ok\":true,\"nodeCount\":"+std::to_string(dht_.nodeCount())+",\"keyCount\":"+std::to_string(dht_.dataSnapshot().size())+"}";
        else if(path.find("/api/nodes")==0){out="{\"nodes\":["; bool first=true; for(auto n:dht_.nodesSnapshot()){if(!first)out+=",";first=false;out+="{\"id\":\""+std::to_string(n.first)+"\",\"keys\":"+std::to_string(n.second)+",\"active\":true}";}out+="]}";}
        else if(path.find("/api/data")==0){out="{\"data\":[";bool first=true;for(auto e:dht_.dataSnapshot()){if(!first)out+=",";first=false;out+="{\"key\":\""+jsonEscape(std::get<0>(e))+"\",\"value\":\""+jsonEscape(std::get<1>(e))+"\",\"node\":\""+std::to_string(std::get<2>(e))+"\"}";}out+="]}";}
        else if(method=="POST" && path=="/api/put"){auto k=field(body,"key"),v=field(body,"value");if(k.empty()||v.empty())out="{\"ok\":false,\"message\":\"Key and value are required\"}";else if(!dht_.nodeCount())out="{\"ok\":false,\"message\":\"Join a node first using the console menu\"}";else{dht_.put(k,v);out="{\"ok\":true,\"message\":\"PUT completed\"}";}}
        else if(method=="POST" && path=="/api/get"){auto k=field(body,"key");std::string v;NodeID owner=0;if(dht_.getValue(k,v,owner))out="{\"ok\":true,\"key\":\""+jsonEscape(k)+"\",\"value\":\""+jsonEscape(v)+"\",\"node\":\""+std::to_string(owner)+"\"}";else out="{\"ok\":false,\"message\":\"Key not found or no nodes joined\"}";}
        else if(method=="POST" && path=="/api/delete"){auto k=field(body,"key");auto before=dht_.dataSnapshot().size();dht_.remove(k);auto after=dht_.dataSnapshot().size();out=after<before?"{\"ok\":true,\"message\":\"Key deleted\"}":"{\"ok\":false,\"message\":\"Key not found\"}";}
        else if(method=="POST" && path=="/api/join"){auto id=field(body,"id");NodeID nid=0;try{nid=std::stoull(id);}catch(...){}bool ok=!id.empty()&&dht_.addNode(nid);out=ok?"{\"ok\":true,\"message\":\"Node joined\"}":"{\"ok\":false,\"message\":\"Invalid or duplicate node ID\"}";}
        else if(method=="POST" && path=="/api/leave"){auto id=field(body,"id");NodeID nid=0;try{nid=std::stoull(id);}catch(...){}bool ok=!id.empty()&&dht_.removeNode(nid);out=ok?"{\"ok\":true,\"message\":\"Node left\"}":"{\"ok\":false,\"message\":\"Node could not leave\"}";}
        auto wire=response(out); send(s,wire.c_str(),static_cast<int>(wire.size()),0);closeSock(s);
    }
}
