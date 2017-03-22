/**
 * \file Consts.cpp
 * \brief
 *
 * LinderScript Database Compiler
 *
 * \version 0.8.10c
 * \date 30/07/2010
 * \author Sergey Kosarevsky, 2005-2010
 * \author Viktor Latypov, 2007-2010
 * \author support@linderdaum.com http://www.linderdaum.com
 */

#include "Consts.h"

clConst::clConst(
	string Name,
	clPackage* PackageIdx,
	string DeclaredIn,
	string Type,
	string Value,
	bool IsArray )
: FName( Name )
, FPackage( PackageIdx )
, FDeclaredIn( DeclaredIn )
, FType( Type )
, FValue( Value )
, FIsArray( IsArray )
{
}

/*
 * 19/05/2010
     It's here
*/
