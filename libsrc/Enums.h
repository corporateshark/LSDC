/**
 * \file Enums.h
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

#ifndef _Enums_h_
#define _Enums_h_

#include "Utils.h"
#include "BufStream.h"
#include "MethodArg.h"

#include <stdlib.h>

using namespace std;

/// Single item of enumeration
struct clEnumItem
{
public:
	clEnumItem();

	/// Name of this item
	LString FItemName;

	/// Numerical value for this item
	LString FItemValue;

	/// If false, we do not export this item in converters
	bool FExported;

	/// If true, we generate the error in ToString converter
	bool FAssertInConverter;

	/// Parse enum item description ( <options> <name> ) contained in FItemName
	void ParseParamsAndValue();

	/// Remove EnumName from the name of the item
	std::string GetStrippedName(const std::string& S) const;
};

struct clPackage;

/// Description of the 'enum' type
struct clEnum
{
public:
	clEnum( clPackage* PackageIdx, string DeclaredIn );

	bool ParseNameAndParams(const std::string& S);

	/// LString <TypeName>_ToString( const TypeName& Value )
	void GenerateToStringConverter( buffered_stream& Out ) const;

	/// TypeName <TypeName>_FromString( const LString& Str )
	void GenerateFromStringConverter( buffered_stream& Out ) const;

	void FromStringConverterHeader( buffered_stream& Out, bool InHeader = true ) const;
	void ToStringConverterHeader( buffered_stream& Out ) const;

	void GenerateConverterHeaders( buffered_stream& Out ) const;

	void AddItem( const clEnumItem& Item )
	{
		clEnumItem It = Item;

		if ( It.FItemValue.empty() )
		{
			// generate next value
			It.FItemValue = Int2Str( atoi( FLastItemValue.c_str() ) + 1 );
		}

		FItems.push_back( It );

		FLastItemValue = It.FItemValue;
	}

	bool CheckOption(const std::string& Part);

public:
	/// Is the name of the enum stripped from the items
	bool   FStripName;

	/// Is it exported to .NET/ or to StringConversions etc.
	bool   FExported;

	/// Custom name for ToString_converter
	string FToStringConverterName;

	/// Custom name for FromString_converter
	string FFromStringConverterName;

	/// Default value to be used with this
	string FDefaultItemValue;

	/// Temporary value counter
	string FLastItemValue;

	/// Native enumeration name
	string FEnumName;

	/// What is the owner package
	clPackage*    FPackage;

	/// Source file name
	string FDeclaredIn;

	/// List of defined values for this enumeration
	vector<clEnumItem> FItems;
};

#endif

/*
 * 19/05/2010
     It's here
*/
