# LiteComp

LiteComp 是一个基于 C++17 的小型编译器 + 虚拟机项目。

项目包含：
- 前端：词法分析（Lexer）、语法分析（Parser）、抽象语法树（AST）
- 后端：字节码编译器（Compiler）、虚拟机（VM）
- 配套程序：交互解释器、基准测试程序、回归测试程序、功能演示程序、GoogleTest 单元测试

## 1. 编译项目

在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build -j
```

编译成功后，可执行文件会出现在 `build/` 目录下。

## 2. REPL 的启动与退出

### 启动 REPL

```bash
./build/litecomp_repl
```

看到提示符：

```text
>> 
```

### REPL 示例

```text
>> declare x = 5;
>> x + 3;
>> declare add = func(a, b) { a + b };
>> add(10, 20);
```

### 退出 REPL

当前版本未实现 `exit` 命令，建议使用：
- `Ctrl + C` 结束 REPL 进程

## 3. 其他应用程序如何运行

### 3.1 基准测试程序（Benchmark）

运行多组基准用例，输出 parse/compile/run 平均耗时。

```bash
./build/litecomp_benchmark
```

### 3.1.1 规模曲线实验（Parse/Compile/Run vs N）

已扩展规模曲线实验：对程序规模 `N`（算术链长度）按 2 倍增长进行 Parse/Compile/VM 三类基准测试。

导出 CSV（用于画曲线）：

```bash
./build/litecomp_benchmark \
	--benchmark_filter='BM_Scaling_.*' \
	--benchmark_out=build/benchmark_scaling.csv \
	--benchmark_out_format=csv
```

生成的 `build/benchmark_scaling.csv` 可直接导入 Python/Excel/Origin 绘制规模曲线。

### 3.2 回归测试程序（Regression）

运行一组固定的端到端回归用例（parser/compiler/vm）。

```bash
./build/litecomp_regression
```

### 3.3 功能演示程序（Demo）

运行完整功能演示，并逐项校验输出结果。

```bash
./build/litecomp_demo
```

功能演示覆盖能力包括：
- 算术与优先级
- 布尔逻辑与 if/else
- 全局变量绑定
- 函数与 return
- 闭包
- 字符串拼接
- 数组与索引
- 哈希表与键访问
- 内置函数（`len`、`first`、`last`、`rest`、`push`）
- 递归

## 4. 测试程序如何运行

### 4.1 运行全部测试（推荐）

```bash
ctest --test-dir build --output-on-failure
```

### 4.2 仅运行 GoogleTest 单元测试二进制

```bash
./build/litecomp_unit_tests
```

### 4.3 按名称过滤测试

```bash
ctest --test-dir build -R VmModule --output-on-failure
```

### 4.4 负向金标测试（Negative Golden Tests）

项目已新增错误路径金标测试，用于固定非法输入下的诊断行为，避免后续重构导致错误信息回退。

```bash
ctest --test-dir build -R NegativeGolden --output-on-failure
```

## 5. 主要 CMake 目标

- `litecomp_core`（静态库）
- `litecomp_repl`
- `litecomp_benchmark`
- `litecomp_regression`
- `litecomp_demo`
- `litecomp_unit_tests`

编译指定目标示例：

```bash
cmake --build build -j --target litecomp_demo
```

## 6. 目录说明

- `include/litecomp/`：头文件
- `src/`：核心实现
- `apps/repl/`：REPL 入口
- `benchmarks/`：Google Benchmark 基准测试模块（按 parser/compiler/vm/end-to-end 拆分）
- `apps/regression/`：回归测试入口
- `apps/demo/`：功能演示入口
- `tests/`：GoogleTest 模块级单元测试

## 7. Demo 与 Regression 的区别

- `demo`：用于展示语言特性是否可用，强调“覆盖面”和“可读性”。
- `regression`：用于快速回归校验，强调“稳定、快速、可自动化（CI）”。

两者通常建议同时保留：`demo` 负责演示，`regression` 负责守护。

## 8. 一键快速验证

```bash
cmake -S . -B build && \
cmake --build build -j && \
ctest --test-dir build --output-on-failure && \
./build/litecomp_demo && \
./build/litecomp_benchmark
```

若流程正常，将看到：
- 测试全部通过
- Demo 全部 PASS
- Benchmark 输出耗时表
