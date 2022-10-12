#pragma once

#include RIGCVM_PCH

#include <RigCVM/Functions.hpp>
#include <RigCVM/Value.hpp>

namespace rigc::vm
{

struct Instance;

namespace builtin
{

/// @brief Print given amount characters to standard output.
/// @param chars (Addr<Char>)- pointer to the first character.
/// @param size (Int32) - number of characters to print.
///
/// @example
/// var text = "Hello, world!";
/// printCharacters(text.data(), text.size());
auto printCharacters(Instance &vm_, Function::ArgSpan args_) -> OptValue;

/// @brief Prints given arguments previously formatting it with fmtlib.
///
/// @note For now this function accepts any combination of arguments and can crash
/// if the arguments are not compatible.
///
/// @param chars (Addr<Char>)- pointer to the first character.
/// @param size (Int32) - number of characters to print.
///
/// @example
///
/// var text = "World";
/// printCharacters("Hello, {}!\n", text);
auto print(Instance &vm_, Function::ArgSpan args_) -> OptValue;

/// @brief Returns the type of the given value, as a StaticArray<Char, ?>.
///
/// @note This function will be completely reworked in the future because of
/// varying return type (could not be properly compiled).
///
/// @example
///
/// var text = "Hello, world!";
/// print("{}\n", typeOf(text)); // prints "StaticArray<Char, 13>"
auto typeOf(Instance &vm_, Function::ArgSpan args_) -> OptValue;

/// @brief Reads an int from standard input.
/// @returns Int32 - the read value.
/// @example
/// var value = readInt();
auto readInt(Instance &vm_, Function::ArgSpan args_) -> OptValue;

/// @brief Reads a float from standard input.
/// @returns Float32 - the read value.
/// @example
/// var value = readFloat();
auto readFloat(Instance &vm_, Function::ArgSpan args_) -> OptValue;

/// @brief Allocates given amount of bytes on the heap.
/// @param size (Int32) - number of bytes to allocate.
/// @returns Addr<Char> - pointer to the allocated memory.
/// @example
/// var ptr = allocateMemory(1024);
/// freeMemory(ptr);
auto allocateMemory(Instance &vm_, Function::ArgSpan args_) -> OptValue;

/// @brief Frees memory previously allocated with allocateMemory().
/// @param ptr (Addr<Char>) - pointer to the memory to free.
/// @example
/// var ptr = allocateMemory(1024);
/// freeMemory(ptr);
auto freeMemory(Instance &vm_, Function::ArgSpan args_) -> OptValue;

}

}
