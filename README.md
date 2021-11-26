<p align="center"><a href="https://www.100ask.net" target="_blank" rel="noopener noreferrer"><img width="100" src="http://wechatapppro-1252524126.file.myqcloud.com/appTVs2sJfo3933/image/b_u_5fb1e35c3d801_CUAzKqf9/knhb7p8x0j7u.png" alt="100ASK logo"></a></p>

<h2 align="center">100ASK Linux LVGL Desktop</h2>

100ASK Linux LVGL desktop 是一个 MIT 许可的开源项目。本项目主要目的是为大家提供一个嵌入式Linux GUI的参考解决方案：低内存占用、简洁美观的视觉效果、稳定且拓展性强的项目架构。

# 介绍(Introduction)

100ASK Linux LVGL desktop 是一个 MIT 许可的开源项目。该项目由 [百问网团队](https://www.100ask.net) 研发、发布，目的是为 100ASK_IMX6ULL、100ASK_STM32MP157开发板提供提供一个基础GUI，使用Makefile组织管理源码、基于GCC编译。通过修改少量的配置便在其他Linux、Linux开发板上运行。

项目效果演示视频：[https://www.bilibili.com/video/BV1nT4y1R7rz](https://www.bilibili.com/video/BV1nT4y1R7rz)

![https://img-blog.csdnimg.cn/da2dfa5b8a624d8f9390c4f3ad9847fb.jpg](https://img-blog.csdnimg.cn/da2dfa5b8a624d8f9390c4f3ad9847fb.jpg)

## 快速开始

如果使用的是[百问网IMX6ULL开发板](https://item.taobao.com/item.htm?id=610613585935)，那么你可以通过直接更新固件，最快获得体验。

烧写工具及镜像获取：

- 百度云：链接： [https://pan.baidu.com/s/17QWdasg3lcSb82JK4uUJuQ](https://pan.baidu.com/s/17QWdasg3lcSb82JK4uUJuQ)  提取码： root 
- 不限速下载地址： [https://download.csdn.net/download/qq_35181236/40774994](https://download.csdn.net/download/qq_35181236/40774994)

或者选择按照下面的章节按步骤开始。

### 编译项目

1. 配置开发环境
    本项目的开发环境为：` VMware Workstation + Ubuntu` ，
    如果是学习韦东山嵌入式Linux教程的小伙伴可以跳过这一步，
    需要搭建开发环境的请点击连接查看详细教程：xxxxxx 。
2. 配置交叉编译环境。如果工具链没有配置正确，可能会导致编译不通过，即使编译通过了也不能在目标平台上运行，请注意检查运行环境，编译环境。
3. 先克隆主仓库：git clone xxxxxxxx
4. 克隆主仓库后，同步子仓库模块： git submodule update --init --recursive
5. 后续更新子仓库模块： git submodule update --remote
6. 进入仓库根目录 `lv_100ask_linux_desktop` ，执行 `make clean && make` 开始编译。
7. 提升编译速度。全速编译命令： `make clean && make -j$(nproc) RUN_JOBS=-j$(nproc)` 
    "make -j$(nproc)" 的 **$(nproc)** 是指定编译 **主桌面程序** 的内核线程数，命令会自动获取系统支持最大的线程数，可以手动指定线程数，如： -j16 ，
    "RUN_JOBS=j$(nproc)"的 **$(nproc)** 是指定编译 **APP程序** 的内核线程数，命令会自动获取系统支持最大的线程数，可以手动指定线程数，如： -j16 ，
    Linux 下输入 `nproc` 命令返回的数字是你机器的线程数。

### 如何运行
#### 手动运行
1. 编译出来的可执行程序会放在项目根目录的 `bin` 目录下，将里面的可执行文件全部传到开发板文件系统中(如家目录)，并修改可执行文件的权限： `sudo chmod  +x  ~/*`
2. 将项目 `assets/icon` 的 **icon** 目录及其中的所有图片文件传到开发板的文件系统中( **icon** 目录必须和可执行程序在同一级目录)
3. 修改 `assets/services` 中的所有 `.services` 文件，假如你将可执行文件放在了家目录，那么就改成这样：
```shell
[D-BUS Service]
Name=net.ask100.lvgl.About
Exec=~/general_About # 原来是这样的Exec=/usr/share/100ask_desktop/general_About
```

4. 将修改后 `assets/services`  中的所有 `.services` 文件，复制到开发板的 dbus 服务目录下，本项目的目录是： `/usr/share/dbus-1/services/` 
5. 在执行之前初始化 dbus 运行环境，执行： export $(dbus-launch)
6. 最后在可执行文件的目录下执行桌面程序： ./100ask_lvgl_Main 
7. 尽情享受吧！

#### 开机自启动
1. 在开发板文件系统中新建目录 `/usr/share/100ask_desktop/`
2. 项目编译出来的可执行程序会放在项目根目录的 `bin` 目录下，将里面的可执行文件全部传到开发板文件系统刚刚新建的目录中： `/usr/share/100ask_desktop/` ，并修改可执行文件的权限： `sudo chmod  +x  /usr/share/100ask_desktop/*`
3. 将项目 `assets/icon` 的整个 **icon** 目录及其中的所有图片文件传到开发板的文件系统中刚刚新建的目录中： `/usr/share/100ask_desktop/`
4. 将项目 `assets/services` 中的所有 `.services` 文件，复制到开发板的 dbus 服务目录下，本项目的目录是： `/usr/share/dbus-1/services/`
5. 修改开机启动脚本为下面的内容：`vi /usr/bin/start.sh`

```shell
#!/bin/sh -e

echo -e "\033[9;0]"  >  /dev/tty0   # 此行请自己手敲，直接复制可能会出错
echo -e "\033[?25l"  > /dev/tty1    # 此行请自己手敲，直接复制可能会出错，其中 25l 是 L 的小写

export $(dbus-launch)

/usr/share/100ask_desktop/100ask_lvgl_Main &

exit 0
```

6. 重启开发板，尽情享受吧！

## 仓库子模块说明

- [x] lv_100ask_demos：百问网 LVGL 示例仓库 (平台无关，可以在支持LVGL的任意平台编译运行)
- [x] lv_100ask_modules：百问网模块仓库 (此仓库为本项目而设，目的是为了提供一些通用的模块供本项目使用，并减少代码量和项目维护难度)
- [ ] lv_demos：LVGL 官方的示例(LVGL官方仓库)
- [x] lv_drivers：用于 LVGL 嵌入式 GUI 库的 TFT 和触摸板驱动程序(LVGL官方仓库)
- [x] lvgl：LVGL库，项目基于此仓库(LVGL官方仓库)

> 上面 √ 选上的是必选仓库，如果不需要可选仓库，请注意修改配置文件和 Makefile。

# 项目架构

## 项目框图

![项目框图](https://img-blog.csdnimg.cn/7a335dabd8ea4642bc7ba60f19d80fe8.jpg)

## 目录说明
``` shell
├── assets  # 程序运行所需的资源，二次开发需要将里面的文件copy开发板的文件系统中
│   ├── icon # 桌面背景和APP图标(默认存放位置：/usr/share/100ask_desktop/assets/icon)。需要将 assets/icon 文件夹复制到主桌面程序的同级目录(默认存放位置：/usr/share/100ask_desktop)下，背景、图标可以替换，注意名称要和之前的一样。
│   └── services # DBus服务列表。增加APP后需要参考里面的格式新建服务文件，并将服务文件(不是整个文件夹)copy到dbus服务文件目录下：  /usr/share/dbus-1/services/
├── lv_100ask_app # APP程序(二次开发新的APP主要在这里添加自己的APP)
│   └── src
│       ├── general_app # 平台无关的通用APP
│       │   ├── general_2048_game # APP程序的目录
│       │   │   ├── obj # 编译过程输出文件存放在这里(执行后 make clean 会清除)，可执行文件放在项目根目录的 bin 目录下
│       │   │   └── src	# APP程序代码
│       │   ├── .......
│       └── imx6ull_app # 硬件相关的APP
│           ├── imx6ull_set_lan # APP程序的目录
│           ├── .......
├── lv_100ask_demos # 百问网平台无关的通用APP
│   ├── docs
│   └── src
│       ├── lv_100ask_demo_2048
│       ├── ......
├── lv_100ask_modules # 百问网模块仓库 (此仓库为本项目而设，目的是为了提供一些通用的模块供本项目使用，并减少代码量和项目维护难度)
│   ├── docs
│   └── src
│       ├── lv_100ask_boot_animation # 开机动画模块
│       ├── lv_100ask_dbus_handler # DBus消息处理模块
│       ├── lv_100ask_dbus_message_dispatch # 消息分发模块(目前是桌面主程序专用)
│       ├── lv_100ask_demo_assistive_touch # 辅助触摸悬浮球模块
│       └── lv_100ask_demo_init_icon # 桌面图标初始化模块(桌面主程序专用)
├── lv_demos # LVGL 官方的示例(LVGL官方仓库)
├── lv_drivers # 用于 LVGL 嵌入式 GUI 库的 TFT 和触摸板驱动程序(LVGL官方仓库)
├── lvgl # LVGL库，项目基于此仓库(LVGL官方仓库)
├── lv_lib_png # 显示png图片库(LVGL官方仓库)
├── bin	# 编译完成后，所有的可执行文件存放在这里(执行后 make clean 会清除)
└── obj # 桌面程序的编译过程输出文件存放在这里(执行后 make clean 会清除)，可执行文件放在项目根目录的 bin 目录下
```

## 如何添加自己的程序

参考 `lv_100ask_app/` 在原有基础上追加或创建自己的应用。

1. 参考 `lv_100ask_app/src` 目录创建新的目录
2. 修改 `lv_100ask_app/lv_100ask_app.h` 包含添加你的应用路径。
3. 最后需要修改项目根目录的Makefile文件，在 SRC_DIR 变量添加自己创建的应用


# 文档(Documentation)

LVGL 文档教程：[http://lvgl.100ask.net/](http://lvgl.100ask.net/)

LVGL 视频教程： TODO

# 技术支持(FAQs)

欢迎来论坛发帖交流：[http://bbs.100ask.net/](http://bbs.100ask.net/)

# Issues
TODO
# Changelog
TODO

# License
MIT

Copyright © 2008-2020 深圳百问网科技有限公司 All Rights Reserved.



# 关于作者
- 百问网官网：[http://www.100ask.net](http://www.100ask.net)
- 百问网官方wiki：[http://wiki.100ask.org](http://wiki.100ask.org)
- 百问网官方论坛：[http://bbs.100ask.net](http://bbs.100ask.net)
- 微信公众号：百问科技
- CSDN：[https://edu.csdn.net/lecturer/90](https://edu.csdn.net/lecturer/90)
- B站：[https://space.bilibili.com/275908810](https://space.bilibili.com/275908810)
- 知乎：[https://www.zhihu.com/people/www.100ask/](https://www.zhihu.com/people/www.100ask/)
- 微博：[https://weibo.com/888wds](https://weibo.com/888wds)
- 电子发烧友学院：[http://t.elecfans.com/teacher/3.html](http://t.elecfans.com/teacher/3.html)


公司名称：深圳百问网科技有限公司
-  电话: 0755-86200561
-  技术支持邮箱: weidongshan@qq.com 
-  地        址: 广东省深圳市龙岗区布吉南湾街道平吉大道建昇大厦B座
-  邮        编: 518114 

