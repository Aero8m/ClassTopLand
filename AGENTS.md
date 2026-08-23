# AGENTS.md

本文档面向在本仓库中工作的自动化开发代理与贡献者。目标是让修改建立在项目真实结构、数据格式和运行约束之上，而不是把本项目当作普通的 Qt 示例工程处理。

## 1. 项目概览

ClassTopLand 是一个使用 **C++23 + Qt 6 Widgets** 开发的桌面课程显示组件。程序常驻系统托盘，在屏幕顶部显示当天课程、当前课程或课间剩余时间，并可在屏幕右下角显示日期倒计时。设置窗口负责编辑每周课表、附加课表、倒计时配置，以及从第三方 API 同步课表。

当前工程具有以下重要特征：

- 应用目标名称为 `ClassTopLand`。
- UI 技术栈是 Qt Widgets，不是 Qt Quick/QML。
- CMake 会自动运行 MOC、UIC 和 RCC。
- 项目支持 Windows、Linux 和 macOS，但窗口效果与构建依赖存在平台分支。
- 用户数据不保存在仓库中，而是写入 `QDir::homePath()/ClassTopLand_Data`。
- 程序关闭最后一个普通窗口时不会自动退出；正常退出入口位于系统托盘菜单。
- 仓库当前没有自动化测试目标，也没有 CTest 配置。
- 项目采用 GPLv2 许可证。引入第三方代码或资源前必须确认许可证兼容性并保留必要声明。

## 2. 指令作用范围

本文件位于仓库根目录，适用于整个仓库。若未来某个子目录出现更具体的 `AGENTS.md`，在该子目录内应同时遵循本文件和更深层文件；发生冲突时，以更深层文件为准。

执行任务时：

1. 先阅读用户请求、本文档、`README.md`、`CMakeLists.txt` 以及与任务直接相关的源码。
2. 修改前运行 `git status --short`，识别用户已有的未提交修改。
3. 不覆盖、不回退、不顺手整理与当前任务无关的改动。
4. 保持修改范围最小，不进行无关重构、全仓格式化或批量重命名。
5. 完成后至少进行一次与改动相匹配的构建或检查，并如实报告未能验证的部分。

## 3. 仓库结构与模块职责

```text
ClassTopLand/
├── CMakeLists.txt                 # 唯一的顶层构建配置
├── main.cpp                       # QApplication 初始化与两个主窗口的创建
├── res.qrc                        # Qt 资源清单
├── resource.rc                    # Windows 可执行文件资源
├── Info.plist                     # macOS Bundle 信息
├── icon.ico / app.icns            # Windows/macOS 应用图标
├── qss/
│   └── global.qss                 # 全局 Qt Style Sheet
├── res/                           # 图片、字体、启动 Logo 等内嵌资源
└── src/
    ├── MainTableWidget/           # 顶部课程条、计时线程、托盘菜单、临时换课
    ├── TableEditWidget/           # 设置主窗口、课表编辑、倒计时编辑
    ├── AppendixTableManager/      # 附加课表的新增、删除和编辑入口
    ├── DayTimerWidget/            # 屏幕右下角日期倒计时
    ├── ExportAPISettingsTab/      # 第三方导入 API 列表、鉴权和课表同步
    ├── NetworkRequests/           # 对 QNetworkAccessManager 的轻量 JSON 封装
    ├── GlassHelper/               # Windows/X11/macOS 的透明或模糊背景适配
    ├── FluentTabWidget/           # 自绘的纵向标签控件
    ├── ClickLabel/                # 支持点击、双击和悬停动画的 QLabel
    ├── AppLog/                    # 带颜色和时间戳的控制台日志
    ├── Utils/                     # JSON 和样式表读取工具
    ├── CloudAPI.h                 # 云端 API 基础地址和 URL 宏
    └── VERSION.h                  # 构建系统替换的版本占位符与产品信息
```

### 3.1 程序启动链路

`main.cpp` 的主要顺序是：

1. 创建 `QApplication`；非 Linux 平台强制使用浅色方案。
2. 输出内嵌的 `:/res/logo.txt`。
3. 创建用户目录 `~/ClassTopLand_Data`。
4. 从 `:/qss/global.qss` 加载全局样式。
5. 创建并显示 `MainTableWidget`。
6. 创建 `DayTimerWidget`；仅当 `config.json` 中的 `disable_timer` 不为 `true` 时显示。
7. 进入 Qt 事件循环。

