/**
 * \file MethodArg.cpp
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

#include "MethodArg.h"

#include "Utils.h"

string clMethodArg::ToString() const
{
   string res = "";

   return res;
}

/**
   MarshalAsIn()
   MarshalAsOut()
   MarshalAsArray(<ArraySizeParamName)

   [MarshalAsVector] is deduced automatically
*/
bool clMethodArg::FromString( string& CurrentArg )
{
   // 1. split to defaults, type and name

   // cout << "Parsing[" << CurrentArg << "]" << endl;

   // 1.1. extract defaults
   size_t DefaultPos = CurrentArg.find( "=" );

   if ( DefaultPos != -1 )
   {
      Defaults = TrimSpaces( CurrentArg.substr( DefaultPos + 1, CurrentArg.length() - 1 ) );
      CurrentArg = TrimSpaces( CurrentArg.substr( 0, DefaultPos ) );
   }
   else
   {
      Defaults = "";
   }

   // 1.2. extract type and name
   size_t SpacePos = CurrentArg.find_last_of( ' ' );

   if ( SpacePos != -1 )
   {
      // it's a proto like: Method(type P, type P);
      //                    Method(class type P, class type P);
      Type = CurrentArg.substr( 0, SpacePos );
      Name = TrimSpaces( CurrentArg.substr( SpacePos + 1, CurrentArg.length() - SpacePos - 1 ) );

      size_t ClassPos = Type.find( "class " );

      if ( ClassPos != -1 )
      {
         Type = Type.substr( ClassPos + 6, Type.length() - 1 );
      }

      if ( !Name.empty() )
      {
         // check for starting "*"...
         if ( Name.at( 0 ) == '*' || Name.at( 0 ) == '&' )
         {
            Type += Name.at( 0 );
            Name = Name.substr( 1, Name.length() );
         }
      }
   }
   else
   {
      Type = CurrentArg;
      Name = "";
   }

   // 2. take care of modifiers
//   cout << "Type = " << Type << endl;

   int asOutPos = Type.find( "MarshalAsOut" );

   if ( asOutPos != -1 )
   {
      IsOut = true;

      // now remove the MarshalAsOut modifier
      Type = TrimSpaces( Type.replace( asOutPos, 12, "" ) );
   }
   else
   {
      // maybe it is true, but we'll figure it out later
      IsOut = false;
   }

   int ArrayMarshPos = Type.find( "MarshalAsArray(" );

   if ( ArrayMarshPos != -1 )
   {
//    cout << "Debug[TypeBeforeArrayParsing = " << Type << "]" << endl;

      // extract MarshalAs parameter - the name of size variable
      int ClosingBracket = Type.find_first_of( ")" );
      ArrayMarshPos += 15; /// <--- what the fuck is this?

      ArraySizeParamName = Type.substr( ArrayMarshPos, ClosingBracket - ArrayMarshPos );

//    Type = Type.substr(ClosingBracket + 1, Type.length() - ClosingBracket);
      // remove MarshalAsArray modifier
      Type = TrimSpaces( Type.replace( ArrayMarshPos - 15, ClosingBracket - ArrayMarshPos + 15 + 1, "" ) );

//    cout << "Debug[ArrayParamSize = " << ArraySizeParamName << "]" << endl;
//    cout << "Debug[TypeAfterReplace = " << Type << "]" << endl;

      IsArray = true;
   }
   else
   {
      IsArray = false;
      ArraySizeParamName = "";
   }

   // 3. check for vector types
   int VectorPos = Type.find( "vector<" );

   if ( VectorPos != -1 )
   {
      IsVector = true;
      // extract vector's item type
      int ClosingBracket = Type.find_last_of( ">" );

      VectorPos += 7;

      VectorItemType = Type.substr( VectorPos, ClosingBracket - VectorPos );
   }
   else
   {
      IsVector = false;
   }

   return true;
}

/// vectors, scalars, arrays, exported class pointers are allowed
bool clMethodArg::CanMarshalToNET() const
{
   return false;
}

// arrays, vectors, scalars, object pointers are allowed
bool clMethodArg::CanMarshalToScript() const
{
   return false;
}

/*
 * 06/05/2010
     Finally, [Type *Param] and [Type* Param] are allowed and equivalent
*/
