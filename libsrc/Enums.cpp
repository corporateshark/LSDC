/**
 * \file Enums.cpp
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

#include "Enums.h"

#include <iostream>

const std::string DefValueStr = "defaultvalue(";
const std::string ToStrConv = "tostringconverter(";
const std::string FromStrConv = "fromstringconverter(";

clEnum::clEnum(clPackage* PackageIdx, string DeclaredIn ): FLastItemValue( "-1" ),
	FPackage( PackageIdx ),
	FDeclaredIn( ReplaceAll(DeclaredIn, '\\', '/' ) ),
	FDefaultItemValue( "" ),
	FToStringConverterName( "" ),
	FFromStringConverterName( "" ),
	FItems(),
	FStripName(false),
	FExported(true)
{
}

// TODO: use inlines ExtractBracketedParameter and ParseParam from HeaderProcessor.cpp
bool clEnum::CheckOption(const std::string& Part)
{
	if(Part == "noexport")
	{
		FExported = false;
	} else
	if(Part == "stripname")
	{
		FStripName = true;
	} else
/*	if(Part.find("asserterrors") != std::string::npos)
	{
	} else */
	{
		size_t dvalStart = Part.find(DefValueStr);

		if(dvalStart != std::string::npos)
		{
			std::string o = Part.substr(DefValueStr.length());

			size_t bracketPos = o.find(")");

			if(bracketPos != std::string::npos)
			{
				o = o.substr(0, bracketPos);

//				std::cout << "DefValue = " << o << std::endl;
				FDefaultItemValue = o;
			} else
			{
				// invalid option
				return false;
			}
		} else
		{
			size_t tostrStart = Part.find(ToStrConv);
			size_t fromstrStart = Part.find(FromStrConv);

			if(tostrStart != std::string::npos)
			{
				std::string o = Part.substr(ToStrConv.length());
				size_t bracketPos = o.find(")");

				if(bracketPos != std::string::npos)
				{
					o = o.substr(0, bracketPos);
//					std::cout << "ToStrConv = " << o << std::endl;

					FToStringConverterName = o;
				} else
				{
					// invalid option
					return false;
				}
			} else
			if(fromstrStart != std::string::npos)
			{
				std::string o = Part.substr(FromStrConv.length());
				size_t bracketPos = o.find(")");

				if(bracketPos != std::string::npos)
				{
					o = o.substr(0, bracketPos);
//					std::cout << "FromStrConv = " << o << std::endl;

					FFromStringConverterName = o;
				} else
				{
					// invalid option
					return false;
				}
			}
		}
	}

	return true;
}

bool clEnum::ParseNameAndParams(const std::string& S)
{
	std::vector<std::string> Parts;
	SplitLine( S, Parts, true);

//	std::cout << "Enum descriptor: " << S << std::endl;

	// first N-1 parts are the options
	for(size_t j = 0 ; j < Parts.size() - 1; j++)
	{
//		std::cout << "Parts[" << j << "] = "<< Parts[j] << std::endl;

		if(!CheckOption(Parts[j]))
		{
//			std::cout << "Invalid option: " << Parts[j] << std::endl;

			return false;
		}
	}

	FEnumName = Parts[Parts.size() - 1];

//	std::cout << "Enum name = " << FEnumName << std::endl;

	return true;
}

void clEnum::FromStringConverterHeader( buffered_stream& Out, bool InHeader ) const
{
	// Out << "/// FromString_Converter for " << FEnumName << endl;
	string RealCvtName = FFromStringConverterName.empty() ? ( FEnumName + string( "_FromString" ) ) : FFromStringConverterName;

	Out << FEnumName << " " << RealCvtName << "(const LString& Str, bool* ErrorCodePtr" << (InHeader ? string(" = NULL") : string("")) << ")";
}

