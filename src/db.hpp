#include <iostream>
#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>
#include <mutex>
#include <string>
using namespace std;

namespace void_system{
#define MYSQL_HOST "127.0.0.1"
#define MYSQL_USER "root"
#define MYSQL_PWD "bl123321"
#define MYSQL_NAME "bl_vod_system"

    static MYSQL* MysqlInit(){
        MYSQL* mysql = mysql_init(NULL);
        if(!mysql){
            cout<<"mysql init failed"<<endl;
            return NULL;
        }
        //初始化成功,连接服务器
        if(NULL == mysql_real_connect(mysql,MYSQL_HOST,MYSQL_USER,MYSQL_PWD,MYSQL_NAME,
                    0,NULL,0)){
            cout<<mysql_errno(mysql)<<endl;
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
                char* name = video["name"].asCString();
                char* vdesc = video["vdesc"].asCString();
                char* video_url = video["video_url"].asCString();
                char* image_url = video["image_url"].asCString();
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
#define VIDEO_UPDATE "update tb_video set name='%s',vdesc='%s' where id=%d;"
                char sql[8192] = {0};
                sprintf(sql, VIDEO_UPDATE,video["name"].asCstring(),
                        video["vdesc"].asCstring(),
                        video_id);
                return MysqlQuery(_mysql,sql);
            }
            //获取视频信息
            bool GetAll(Json::Value* video){
#define VIDEO_GETALL "select * from tb_video;"
                bool ret = MysqlQuery(_mysql,VIDEO_GETALL);
                if(!ret)
                    return false;
                MYSQL_RES* res = mysql_store_result(_mysql);
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
            bool GetOne(const size_t video_id,Json::Value* video){
#define VIDEO_GETONE "select * from tb_video where id=%d;"
                char sql[4096] = {0};
                sprintf(sql,VIDEO_GETONE,video_id);
                bool ret = MysqlQuery(_mysql,sql);
                if(!ret)
                    return false;
                MYSQL_RES* res = mysql_store_result(_mysql);
                if(NULL == res){
                    cout<<"store result failed!"<<endl;
                    return false;
                }
                MYSQL_ROW row = mysql_fetch_row(res);
                Json::Value val;
                val["id"] = stoi(row[0]);
                val["name"] = row[1];
                val["vdesc"] = row[2];
                val["video_url"] = row[3];
                val["image_url"] = row[4];
                val["ctime"] = row[5];
                video->append(val);
                mysql_free_result(res);
                return true;
            }
    };
}; 
