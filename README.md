# Qt Node Editor

Qt Node Editor 是一个基于 Qt 的节点流程编辑器，支持创建、连接和管理节点，适用于构建可视化流程工具、游戏引擎蓝图编辑、数据处理管线等场景。

## 功能特性

- **节点系统**: 支持创建、拖拽、对齐吸附、删除节点
- **输入/输出端口**: 每个节点包含一个输入 Socket 和一个输出 Socket
- **节点连接**: 可视化折线连接节点端口，支持环检测防止死循环
- **异步执行**: 使用 QThreadPool 异步执行节点任务，提供状态反馈（未开始 / 运行中 / 成功 / 失败）
- **拓扑排序**: 自动对节点图进行拓扑排序，按依赖顺序执行
- **多标签页**: 支持多个流程的同时编辑和管理
- **流程持久化**: 支持 JSON 格式的保存和加载流程
- **缩略图导航**: 右下角小地图实时同步场景全局视图
- **交互式 UI**: 滚轮缩放、右键平移、右键菜单等

## 架构设计

项目采用前后端分离的三层架构：

```
┌─────────────────────────────────────────────────────┐
│  UI 层 — 纯渲染和用户交互                              │
│  Node (QGraphicsItem)  NodeSocket  NodeConnection    │
│  NodeView (QGraphicsView)  NodeScene (QGraphicsScene) │
│  NodeSceneMap (缩略图)  QtNodes (QMainWindow)         │
├─────────────────────────────────────────────────────┤
│  逻辑层 — 纯 C++，可脱离 Qt GUI 独立测试              │
│  NodeData (数据模型 struct)                           │
│  NodeGraph (DAG 图管理: 拓扑排序、环检测、连接规则)     │
│  FlowEngine (异步执行引擎)                            │
│  Task (可执行任务单元 QRunnable)                       │
├─────────────────────────────────────────────────────┤
│  序列化层                                             │
│  FlowSerializer (JSON 读写)                          │
└─────────────────────────────────────────────────────┘
```

**设计要点**：
- `NodeData` 是纯数据模型，不继承任何 Qt GUI 类
- `NodeGraph` 负责图逻辑（邻接表管理、拓扑排序、环检测），通过信号通知 UI 层
- `FlowEngine` 负责异步执行调度，上游失败时下游仍可继续执行
- `FlowSerializer` 独立管理 JSON 序列化，不耦合到窗口类中

## 快速开始

### 环境要求

- **Qt 5.15+** (使用 MSVC 编译器)
- **C++14** 编译器 (MSVC 2022)
- **Visual Studio 2022**

### 编译步骤

1. 克隆仓库：
    ```bash
    git clone https://github.com/bingle.bai/QtNodeEditor.git
    cd QtNodeEditor
    ```
2. 使用 Visual Studio 2022 打开 `QtNodes.sln`
3. 确保已安装 Qt VS Tools 扩展，并配置 Qt Kit 路径
4. 选择 `Debug x64` 配置，编译运行

### 运行程序

编译成功后运行，将显示一个包含单个标签页的节点编辑器窗口。

## 使用说明

### 基础操作

| 操作 | 方法 |
|------|------|
| 创建节点 | 右键空白处 → "创建默认节点" 或 "创建自定义节点" |
| 连接节点 | 从输出端口拖拽到另一个节点的输入端口 |
| 移动节点 | 左键拖拽节点（拖拽时有对齐辅助线） |
| 选择/多选 | 左键框选或 Ctrl+点击 |
| 删除 | 选中后按 Delete 键 |
| 缩放 | 鼠标滚轮 |
| 平移画布 | 右键拖拽 |
| 重命名标签页 | 双击标签页标题 |

### 菜单功能

| 菜单 | 功能 |
|------|------|
| 文件 → 退出 | 关闭程序 |
| 编辑 → 创建流程 | 新建标签页 |
| 编辑 → 删除流程 | 删除当前标签页 |
| 编辑 → 保存流程 | 将所有流程保存为 JSON |
| 编辑 → 加载流程 | 从 JSON 加载流程 |
| 编辑 → 执行流程 | 运行当前流程的所有节点 |
| 帮助 → 关于 | 查看关于信息 |

