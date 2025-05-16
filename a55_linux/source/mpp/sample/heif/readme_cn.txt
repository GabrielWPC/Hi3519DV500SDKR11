heif sample使用说明

运行步骤：
1）linux系统运行，进入到当前目录，在当前目录下输入命令启动sample
封装命令：  ./sample_heif muxer ./xxx.h265 ./xxx.heic
           ./sample_heif muxer ./xxx.h264 ./xxx.avci
解封装命令: ./sample_heif demuxer ./xxx.heic ./xxx.h265
           ./sample_heif demuxer ./xxx.avci ./xxx.264
封装与解封装整合命令：./sample_heif combine ./xxx.h265 ./xxx.h265 ./temp.heic
                    ./sample_heif combine ./xxx.264  ./xxx.h264 ./temp.avci

运行结果
1）用例执行失败会有相应提示信息，否则为执行成功。
2）运行生成文件查看
heic、avci文件可以使用photo-viewer.exe工具查看；
h265文件可以使用Elecard StreamEye工具打开查看；
heic、avci文件的具体box信息可以使用ISO.Viewer工具查看。

备注：
1）H265、H264 IDR帧单帧包含VPS/SPS/PPS等信息
2）当前支持H265、H264单帧图片封装和解封装，不支持grid模式
