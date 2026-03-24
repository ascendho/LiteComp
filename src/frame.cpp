#include "litecomp/frame.hpp"
#include "litecomp/object.hpp"

/**
 * @brief 栈帧构造函数
 * @param cl 当前栈帧要执行的闭包 (包含字节码和捕获的自由变量)
 * @param ip 指令指针初始位置
 * @param base_pointer 栈基址，指向该函数在虚拟机全局栈中的起始位置
 */
Frame::Frame(std::shared_ptr<Closure> cl, int ip, int base_pointer) :
    cl{cl}, ip{ip}, base_pointer{base_pointer} {}

/**
 * @brief 获取当前栈帧正在执行的指令集
 * 
 * 逻辑链路：栈帧 -> 闭包 (Closure) -> 编译后的函数 (CompiledFunction) -> 指令序列 (Instructions)
 * 这样设计是为了让虚拟机在主循环中能通过 frame->instructions() 快速获取字节码。
 * 
 * @return 字节码指令集的引用
 */
const Instructions& Frame::instructions() const {
    return cl->fn->instructions;
}

/**
 * @brief 创建并返回一个新栈帧的便捷函数
 * 
 * @param cl 要执行的闭包
 * @param base_pointer 栈基址（用于定位局部变量）
 * 
 * 注意：这里将 ip 初始化为 -1。
 * 原因是虚拟机的执行循环通常是：先执行 ip++，然后再读取指令。
 * 将初始值设为 -1，第一次自增后 ip 变为 0，正好指向指令序列的第一条指令。
 * 
 * @return 包装在智能指针中的新栈帧
 */
std::shared_ptr<Frame> new_frame(std::shared_ptr<Closure> cl, int base_pointer) {
    return std::make_shared<Frame>(Frame(cl, -1, base_pointer));
}