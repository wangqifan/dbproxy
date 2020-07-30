#include <errno.h>
#include <thread>
#include <strings.h>
#include <poll.h>
#include <memory>
#include "EventLoop.hh"
#include "Channel.hh"
#include "Poller.hh"
#include "Logger.hh"
#include "AsyncLogging.hh"
#include "TcpServer.hh"
#include "TcpClient.hh"
#include <sstream>
#include <vector>

const off_t kRollSize = 2048*1000;

AsyncLogging* g_asynclog = NULL;

void asyncOutput(const char* logline, int len){
  g_asynclog->append(logline, len);
}

void AsyncFlush()
{
  g_asynclog->stop();
}


void split(const std::string& s,
    std::vector<std::string>& sv,
                   const char delim = ' ') {
    sv.clear();
    std::istringstream iss(s);
    std::string temp;

    while (std::getline(iss, temp, delim)) {
        sv.emplace_back(std::move(temp));
    }
    return;
}

#include "Acceptor.hh"
#include "SocketHelp.hh"
#include "InetAddress.hh"
#include "etcdhelp.h"

#include <set>

std::set<std::shared_ptr<TcpClient>> clients;
std::string etcdurl = "http://192.168.134.128:2379";
etcdhelp etcdtool(etcdurl);


void onConnection(const TcpConnectionPtr& conn)
{
  printf("onConnection\n");
 // conn->send("123456789\n");
}

void onClientMessage(const TcpConnectionPtr& conn, Buffer* interBuffer, ssize_t len) {
      // std::string s = interBuffer->retrieveAsString(len);
     //  printf("onMessage : received %d Bytes from backend ++++++ [%s]\n", interBuffer->readableBytes(), s.c_str());
}

void onMessage(const TcpConnectionPtr& conn, Buffer* interBuffer, ssize_t len)
{
    printf("onMessage : received %d Bytes from connection [%s]\n", interBuffer->readableBytes(), conn->name());
    std::string input = interBuffer->retrieveAsString(len - 2);
    interBuffer->retrieve(2);

    printf("onMessage : %s", input);
    //
    std::vector<std::string> terms;
    split(input, terms);
    std::cout << "size: " << terms.size() << std::endl;
    if(terms.size() <= 1) {
      conn->send("bad request\n");
      return;
    }
    std::string key = terms[1];
    std::string ip = "";
    int port = 0;

    etcdtool.getnode(key, ip, port);

    InetAddress serverAddr(ip, port); 
    std::shared_ptr<TcpClient> client(new TcpClient(conn->getLoop(), serverAddr));
    client->setConnectionCallBack(onConnection);
    client->setMessageCallBack(onClientMessage);
    client->start();
    printf("start end: \n");
    // while (!client.isConnected()){}
    client->setRequest(input + "\r\n");
    clients.insert(client);
    client->setFrontConnection(conn);
    printf("send end: \n");
}


void newConnetion(int sockfd, const InetAddress& peeraddr)
{
  LOG_DEBUG << "newConnetion() : accepted a new connection from";
//  ::write(sockfd, "How are you?\n", 13);
  //::sockets::close(sockfd);
}

int main()
{
  AsyncLogging log("/dev/stdout", kRollSize, 0.1);
  g_asynclog = &log;
  Logger::setOutput(asyncOutput);
  Logger::setFlush(AsyncFlush);
  g_asynclog->start();

  InetAddress listenAddr(8888);
  EventLoop loop;
  TcpServer Tserver(&loop, listenAddr, "TcpServer");
  Tserver.setConnectionCallBack(onConnection);
  Tserver.setMessageCallBack(onMessage);
  Tserver.start();

  loop.loop();

}