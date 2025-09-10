# Simple HTTP Server

一个基于C++实现的轻量级HTTP服务器项目，使用CMake构建系统。

## 项目结构

```
.
├── HttpServer.h      # HTTP服务器类头文件
├── HttpServer.cpp    # HTTP服务器实现
├── main.cpp         # 示例程序
├── CMakeLists.txt   # CMake配置文件
└── README.md        # 项目说明文档
```

## 功能特性

- **多线程支持**：每个客户端连接在独立线程中处理
- **路由系统**：支持注册不同路径的处理函数
- **HTTP/1.1协议**：基本的HTTP请求解析和响应生成
- **优雅关闭**：支持Ctrl+C信号处理，安全关闭服务器
- **简单易用**：提供简洁的API接口

## 编译要求

- C++17或更高版本
- CMake 3.10或更高版本
- Linux/Unix系统（使用POSIX socket）
- GCC/Clang编译器

## 编译步骤

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译
make

# 或者使用一行命令
mkdir -p build && cd build && cmake .. && make
```

## 运行服务器

```bash
# 使用默认端口（8080）
./httpserver

# 指定端口
./httpserver 3000
```

服务器启动后，在浏览器中访问：
- http://localhost:8080

## 示例路由

项目包含以下示例页面：

| 路径 | 描述 |
|------|------|
| `/` | 主页，显示所有可用路由 |
| `/about` | 关于页面，介绍服务器特性 |
| `/hello` | Hello World页面 |
| `/time` | 显示服务器当前时间 |

## API使用示例

```cpp
#include "HttpServer.h"

int main() {
    // 创建服务器实例
    HttpServer server(8080);
    
    // 注册路由处理函数
    server.route("/api/data", [](const std::string& request) {
        return "{\"status\": \"ok\", \"data\": \"Hello API\"}";
    });
    
    // 启动服务器
    server.start();
    
    // 保持运行
    while (server.isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
```

## 主要类说明

### HttpServer类

主要公共方法：
- `HttpServer(int port)` - 构造函数，指定监听端口
- `void route(const std::string& path, RequestHandler handler)` - 注册路由
- `void start()` - 启动服务器
- `void stop()` - 停止服务器
- `bool isRunning()` - 检查服务器运行状态

### HttpRequest结构体

包含HTTP请求的解析结果：
- `method` - 请求方法（GET, POST等）
- `path` - 请求路径
- `version` - HTTP版本
- `headers` - 请求头映射表
- `body` - 请求体内容

### HttpResponse结构体

用于构建HTTP响应：
- `status_code` - 状态码（默认200）
- `status_text` - 状态文本
- `headers` - 响应头映射表
- `body` - 响应体内容

## 限制和注意事项

1. 仅支持基本的HTTP/1.1协议
2. 不支持HTTPS
3. 适用于学习和小型项目，不建议用于生产环境
4. 当前仅在Linux/Unix系统上测试
5. 请求大小限制为4KB

## 扩展建议

- 添加POST请求体解析
- 实现静态文件服务
- 添加中间件支持
- 实现WebSocket支持
- 添加日志系统
- 支持配置文件
- 添加单元测试

## 许可证

MIT License

## 贡献

欢迎提交Issue和Pull Request！