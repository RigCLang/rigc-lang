#include RIGCVM_PCH

#include <RigCVM/Scope.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/Functions.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/FuncType.hpp>

#include <RigCVM/Builtin/Functions.hpp>

namespace rigc::vm
{
////////////////////////////////////
auto setupUniverseScope(Instance &vm_, Scope& scope_) -> void
{
#define MAKE_BUILTIN_TYPE(CppName, RigCName) \
	CreateCoreType<CppName>(vm_, scope_, #RigCName)

	MAKE_BUILTIN_TYPE(void,		Void);
	MAKE_BUILTIN_TYPE(bool,		Bool);
	MAKE_BUILTIN_TYPE(char,		Char);
	MAKE_BUILTIN_TYPE(char16_t,	Char16);
	MAKE_BUILTIN_TYPE(char32_t,	Char32);
	MAKE_BUILTIN_TYPE(int16_t,	Int16);
	MAKE_BUILTIN_TYPE(int32_t,	Int32);
	MAKE_BUILTIN_TYPE(int64_t,	Int64);
	MAKE_BUILTIN_TYPE(uint16_t,	Uint16);
	MAKE_BUILTIN_TYPE(uint32_t,	Uint32);
	MAKE_BUILTIN_TYPE(uint64_t,	Uint64);
	MAKE_BUILTIN_TYPE(float,	Float32);
	MAKE_BUILTIN_TYPE(double,	Float64);

	scope_.addType(std::make_unique<FuncType>());
	scope_.addType(std::make_unique<MethodType>());
	// "allocateMemory" builtin function
	{
		auto func = Function{ &builtin::allocateMemory, {}, 0 };
		func.variadic = true;

		scope_.registerFunction(vm_, "allocateMemory", std::move(func));
	}
	// "freeMemory" builtin function
	{
		auto func = Function{ &builtin::freeMemory, {}, 0 };
		func.variadic = true;

		scope_.registerFunction(vm_, "freeMemory", std::move(func));
	}
	// "print" builtin function
	{
		auto func = Function{ &builtin::print, {}, 0 };
		func.variadic = true;

		scope_.registerFunction(vm_, "print", std::move(func));
	}
	// "typeof" builtin function
	{
		auto func = Function{ &builtin::typeOf, {}, 0 };
		func.variadic = true;

		scope_.registerFunction(vm_, "typeOf", std::move(func));
	}
	// "readInt" builtin function
	{
		auto func = Function{ &builtin::readInt, {}, 0 };
		func.variadic = true;

		scope_.registerFunction(vm_, "readInt", std::move(func));
	}
	// "readFloat" builtin function
	{
		auto func = Function{ &builtin::readFloat, {}, 0 };
		func.variadic = true;

		scope_.registerFunction(vm_, "readFloat", std::move(func));
	}

#undef ADD_BUILTIN_TYPE
#undef MAKE_BUILTIN_TYPE
}

///////////////////////////////////////////////////////////////
auto Scope::formatOperatorName(std::string_view opName_, Operator::Type type_) -> StaticString<char, 512>
{
	constexpr char opPrefix[]		= "operator ";
	constexpr char prefixOpText[]	= "pr";
	constexpr char postfixOpText[]	= "po";
	constexpr char infixOpText[]	= "in";

	StaticString<char, 512> fmtName;
	fmtName += opPrefix;
	if (type_ == Operator::Infix)
		fmtName += infixOpText;
	else if (type_ == Operator::Prefix)
		fmtName += prefixOpText;
	else if (type_ == Operator::Postfix)
		fmtName += postfixOpText;

	fmtName += opName_;
	return fmtName;
}

///////////////////////////////////////////////////////////////
auto Scope::findConversion(DeclType const& from_, DeclType const& to_) const -> Function const*
{
	auto overloads = this->findFunction("operator convert");
	if (!overloads)
		return nullptr;

	auto types = FunctionParamTypes{ from_ };
	return findOverload(*overloads,  { types.data(), 1 }, false, to_);
}

// TODO: add support for second-pass functions
///////////////////////////////////////////////////////////////
auto testFunctionOverload(Function& func_, FunctionParamTypeSpan paramTypes_) -> bool
{
	auto visibleParamCount = func_.paramCount;

	// "self" param is not visible in constructors
	if (func_.isConstructor)
		visibleParamCount -= 1;

	if (visibleParamCount != paramTypes_.size())
		return false;

	// Ignore the `self` parameter for constructors
	size_t i = func_.isConstructor ? 1 : 0;
	size_t testedIdx = 0;
	for(; i < func_.paramCount; ++i, ++testedIdx)
	{
		if (func_.params[i].type != paramTypes_[testedIdx])
		{
			if (auto ref = paramTypes_[testedIdx]->as<RefType>())
			{
				if (ref->inner().get() != func_.params[i].type.get())
					return false;
			}
			else
				return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////
auto Scope::findFunction(std::string_view funcName_) const -> FunctionOverloads const*
{
	auto it = functions.find(funcName_);
	if (it != functions.end())
		return &it->second;

	return nullptr;
}

enum class DeductionResult
{
	Failed,				// couldn't deduce
	FailedWithError,	// deduction resulted in a different type or value
	Succeeded,			// deduction succeeded
};

auto tryDeduceFromSingleParamType(
		DeclType const&				paramType_,
		rigc::ParserNode const&		requiredTypeTemplate,
		TemplateParameters const&	templateParams,
		TemplateArguments&			deduced
	) -> DeductionResult
{
	// Ensure that we got either `PossiblyTemplatedSymbol` or `PossiblyTemplatedSymbolNoDisamb`
	auto& actualReqTypeTempl = *requiredTypeTemplate.children.front();

	auto reqTemplArgs = findElem<rigc::TemplateParams>(actualReqTypeTempl);
	auto& reqTypeName = *findElem<rigc::Name>(actualReqTypeTempl);

	auto const& subTemplArgs = paramType_->getTemplateArguments();

	if (!reqTemplArgs)
	{
		// If `T` is compared with `Ref< SomeType >`, fallback to `SomeType`
		auto& actualType = paramType_->is<RefType>() ? paramType_->getTemplateArguments().front().as<DeclType>() : paramType_;

		auto name = reqTypeName.string();
		if (templateParams.find(name) == templateParams.end())
			return DeductionResult::Failed;

		auto existing = deduced.find(name);
		if (existing != deduced.end())
		{
			// Deduced param wasn't a type
			if (!existing->second.is<DeclType>())
				return DeductionResult::FailedWithError;

			// fmt::print("{} is '{}' (addr: {}) and now deduced to '{}' (addr: {})\n", name,
			// 		existing->second.as<DeclType>()->name(),
			// 		(void*)existing->second.as<DeclType>().get(),
			// 		actualType->name(),
			// 		(void*)actualType.get()
			// 	);

			// Deduced something different (deduction mismatch)
			if (existing->second.as<DeclType>() != actualType)
				return DeductionResult::FailedWithError;
		}

		auto constraint = templateParams.find(name);
		if (constraint != templateParams.end())
		{
			// A NTTP specified but the deduced parameter is a type
			if (constraint->second.name != "type_name")
				return DeductionResult::FailedWithError;
		}

		deduced[name] = actualType;
		return DeductionResult::Succeeded;
	}

	if (reqTemplArgs->children.size() != subTemplArgs.size())
		return DeductionResult::Failed;

	if (paramType_->symbolName() != reqTypeName.string_view())
		return DeductionResult::Failed;

	for (size_t i = 0; i < subTemplArgs.size(); ++i)
	{
		auto& reqTemplArg = *reqTemplArgs->children[i];

		auto& subTemplArg = subTemplArgs[i];

		if (subTemplArg.is<DeclType>())
		{
			auto res = tryDeduceFromSingleParamType(subTemplArg.as<DeclType>(), reqTemplArg, templateParams, deduced);
			// Skip if deduction failed
			// if (res != DeductionResult::Succeeded)
			// 	return res;
		}
		else // NTTP:
		{
			auto const& value = subTemplArg.as<int>();

			auto name		= reqTemplArg.string();
			auto existing	= deduced.find(name);
			if (existing != deduced.end())
			{
				// Deduced param wasn't a NTTP
				if (!existing->second.is<int>())
					return DeductionResult::FailedWithError;

				// Deduced something different (deduction mismatch)
				if (existing->second.as<int>() != value)
					return DeductionResult::FailedWithError;
			}

			auto constraint = templateParams.find(name);
			if (constraint != templateParams.end())
			{
				// A type_name required but the deduced parameter is a NTTP
				if (constraint->second.name == "type_name")
					return DeductionResult::FailedWithError;
			}

			deduced[name] = value;
		}
	}

	// fmt::print("> matching {} against {}\n", paramType_->name(), requiredTypeTemplate.string_view());

	return DeductionResult::Succeeded;
}

auto tryDeduceTemplateParams(
		FunctionParamTypeSpan		paramTypes_,
		Function::ParamSpan			functionParams,
		TemplateParameters const&	templateParams
	)
{
	// templateParams is a map (string -> constrain or "type_name")

	auto result = TemplateArguments();
	for (size_t i = 0; i < paramTypes_.size(); ++i)
	{
		if (!functionParams[i].typeNode) // not a template param
			continue;

		auto res = tryDeduceFromSingleParamType(paramTypes_[i], *functionParams[i].typeNode, templateParams, result);
		if (res == DeductionResult::FailedWithError)
			return TemplateArguments(); // Empty

	}

	return result;
}

///////////////////////////////////////////////////////////////
auto Scope::tryGenerateFunction(
		Instance &vm_,
		std::string_view			funcName_,
		FunctionParamTypeSpan		paramTypes_
	) -> Function const*
{
	auto templIt = functionTemplates.find(funcName_);
	if (templIt == functionTemplates.end())
	{
		if (parent)
			return parent->tryGenerateFunction(vm_, funcName_, paramTypes_);
		return nullptr;
	}

	for (auto& templ : templIt->second)
	{
		if (paramTypes_.size() != templ->paramCount)
			continue;

		auto& fnScope = vm_.scopeOf(templ);
		auto deduced = tryDeduceTemplateParams(
				paramTypes_,
				viewArray(templ->params, 0, templ->paramCount),
				fnScope.templateParams
			);
		// if (deduced.size() > 0)
		// {
		// 	fmt::print("Deduced {} out of {} required template params\n", deduced.size(), fnScope.templateParams.size());
		// 	for (auto const& [name, value] : deduced)
		// 	{
		// 		if (value.is<int>())
		// 			fmt::print(" - {} = {}\n", name, value.as<int>());
		// 		else
		// 			fmt::print(" - {} = {}\n", name, value.as<DeclType>()->name());
		// 	}
		// }

		if (deduced.size() < fnScope.templateParams.size())
			continue;

		// if (testFunctionOverload(*templ, paramTypes_))
		{
			auto params = Function::Params();
			for (size_t i = 0; i < paramTypes_.size(); ++i)
			{
				if (!templ->params[i].type)
				{
					auto type = paramTypes_[i];
					auto isRef = type->is<RefType>();
					if (isRef) {
						auto& typeName = *findElem<rigc::Name>(*templ->params[i].typeNode);

						if (typeName.string_view() != "Ref") {
							type = type->as<RefType>()->inner();
						}
					}

					params[i] = {
						templ->params[i].name,
						type // TODO: evaluate constraint
					};
				}
				else // Param doesn't relay on any template params
				{
					params[i] = {
						templ->params[i].name,
						templ->params[i].type
					};
				}
			}

			auto paramsString = std::string();
			for (size_t i = 0; i < paramTypes_.size(); ++i)
			{
				if (i > 0)
					paramsString += ", ";
				paramsString += params[i].type->name();
			}

			// fmt::print("Instantiating function template {} with params: {}\n", funcName_, paramsString);

			auto& func = this->registerFunction(vm_, funcName_,
					Function(
						Function::RuntimeFn(templ->runtimeImpl().node),
						std::move(params),
						paramTypes_.size()
					)
				);


			auto& funcScope = vm_.scopeOf(&func);
			funcScope.func = &func;
			funcScope.templateArguments = std::move(deduced);

			if (!templ->returnType)
			{
				auto& returnTypeNode = *findElem<rigc::ExplicitReturnType>(*templ->runtimeImpl().node);

				func.returnType = vm_.evaluateType(returnTypeNode, &funcScope);
				// fmt::print("Evaluated type {} to {}\n", returnTypeNode.string_view(), func.returnType->name());
			}
			else
				func.returnType = templ->returnType;

			return &func;
		}
	}

	if (parent)
		return parent->tryGenerateFunction(vm_, funcName_, paramTypes_);
	return nullptr;
}

///////////////////////////////////////////////////////////////
auto findOverload(
		FunctionCandidates const&	funcs_,
		FunctionParamTypeSpan		paramTypes_,
		bool						method_,
		Function::ReturnType		returnType_
	) -> Function const*
{
	for (auto& [scope, overloads] : funcs_)
	{
		auto result = findOverload(*overloads, paramTypes_, method_, returnType_);
		if (result)
			return result;
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////
auto findOverload(
		FunctionOverloads const&	overloads_,
		FunctionParamTypeSpan		paramTypes_,
		bool						method_,
		Function::ReturnType		returnType_
	) -> Function const*
{
	if (overloads_.size() == 1 && overloads_[0]->variadic && overloads_[0]->isRaw())
	{
		return overloads_[0];
	}

	for (size_t i = 0; i < overloads_.size(); ++i)
	{
		if (method_ && overloads_[i]->params[0].name != "self")
			continue;

		if (testFunctionOverload(*overloads_[i], paramTypes_))
		{
			if (!returnType_ || overloads_[i]->returnType == returnType_)
				return overloads_[i];
		}
	}

	return nullptr;
}


///////////////////////////////////////////////////////////////
auto Scope::findType(std::string_view typeName_) const -> IType const*
{
	{
		if (auto type = types.find(typeName_).get())
			return type;
	}

	{
		auto it = typeAliases.find(typeName_);
		if (it != typeAliases.end())
			return it->second;
	}

	// Find within function instantiation
	{
		auto it = templateArguments.find(typeName_);
		if (it != templateArguments.end())
		{
			auto& arg = it->second;

			if (arg.is<DeclType>())
				return arg.as<DeclType>().get();
		}
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////
auto Scope::traceForType(std::string_view typeName_) const
	-> Opt< ScopeTraceResult<IType const*> >
{
	auto type = findType(typeName_);
	if (type)
		return ScopeTraceResult{ this, type };

	if (parent)
		return parent->traceForType(typeName_);

	return std::nullopt;
}

///////////////////////////////////////////////////////////////
auto Scope::findOperator(std::string_view opName_, Operator::Type type_) const -> FunctionOverloads const*
{
	auto fmtName = formatOperatorName(opName_, type_);
	return this->findFunction(
			std::string_view( fmtName.data(), fmtName.numChars )
		);
}


///////////////////////////////////////////////////////////////
auto Scope::registerType(Instance& vm_, std::string_view name_, IType& type_) -> IType&
{
	typeAliases[std::string(name_)] = &type_;

	return type_;
}


///////////////////////////////////////////////////////////////
auto Scope::addType(MutDeclType type_) -> void
{
	types.add(type_);
	type_->postInitialize(*vm);
}

///////////////////////////////////////////////////////////////
auto Scope::registerFunction(Instance& vm_, std::string_view name_, Function func_) -> Function&
{
	functionStorage.emplace_back( std::make_unique<Function>(std::move(func_)) );
	Function& f = *functionStorage.back();

	// TODO: ensure unique overload signature
	FunctionOverloads& overloads = functions[ std::string(name_) ];
	overloads.emplace_back( &f );

	return f;
}

///////////////////////////////////////////////////////////////
auto Scope::registerFunctionTemplate(Instance& vm_, std::string_view name_, Function func_) -> Function&
{
	functionStorage.emplace_back( std::make_unique<Function>(std::move(func_)) );
	Function& f = *functionStorage.back();

	// TODO: ensure unique overload signature
	FunctionOverloads& overloads = functionTemplates[ std::string(name_) ];
	overloads.emplace_back( &f );

	return f;
}

///////////////////////////////////////////////////////////////
auto Scope::registerOperator(Instance& vm_, std::string_view name_, Operator::Type type_, Function func_) -> Function&
{
	auto fmtName = formatOperatorName(name_, type_);
	return this->registerFunction(
			vm_,
			std::string_view( fmtName.data(), fmtName.numChars ),
			std::move(func_)
		);
}
}
