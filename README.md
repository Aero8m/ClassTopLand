<p align="center">
  <img width="18%" alt="ClassTopLand 图标" src="https://cdn.luogu.com.cn/upload/image_hosting/cq2imakn.png">
</p>

<h1 align="center">ClassTopLand</h1>

<p align="center">
  基于 Qt 6 Widgets 的桌面课程显示组件
</p>

<p align="center">
  <a href="https://github.com/Aero80wd/ClassTopLand/releases">
    <img src="https://img.shields.io/badge/Version-Rolling%20Build-2334D05" alt="Rolling Build">
  </a>
  <a href="./LICENSE">
    <img src="https://img.shields.io/badge/License-GPLv2-4ec820" alt="GPLv2">
  </a>
  <a href="https://github.com/Aero80wd/ClassTopLand/releases">
    <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-4ec820" alt="Windows | Linux | macOS">
  </a>
</p>


## 主要功能

- 在屏幕顶部显示当天的全部课程。
- 根据时间显示课前、上课中、课间和课程结束状态。
- 编辑周一至周日课表，并按课程开始时间排序。
- 创建、编辑和删除附加课表。
- 从托盘临时切换到其他星期或附加课表。
- 自定义重要日期倒计时及其中英文显示文字。
- 通过第三方 API 同步课表。
- 支持课程条隐藏和展开，并保持窗口置顶。
- 通过系统托盘打开设置、临时换课、重启或退出程序。

## 下载与安装

1. 从 [GitHub Releases](https://github.com/Aero80wd/ClassTopLand/releases) 下载适合当前平台的版本。网络环境不佳时，也可以使用 [Aero8m AutoBuild](https://autobuild.aero8m.cn/ClassTopLand)。
2. 安装或解压后启动 ClassTopLand。
3. 从系统托盘打开设置并填写课表。

如果 Windows 提示缺少 Microsoft Visual C++ 运行库，请安装 [Microsoft Visual C++ 2015–2022 Redistributable（x64）](https://aka.ms/vs/17/release/vc_redist.x64.exe)。发布包是否包含 Qt 运行库，请以对应版本的发布说明为准。

## 首次使用

1. 启动程序后，在系统托盘中找到 ClassTopLand 图标。
2. 单击托盘图标，或右键选择“设置”，进入课表编辑页面。
3. 选择星期，填写课程名称、上课时间和下课时间，然后添加课程。
4. 如需日期倒计时，在“倒计时设置”中填写目标时间和名称并保存。倒计时设置需要重启后生效。
5. 如需临时使用其他星期或附加课表，从托盘菜单选择“临时换课”。

关闭设置窗口只会隐藏设置并重新加载课表，不会退出程序。需要完全退出时，请使用托盘菜单中的“退出”。

## 从源码构建

### 依赖

- CMake 3.16 或更高版本；
- 支持 C++23 的编译器；
- Qt 6，包含 `Core`、`Gui`、`Widgets` 和 `Network` 模块；
- Ninja 或其他受 CMake 支持的构建工具；
- Linux 额外需要 X11 开发库。

编译器、Qt 套件、架构和 ABI 必须匹配。例如，在 Windows 上使用 MSVC ABI 的 Qt 时，应配合 MSVC 或兼容的 clang-cl 工具链。

### Windows

以下示例使用 PowerShell 和 Ninja。请将 Qt 路径替换为本机实际安装位置：

```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64/lib/cmake"
cmake --build build --parallel
```

构建完成后可运行：

```powershell
.\build\ClassTopLand.exe
```

### Linux 与 macOS

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/path/to/Qt/6.x.x/platform/lib/cmake"
cmake --build build --parallel
```

Linux 配置阶段需要能够找到 X11。macOS 构建结果为应用 Bundle。

## 用户数据与备份

ClassTopLand 将用户数据保存在用户主目录下，不会写入源码目录：

```text
ClassTopLand_Data/
├── config.json
└── tables.json
```

常见完整路径：

- Windows：`%USERPROFILE%\ClassTopLand_Data`
- Linux/macOS：`$HOME/ClassTopLand_Data`

`tables.json` 的基本结构如下：

```json
{
  "Mon": [
    { "name": "语文", "start": "08:00", "end": "08:45" }
  ],
  "Tue": [],
  "Wed": [],
  "Thu": [],
  "Fri": [],
  "Sat": [],
  "Sun": [],
  "appendixTables": {
    "考试周": [
      { "name": "数学", "start": "09:00", "end": "10:30" }
    ]
  }
}
```

手动编辑时请保持以下格式：

- 星期键固定为 `Mon`、`Tue`、`Wed`、`Thu`、`Fri`、`Sat` 和 `Sun`；
- 每门课程都需要非空的 `name`、`start` 和 `end`；
- 时间使用 24 小时制 `HH:mm`，并确保开始时间早于结束时间；
- 课程应按开始时间升序排列。

第三方 API 同步会替换本地课表。同步前建议备份 `tables.json`。

## 平台说明

- Windows：支持置顶和亚克力窗口效果；发布版通常需要 x64 Visual C++ 运行库。
- Linux：构建依赖 X11；模糊背景只在 `xcb` 或 X11 环境下设置，Wayland 下不保证具有相同效果。
- macOS：以应用 Bundle 形式构建；当前使用半透明样式作为模糊效果的回退实现。

源码主要适配Windows平台，在Linux和macOS上不适配的功能会在编译时禁用。
可下载版本和实际验证范围可能随构建批次变化，请以 [Releases](https://github.com/Aero8m/ClassTopLand/releases) 中的说明为准。

## 常见问题

### 启动后找不到设置窗口

程序会常驻系统托盘。请检查任务栏或菜单栏的托盘区域以及被折叠的图标，然后单击 ClassTopLand 图标。

### 关闭窗口后程序仍在运行

这是托盘常驻程序的预期行为。请使用托盘菜单中的“退出”完全关闭程序。

### 顶部没有显示课程

请确认当天课表不是空的，课程时间格式正确，并尝试关闭设置窗口以重新加载课表。也可以检查 `ClassTopLand_Data/tables.json` 是否存在且为有效 JSON。

### Windows 提示缺少 DLL

先安装上文链接的 Visual C++ x64 运行库。如果缺少的是 Qt DLL，请重新下载完整发布包，或使用已配置 Qt 运行环境的开发终端运行源码构建结果。

## 参与开发

欢迎通过 Issue 或 Pull Request 报告问题和改进项目。提交修改前建议至少完成：

```powershell
cmake --build build --parallel
git diff --check
```

仓库当前没有自动化测试目标。涉及界面、托盘、课表状态或平台窗口效果的修改，还需要在对应平台进行人工验证。请勿提交 `build/`、`cmake-build-*`、Qt 运行库、IDE 用户配置或 `ClassTopLand_Data` 中的个人数据。

版本变化可查看 [Releases](https://github.com/Aero80wd/ClassTopLand/releases) 和 [提交记录](https://github.com/Aero80wd/ClassTopLand/commits)。

## 许可证

本项目采用 [GNU General Public License v2](./LICENSE) 开源。复制、修改或分发本项目时，请遵守许可证中的相应条款。
