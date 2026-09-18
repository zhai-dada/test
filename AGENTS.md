# 边缘网关项目 - 开发指南

## 项目概述

基于BM1688的边缘智能网关，支持工业协议采集、数据处理、Web UI管理、OTA升级、容器管理。

## 技术栈

- **语言**: C++11
- **构建**: CMake（支持交叉编译，通过toolchain文件配置）
- **缩进**: TAB键
- **数据库**: SQLite
- **UI**: 内嵌Web服务器（REST API + TypeScript前端）
- **协议**: 插件化（IEC104、Modbus RTU/TCP、CAN、MQTT）
- **OTA**: 复用BM1688现成OTA脚本（封装为插件）
- **容器管理**: Docker Engine REST API

## 构建方式

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-linux-toolchain.cmake  # 交叉编译
# 或
cmake ..  # 本机编译
make -j$(nproc)
```

## 项目目录结构

```
edge-gateway/
├── CMakeLists.txt                  # 顶层CMake
├── cmake/                          # 工具链 + Find模块
│   ├── arm-linux-toolchain.cmake
│   ├── aarch64-linux-toolchain.cmake
│   ├── FindLibModbus.cmake
│   ├── FindLib60870.cmake
│   ├── FindSQLite3.cmake
│   ├── FindMosquitto.cmake
│   └── CompilerFlags.cmake
│
├── src/                            # 源代码
│   ├── core/                       # 核心框架
│   │   ├── main.cpp                # 程序入口
│   │   ├── daemon.h/cpp            # 守护进程（信号、看门狗）
│   │   ├── plugin_manager.h/cpp    # 插件管理器
│   │   ├── config_manager.h/cpp    # 配置管理器
│   │   ├── data_bus.h/cpp          # 数据总线（协议->处理->存储）
│   │   └── app.h/cpp               # 应用主类
│   │
│   ├── plugin_api/                 # 插件公共接口
│   │   ├── i_plugin.h              # 插件基类
│   │   ├── i_protocol_plugin.h     # 协议插件接口
│   │   ├── data_point.h            # 数据点定义
│   │   ├── alarm_event.h           # 告警事件结构
│   │   ├── plugin_export.h         # 导出宏
│   │   └── CMakeLists.txt
│   │
│   ├── protocol/                   # 协议插件
│   │   ├── iec104/                 # IEC60870-5-104
│   │   ├── modbus/                 # Modbus RTU/TCP
│   │   ├── can/                    # CAN总线
│   │   └── mqtt/                   # MQTT客户端（北向+南向）
│   │
│   ├── data/                       # 数据处理
│   │   ├── data_processor.h/cpp    # 处理引擎
│   │   ├── filter/                 # 异常剔除、缺失补全、滤波
│   │   ├── storage/                # SQLite存储（DAO层）
│   │   ├── alarm/                  # 告警引擎（三级告警）
│   │   └── report/                 # 报表生成
│   │
│   ├── ui/                         # Web UI
│   │   ├── http_server.h/cpp       # HTTP服务器
│   │   ├── api/                    # REST API（按功能拆分）
│   │   └── websocket.h/cpp         # WebSocket推送
│   │
│   ├── ota/                        # OTA插件（封装BM1688脚本）
│   ├── container_mgr/              # Docker管理插件
│   ├── system/                     # 网络工具、时钟、LED、MCU通信
│   └── utils/                      # 日志、JSON、字符串、时间工具
│
├── third_party/                    # 第三方库（git submodule）
│   ├── lib60870/
│   ├── libmodbus/
│   ├── nlohmann_json/
│   ├── cpp-httplib/
│   └── paho_mqtt_cpp/
│
├── config/                         # JSON配置文件（详见 config_reference.md）
├── web/                            # 前端 TypeScript/HTML/CSS
│   ├── index.html
│   ├── tsconfig.json
│   ├── package.json
│   ├── src/
│   │   ├── main.ts                 # 入口
│   │   ├── api.ts                  # REST API封装
│   │   ├── websocket.ts            # WebSocket客户端
│   │   ├── pages/                  # 页面模块
│   │   └── components/             # 可复用组件
│   ├── css/
│   └── dist/                       # 编译输出（静态资源）
├── sql/                            # SQLite建表脚本
├── scripts/                        # 安装、启动、停止脚本
├── systemd/                        # 服务文件
├── tests/                          # 单元测试 + 协议测试
└── docs/                           # 文档
```

## 编码规范

- 使用TAB缩进，不使用空格，行尾不留空格
- C++11标准（`-std=c++11`）
- 命名规则：函数/变量 `snake_case`，类 `PascalCase`，宏 `UPPER_CASE`
- 头文件保护：`#pragma once`
- 优先使用前向声明，避免不必要的头文件包含
- 资源管理使用RAII
- 优先使用智能指针（`std::unique_ptr`、`std::shared_ptr`），避免裸指针
- 优先使用`enum class`代替普通枚举
- 热路径中不使用异常，用错误码或`std::error_code`
- 日志使用spdlog或简单文件日志

## 版本管理

- 语义化版本：`v主版本.次版本.修订号`，如 `v1.0.0`
- 主版本：架构大改动或不兼容变更
- 次版本：新增功能
- 修订号：bug修复

## Git规范

### 分支策略

```
main        # 稳定发布版本
develop     # 开发主干
feature-xxx # 功能分支（从develop拉出，完成后合并回develop）
fix-xxx     # 修复分支
release-xxx # 发布准备分支
```

