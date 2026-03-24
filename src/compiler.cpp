#include "litecomp/compiler.hpp"
#include "litecomp/ast.hpp"
#include "litecomp/bytecode.hpp"
#include "litecomp/builtins.hpp"

/**
 * @brief 核心递归编译函数
 * 根据 AST 节点的具体类型，将其翻译成对应的指令。
 */
std::shared_ptr<Error> Compiler::compile(std::shared_ptr<Node> node) {
    std::shared_ptr<Error> err;

    // --- 1. 处理程序根节点 (Program) ---
    if (auto p = std::dynamic_pointer_cast<Program>(node)) {
        for (const auto &s: p->statements) {
            // 递归编译每一条语句
            err = compile(s); 
            if (is_error(err)) return err;
        }

    // --- 2. 处理表达式语句 (ExpressionStatement) ---
    } else if (auto e = std::dynamic_pointer_cast<ExpressionStatement>(node)) {
        // 编译表达式部分
        err = compile(e->expression); 
        if (is_error(err)) return err;
        // 表达式语句执行完后，其结果留在栈顶。
        // 为了保持栈的整洁，必须发出 OpPop 将其弹出。
        emit(OpType::OpPop);

    // --- 3. 处理中缀表达式 (InfixExpression，如 1 + 2) ---
    } else if (auto ie = std::dynamic_pointer_cast<InfixExpression>(node)) {
        // 特殊优化：将 "x < y" 转换为 "y > x"
        // 这样虚拟机只需要实现 "大于" 比较指令，减少了指令集复杂度。
        if (ie->op == "<") {
            err = compile(ie->right);    // 先编译右侧
            if (is_error(err)) return err;
            err = compile(ie->left);     // 再编译左侧
            if (is_error(err)) return err;
            emit(OpType::OpGreaterThan); // 发射大于指令
            return nullptr;
        }

        // 正常的二元运算逻辑：先编译左，再编译右
        err = compile(ie->left);
        if (is_error(err)) return err;
        err = compile(ie->right);
        if (is_error(err)) return err;

        // 根据操作符发射对应的运算指令
        if (ie->op == "+")      emit(OpType::OpAdd);
        else if (ie->op == "-") emit(OpType::OpSub);
        else if (ie->op == "*") emit(OpType::OpMul);
        else if (ie->op == "/") emit(OpType::OpDiv);
        else if (ie->op == ">") emit(OpType::OpGreaterThan);
        else if (ie->op == "==") emit(OpType::OpEqual);
        else if (ie->op == "!=") emit(OpType::OpNotEqual);
        else return new_error("unknown operator " + ie->op);

    // --- 4. 处理整数字面量 (IntegerLiteral) ---
    } else if (auto il = std::dynamic_pointer_cast<IntegerLiteral>(node)) {
        auto integer = std::make_shared<Integer>(Integer{il->value});
        auto position = add_constant(integer); // 将整数放入常量池
        emit(OpType::OpConstant, position);    // 发射加载常量指令

    // --- 5. 处理布尔字面量 (BooleanLiteral) ---
    } else if (auto bl = std::dynamic_pointer_cast<BooleanLiteral>(node)) {
        // 布尔值有专用指令，无需占用常量池
        if (bl->value) emit(OpType::OpTrue);
        else emit(OpType::OpFalse);

    // --- 6. 处理前缀表达式 (PrefixExpression，如 !true, -5) ---
    } else if (auto pe = std::dynamic_pointer_cast<PrefixExpression>(node)) {
        // 编译右侧操作数
        err = compile(pe->right); 
        if (is_error(err)) return err;

        if (pe->op == "!") emit(OpType::OpBang);
        else if (pe->op == "-") emit(OpType::OpMinus);
        else return new_error("unknown operator " + pe->op);

    // --- 7. 处理 If 表达式 (IfExpression) ---
    } else if (auto i = std::dynamic_pointer_cast<IfExpression>(node)) {
        // 1. 编译条件
        err = compile(i->condition); 
        if (is_error(err)) return err;

        // 2. 发射条件跳转。此时还不知道跳到哪，先填入占位符 9999
        auto jump_not_truthy_pos = emit(OpType::OpJumpNotTruthy, 9999);

        // 3. 编译 if 分支
        err = compile(i->consequence); 
        if (is_error(err)) return err;

        // 优化：If 表达式的结果应该是其内部最后一个表达式的值。
        // 如果内部最后一条是表达式语句生成的 Pop，我们需要删掉它，把值留在栈顶。
        if (last_instruction_is(OpType::OpPop)) remove_last_pop();

        // 4. 发射无条件跳转（执行完 if 分支后跳过 else 部分）
        auto jump_pos = emit(OpType::OpJump, 9999);

        // 5. 回填 JumpNotTruthy 的目标地址（即 if 条件失败后跳到这里开始 else）
        auto after_consequence_pos = static_cast<int>(scopes.at(scope_index).instructions.size());
        change_operand(jump_not_truthy_pos, after_consequence_pos);

        if (!i->alternative) {
            // 如果没有 else，结果就是 null
            emit(OpType::OpNull); 
        } else {
            // 6. 编译 else 分支
            err = compile(i->alternative);
            if (is_error(err)) return err;
            if (last_instruction_is(OpType::OpPop)) remove_last_pop();
        }

        // 7. 回填无条件跳转的目标地址（即跳过 else 后到这里继续执行）
        auto after_alternative_pos = static_cast<int>(scopes.at(scope_index).instructions.size());
        change_operand(jump_pos, after_alternative_pos);

    // --- 8. 处理变量声明 (DeclareStatement，如 let x = 5) ---
    } else if (auto l = std::dynamic_pointer_cast<DeclareStatement>(node)) {
        auto [symbol_pre, ok] = symbol_table->resolve(l->name->value);
        if (ok) return new_error(l->name->value + " is Already Defined");

        // 在符号表中定义新变量
        auto symbol = symbol_table->define(l->name->value);

        // 编译初始值表达式
        err = compile(l->value); 
        if (is_error(err)) return err;

        // 根据符号作用域发射 Set 指令
        if (symbol.scope == SymbolScope::GlobalScope) {
            emit(OpType::OpSetGlobal, symbol.index);
        } else {
            emit(OpType::OpSetLocal, symbol.index);
        }

    // --- 9. 处理标识符/变量访问 (Identifier) ---
    } else if (auto id = std::dynamic_pointer_cast<Identifier>(node)) {
        auto [symbol, ok] = symbol_table->resolve(id->value);
        if (!ok) return new_error(id->value + " is Not Defined");

        // 根据作用域发射 Get 指令
        load_symbol(symbol); 

    // --- 10. 处理函数字面量 (FuncLiteral，如 fn(x) { x + 1 }) ---
    } else if (auto f = std::dynamic_pointer_cast<FuncLiteral>(node)) {
        // 进入新的编译作用域
        enter_scope(); 

        if (f->name != "") {
            // 支持递归调用自身
            symbol_table->define_function_name(f->name); 
        }

        for (const auto &p: f->parameters) {
            // 将参数定义为局部变量
            symbol_table->define(p->value); 
        }

        // 编译函数体
        err = compile(f->body); 
        if (is_error(err)) return err;

        // 将函数体最后生成的 Pop 转换为 ReturnValue
        if (last_instruction_is(OpType::OpPop)) {
            replace_last_pop_with_return();
        }

        // 处理空函数体，补全 Return
        if (!last_instruction_is(OpType::OpReturnValue)) {
            emit(OpType::OpReturn);
        }

        auto free_symbols = symbol_table->free_symbols;  // 获取捕获的闭包变量
        auto num_locals = symbol_table->num_definitions; // 记录局部变量总数
        auto instructions = leave_scope();               // 离开作用域，提取该函数的指令集

        // 在外部作用域加载所有闭包捕获的自由变量到栈中
        for (const auto &s: free_symbols) load_symbol(s);

        // 创建编译后的函数对象
        auto compiled_fn = std::make_shared<CompiledFunction>(CompiledFunction(instructions));
        compiled_fn->num_locals = num_locals;
        compiled_fn->num_parameters = f->parameters.size();

        auto fn_index = add_constant(compiled_fn);
        // 发射 OpClosure：它是包含代码及其捕获环境的运行时对象
        emit(OpType::OpClosure, fn_index, free_symbols.size());

    // --- 11. 处理函数调用 (CallExpression，如 add(1, 2)) ---
    } else if (auto c = std::dynamic_pointer_cast<CallExpression>(node)) {
        // 编译被调用的函数对象
        err = compile(c->function); 
        if (is_error(err)) return err;

        for (const auto &a: c->arguments) {
            // 逐个编译参数
            err = compile(a); 
            if (is_error(err)) return err;
        }

        // 发射调用指令，并注明参数个数
        emit(OpType::OpCall, c->arguments.size());
    }

    // (此处省略了数组、哈希、返回语句等其他类型的类似逻辑)
    return nullptr;
}

