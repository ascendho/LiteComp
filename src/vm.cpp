#include "litecomp/vm.hpp"
#include "litecomp/builtins.hpp"
#include "litecomp/bytecode.hpp"
#include "litecomp/frame.hpp"
#include "litecomp/object.hpp"

/**
 * @brief 构造函数：初始化虚拟机
 * @param bytecode 包含编译后的指令和常量池
 */
VM::VM(std::shared_ptr<Bytecode> &&bytecode) : frames{}, stack{}, globals{} {
    // 1. 将所有的指令打包进一个“主函数”编译对象中
    auto main_fn = std::make_shared<CompiledFunction>(CompiledFunction(bytecode->instructions));
    // 2. 将主函数包装成闭包（因为 VM 统一执行闭包对象）
    auto main_closure = std::make_shared<Closure>(Closure(main_fn));
    // 3. 创建主程序的第一个栈帧（Frame），设置 IP 为 -1
    auto main_frame = new_frame(main_closure, 0);

    // 4. 将主帧放入调用栈底部
    frames[0] = main_frame;
    frames_index = 1;

    // 转移常量池所有权
    constants = std::move(bytecode->constants);

    // 初始化数据栈指针
    sp = 0;
}

/**
 * @brief 构造函数：保留全局变量状态（用于 REPL 等增量执行场景）
 */
VM::VM(std::shared_ptr<Bytecode> &&bytecode, std::array<std::shared_ptr<Object>, GLOBALSSIZE> s)
        : frames{}, stack{}, globals{} {
    auto main_fn = std::make_shared<CompiledFunction>(CompiledFunction(bytecode->instructions));
    auto main_closure = std::make_shared<Closure>(Closure(main_fn));
    auto main_frame = new_frame(main_closure, 0);

    frames[0] = main_frame;
    frames_index = 1;

    constants = std::move(bytecode->constants);

    // 从外部恢复全局变量映射表
    std::copy(std::begin(s), std::end(s), std::begin(globals));

    sp = 0;
}

// --- 栈帧与数据栈基础操作 ---

std::shared_ptr<Frame> VM::current_frame() {
    return frames[frames_index - 1];
}

std::shared_ptr<Error> VM::push_frame(std::shared_ptr<Frame> f) {
    if (frames_index >= MAXFRAMES) return new_error("frame overflow");
    frames[frames_index++] = f;
    return nullptr;
}

std::shared_ptr<Frame> VM::pop_frame() {
    return frames[--frames_index];
}

std::shared_ptr<Error> VM::push(std::shared_ptr<Object> o) {
    if (sp >= STACKSIZE) return new_error("stack overflow");
    stack[sp++] = o;
    return nullptr;
}

std::shared_ptr<Object> VM::pop() {
    return stack[--sp];
}

std::shared_ptr<Object> VM::last_popped_stack_elem() {
    return stack[sp];
}

// --- 函数调用与闭包处理 ---

/**
 * @brief 执行调用指令 (OpCall)
 * 分发逻辑：判断被调用者是用户定义的闭包还是系统内置函数
 */
std::shared_ptr<Error> VM::execute_call(int num_args) {
    // 被调用者在栈中的位置：当前栈顶向下偏移 (参数个数 + 1)
    auto callee = stack[sp - 1 - num_args];

    if (callee->type() == ObjectType::CLOSURE_OBJ) {
        return call_closure(callee, num_args);
    } else if (callee->type() == ObjectType::BUILTIN_OBJ) {
        return call_builtin(callee, num_args);
    } else {
        return new_error("Calling Neither User-Defined nor Built-In function");
    }
}

/**
 * @brief 调用闭包 (User Function)
 */
