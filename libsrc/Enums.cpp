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

void clEnum::FromStringConverterHeader( buffered_stream& Out ) const
{
// Out << "/// FromString_Converter for " << FEnumName << endl;

   string RealCvtName = FFromStringConverterName.empty() ? ( FEnumName + string( "_FromString" ) ) : FFromStringConverterName;

   Out << FEnumName << " " << RealCvtName << "(const LString& Str)";
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
   Out << endl;

   Out << "{" << endl;

   for ( size_t i = 0 ; i < FItems.size() ; i++ )
   {
      Out << "\tif (TheValue == " << FItems[i].FItemName << ") return \"" << FItems[i].FItemName << "\";" << endl;
   }

   Out << endl << "\treturn \"\";" << endl;

   Out << "}" << endl;
}

void clEnum::GenerateFromStringConverter( buffered_stream& Out ) const
{
   FromStringConverterHeader( Out );
   Out << endl;

   Out << "{" << endl;

   for ( size_t i = 0 ; i < FItems.size() ; i++ )
   {
      Out << "\tif (Str == \"" << FItems[i].FItemName << "\") return " << FItems[i].FItemName << ";" << endl;
   }

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

   Out << ";" << endl;

   Out << "}" << endl;
}

/*
 * 19/05/2010
     It's here
*/
