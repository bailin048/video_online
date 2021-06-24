#include "db.hpp"
#include "httplib.h"

using namespace httplib;

void VideoDelete(const Request& req, Response& rsp){

}
void VideoUpdate(const Request& req, Response& rsp){

}
void VideoGetAll(const Request& req, Response& rsp){

}
void VideoGetOne(const Request& req, Response& rsp){

}
void VideoUpLoad(const Request& req, Response& rsp){

}

int main(){
    Server srv;
    srv.Delete(R"(/video/\d+)",VideoDelete);
    srv.Put(R"(/video/\d+)",VideoUpdate);
    srv.Get(R"(/video)",VideoGetAll);
    srv.Get(R"(/video/\d+)",VideoGetOne);
    srv.Post(R"(/video)",VideoUpLoad);
    srv.listen("0.0.0.0",9000);
    return 0;
}