std::shared_ptr<Error> VM::call_closure(std::shared_ptr<Object> cl, int num_args) {
    auto closure = std::dynamic_pointer_cast<Closure>(cl);

    // 参数校验
    if (num_args != closure->fn->num_parameters) {
        return new_error("Wrong Number of Arguments: want=" +
                         std::to_string(closure->fn->num_parameters) + ", got=" +
                         std::to_string(num_args));
    }

    // 创建新帧。base_pointer 指向参数在栈中的起始位置。
    auto frame = new_frame(closure, sp - num_args);
    auto err = push_frame(frame);
    if (err) return err;

    // 为局部变量预留空间：栈指针 = 基址 + 该函数定义的局部变量总数
    sp = frame->base_pointer + closure->fn->num_locals;

    return nullptr;
}

/**
 * @brief 实例化闭包指令 (OpClosure)
 * 从栈中捕获“自由变量”并将代码绑定在一起。
 */
std::shared_ptr<Error> VM::push_closure(int const_index, int num_free) {
    auto constant = constants.at(const_index);
    auto function = std::dynamic_pointer_cast<CompiledFunction>(constant);

    if (!function) return new_error("Not a Function");

    // 从栈顶拷贝捕获的自由变量
    std::vector<std::shared_ptr<Object>> free;
    for (int i = 0; i < num_free; i++) {
        free.push_back(stack[sp - num_free + i]);
    }

    // 弹出被捕获的变量
    sp = sp - num_free; 

    auto closure = std::make_shared<Closure>(Closure(function));
    closure->free = std::move(free);
    return push(closure);
}

/**
 * @brief 调用 C++ 实现的内置函数
 */
std::shared_ptr<Error> VM::call_builtin(std::shared_ptr<Object> builtin, int num_args) {
    auto builtin_fn = std::dynamic_pointer_cast<Builtin>(builtin);

    // 提取参数
    auto args = std::vector<std::shared_ptr<Object>>(stack.begin() + sp - num_args, stack.begin() + sp);
    auto result = builtin_fn->builtin_function(args);

    // 清理栈：弹出参数和函数本身
    sp = sp - num_args - 1;

    if (is_error(result)) return std::dynamic_pointer_cast<Error>(result);

    return push(result);
}

// --- 运算逻辑实现 ---

/**
 * @brief 执行二元算术运算 (+ - * /)
 */
std::shared_ptr<Error> VM::execute_binary_operation(OpType op) {
    auto right = pop(); // 先弹出的在右边
    auto left = pop();  // 后弹出的在左边

    if (left->type() == ObjectType::INTEGER_OBJ && right->type() == ObjectType::INTEGER_OBJ) {
        return execute_binary_integer_operation(op, left, right);
    } else if (left->type() == ObjectType::STRING_OBJ && right->type() == ObjectType::STRING_OBJ) {
        return execute_binary_string_operation(op, left, right);
    }
    return new_error("unsupported types for binary operation");
}

/**
 * @brief 执行逻辑非 (!)
 * 规则：!true -> false, !false -> true, !null -> true, !other -> false
 */
std::shared_ptr<Error> VM::execute_bang_operator() {
    auto operand = pop();
    if (operand == get_true_ref()) return push(get_false_ref());
    if (operand == get_false_ref() || operand == get_null_ref()) return push(get_true_ref());
    return push(get_false_ref());
}

/**
 * @brief 执行数组/哈希索引访问 (obj[index])
 */
std::shared_ptr<Error> VM::execute_index_expression(std::shared_ptr<Object> left, std::shared_ptr<Object> index) {
    if (left->type() == ObjectType::ARRAY_OBJ && index->type() == ObjectType::INTEGER_OBJ) {
        return execute_array_index(left, index);
    } else if (left->type() == ObjectType::HASH_OBJ) {
        return execute_hash_index(left, index);
    }
    return new_error("index operator not supported");
}

// =============================================================================
// VM 核心主循环 (Fetch-Decode-Execute)
// =============================================================================