void clEnum::ToStringConverterHeader( buffered_stream& Out ) const
{
	// Out << "/// ToString_Converter for " << FEnumName << endl;

	string RealCvtName = FToStringConverterName.empty() ? ( FEnumName + string( "_ToString" ) ) : FToStringConverterName;

	Out << "LString " << RealCvtName << "(" << FEnumName << " TheValue)";
}

void clEnum::GenerateConverterHeaders( buffered_stream& Out ) const
{
	ToStringConverterHeader( Out );
	Out << ";" << endl;
	FromStringConverterHeader( Out );
	Out << ";" << endl;
}

void clEnum::GenerateToStringConverter( buffered_stream& Out ) const
{
	ToStringConverterHeader( Out );

	Out << endl << "{" << endl;

	for ( size_t i = 0 ; i < FItems.size() ; i++ )
	{
		std::string ItemName = FItems[i].FItemName;

		// optional name stripping
		if(FStripName) { ItemName = FItems[i].GetStrippedName(FEnumName); }

		Out << "\tif (TheValue == " << FItems[i].FItemName << ") return \"" << ItemName << "\";" << endl;
	}

	Out << endl << "\treturn \"\";" << endl << "}" << endl;
}

void clEnum::GenerateFromStringConverter( buffered_stream& Out ) const
{
	FromStringConverterHeader( Out, false );

	Out << endl << "{" << endl;

	Out << "\tif(ErrorCodePtr)" << endl << "\t{" << endl;
	Out << "\t\t*ErrorCodePtr = false;" << endl;
	Out << "\t}" << endl << endl;

	for ( size_t i = 0 ; i < FItems.size() ; i++ )
	{
		if(!FItems[i].FExported) { continue; }

		Out << "\tif (Str == \"" << FItems[i].FItemName << "\"";

		LString StrippedName = FItems[i].GetStrippedName(FEnumName);

		if ( StrippedName != FItems[i].FItemName )
		{
			// optional name stripping
			Out << " || Str == \"" << StrippedName << "\"";
		}

		Out << ")" << endl;

		Out << "\t{" << endl;

		if(FItems[i].FAssertInConverter)
		{
			// invalid value, generate assert
			Out << "\t\t// This value was marked as invalid" << endl;
			Out << "\t\tLASSERT(false);" << endl;
		} else
		{
			Out << "\t\treturn " << FItems[i].FItemName << ";" << endl;
		}

		Out << "\t}" << endl;
	}

	Out << endl;
	Out << "\tif(ErrorCodePtr)" << endl << "\t{" << endl;
	Out << "\t\t*ErrorCodePtr = true;" << endl;
	Out << "\t}" << endl;

	// return some default value
	Out << endl << "\treturn ";

	if ( FDefaultItemValue.empty() )
	{
		Out << "static_cast<" << FEnumName << ">(-1)";
	}
	else
	{
		Out << FDefaultItemValue;
	}

	Out << ";" << endl << "}" << endl;
}

void clEnumItem::ParseParamsAndValue()
{
	LString S = FItemName;

	std::vector<std::string> Parts;
	SplitLine( S, Parts, true);

//	std::cout << "Enum item descriptor: " << S << std::endl;

	// first N-1 parts are the options
	for(size_t j = 0 ; j < Parts.size() - 1; j++)
	{
//		std::cout << "Parts[" << j << "] = "<< Parts[j] << std::endl;

		if(Parts[j] == "noexport")
		{
			FExported = false;
		} else
		if(Parts[j] == "assertinconverter")
		{
			FAssertInConverter = true;
		}
	}

	FItemName = Parts[Parts.size() - 1];

//	std::cout << "Enum item name = " << FItemName << std::endl;
}

clEnumItem::clEnumItem():
	FExported(true),
	FAssertInConverter(false)
{
}

std::string clEnumItem::GetStrippedName(const std::string& S) const
{
	if( (FItemName.length() < S.length() + 1) || (FItemName.find(S) == std::string::npos)) { return FItemName; }

	return FItemName.substr(S.length() + 1);
}

/*
 * 19/05/2010
     It's here
*/