修改初始化顺序时要特别小心：多个窗口的构造函数会立即读取用户 JSON 文件，资源字体和样式也依赖 `res.qrc` 已正确编译。

### 3.2 主课程条

`MainTableWidget` 同时负责：

- 读取当前星期的课表；
- 创建屏幕顶部的无边框、置顶、不抢焦点窗口；
- 管理课程文字块和隐藏/展开动画；
- 创建系统托盘及“设置”“临时换课”“重启”“退出”动作；
- 在日期变化时通过启动新进程的方式重启程序；
- 启停 `RefetchTableThread`。

`RefetchTableThread` 根据课程开始/结束时间循环计算显示状态。它只持有一份 `QJsonArray` 课程快照，不应直接访问任何 QWidget。所有 UI 更新必须通过信号发送，并在主线程中以 queued connection 处理。

### 3.3 设置与数据编辑

`TableEditWidget` 是设置主窗口。关闭它时，窗口会隐藏、发出 `refetchTableSignal()`，并忽略真正的 close 事件。不要把普通的“关闭设置”误改为退出整个应用。

课表编辑支持：

- 周一到周日固定课表；
- 用户命名的附加课表；
- 添加、删除和直接编辑单元格；
- 按课程开始时间排序；
- 对 `HH:mm` 时间格式以及开始时间早于结束时间进行校验。

`ExportAPISettingsTab` 会从服务器取得 API 提供商列表，使用用户名和密码换取 token，再下载完整课表。服务器响应在落盘前必须经过结构和时间校验，并使用 `QSaveFile` 原子替换 `tables.json`。不要弱化这些校验或改回直接覆盖文件。

## 4. 构建环境

### 4.1 必需依赖

- CMake 3.16 或更高版本；
- 支持 C++23 的编译器；
- Qt 6，至少包含 `Core`、`Gui`、`Widgets`、`Network`；
- Linux 额外需要 X11 开发库；
- Ninja 是当前本机构建目录使用的生成器，但不是源码层面的硬性要求。

`CMakeLists.txt` 内含作者机器上的默认 Qt 路径：

- Windows：`C:/Qt/6.9.3/msvc2022_64/lib/cmake`
- Linux：`/home/jiahang/Qt/6.7.3/gcc_64/lib/cmake/`
- macOS：`/Users/jiahang/Qt/6.7.3/macos/lib/cmake/`

这些只是回退值。其他环境应显式设置 `CMAKE_PREFIX_PATH`，不要为了适配单台机器而反复修改并提交默认路径。

### 4.2 当前 Windows/CLion 环境

仓库现有的 `cmake-build-debug` 和 `cmake-build-release` 缓存使用：

- CLion 2025.2.3 内置 CMake；
- CLion 内置 Ninja；
- `C:/Program Files/LLVM/bin/clang-cl.exe`；
- Qt 6.9.3 的 MSVC 2022 64 位构建。

普通 PowerShell 中的 `cmake` 可能不在 `PATH`。此时可直接使用：

```powershell
& 'C:\Program Files\JetBrains\CLion 2025.2.3\bin\cmake\win\x64\bin\cmake.exe' --build cmake-build-debug --parallel
```

如果 CLion 版本或安装位置变化，应先从已有缓存定位实际路径：

```powershell
rg '^(CMAKE_COMMAND|CMAKE_MAKE_PROGRAM):' cmake-build-debug/CMakeCache.txt
```

不要把某个开发者的 CLion 绝对路径写入 `CMakeLists.txt`。

### 4.3 从零配置

Windows + Ninja 示例：

```powershell
cmake -S . -B cmake-build-debug -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_PREFIX_PATH='C:/Qt/6.9.3/msvc2022_64/lib/cmake'
cmake --build cmake-build-debug --parallel
```

Release 构建：

```powershell
cmake -S . -B cmake-build-release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH='C:/Qt/6.9.3/msvc2022_64/lib/cmake'
cmake --build cmake-build-release --parallel
```

Linux/macOS 使用相同的 `cmake -S/-B` 模式并替换 Qt 路径。Linux 配置阶段还会执行 `find_package(X11 REQUIRED)`，链接 `X11_LIBRARIES`，并添加 `-static-libstdc++ -static-libgcc`。

### 4.4 运行

在当前 Windows Debug 构建中：

```powershell
.\cmake-build-debug\ClassTopLand.exe
```