// =============================================================================
// 辅助与状态管理方法
// =============================================================================

/**
 * @brief 构造函数：初始化编译器环境，注册内置函数。
 */
std::shared_ptr<Compiler> newCompiler() {
    auto main_scope = std::vector<CompilationScope>{CompilationScope{}};
    auto symbol_table = new_symbol_table();

    // 默认在符号表中注册所有内置函数（如 len, print）
    for (int i = 0; i < static_cast<int>(builtinsNames.size()); i++) {
        symbol_table->define_builtin(i, builtinsNames[i]);
    }

    auto compiler = std::make_shared<Compiler>(Compiler{});
    compiler->symbol_table = symbol_table;
    compiler->scopes = main_scope;
    compiler->scope_index = 0;
    return compiler;
}

/**
 * @brief 指令发射方法族
 * 将操作码及其参数打包，并追加到当前作用域的指令序列末尾。
 */
int Compiler::emit(OpType op, int operand) {
    auto ins = make(op, operand);    // 生成二进制指令
    auto pos = add_instruction(ins); // 写入内存
    set_last_instruction(op, pos);   // 记录最后指令位置，用于后续优化（如 removePop）
    return pos;
}

/**
 * @brief 回填机制的核心：修改已有指令的操作数。
 * 常用于在编译完 if 块后，修正之前的跳转地址。
 */
