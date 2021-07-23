#include "db.hpp"
#include "httplib.h"
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>

#define WWWROOT "./wwwroot" 
#define VIDEO_PATH "/video/"
#define IMAGE_PATH "/image/"
using namespace httplib;
vod_system::TableVod* tb_video;

void VideoDelete(const Request& req, Response& rsp){
    //1.获取视频id
    int video_id = stoi(req.matches[1]);
    //2.从数据库中获取到对应视频信息
    Json::Value json_rsp;
    Json::Value video;
    Json::FastWriter writer;
    bool ret = tb_video->GetOne(video_id, &video);
    if(!ret){
        cout<<"mysql get video info failed!"<<endl;
        rsp.status = 500;
        json_rsp["result"] = false;
        json_rsp["reason"] = "mysql get video info failed!";
        rsp.body = writer.write(json_rsp);
        rsp.set_header("Content-Type","application/json");
        return;
    }
    //组织文件路径
    string vpath = WWWROOT + video["video_url"].asString();
    string ipath = WWWROOT + video["image_url"].asString();
    //3.删除视频文件，封面图片文件
    unlink(vpath.c_str());
    unlink(ipath.c_str());
    //4.删除数据库中的数据
    ret = tb_video->Delete(video_id);
    if(!ret){
        rsp.status = 500;
        cout<<"mysql delete video failed"<<endl;
        return;
    }
    rsp.status = 200;
    return;
}
void VideoUpdate(const Request& req, Response& rsp){
    //更新视频信息：目前仅限name与vdesc
    int video_id = stoi(req.matches[1]);
    Json::Value video;
    Json::Reader reader;
    bool ret = reader.parse(req.body, video);
    if(!ret){
        cout<<"update tb_video:parse video json failed!"<<endl;
        rsp.status = 400;
        return;
    }
    ret = tb_video->Update(video_id,video);
    if(!ret){
        cout<<"update video:mysql update failed!"<<endl;
        rsp.status = 500;
        return;
    }
    rsp.status = 200;
}
void VideoGetAll(const Request& req, Response& rsp){
    //获取全部视频信息
    Json::Value videos;
    Json::FastWriter writer;
    bool ret = tb_video->GetAll(&videos);
    if(!ret){
        cout<<"getall video:mysql operation failed!"<<endl;
        rsp.status = 500;
        return;
    }
    rsp.body = writer.write(videos);
    rsp.set_header("Content-Type","application/json");
}
void VideoGetOne(const Request& req, Response& rsp){
    //获取指定视频信息
    int video_id = stoi(req.matches[1]);
    Json::Value video;
    Json::FastWriter writer;
    bool ret = tb_video->GetOne(video_id,&video);
    if(!ret){
        cout<<"getone video:mysql operation failed!"<<endl;
        rsp.status = 500;
        return;
    }
    rsp.body = writer.write(video);
    rsp.set_header("Content-Type","application/json");
}
void VideoUpLoad(const Request& req, Response& rsp){
    //1.校验是否有文件上传
    //2.接收文件内容
    //3.组织数据：该上传数据库的组织成Json::Value,该存放的文件写入系统
    auto ret = req.has_file("video_name");
    if(!ret){
        cout<<"have no video name"<<endl;
        rsp.status = 400;
        return;
    }
    const auto& file1 = req.get_file_value("video_name");

    ret = req.has_file("video_desc");
    if(!ret){
        cout<<"have no video description"<<endl;
        rsp.status = 400;
        return;
    }
    const auto& file2 = req.get_file_value("video_desc");

    ret = req.has_file("video_file");
    if(!ret){
        cout<<"have no video file"<<endl;
        rsp.status = 400;
        return;
    }
    const auto& file3 = req.get_file_value("video_file");

    ret = req.has_file("image_file");
    if(!ret){
        cout<<"have no image file"<<endl;
        rsp.status = 400;
        return;
    }
    const auto& file4 = req.get_file_value("image_file");
    
    const string& vname = file1.content;//视频名称
    const string& vdesc = file2.content;//视频描述
    const string& vfile = file3.filename;//视频文件名
    const string& vcont = file3.content;//视频文件内容
    const string& ifile = file4.filename;//封面图片名称
    const string& icont = file4.content;//封面图片内容
    //写文件入系统
    string vurl = VIDEO_PATH + vfile;
    string wwwroot = WWWROOT;
    string v_path(wwwroot);
    v_path += VIDEO_PATH; 
    mkdir(v_path.c_str(),0775);
    vod_system::Tool::WriteFile(wwwroot + vurl, vcont);
    string iurl = IMAGE_PATH + ifile;
    string i_path(wwwroot);
    i_path += IMAGE_PATH;
    mkdir(i_path.c_str(),0775);
    vod_system::Tool::WriteFile(wwwroot + iurl, icont);
    //写数据入数据库
    Json::Value video;
    video["name"] = vname;
    video["vdesc"] = vdesc;
    video["video_url"] = vurl;
    video["image_url"] = iurl;
    ret = tb_video->Insert(video);
    if(!ret){
        rsp.status = 500;
        cout<<"insert video:mysql operation failed!"<<endl;
        return;
    }
    rsp.set_redirect("/");//上传成功刷新界面
    rsp.set_header("Content-Type","text/html");
    return;
}

void VideoPlay(const Request& req, Response& rsp){
    Json::Value video;
    int video_id = std::stoi(req.matches[1]);
    bool ret = tb_video->GetOne(video_id,&video);
    if(!ret){
        cout<<"getone video:mysql operation failed!"<<endl;
        rsp.status = 500;
        return;
    }
    std::string oldstr = "{{video_url}}";
    std::string newstr =  video["video_url"].asString();
    std::string play_html = "./wwwroot/single-video.html";
    boost::algorithm::replace_all(rsp.body, oldstr, newstr);
    vod_system::Tool::ReadFile(play_html, &rsp.body);
    rsp.set_header("Content-Type","text/html");
    return;
}
int main(){
    tb_video = new vod_system::TableVod();
    //搭建http服务器
    Server srv;
    srv.set_base_dir(WWWROOT);
    srv.Delete(R"(/video/(\d+))",VideoDelete);//删除
    srv.Put(R"(/video/(\d+))",VideoUpdate);//修改信息
    srv.Get(R"(/video)",VideoGetAll);//获取全部
    srv.Get(R"(/video/(\d+))",VideoGetOne);//获取单条
    srv.Post(R"(/video)",VideoUpLoad);//上传视频
    srv.Get(R"(/play/(\d+))",VideoPlay);
    srv.listen("0.0.0.0",9000);//监听
    return 0;
}
