create database if not exists BL_vod_system;

use BL_vod_system;

create table if not exists tb_video(
    id int primary key auto_increment,
    name varchar(32),
    vdesc text,
    video_url varchar(255),
    image_url varchar(255),
    ctime datetime
);
