#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/IType.hpp>
#include <RigCInterpreter/Functions.hpp>

namespace rigc::vm
{


struct FuncType
	: IType
{
	InnerType result;
	std::vector<InnerType> parameters;

	bool isVariadic = false;

	std::string name() const;
	std::string symbolName() const override {
		return "Func";
	}
	std::size_t size() const override {
		return sizeof(void*);
	}
	bool isArray() const override {
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


	std::string name() const;
	std::string symbolName() const override {
		return "Method";
	}
	std::size_t size() const override {
		return sizeof(void*) * 2; // self reference + method pointer
	}
	bool isArray() const override {
		return false;
	}
	auto decay() const -> InnerType override { return nullptr; }
};

Value allocateMethodOverloads(Instance& vm_, Value self_, FunctionOverloads const* overloads_);


}
