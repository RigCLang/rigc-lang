#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/Functions.hpp>

namespace rigc::vm
{
struct FuncType : IType
{
private:
	InnerType result;
	std::vector<InnerType> parameters;

public:
	bool isVariadic = false;

	auto name() const -> std::string override;
	auto symbolName() const -> std::string override {
		return "Func";
	}

	auto size() const -> std::size_t override {
		return sizeof(void*);
	}

	auto isArray() const -> bool override {
		return false;
	}

	auto decay() const -> InnerType override { return nullptr; }

	auto postInitialize(Instance& vm_) -> void override;
};

struct MethodType : IType
{
private:
	InnerType result;
	InnerType classType;
	std::vector<InnerType> parameters;

public:
	auto name() const -> std::string override;
	auto symbolName() const -> std::string override {
		return "Method";
	}

	auto size() const -> std::size_t override {
		return sizeof(void*) * 2; // self reference + method pointer
	}

	auto isArray() const -> bool override {
		return false;
	}

	auto decay() const -> InnerType override { return nullptr; }
};

Value allocateMethodOverloads(Instance& vm_, Value self_, FunctionOverloads const* overloads_);
}
