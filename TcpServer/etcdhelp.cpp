#include "etcdhelp.h"
#include <algorithm>
#include <map>

etcdhelp::etcdhelp(std::string &url):
  etcd(url) {

}
void etcdhelp::getnode(std::string &key, std::string &ip, int &port) {
     int code = hash_(key) % len;
     etcd::Response response = etcd.get("/item/" + std::to_string(code)).get();
     std::string val = response.value().as_string();
     if(val.size() == 0) {
         geteasy(ip, port);
         etcd.set("/item/" + std::to_string(code), ip + ":" + std::to_string(port)).get();
     } else {
         int index = val.find(':');
         ip = val.substr(0, index);
         port = std::stoi(val.substr(index + 1));
     }
}

void etcdhelp::geteasy(std::string &ip, int &port) {

    std::map<std::string, int> data;
    etcd::Response resp1 = etcd.ls("/node/").get();
    for (int i = 0; i < resp1.keys().size(); ++i)
    {
        std::string key = resp1.key(i);
        data[key] = 0;
    }

    etcd::Response resp = etcd.ls("/item/").get();
    for (int i = 0; i < resp.keys().size(); ++i)
    {
        std::string key = resp.key(i);
        std::string val = resp.value(i).as_string();
        if (resp.value(i).is_dir())
           std::cout << "/" << std::endl;
        else
           if(data.find(val) != data.end()) {
               data[val]++;
           }
    }
    int min = 1024;
    std::string index = "";
    for(auto it = data.begin(); it != data.end(); it++) {
        if(it->second < min) {
            min = it -> second;
            index = it -> first;
        }
    }
    int in = index.find(':');
    ip = index.substr(0, in);
    port = std::stoi(index.substr(in + 1));
}