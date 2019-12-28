# Visual BDCoder

By Dict Xiong.

[项目主页](https://github.com/DictXiong/Visual-BDCoder)



## 项目介绍

可视化的 BDCoder ([项目地址]( https://github.com/DictXiong/bdcoder )). 基于 FFmpeg ([主页](http://ffmpeg.org/)) 与 MFC, 旨在提供简单快捷的视频转码功能. 

我个人常有视频转码需求, 而一些商业软件收费且配置复杂, 一些个人免费软件功能又不全面, 常用的视频转码配置也就只有少数几套. 因此之前写了 BDCoder, 基于简单的几种预设, 采用拖放与命令行结合方式完成转码. 但是它的不足也十分明显: 当需要一次性转码多个不同预设、不同裁剪方式的视频时, 需要来回处理好几次, 耗费时间与精力; 同时 BDCoder 也不支持 H265, 不支持 NVENC, 不支持自定义预设, 对其它用户不友好. 因此, BDCoder 需要一次彻头彻尾的更新. 

在新时代的号召下, 历经数不清的日日夜夜, 勇于担当、攻坚克难, 适应时代潮流、贴合用户需求的 Visual BDCoder 横空出世, 再一次在时代的挑战中站稳了脚跟. 

*如果你对参与项目开发感兴趣, 请毫不犹豫地提交 PR.



## 功能特点

- 完全基于 FFmpeg, 没有任何功能限制.
- 支持鼠标拖放多文件到列表中. 
- 支持 H264 与 H265 格式, 支持 2pass, CRF 和 VBR (nvenc) 模式, 支持复制视频/音频流! 
- 支持视频剪切!
- 支持一次性同时更改多个项目的转码设置: 只需要选中列表中多个项目. 
- 使用管道与多线程启动 FFmpeg, 在窗口中即可看到转码实时输出信息. 
- 支持暂停/当前项目完成后停止/立即停止等操作.
- 支持自定义输出文件夹. 留空则默认为当前文件夹. 
- 列表中的项目, 选中按 Delete 键即可删除.
- 双击列表中的项目, 可用系统关联的程序打开之. 
- 支持重置转码状态.



## 安装与使用

请先[下载 FFmpeg](http://ffmpeg.org/download.html), **保证 FFmpeg 在环境变量中** (即, 命令行中键入 `ffmpeg` 就能够运行 ffmpeg).



## 许可证

[MIT License](https://github.com/DictXiong/Visual-BDCoder/blob/master/LICENSE)



## TO-DO

- 预设管理相关功能尚在开发. 
- 管道功能不太正常.
- 应当给个 gop 的设置. 



# 开发文档



## `InputFile` 类

对应列表中的项目, 存储文件名、转码参数等. 也用于暂存变量与预设的存储. 

类拥有一系列的 set/get 函数, 有将转码配置转换为 FFmpeg 命令的核心函数, 有将转码配置转换为列表信息的函数.



##### `InputFile::InputFile(CString, int, int, int, int, bool, bool)`

构造函数. 

参数表: 路径, 格式, 编码, 视频码率 (CRF), 音频码率, 是否复制视频流, 是否复制音频流. 

注意: 未指定的编码, CRF 将设置为 25, 码率设置为 5000.



##### `InputFile::InputFile(CString, InputFile)`

构造函数.

参数表: 路径, 设置 (`InputFile`)

使用现有的 `InputFile` 的设置来初始化.

注意: 未指定的编码, CRF 将设置为 25, 码率设置为 5000.



##### `std::vector<CString> InputFile::translate(CString)`

`CString`: 输出目录

根据当前文件的转码设置, 生成一组转码命令行 (一个或者两个). 如果 `CString`为空, 那么设定为当前文件夹.

注: 只有使用 2pass 的时候, 才会有两个命令. 



##### `std::vector<CString> InputFile::getItemInfo()`

将转码设置转换为人类可读的字符数组. 用于显示视图中的列表. 



##### `std::vector<int> InputFile::getData()`

将转码设置转换为一组数值.



##### `bool InputFile::setData(std::vector<int>, bool)`

`vector<int>`: 用一组数值表示的转码设置, bool: 标记是否写 -1 值, 默认为真.

设置转码设置为输入的转码设置.



##### `bool InputFile::setData(InputFile, bool, bool)`

`vector<int>`: 用一组数值表示的转码设置, bool: 标记是否写 -1 值, 默认为真, bool: 标记是否写未指定的编码的设置, 默认为否.

重载函数. 设置转码设置为输入的转码设置.



##### 其它 set/get 函数

```c++
int setVideoBitrate(int);
int getVideoBitrate();
int setAudioBitrate(int);
int getStatus();
int setStatus(int);
bool setCut(bool);
int setCode(int);
int setFormat(int);
bool setCopyVideoStream(bool);
bool setCopyAudioStream(bool);
CString getPath();
std::pair<int, int> setCutRange(int, int);
std::pair<int, int> setCutRange(std::pair<int, int>);
```


## `CMyListCtrl` 类 (继承于 `CListCtrl`)

##### `void CMyListCtrl::OnDropFiles(HDROP)`

消息处理函数.

重写了列表的拖放操作, 支持多文件同时拖放. 如果拖入的是文件, 则加入列表. 

注: 检测了后缀名, 如果不合法则弹出窗口提示.



## `CvbdcoderDlg` 类 (继承于 `CDialogEx`)

主对话框类.



##### `const int COLUMN`

列表列数



##### `const int MAX_ITEM`

列表容量



##### `const CString availableExts[]`

 支持的后缀名



##### `std::vector<InputFile> listItems`

存储的所有项目的容器. 某种意义上来说它类似于 `ListCtrl` 的一个值变量. 

通过 `updateList` 来把内容同步到视图列表中.



##### `std::vector<InputFile> presets`

预设.



##### `InputFile settingsNow`

暂存变量, 表示了当前的转码设置. 某种意义上来说它类似于转码配置区的值变量, 

通过 `updateSettings(bool)` 来同步设置.



##### `volatile BOOL threadRunning, toBeTerminated, terminateNow, suspendNow`

多线程操作的状态标记.



##### `PROCESS_INFORMATION ffmpegProcess`

FFmpeg 线程的信息, 用于线程的挂起与终止.



##### `CWinThread* cThread`

##### ` unsigned int ThreadID`

子线程的信息.



##### `BOOL CvbdcoderDlg::OnInitDialog()`

基本的初始化, 加入了:

- 快捷键
- ListCtrl 
- SPIN buddy
- 默认预设
- 时间选择器的默认显示
- 输出文件夹对话的 hint



##### `bool CvbdcoderDlg::updateList()`

把当前的 `listItems` 列表同步到视图的列表中.



##### `void CvbdcoderDlg::OnBnClickedCheckCut()`

##### `void CvbdcoderDlg::OnBnClickedCheckCopyVideoStream()`

##### `void CvbdcoderDlg::OnBnClickedCheckCopyAudioStream()`

##### `void CvbdcoderDlg::OnBnClickedRadioFormat(UINT)`

##### `void CvbdcoderDlg::OnBnClickedRadioCode(UINT)`

##### `void CvbdcoderDlg::OnEnChangeEditVideoBitrate()`

一系列的消息处理函数. 主要行为是: 首先将视图同步到关联变量 -> 修改暂存变量的值
-> 改变选中的项对应的 `listItems` 值 -> 将 `listItems` 同步到视图 
-> 将暂存变量同步到视图



##### `bool CvbdcoderDlg::addToListItems(CString, int)`

`CString`: 欲添加的文件的路径; `int`: 欲添加到列表中的位置, -1 表示添加到最后

把文件添加到 `listItems` 中

注意: 不会自动同步视图



##### `template<typename Lambda> bool CvbdcoderDlg::setListItem(Lambda)`

`Lambda`: 一个函数, 接受一个类型为 `InputFile` 的参数

将遍历当前列表中高亮选中的项目, 并使用 `Lambda` 作用于每个项目对应的 `listItems`.

注意: `Lambda` 的形参列表应当自带引用; 不自动更新到视图



##### `bool CvbdcoderDlg::setListItem()`

重载的函数. 将暂存变量的设置应用到每一个高亮选中的项目对应的 `listItems` 中. 

注意: 不自动更新到视图. 



##### `void CvbdcoderDlg::OnLvnItemchangedList(NMHDR*, LRESULT*)`

消息处理函数. 

当列表中改变高亮选中时, 更新暂存变量与视图的设置区



##### `bool CvbdcoderDlg::syncToSettings()`

重置视图中的预设列表为 `presets` 中的值



##### `bool CvbdcoderDlg::applySettings()`

将暂存变量应用到列表中高亮选中的项目, 并更新视图



##### `bool CvbdcoderDlg::updateSettings(bool)`

`bool`: 标记同步方向

当为 true 时, 将视图设置区的值更新到暂存变量中; 否则, 将暂存变量中的值更新到视图. 
支持暂存变量值为 -1 的情况 (这代表着多个值)

注: 效果类似于 `UpdateData`



##### `bool CvbdcoderDlg::RefreshSettingBox()`

单纯的视图操作. 从视图上获取值, 以此为凭据来保证各个编辑框的可用性 ([en/dis]able) 合理. 



##### `void CvbdcoderDlg::OnCbnSelchangeComboPreset()`

消息处理函数

当 preset 组合框改变时, 将对应的预设加载到暂存变量并同步到视图



##### `void CvbdcoderDlg::OnDtnDatetimechangeTimepicker(UINT, NMHDR*, LRESULT*)`

范围消息处理函数

当时间控件改变时, 同步到暂存变量、`listItems`、视图

注意: 该函数保证了终点时间大于起点



##### `void CvbdcoderDlg::OnAbout()`

消息处理函数

显示 "关于" 对话框



##### `BOOL FolderExist(CString)`

`CString`: 路径

基于 API, 返回目标文件是否存在且为目录



##### `BOOL CreateFolder(CString)`

`CString`: 路径

基于 API, 创建文件夹, 返回创建是否成功



##### `void CvbdcoderDlg::OnBnClickedButtonStart()`

消息处理函数. 点击 "开始" 按钮, 开始转码. 如果当前没有进行转码任务, 则创建线程开始转码. 

注: 函数会检测目标文件夹的合法性. 若不合法, 将弹出对话框后退出.



##### `UINT CvbdcoderDlg::runCoding(LPVOID)`

线程主体函数, 静态成员

接受类的一个实例指针, 找到列表中的第一个勾选并就绪的项目, 获取其 `listItems` 中的指针, 
使用 `InputFile` 的 `translate` 函数获得命令组. 使用管道运行 FFmpeg, 并显示到视图中. 
每次更新显示的间隔, 检测是否标记了 `terminateNow`, 若为真则立刻终止转码并退出;
每次寻找下一个转码项目之前, 检测是否标记了 `toBeTerminated`, 若为真则退出.

注: 调用函数前应保证 `listItems` 与视图同步



##### `void CvbdcoderDlg::OnResetStatus()`

消息处理函数. 

将 `listItems` 中所有项目的状态重置为就绪, 并更新视图. 



##### `void CvbdcoderDlg::OnBnClickedButtonStopafter()`

消息处理函数.

点击后, 标记 `toBeTerminated`, 使得 `runCoding` 在完成当前文件转码后停止.



##### `void CvbdcoderDlg::OnBnClickedButtonStopnow()`

消息处理函数

点击后, 标记 `terminateNow`, 使得 `runCoding` 立刻终止. 

注意: 如果线程被挂起, 则会先恢复线程.



##### `void CvbdcoderDlg::OnBnClickedButtonPause()`

消息处理函数

切换线程的挂起与恢复的状态, 并更新视图上对应按钮的文字



##### `void CvbdcoderDlg::resetControlArea()`

恢复各标记变量, 恢复视图中控制按钮的文字



##### `void CvbdcoderDlg::OnAcceleratorDelete()`

消息处理函数

响应键盘上 Delete 按钮. 删除列表中高亮选中的项目.

注意: 正在转码中的项目不会被删除. 



##### `BOOL CvbdcoderDlg::PreTranslateMessage(MSG*)`

重写虚函数, 以对 Delete 按钮做出响应. 



##### `void CvbdcoderDlg::OnBnClickedButton1()`

消息处理函数

弹出文件夹选择对话框, 选择一个文件夹并将其放入视图中的输出文件夹编辑框中.



##### `void CvbdcoderDlg::OnNMDblclkList(NMHDR*, LRESULT*)`

消息处理函数

当双击列表中的项目时, 使用系统关联的程序打开之.



##### `void CvbdcoderDlg::OnClearlist()`

消息处理函数

清空列表



##### `void CvbdcoderDlg::OnAddfile()`

消息处理函数

添加文件到列表



##### `bool CvbdcoderDlg::checkPath(CString)`

`CString`: 欲检测文件名或完整路径

检测文件的后缀名是否合法 (`availableExts`)



## `CAboutDlg` 类 (继承于 `CDialogEx`)

「关于」对话框.



##### `BOOL CvbdcoderDlg::OnInitDialog()`

基本的初始化, 加入了「关于」文字. 



## 资源

所有的控件类变量以 "m\_" 开头, 所有的值变量以 "v\_" 开头.



##### 单选控件

| 控件   | ID               | 变量       |
| ------ | ---------------- | ---------- |
| H264   | IDC_RADIO_FORMAT | `v_format` |
| H265   | IDC_RADIO5       |            |
| 2-pass | IDC_RADIO_CODE   | `v_code`   |
| CRF    | IDC_RADIO2       |            |
| VBR    | IDC_RADIO3       |            |



##### 复选控件

| 控件       | ID                          | 变量                |
| ---------- | --------------------------- | ------------------- |
| 复制视频流 | IDC_CHECK_COPY_VIDEO_STREAM | `v_copyVideoStream` |
| 复制音频流 | IDC_CHECK_COPY_AUDIO_STREAM | `v_copyAudioStream` |
| 剪切       | IDC_CHECK_CUT               | `v_cut`             |



##### 按钮控件

| 控件                | ID                        | 变量      |
| ------------------- | ------------------------- | --------- |
| 开始                | IDC_BUTTON_START          |           |
| 暂停                | IDC_BUTTON_PAUSE          | `m_pause` |
| 之后停止            | IDC_BUTTON_STOPAFTER      |           |
| 立即停止            | IDC_BUTTON_STOPNOW        |           |
| 存储预设            | IDC_BUTTON_SAVE_PRESET    |           |
| 管理...             | IDC_BUTTON_MANAGE_PRESETS |           |
| ...(选择输出文件夹) | IDC_BUTTON1               |           |



##### 编辑框控件

| 控件       | ID                     | 变量                     |
| ---------- | ---------------------- | ------------------------ |
| 视频码率   | IDC_EDIT_VIDEO_BITRATE | `v_videoBitrate`         |
| 音频码率   | IDC_EDIT_AUDIO_BITRATE | `v_audioBitrate`         |
| 输出文件夹 | IDC_EDIT_OUTPUT_DIR    | `v_outputDir`            |
| 命令行     | IDC_EDIT_COMMAND       | `v_command`, `m_command` |
| 命令输出   | IDC_EDIT_OUTPUT        | `v_output`, `m_output`   |



##### 其它控件

| 控件                  | ID                   | 变量       |
| --------------------- | -------------------- | ---------- |
| 主列表 ListCtrl       | IDC_LIST             | `m_list`   |
| 视频码率 Spin         | IDC_SPIN1            |            |
| 音频码率 Spin         | IDC_SPIN2            |            |
| 剪切起点 DateTimePick | IDC_TIMEPICKER_START | `v_start`  |
| 剪切终点 DateTimePick | IDC_TIMEPICKER_END   | `v_end`    |
| 预设 Combo            | IDC_COMBO_PRESET     | `m_preset` |
| 视频码率/CRF Static   | IDC_STATIC_CODE      | `m_static` |



##### 菜单

| 菜单项            | ID              |
| ----------------- | --------------- |
| 选项/添加文件     | ID_ADDFILE      |
| 选项/清空列表     | ID_CLEARLIST    |
| 选项/重置转码状态 | ID_RESET_STATUS |
| 关于/关于...      | ID_ABOUT        |



##### 加速键

| 功能         | 键        | ID                    |
| ------------ | --------- | --------------------- |
| 删除列表项目 | VK_DELETE | ID_ACCELERATOR_DELETE |
| 重置状态     | ^R        | ID_RESET_STATUS       |