如果系统找不到 Qt DLL，应从配置了 Qt 运行环境的 CLion 中启动，或先进行本地部署；仓库目前没有正式的 `windeployqt`/打包脚本。不要把复制出的 Qt DLL 或平台插件提交到源码目录。

应用是托盘常驻程序。人工测试结束时应通过托盘“退出”动作关闭它，确认没有遗留 `ClassTopLand.exe` 进程。

## 5. 用户数据与 JSON 契约

运行时目录固定为：

```text
QDir::homePath()/ClassTopLand_Data/
├── config.json
└── tables.json
```

这两个文件是用户数据，不属于仓库资源。测试涉及写入时，不要覆盖开发者的真实课表；应先备份，或使用独立的测试用户环境。任何测试生成的数据都不应提交。

### 5.1 `tables.json`

完整结构如下：

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

必须保持的契约：

- 星期键固定为英文缩写 `Mon`、`Tue`、`Wed`、`Thu`、`Fri`、`Sat`、`Sun`，与 `QDateTime::toString("ddd")` 的当前使用方式一致。
- 每一天和每个附加课表的值都是数组。
- 每门课程是对象，包含非空 `name`、`start`、`end`。
- 时间格式为 24 小时制 `HH:mm`，且 `start < end`。
- 编辑后的课程数组应按 `start` 升序排列。
- 修改嵌套的 `appendixTables` 时，先取出子对象，修改后再赋回顶层 `QJsonObject`，避免修改临时值而没有落盘。

### 5.2 `config.json`

设置窗口当前会读写以下字段：

```json
{
  "disable_timer": false,
  "end_time": "2026-09-01 08:00:00",
  "label_tag": "开学",
  "english_tag": "School starts",
  "english": "There are () $\nleft until School starts",
  "english_end": "There is not a $\nleft until School starts"
}
```

字段含义：

- `disable_timer`：`true` 表示不显示倒计时；名称是反向语义，使用时不要误判。
- `end_time`：格式为 `yyyy-MM-dd hh:mm:ss`。
- `label_tag`：中文标题，界面显示为“距离……”；
- `english_tag`：设置页保存英文名称时使用；
- `english`：倒计时进行中模板，`()` 会替换为数值，`$` 会替换为大写单位；
- `english_end`：倒计时结束模板，`$` 会替换为 `DAY`。

`readJsonFile()` 读取失败时返回 `std::nullopt` 并记录日志。新增读取逻辑时必须处理缺失、不可读、根节点类型错误和 JSON 解析失败，不能直接假定文件有效。

## 6. Qt 与 C++ 修改约定

### 6.1 通用原则

- 保持 C++23 和现有 Qt 6 API，不引入 Qt 5 兼容写法。
- 尊重所在文件的局部风格；仓库历史格式并不完全统一，不要借功能修改之机格式化整个文件。
- 新代码使用 4 空格缩进，避免制表符；头文件和实现文件同步更新。
- 参数和局部变量尽量使用明确的 Qt 类型；跨线程传递数据时优先传值或不可变快照。
- 用户可见文字目前主要是中文。新增文字应与所在界面语言保持一致，并在适合的位置使用 `tr()`。
- 日志使用 `showLog()` 或 Qt 自带的 `qWarning()/qDebug()`，不得输出密码、token、Cookie 或完整鉴权响应。
- 不要静默忽略文件打开、网络请求、JSON 解析和进程启动失败。

### 6.2 QObject 所有权与生命周期

Qt 对象应优先通过 parent 管理；如果对象没有 parent，则必须明确在析构函数中释放。修改以下已有模式时需保持生命周期一致：

- `MainTableWidget` 手动管理 `editWindow`、`refetchThread` 和无 parent 的 `topTimer`；
- `restartTimer` 以 `this` 为 parent；
- `QNetworkReply` 在处理后调用 `deleteLater()`；
- `AppendixTableManager` 使用 `WA_DeleteOnClose`；
- API 请求对象在重复请求前会删除并重新创建。

连接 lambda 时注意捕获对象的生命周期。异步回调中若目标可能提前销毁，应依赖 context object 自动断开连接，或使用安全的 Qt 指针策略。

### 6.3 信号、槽与线程

