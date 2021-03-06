#include <iostream>
#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>
#include <mutex>
#include <string>
#include <fstream>
using namespace std;

namespace vod_system{
#define MYSQL_HOST "127.0.0.1"
#define MYSQL_USER "root"
#define MYSQL_PWD  ""
#define MYSQL_NAME "BL_vod_system" //库名

    static MYSQL* MysqlInit(){
        MYSQL* mysql = mysql_init(NULL);
        if(!mysql){
            cout<<"mysql init failed"<<endl;
            return NULL;
        }
        //初始化成功,连接服务器
        if(NULL == mysql_real_connect(mysql,MYSQL_HOST,MYSQL_USER,MYSQL_PWD,MYSQL_NAME,
                    0,NULL,0)){
            cout<<"mysql connect: " << mysql_errno(mysql)<<endl;
            mysql_close(mysql);
            return NULL;
        }
        //连接成功，设置字符集
        if(mysql_set_character_set(mysql,"utf8")){
            cout<<mysql_errno(mysql)<<endl;
            mysql_close(mysql);
            return NULL;
        }
        return mysql;
    }

    //释放句柄
    static void MysqlRelease(MYSQL* mysql){
        if(mysql)
            mysql_close(mysql);
    }

    //执行操作
    static bool MysqlQuery(MYSQL* mysql,const string& sql) {
        int ret = mysql_query(mysql, sql.c_str());
        if(ret){
            cout<<sql<<endl;
            cout<<mysql_errno(mysql)<<endl;
            return false;
        }
        return true;
    }

    class TableVod{
        private:
            MYSQL* _mysql;//数据库句柄
            mutex _mutex;//互斥锁
        public:
            //句柄初始化链接服务器
            TableVod(){ 
                _mysql = MysqlInit();
                if(NULL == _mysql)
                    exit(0);
            }
            //销毁句柄
            ~TableVod(){ MysqlRelease(_mysql); }
            //插入视频
            bool Insert(const Json::Value& video){
                const char* name = video["name"].asCString();
                const char* vdesc = video["vdesc"].asCString();
                const char* video_url = video["video_url"].asCString();
                const char* image_url = video["image_url"].asCString();
                char sql[4096] = {0};
#define VIDEO_INSERT "insert tb_video values(null,'%s','%s','%s','%s',now());"
                sprintf(sql,VIDEO_INSERT,name,vdesc,video_url,image_url);
                return MysqlQuery(_mysql,sql);
            }
            //删除视频
            bool Delete(const size_t video_id){
#define VIDEO_DELETE "delete from tb_video where id=%d;"
                char sql[4096] = {0};
                sprintf(sql,VIDEO_DELETE,video_id);
                return MysqlQuery(_mysql,sql);
            }
            //更改信息
            bool Update(const size_t video_id,const Json::Value& video){
#define VIDEO_UPDATE "update tb_video set name='%s', vdesc='%s' where id=%d;"
                char sql[8192] = {0};
                sprintf(sql, VIDEO_UPDATE,video["name"].asCString(),
                        video["vdesc"].asCString(),video_id);
                return MysqlQuery(_mysql,sql);
            }
            //获取全部视频信息
            bool GetAll(Json::Value* video){
#define VIDEO_GETALL "select * from tb_video;"
                _mutex.lock();
                bool ret = MysqlQuery(_mysql,VIDEO_GETALL);
                if(!ret){
                    _mutex.unlock();
                    return false;
                }
                MYSQL_RES* res = mysql_store_result(_mysql);
				_mutex.unlock(); 
                if(NULL == res){
                    cout<<"store result failed!"<<endl;
                    return false;
                }
                int num = mysql_num_rows(res);
                for(int  i = 0; i < num; ++i){
                    MYSQL_ROW row = mysql_fetch_row(res);
                    Json::Value val;
                    val["id"] = stoi(row[0]);
                    val["name"] = row[1];
                    val["vdesc"] = row[2];
                    val["video_url"] = row[3];
                    val["image_url"] = row[4];
                    val["ctime"] = row[5];
                    video->append(val);
                }
                mysql_free_result(res);
                return true;
            }
            //获取单条视频信息
            bool GetOne(const int video_id,Json::Value* video){
#define VIDEO_GETONE "select * from tb_video where id=%d;"
                char sql[4096] = {0};
                sprintf(sql,VIDEO_GETONE,video_id);
                _mutex.lock();
                bool ret = MysqlQuery(_mysql,sql);
                if(!ret){
                    _mutex.unlock();
                    return false;
                }
                MYSQL_RES* res = mysql_store_result(_mysql);
                _mutex.unlock();
                if(NULL == res){
                    cout<<mysql_error(_mysql)<<endl;
                    return false;
                }
                int num_row = mysql_num_rows(res);
                if(1 != num_row){
                    cout<<"getone result error!"<<endl;
                    mysql_free_result(res);
                    return false;
                }
                MYSQL_ROW row = mysql_fetch_row(res);
                (*video)["id"] = video_id;
                (*video)["name"] = row[1];
                (*video)["vdesc"] = row[2];
                (*video)["video_url"] = row[3];
                (*video)["image_url"] = row[4];
                (*video)["ctime"] = row[5];
                mysql_free_result(res);
                return true;
            }
    };
    class Tool{
        //工具类
        public:
            //接受数据写入文件
        static bool WriteFile(const string& name,const string& content){
            ofstream of;
            of.open(name,ios::binary);
            if(!of.is_open()){
                cout<<"open file failed!"<<endl;
                return false;
            }
            of.write(content.c_str(),content.size());
            if(!of.good()){
                cout<<"write file failed!"<<endl; 
                return false;
            }
            of.close();
            return true;
        }
        static bool ReadFile(const std::string& name,std::string* body){
            std::ifstream ifile;
            ifile.open(name,std::ios::binary);
            if(!ifile.is_open()){
                cout<<"open "<<name<<" failed!"<<endl;
                ifile.close();
                return false;
            }
            ifile.seekg(0,std::ios::end);
            uint64_t length = ifile.tellg();
            ifile.seekg(0,std::ios::beg);
            body->resize(length);
            ifile.read(&(*body)[0],length);
            if(!ifile.good()){
                cout<<"read "<<name<<" failed!"<<endl;
                ifile.close();
                return false;
            }
            ifile.close();
            return true;
        }
    };
};
