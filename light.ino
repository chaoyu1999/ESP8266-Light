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