- QWidget 只能在主线程访问。
- `RefetchTableThread::run()` 内只做时间计算和发射信号。
- 工作线程到界面的连接应保持 `Qt::QueuedConnection`。
- 更新 `todayTable` 前必须确保线程未运行；`setTodayTable()` 内已有 `Q_ASSERT(!isRunning())`。
- 停止线程的顺序是 `requestInterruption()`，循环检查中断，然后 `wait()`。不要使用 `terminate()`。
- 所有新增的长循环都必须定期检查 `isInterruptionRequested()`，并避免忙等。
- 修改课程标签数量或索引时，要同时验证 `setClassStyleSheet` 的目标索引，防止跨线程信号到达时访问不存在的控件。

### 6.4 UI 文件

- `.ui` 文件是 Qt Designer 源文件，可以直接在 Designer 中修改，也可以谨慎编辑 XML。
- 绝对不要编辑构建目录中生成的 `ui_*.h`、`moc_*.cpp`、`qrc_*.cpp`。
- 改动 object name 后，必须全仓搜索其在 C++、QSS 和信号连接中的引用。
- 使用自定义控件时，保持 `.ui` 底部 `<customwidgets>` 声明与真实头文件一致。
- 布局修改应考虑主课程条的动态宽度、`SIDEBAR_WIDTH`、`CLASS_BLOCK_SIZE`、`BLOCK_SPACING` 和动画几何值。
- UI 改动完成后要实际运行检查，而不能只以“编译通过”作为验收。

### 6.5 样式与资源

- 全局样式位于 `qss/global.qss`，通过资源路径 `:/qss/global.qss` 加载。
- 图片和字体位于 `res/`，运行时通常通过 `:/res/...` 访问。
- 新增、删除或重命名资源时必须同步修改 `res.qrc`。
- 不要使用源码目录的相对磁盘路径代替 Qt 资源路径，否则打包后会失效。
- 字体加载依赖 `QFontDatabase::addApplicationFont()`；替换字体时应检查加载失败及空 family 列表的情况。
- 平台特定的窗口透明、模糊或置顶代码集中在 `GlassHelper` 和 `MainTableWidget`，不要把新的平台 API 散落到无关模块。

### 6.6 CMake

- 新源码位于 `src/` 下时，`file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` 会自动收集 `.cpp`、`.h` 和 `.ui`，但新增文件后仍需让 CMake 重新检查 glob。
- 新 Qt 模块必须同时加入 `find_package(Qt6 COMPONENTS ...)` 和 `target_link_libraries()`。
- 平台代码使用清晰的 `WIN32`、`UNIX AND NOT APPLE`、`APPLE` 或编译器宏保护。
- 不提交 `cmake-build-*`、`build/`、IDE 用户配置或生成文件。
- 修改构建配置后至少验证一次全新配置或明确说明只验证了增量构建。

## 7. 网络与安全约定

云服务地址集中定义在 `src/CloudAPI.h`。当前流程涉及 `/exapi/list`、`get_token` 和 `get_classtable`。

修改网络代码时：

- 保持请求异步，不要在 GUI 线程等待网络结果。
- 区分传输错误、HTTP 状态错误、JSON 解析错误和业务字段缺失。
- 响应根节点必须是 JSON object；需要其他根类型时应显式扩展接口，而不是绕过检查。
- 密码和 token 只用于请求，不写入日志、不写入仓库、不放进错误消息。
- 不擅自更换服务器地址、降低 TLS 安全性或接受无效证书。
- 下载课表后先验证全部结构，再使用 `QSaveFile` 提交。
- 自动化验证默认不要调用真实服务。只有任务明确需要且用户授权时，才进行会发送凭据或改变远端状态的测试。

## 8. 平台注意事项

### Windows

- 顶部窗口每 3 秒通过 `SetWindowPos(..., HWND_TOPMOST, ...)` 维持置顶。
- 亚克力效果使用动态取得的 `SetWindowCompositionAttribute`。
- Qt 套件、编译器 ABI 和架构必须匹配；当前是 Qt MSVC 2022 64 位配合 clang-cl。
- `resource.rc` 和 `icon.ico` 参与 Windows 可执行文件资源构建。

### Linux

- CMake 必须找到 X11。
- 模糊背景只在 Qt 平台名为 `xcb` 或 `x11` 时设置 KDE blur property；Wayland 下不保证生效。
- 链接参数包含静态 libstdc++/libgcc，但 Qt 和 X11 本身并未因此自动变成全静态。
- 不要把仅在 X11 可用的实现无条件用于 Wayland。

### macOS

