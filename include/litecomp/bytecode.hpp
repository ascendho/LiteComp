#pragma once


#include <memory>
#include <vector>

#include "litecomp/code.hpp"
#include "litecomp/object.hpp"

struct Bytecode {
    Bytecode() = default;

    Bytecode(const Bytecode &other) = default;

    Bytecode(Bytecode &&other) noexcept = default;

    Bytecode &operator=(const Bytecode &other) = default;

    Bytecode &operator=(Bytecode &&other) noexcept = default;

    Instructions instructions;

    std::vector<std::shared_ptr<Object>> constants;
};