### 节点执行流程

1. 点击"执行流程"后，所有节点重置为白色（未开始状态）
2. 系统对节点图进行拓扑排序
3. 从入度为 0 的根节点开始异步执行
4. 执行成功的节点变为绿色，失败的变为红色
5. 一个节点的所有上游完成后，下游自动继续执行

## 项目结构

```
Qt-Node-Editor/
├── include/                 # 头文件
│   ├── Node.h               # 节点 UI 类 (QGraphicsItem: 渲染、交互)
│   ├── NodeSocket.h         # 端口类 (输入/输出圆形端口)
│   ├── NodeConnection.h     # 连线类 (Bezier 曲线 + 箭头)
│   ├── NodeScene.h          # 场景管理 (QGraphicsScene: 事件、桥接逻辑层)
│   ├── NodeView.h           # 视图类 (QGraphicsView: 缩放、平移、网格)
│   ├── NodeSceneMap.h       # 缩略图导航小地图
│   ├── QtNodes.h            # 主窗口 (QMainWindow)
│   ├── NodeData.h           # 节点纯数据模型 (脱离 GUI)
│   ├── NodeGraph.h          # DAG 图逻辑 (拓扑排序、环检测、连接规则)
│   ├── FlowEngine.h         # 异步执行引擎
│   ├── FlowSerializer.h     # JSON 序列化
│   └── Task.h               # 异步任务单元
├── src/                     # 源文件 (与 include/ 对应)
│   ├── main.cpp             # 入口
│   ├── Node.cpp            
│   ├── NodeSocket.cpp       
│   ├── NodeConnection.cpp   
│   ├── NodeScene.cpp        
│   ├── NodeView.cpp         
│   ├── NodeSceneMap.cpp     
│   ├── QtNodes.cpp          
│   ├── NodeGraph.cpp        
│   ├── FlowEngine.cpp       
│   ├── FlowSerializer.cpp   
│   └── Task.cpp             
├── ui/                      # Qt Designer UI 文件和资源
│   ├── QtNodes.ui
│   ├── NodeSceneMap.ui
│   └── QtNodes.qrc
├── solution/                # 应用参数
│   └── app_params.json      # 流程保存文件
├── image/                   # 截图
│   └── demo.png
├── QtNodes.sln              # VS 解决方案文件
├── QtNodes.vcxproj          # VS 项目文件
├── LICENSE                  # MIT 许可证
└── README.md
```

## 自定义开发

### 自定义节点任务逻辑

修改 [Task.cpp](src/Task.cpp) 中的 `run()` 方法：

```cpp
void Task::run() {
    // 实现具体的节点业务逻辑
    // m_id >= 0 表示成功，调用 m_onComplete(true)
    // 否则调用 m_onComplete(false) 表示失败
    if (m_id >= 0) {
        QThread::msleep(200);
        m_onComplete(true);
    } else {
        QThread::msleep(500);
        m_onComplete(false);
    }
}
```

### 自定义节点外观

修改 [Node.cpp](src/Node.cpp) 中的 `paint()` 方法可更改节点渲染样式、颜色、字体等。

### 扩展节点端口

当前每个节点固定 1 输入 1 输出。如需多端口支持，修改 `Node` 类中的 `m_inputSockets` 和 `m_outputSockets` 为 `QVector<NodeSocket*>`。

### 添加节点类型

你可以通过继承方式添加不同类型的节点：
1. 继承 `Task` 类实现自定义任务逻辑
2. 在 `FlowEngine` 中根据节点类型实例化不同的 `Task`

## 贡献

欢迎提交 Issue、Pull Request 或任何形式的贡献！

1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/your-feature`)
3. 提交修改 (`git commit -m 'Add some feature'`)
4. 推送到分支 (`git push origin feature/your-feature`)
5. 提交 Pull Request

## 许可证

本项目基于 [MIT License](LICENSE) 开源。

---

Qt Node Editor 是一个开源项目，旨在为开发者提供一个易用的节点编辑器框架。如果你有任何问题或建议，请随时提交 Issue 或联系作者。