- 目标以 Bundle 方式构建，使用 `Info.plist` 和 `app.icns`。
- 当前模糊效果是半透明样式回退，并非原生 NSVisualEffectView。
- 修改版本、Bundle identifier 或图标时，检查 CMake target properties、plist 和资源位置是否一致。

## 9. 验证要求

仓库目前没有单元测试或集成测试，因此“测试通过”不能表述为运行了不存在的测试套件。根据修改类型选择以下检查。

### 9.1 每次代码修改的最低检查

```powershell
git diff --check
& 'C:\Program Files\JetBrains\CLion 2025.2.3\bin\cmake\win\x64\bin\cmake.exe' --build cmake-build-debug --parallel
git status --short
```

若当前环境中 `cmake` 已在 `PATH`，可使用通用的 `cmake --build ...` 命令。构建输出必须没有编译错误；新增警告也应处理，工程在 MSVC 下启用 `/W4`，其他编译器启用 `-Wall -Wextra`。

### 9.2 按改动类型进行人工检查

课表逻辑：

- 当天无课程；
- 第一节课前；
- 上课中；
- 两节课之间；
- 最后一节课结束后；
- 临时换到另一个星期；
- 临时换到附加课表；
- 设置窗口关闭后主窗口重新加载。

课表编辑：

- 添加后按开始时间排序；
- 空课程名被拒绝；
- 非法时间、开始晚于结束、结束早于开始均被拒绝；
- 直接编辑开始时间后顺序刷新；
- 删除普通课表和附加课表项目；
- JSON 重启后仍可正确读取。

倒计时：

- `disable_timer` 的显示/隐藏语义正确；
- 未来日期按天、小时、分钟、秒和毫秒切换；
- 到期后停止定时器并显示结束文本；
- 资源字体可以加载，长文本没有明显裁切。

网络同步：

- API 列表请求失败时有清楚提示；
- 空凭据和未选 API 被拒绝；
- 空 token、HTTP 错误和非对象 JSON 被拒绝；
- 非法课表不会覆盖本地文件；
- 有效课表通过 `QSaveFile` 完整写入。

UI/平台行为：

- 顶部窗口位置、动态宽度、隐藏/展开动画正确；
- 设置窗口可从托盘和单击托盘图标打开；
- 关闭设置不退出程序；
- 托盘“重启”和“退出”工作正常；
- 没有退出后仍存活的线程或进程。

无法执行 GUI、网络或某个平台验证时，最终说明中要明确列出“已验证”和“未验证”，不要用构建成功代替运行验证。

## 10. 常见陷阱

- 不要把 `disable_timer == true` 理解为显示倒计时。
- 不要在 `RefetchTableThread` 运行期间调用 `setTodayTable()`。
- 不要从工作线程直接修改 QLabel、QStackedWidget 或其他 GUI 对象。
- 不要编辑 `cmake-build-*` 中的生成文件来修复源码问题。
- 不要忘记将新增资源加入 `res.qrc`。
- 不要假设 `config.json` 和 `tables.json` 必然存在或格式正确。
- 不要用直接覆盖替代同步课表时的 `QSaveFile` 原子写入。
- 不要在日志中打印 API 密码、token 或 Cookie。
- 不要依赖星期中文名称作为持久化键；持久化键是 `Mon` 到 `Sun`。
- 不要删除 `QApplication::setQuitOnLastWindowClosed(false)`，除非任务明确要求改变托盘常驻行为。
- 不要随意改变主窗口常量或 `.ui` 几何值；动画、隐藏宽度和课程块数量相互耦合。
- 不要用 `QThread::terminate()` 处理退出；保持中断请求和 `wait()` 的有序关闭。
- 不要把本机绝对 Qt/CLion 路径、用户数据或构建产物提交到仓库。

## 11. 提交前交付清单

完成任务前逐项确认：

- [ ] 修改只覆盖用户要求，没有回退已有工作区改动。
- [ ] 新增或修改的 `.ui`、C++、QSS 和资源引用保持一致。
- [ ] JSON 字段、时间格式和课表排序契约未被破坏。
- [ ] QObject 所有权、异步回调和线程退出路径清晰。
- [ ] 用户凭据与个人数据没有进入日志、补丁或测试文件。
- [ ] Debug 构建成功，或已清楚记录构建受阻原因。
- [ ] 相关人工场景已验证，无法验证的场景已单独列出。
- [ ] `git diff --check` 无空白错误。
- [ ] `git status --short` 中没有意外生成物。
- [ ] 最终回复简要说明改了什么、如何验证、还有什么限制。

