# 基于ESP8266和微型舵机的远程灯控制系统
# 1. 项目简介
&ensp;&ensp;本项目基于ESP8266开发板和微型舵机，实现了一套远程灯控系统，达到了通过WIFI控制灯具开关的目的。
# 2.项目所需硬件
 - ESP8266开发版一块
 ![ESP8266](https://img-blog.csdnimg.cn/42c0dbd071e6491da620b55ed6a3d410.png#pic_center)

 - 具有数据传输功能的USB线一根
 - 杜邦线若干（母对母、公对母都要有）
 - SG90微型舵机一个
 ![舵机](https://img-blog.csdnimg.cn/4c9a3313b72a4d629cc0ea42a6f11d23.png#pic_center)

# 3.项目所需软件
- Arduino IDE[点击访问官网下载](https://www.arduino.cc/en/software)
# 4.项目整体架构
 &ensp;&ensp;项目整体采用B/S架构，ESP8266充当服务器，通过手机浏览器访问服务器网页，在网页上点击舵机控制按钮，控制舵机按一定角度旋转从而实现开关灯具的功能。
 &ensp;&ensp;下图是系统整体架构图：
  ![系统架构图](https://img-blog.csdnimg.cn/70a69554a9eb4419a7fce015af458f76.png#pic_center)
# 5.前端界面设计
 &ensp;&ensp;系统前端界面主要有两个：身份验证登录界面、舵机控制界面
 &ensp;&ensp;身份验证登录界面如下：
![前端界面](https://img-blog.csdnimg.cn/88a6eac2ef994733b4e833d86f0ad3f1.png#pic_center)
 &ensp;&ensp;前端界面比较简单，主要就是输入密钥进行身份验证。验证过程在后端执行，密钥是一个固定值，也写在后端代码里。一个ip短时间内有3次试错机会，超过3次会暂时无法进入控制界面。
 &ensp;&ensp;舵机控制界面如下：
 ![控制界面](https://img-blog.csdnimg.cn/fbd32e69d6d4452a96e92d13271a81ef.png#pic_center)
&ensp;&ensp;点击按钮可以控制舵机进行关灯操作。
# 5.后端功能设计
&ensp;&ensp;后端主要实现了：esp8266热点功能、登录身份验证功能、舵机控制功能
Arduino代码如下：
```cpp
#include <ESP8266WiFi.h>        // 本程序使用 ESP8266WiFi库
#include <ESP8266WebServer.h>   //  ESP8266WebServer库
#include <FS.h>  //esp8266闪存读写库
#include <Servo.h> //舵机控制库
#include <map>  //c++字典库
#include <set>  //c++集合库

Servo myservo; //实例化舵机类
std::map<String, int> userCountMap; //记录用户试错次数
std::set<String> userLogin; //记录用户是否登录成功
const char *ssid = "远程灯控";  //esp8266的wifi热点名称
const char *password = ""; //不设置wifi密码
const String PSW = "2333";  //登录密钥
const char *headerKeys[] = {"Content-Length", "Content-Type", "Connection", "Date"}; //获取登录用户ip
ESP8266WebServer esp8266_server(80);  //设置esp8266服务器开发的端口

void setup(void){//初始化
  myservo.attach(2,500,2500);  //设置舵机控制信号输出端口
  myservo.write(0); //舵机归零
  myservo.detach();  //关闭舵机
  WiFi.softAP(ssid, password);  //打开热点功能
  SPIFFS.begin();// 启动闪存文件系统
 
  esp8266_server.onNotFound(handleUserRequet);  //网页资源未找到时启动的回调函数
  esp8266_server.begin();// 启动网页服务器                  
  esp8266_server.on("/login", handleLogin);//设置登录界面路由  
  esp8266_server.on("/control", handleControl); //设置控制界面路由   
  esp8266_server.collectHeaders(headerKeys, sizeof(headerKeys) / sizeof(headerKeys[0])); //设置收集用户ip地址     
}


void loop(void){
  esp8266_server.handleClient();  //监听客户端连接
}

void handleLogin(){//登录功能实现
  // 根据ip地址限制试错次数&设置登录状态
  String key(esp8266_server.arg("password"));  //获取前端传过来的密钥
  String ip(esp8266_server.client().remoteIP().toString());  //获取用户ip
  if(userCountMap.count(ip)== 0){//首次登录创建用户试错次数字典
    userCountMap[ip] = 0;  //首次登录，试错次数置为0
    if(PSW.equals(key)){//如果前端密钥和后端密钥一致
      userLogin.insert(ip); //设置用户为登录状态
      handleFileRead("/home.html");  //路由至控制界面
    }else{//如果密钥不一致
      userCountMap[ip] += 1;  //试错次数+1
      handleFileRead("/index.html"); // 路由至登录界面
    }
  }else{//如果之前有登录记录
    if(userCountMap[ip]>3){//如果试错次数超限
      handleFileRead("/index.html"); //跳转值登录界面
    }else{//如果没有超限
      if(PSW.equals(key)){ //密钥一致
        userLogin.insert(ip);  //设置为登录状态
        handleFileRead("/home.html"); //跳转至控制界面
      }else{ //密钥不一致
        userCountMap[ip] += 1;  //试错次数+1
        handleFileRead("/index.html");  //跳转至首页继续登录
      }
    }
  }
}

void handleControl(){//控制功能实现                                        
  String ip(esp8266_server.client().remoteIP().toString());  //获取用户ip
  if(userLogin.count(ip)== 1){ //如果是已经登录的用户就可以控制舵机
    myservo.attach(2,500,2500);  //设置舵机参数
    myservo.write(0);//复位
    delay(500);
    myservo.write(90);//关灯
    delay(500);
    myservo.write(0);//复位
    delay(20);
    myservo.detach();  //关闭舵机
    handleFileRead("/home.html");  //继续路由至控制界面
  }else{  //如果是未登录用户则返回访问受限
    esp8266_server.send(500, "text/plain", "500: promise deny");   
  }
}

// 处理用户浏览器的HTTP访问
void handleUserRequet() {         
  // 获取用户请求网址信息
  String webAddress = esp8266_server.uri();
  
  // 通过handleFileRead函数处处理用户访问
  bool fileReadOK = handleFileRead(webAddress);

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if (!fileReadOK){                                                 
    esp8266_server.send(404, "text/plain", "404 Not Found"); 
  }
}

bool handleFileRead(String path) {            //处理浏览器HTTP访问
  if (path.endsWith("/")) {                   // 如果访问地址以"/"为结尾
    path = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
  } 
  String contentType = getContentType(path);  // 获取文件类型
  
  if (SPIFFS.exists(path)) {                     // 如果访问的文件可以在SPIFFS中找到
    File file = SPIFFS.open(path, "r");          // 则尝试打开该文件
    esp8266_server.streamFile(file, contentType);// 并且将该文件返回给浏览器
    file.close();                                // 并且关闭文件
    return true;                                 // 返回true
  }
  return false;                                  // 如果文件未找到，则返回false
}

// 获取文件类型
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
```
# 6.整体效果演示
[video(video-Ln7dVlfq-1660002411021)(type-csdn)(url-https://live.csdn.net/v/embed/230613)(image-https://video-community.csdnimg.cn/vod-84deb4/c607cab86c8840d4a446423760428b3b/snapshots/ec6c211507904c92bc7f1ecf1a975216-00003.jpg?auth_key=4813564971-0-0-09b5c7b91a70e690b04ed0e3342a4563)(title-基于ESP8266和微型舵机的远程灯控制系统)]



