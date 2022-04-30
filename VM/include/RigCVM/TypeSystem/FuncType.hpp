#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/Functions.hpp>

namespace rigc::vm
{


struct FuncType
	: IType
{
	InnerType result;
	std::vector<InnerType> parameters;

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

	void postInitialize(Instance& vm_) override;
};


struct MethodType
	: IType
{
	InnerType result;
	InnerType classType;
	std::vector<InnerType> parameters;


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
