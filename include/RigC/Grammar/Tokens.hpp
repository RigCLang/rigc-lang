#pragma once

#include RIGC_PCH

#include <RigC/Grammar/Keywords.hpp>

namespace rigc
{

struct PackageImportNames
	: p::seq< p::identifier, p::opt< p::seq< p::one<'.'>, PackageImportNames> > >
{
};

struct PackageImportFullName
	: p::must< PackageImportNames >
{
};

struct Name
	: p::identifier
{};

struct Type
	: Name
{};

struct DeclType
	:
	p::sor<
		VarKeyword,
		ConstKeyword,
		Type
	>
{};

}