std::shared_ptr<Error> VM::run() {
    int ip;
    OpType op;

    // --- 虚拟机主循环：只要当前栈帧还没运行到最后一条指令就继续 ---
    while (current_frame()->ip < static_cast<int>(current_frame()->instructions().size()) - 1) {
        auto frame = current_frame();

        // 1. 指令步进：增加当前栈帧的指令指针 (Instruction Pointer)
        ip = ++(frame->ip);

        // 2. 取指：从当前帧的指令集中读取操作码
        const auto& ins = frame->instructions();
        op = static_cast<OpType>(ins.at(ip));

        // =====================================================================
        // 指令解码与执行 (Decode & Execute)
        // =====================================================================

        if (op == OpType::OpConstant) {
            // 加载常量：读取 2 字节的常量池索引
            auto const_index = read_uint_16(ins, ip + 1);
            frame->ip += 2; // 跳过操作数部分
            auto err = push(constants.at(const_index)); // 压入常量
            if (err) return err;

        } else if (op == OpType::OpPop) {
            // 弹出栈顶：通常用于清理表达式语句的结果
            pop();

        } else if (op == OpType::OpAdd || op == OpType::OpSub || op == OpType::OpMul || op == OpType::OpDiv) {
            // 二元算术运算：弹出两个操作数进行计算并压回结果
            auto err = execute_binary_operation(op);
            if (err) return err;

        } else if (op == OpType::OpTrue) {
            // 压入布尔真
            auto err = push(get_true_ref());
            if (err) return err;

        } else if (op == OpType::OpFalse) {
            // 压入布尔假
            auto err = push(get_false_ref());
            if (err) return err;

        } else if (op == OpType::OpEqual || op == OpType::OpNotEqual || op == OpType::OpGreaterThan) {
            // 比较运算：处理数字或布尔值的等值/大小判定
            auto err = execute_comparison(op);
            if (err) return err;

        } else if (op == OpType::OpBang) {
            // 逻辑取反 (!)
            auto err = execute_bang_operator();
            if (err) return err;

        } else if (op == OpType::OpMinus) {
            // 数值取负 (-)
            auto err = execute_minus_operator();
            if (err) return err;

        } else if (op == OpType::OpJumpNotTruthy) {
            // 条件跳转：如果栈顶为假则跳转到目标地址
            auto pos = read_uint_16(ins, ip + 1);
            frame->ip += 2; // 跳过 2 字节地址操作数
            auto condition = pop();
            if (!is_truthy(condition)) {
                // 如果条件不成立，修改 IP 到跳转目标（减1是因为循环开头会自增）
                frame->ip = pos - 1;
            }

        } else if (op == OpType::OpJump) {
            // 无条件跳转
            auto pos = read_uint_16(ins, ip + 1);
            frame->ip = pos - 1;

        } else if (op == OpType::OpNull) {
            // 压入空值 (null)
            auto err = push(get_null_ref());
            if (err) return err;

        } else if (op == OpType::OpSetGlobal) {
            // 设置全局变量：弹出栈顶值存入全局池
            auto global_index = read_uint_16(ins, ip + 1);
            frame->ip += 2;
            globals[global_index] = pop();

        } else if (op == OpType::OpGetGlobal) {
            // 获取全局变量：从全局池加载到栈顶
            auto global_index = read_uint_16(ins, ip + 1);
            frame->ip += 2;
            auto err = push(globals[global_index]);
            if (err) return err;

        } else if (op == OpType::OpArray) {
            // 构建数组：读取数组长度（2字节）
            auto num_elements = read_uint_16(ins, ip + 1);
            frame->ip += 2;

            // 从栈中提取 num_elements 个元素构建 Array 对象
            auto array = build_array(sp - num_elements, sp);
            sp -= num_elements; // 清理掉这些被使用的操作数

            auto err = push(array); // 将生成的数组对象压回栈顶
            if (err) return err;

        } else if (op == OpType::OpHash) {
            // 构建哈希表：读取键值对的总数量（2字节，即 2 * 元素个数）
            auto num_elements = read_uint_16(ins, ip + 1);
            frame->ip += 2;

            // 从栈中提取元素并配对
            auto [hash, err] = build_hash(sp - num_elements, sp);
            if (err) return err;
            sp -= num_elements;

            err = push(hash);
            if (err) return err;

        } else if (op == OpType::OpIndex) {
            // 索引访问：pop 出索引和被索引对象（如 array[0]）
            auto index = pop();
            auto left = pop();

            auto err = execute_index_expression(left, index);
            if (err) return err;

        } else if (op == OpType::OpCall) {
            // 函数调用：读取参数个数（1字节）
            auto num_args = read_uint_8(ins, ip + 1);
            frame->ip += 1;

            // 执行分发逻辑，进入新函数或执行内置逻辑
            auto err = execute_call(num_args);
            if (err) return err;

        } else if (op == OpType::OpReturnValue) {
            // 带值返回：pop 出返回值
            auto return_value = pop();

            // 弹出当前栈帧，回到上一个调用者的环境
            auto frame = pop_frame();
            // 重置栈指针：清理当前函数的局部变量空间，并弹出函数本身
            sp = frame->base_pointer - 1; 

            auto err = push(return_value); // 把返回值压回父级环境的栈
            if (err) return err;

        } else if (op == OpType::OpReturn) {
            // 空返回（无显式 return 或 return ;）
            auto frame = pop_frame();
            sp = frame->base_pointer - 1;

            auto err = push(get_null_ref()); // 统一压入 null
            if (err) return err;

        } else if (op == OpType::OpSetLocal) {
            // 设置局部变量：读取索引（1字节）
            auto local_index = read_uint_8(ins, ip + 1);
            frame->ip += 1;

            auto frame = current_frame();
            // 直接操作当前帧 base_pointer 之后的栈空间
            stack[frame->base_pointer + local_index] = pop();

        } else if (op == OpType::OpGetLocal) {
            // 获取局部变量
            auto local_index = read_uint_8(ins, ip + 1);
            frame->ip += 1;

            auto frame = current_frame();
            auto err = push(stack[frame->base_pointer + local_index]);
            if (err) return err;

        } else if (op == OpType::OpGetBuiltin) {
            // 获取内置函数：通过索引（1字节）从内置函数表中加载
            auto builtin_index = read_uint_8(ins, ip + 1);
            frame->ip += 1;

            auto definition = getBuiltinByIndex(builtin_index);
            auto err = push(definition);
            if (err) return err;

        } else if (op == OpType::OpClosure) {
            // 构建闭包：
            // 1. 读取常量池索引（2字节，指向 CompiledFunction）
            // 2. 读取自由变量个数（1字节）
            auto const_index = read_uint_16(ins, ip + 1);
            auto num_free = read_uint_8(ins, ip + 3);
            frame->ip += 3;

            // 实例化闭包并捕获栈上的自由变量
            auto err = push_closure(const_index, num_free);
            if (err) return err;

        } else if (op == OpType::OpGetFree) {
            // 获取自由变量：
            // 从当前闭包对象的 free 列表中提取变量并压入栈顶
            auto free_index = read_uint_8(ins, ip + 1);
            frame->ip += 1;

            auto current_closure = current_frame()->cl;
            auto err = push(current_closure->free.at(free_index));
            if (err) return err;

        } else if (op == OpType::OpCurrentClosure) {
            // 获取当前闭包对象：用于支持递归调用自身
            auto current_closure = current_frame()->cl;
            auto err = push(current_closure);
            if (err) return err;
        }
    }

    return nullptr; // 运行完成，无错误
}

/**
 * @brief 真值判断规则
 * 遵循大部分动态语言规则：null 和 false 为假，其余皆为真。
 */
bool is_truthy(std::shared_ptr<Object> obj) {
    if (obj->type() == ObjectType::BOOLEAN_OBJ) {
        return std::dynamic_pointer_cast<Boolean>(obj)->value;
    } else if (obj == get_null_ref()) {
        return false;
    }
    return true;
}