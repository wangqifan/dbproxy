#include<etcd/Client.hpp>
#include<string>

class etcdhelp {
private:
   const int len = 1024;
   etcd::Client etcd;
   std::hash<std::string> hash_;
public:
    etcdhelp(std::string &url);
    void getnode(std::string &key, std::string &ip, int &port);
    void geteasy(std::string &ip, int &port);
};