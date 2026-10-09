# QML + C++ 项目 全自动规则
启动自动加载，规则永久生效

## 自定义要求
- 编码前先给出思路，存在问题主动说明并给出优化方案
- 输出精简，只讲重点，省略冗余、常识性解释
- 遵循极简原则，用最少代码实现需求，不做无依据推测
- 代码修改仅改动必要部分，保持原有编码风格
- 如遇逻辑漏洞或错误，提出问题，而不是按照错误逻辑执行

## 一、项目基础信息
### 技术栈
- 编译标准：C++17
- 框架版本：Qt 6.5+、QML
- 构建工具：CMake

### 架构约束
- 分层设计：C++ 负责业务逻辑层，QML 负责 UI 展示层，两层严格分离
- 禁用规范：不使用 Qt5 相关接口；不在 QML 编写复杂 JS 逻辑、业务代码；不在 C++ 编写界面相关代码

## 二、核心架构铁律（强制遵守）
1. C++ 职责：数据模型、网络请求、数据存储、算法运算、业务逻辑、状态管理
2. QML 职责：界面布局、视觉样式、交互动效、页面展示
3. 所有业务数据统一由 C++ Model 向 QML 提供
4. QML 严禁编写复杂运算、循环、数据存储等业务逻辑
5. C++ 严禁直接操作界面、绘制元素、修改样式
6. 对外暴露的 C++ 类必须通过 `QML_ELEMENT` 注册至 QML，单例类按规范注册
7. 数据 Model 必须完整实现 `rowCount()`、`data()`、`roleNames()` 三个接口
8. 同一问题修改3次仍未解决，新增调试日志，重新定位根因

## 三、C++ 编码规范
- 缩进：4个空格，不使用 Tab
- 命名规范：类名使用大驼峰（例：UserModel）；成员变量加 `m_` 前缀（例：m_userName）
- 元对象：涉及信号槽/属性的类必须声明 `Q_OBJECT` / `Q_GADGET`
- 指针管理：禁止长期持有裸指针，优先使用 `QPointer`、`std::unique_ptr`
- 线程规范：子线程严禁操作UI相关内容
- 信号槽：严格按照场景选用正确的连接方式

## 四、QML 编码规范
- 缩进：4个空格，不使用 Tab
- 代码书写顺序：id → anchors → 普通属性 → 信号与事件
- 样式规范：禁止硬编码宽高、颜色、文本内容；颜色十六进制值统一使用大写字母
- 逻辑约束：禁止在QML内编写复杂业务逻辑
- 组件命名：自定义组件名称首字母大写

## 五、绝对禁止行为
1. 禁止在 QML 中编写业务逻辑
2. 禁止在 C++ 中编写界面样式、布局相关代码
3. 禁止混用 Qt5 与 Qt6 差异化 API
4. 禁止 Model 缺失 `roleNames()` 实现
5. 禁止子线程直接操作 UI
6. 禁止擅自删除项目已有对外接口
7. 禁止将 `console.log` 用于业务逻辑判断
8. 仅保证代码编译通过，**不运行程序做功能测试**

## 六、工作模式
当前启用 QML+C++ 专家模式：
- 全程遵循项目分层架构、编码规范与约束要求
- 自动规避违规写法与常见错误
- 规则持续生效，无需重复提醒

<!-- CODEGRAPH_START -->
## CodeGraph

In repositories indexed by CodeGraph (a `.codegraph/` directory exists at the repo root), reach for it BEFORE grep/find or reading files when you need to understand or locate code:

- **MCP tool** (when available): `codegraph_explore` answers most code questions in one call — the relevant symbols' verbatim source plus the call paths between them, including dynamic-dispatch hops grep can't follow. Name a file or symbol in the query to read its current line-numbered source. If it's listed but deferred, load it by name via tool search.
- **Shell** (always works): `codegraph explore "<symbol names or question>"` prints the same output.

If there is no `.codegraph/` directory, skip CodeGraph entirely — indexing is the user's decision.
<!-- CODEGRAPH_END -->