void Compiler::change_operand(int op_pos, int first_operand) {
    auto op = OpType(scopes.at(scope_index).instructions.at(op_pos));
    auto new_instruction = make(op, first_operand); // 重新生成带正确地址的指令
    replace_instruction(op_pos, new_instruction);   // 原位替换
}

/**
 * @brief 作用域切换
 * 当开始编译一个函数定义时，开启一个新的指令缓冲区和符号表。
 */
void Compiler::enter_scope() {
    scopes.push_back(CompilationScope{});
    scope_index++;
    symbol_table = new_enclosed_symbol_table(symbol_table);
}

/**
 * @brief 加载符号
 * 自动根据符号在编译期被识别的作用域，选择正确的 Get 指令。
 */
void Compiler::load_symbol(Symbol s) {
    if (s.scope == SymbolScope::GlobalScope)        emit(OpType::OpGetGlobal, s.index);
    else if (s.scope == SymbolScope::LocalScope)   emit(OpType::OpGetLocal, s.index);
    else if (s.scope == SymbolScope::BuiltinScope) emit(OpType::OpGetBuiltin, s.index);
    else if (s.scope == SymbolScope::FreeScope)    emit(OpType::OpGetFree, s.index);
    else if (s.scope == SymbolScope::FunctionScope) emit(OpType::OpCurrentClosure);
}