**合并策略**：功能分支开发完成后，由用户确认是否合并到develop，不自动合并。

### 提交格式（Linux内核风格）

```
<子系统>: <简短描述>

<详细说明（可选）>

Signed-off-by: 开发者姓名 <邮箱>
```

规则：
- 子系统前缀：标识改动属于哪个模块，如 `ie104:`, `modbus:`, `data:`, `ui:`, `ota:`, `core:`, `docs:` 等
- 简短描述：使用祈使语气，首字母小写，不加句号，不超过72字符
- 详细说明：解释why而不是what，每行不超过72字符
- Signed-off-by：必须添加，表示开发者担保代码质量

示例：
```
ie104: add automatic reconnection on connection loss

When the IEC104 connection drops, the plugin now attempts to
reconnect every 5 seconds instead of requiring a manual restart.

This fixes the issue where long-running connections would silently
die after network interruptions.

Signed-off-by: Zhang San <zhangsan@example.com>
```

```
modbus: fix crc validation for rtu frames

The CRC calculation was using the wrong byte order for big-endian
platforms, causing frame rejection on ARM64 systems.

Signed-off-by: Li Si <lisi@example.com>
```

```
data: add outlier detection for analog signals

Implement range-based outlier filtering with configurable min/max
thresholds per data point. Values outside the range are replaced
with the last valid reading.

Signed-off-by: Wang Wu <wangwu@example.com>
```

### 常用子系统前缀

| 前缀 | 模块 |
|---|---|
| `core:` | 主进程、插件管理、配置管理 |
| `ie104:` | IEC104协议插件 |
| `modbus:` | Modbus协议插件 |
| `can:` | CAN总线协议插件 |
| `mqtt:` | MQTT客户端插件 |
| `data:` | 数据处理、存储、告警 |
| `ui:` | Web服务器、REST API |
| `ota:` | OTA升级插件 |
| `container:` | Docker容器管理 |
| `system:` | 网络工具、时钟、LED、MCU |
| `build:` | CMake、工具链 |
| `docs:` | 文档 |

## 插件架构

每个功能模块以动态加载插件方式实现，统一接口：

```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual const char* name() const = 0;
    virtual const char* version() const = 0;
    virtual int init(const char* config_path) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
    virtual int reload(const char* config_path) = 0;
    virtual const char* status_json() = 0;
};

// 协议插件扩展接口
class IProtocolPlugin : public IPlugin {
public:
    virtual const char* get_data_points_json() = 0;
    virtual int send_command(const char* command_json) = 0;
    virtual const char* get_connection_status() = 0;
};
```

插件从`plugins/`目录动态加载，配置文件位于`config/<插件名>.json`。

## 错误处理

统一错误码，不用异常：

```cpp
enum class ErrorCode : int {
    OK = 0,
    GENERAL_ERROR = -1,
    INVALID_PARAM = -2,
    TIMEOUT = -3,
    NOT_CONNECTED = -4,
    PARSE_ERROR = -5,
    FILE_ERROR = -6,
    DB_ERROR = -7,
    PLUGIN_NOT_FOUND = -8,
};
```

热路径返回`ErrorCode`，非热路径可使用`std::error_code`或`std::system_error`。

## 日志级别

| 级别 | 用途 |
|---|---|
| ERROR | 系统错误、功能不可用 |
| WARN | 异常但可恢复、降级运行 |
| INFO | 关键状态变更、启动/停止 |
| DEBUG | 调试信息、协议报文 |

## 配置文件

所有配置文件使用JSON格式，位于`config/`目录。详细字段说明见 `docs/config_reference.md`。

```
config/
  system.json        # 系统全局（设备ID、日志、数据库路径）
  plugins.json       # 插件注册表
  iec104.json        # IEC104连接 + 数据点映射
  modbus.json        # Modbus TCP/RTU连接 + 数据点
  can.json           # CAN接口 + 协议解析
  mqtt.json          # MQTT Broker + 发布订阅 + 断点续传
  alarm.json         # 告警规则（三级）
  data_process.json  # 数据处理（异常剔除、补全、滤波）
  ota.json           # OTA升级脚本路径
  container.json     # Docker API + 镜像仓库 + 资源限制
  network.json       # 北向链路（WiFi/4G/5G/ETH/卫星）+ 故障转移
  ui.json            # Web服务器端口、鉴权
  led.json           # LED GPIO + 显示模式
  clock.json         # NTP/GPS时钟同步
  mcu.json           # MCU串口通信 + 数据映射
```

## 关键依赖库

| 模块 | 库 | 许可证 |
|---|---|---|
| IEC104 | lib60870 | GPL-3.0（或商业授权） |
| Modbus | libmodbus | LGPL-2.1 |
| CAN | SocketCAN + can-utils | GPL-2.0 |
| MQTT客户端 | paho.mqtt.cpp | EPL-2.0 |
| MQTT Broker | Mosquitto | EPL-2.0 |
| 数据库 | sqlite3 | Public Domain |
| JSON | nlohmann/json | MIT |
| HTTP服务器 | cpp-httplib | MIT |

## Web UI功能

1. 协议数据实时展示
2. 告警管理（三级）
3. 系统日志查询
4. Docker容器管理
5. 网络工具（ping/traceroute/nslookup）
6. OTA固件升级
7. LED显示配置
8. 时钟同步配置
9. 北向链路配置
10. MCU通信配置

## 测试

- 单元测试：Google Test
- 协议测试：与模拟器联调
- UI验证：通过Web页面手动验证
