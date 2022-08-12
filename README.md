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
# 6.项目结构
- data为前端界面
- light.ino为esp8266的固件，使用Arduino编写


