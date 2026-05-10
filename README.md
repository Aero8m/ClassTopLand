<p align="center">
  <img width="18%" alt="icon" src="https://cdn.luogu.com.cn/upload/image_hosting/cq2imakn.png">

</p>
  <h1 align="center">
  ClassTopLand
</h1>
<h3 style="text-align: center">注意：新发行版采用自动构建，请前往 安装 部分的链接下载！</h3>

<p align="center">
  基于Qt6 Widgets的校园桌面课程显示组件
</p>
<p align="center">
    <img src="https://img.shields.io/badge/Version-v1.0-2334D05" alt="Version">

  <a style="text-decoration:none" href="/LICENSE">
    <img src="https://img.shields.io/badge/License-GPLv2-blue?color=#4ec820" alt="GPLv2"/>
  </a>

  <a style="text-decoration:none" href="https://github.com/Aero80wd/ClassTopLand/releases">
    <img src="https://img.shields.io/badge/Platform-Win32%20|%20Linux%20|%20macOS-blue?color=#4ec820" alt="Platform Win32 | Linux | macOS"/>
    h
  </a>
</p>






## 功能
- 在屏幕上方显示今日的课程
- 在一言窗口上显示每日诗词
- 在悬浮窗或主窗口显示Todo
- 在屏幕侧边显示工具栏，可显示插件或快捷方式。
> **注意**<br/>
> 本项目以<a style="text-decoration:none" href="/LICENSE">GPLv2</a>协议开源，如需修改，请开源并遵守协议


## 安装
1. 下载<a href="https://autobuild.aero8m.cn/ClassTopLand">https://autobuild.aero8m.cn/ClassTopLand</a>内的安装包，根据安装包提示进行安装。
> **提示**<br/>
> 最上面的安装包通常是最新版本，列表按时间倒序排序！

2. 打开程序后根据提示进行设置。
3. 开始使用。
> **注意**
> 如运行出现缺失DLL，请下载安装[VS2019运行库](https://aka.ms/vs/17/release/vc_redist.x64.exe)。
## 更新日志
### 新更新日志请查看提交记录！
### [v7.0] - 2025-7-27
- 更改设置UI
- 更改软件存储位置为用户主目录下的ClassTopLand_Data目录
### [v6.2] - 2025-2-23
- 重构课程更新机制（md原来的我现在都看不懂😰😰😰）
- 删除检查更新功能（I FXXK GITEE！！！😡😡😡😡😡😡😡😡😡😡）
- 修复打开设置后打开其他窗口关闭后崩溃的bug
### [v6.1] - 2025-2-15
- 暂时关闭同步功能
- 添加检查更新功能
- 修复了一些bug（记不清了）
### [6.0] - 2025-1-12 
Happy New Year!
大大大大大大大更新
- 将软件名改为ClassTopLand并更改图标
- 修复倒计时位置bug
- 大量修改设置ui
- 增加希沃对抗功能，防止希沃置顶（使用了曲柄检测，对希沃只隐藏窗口可用）
### [v5.4] - 2024-11-24
- 将隐藏窗口箭头改为色块
- 修复倒计时隐藏bug
- 添加临时课表
- 添加调试码
### [v5.3] - 2024-11-3
- 添加倒计时
### [v5.2] - 2024-10-27
- 添加课表隐藏功能
- 将运势改为天气
### [v5.1] - 2024-10-13
- 将一言回归到工具箱
- 修复当课间为0时当前课程显示错误的bug
- 修复编辑器关闭后的课程表空白bug
- 修复换课时没有resize的bug
### [v5.0] - 2024-10-5
- UI大更新：
1. 将灵动岛连体，更改主题色，添加毛玻璃效果
2. 将一言和岛上Todo删除，一言将会在5.1回归。
3. 在悬浮窗上显示所有课程
- 支持课表同步，可同步服务端上的课表。
- ~~移除了him~~
#### [v4.1] - 2024-9-15
- 该版本为第一个较稳定版本，功能bug基本修复完毕
- 更改项目目录结构，将源文件、头文件、UI文件、资源文件进行分类
- 使用资源文件字体避免需安装字体后才可使用倒计时

## 关于插件
### 安装
打开设置中的工具栏设置，将插件文件或快捷方式放入插件列表中。
### 制作
请查看[SchoolTools-Plugin-example-win32](https://github.com/Aero80wd/SchoolTools-Plugin-example-win32)仓库的示例和README。
## 关于同步
具体见[SchoolTools-Sync-Server](https://github.com/Aero80wd/SchoolTools-Sync-Server)仓库。
## 许可证
本项目使用<a style="text-decoration:none" href="/LICENSE">GPLv2</a>协议开源，修改请遵循协议。
