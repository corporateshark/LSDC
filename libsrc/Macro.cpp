/**
 * \file Macro.cpp
 * \brief
 *
 * LinderScript Database Compiler
 *
 * \version 0.9.10
 * \date 08/10/2010
 * \author Sergey Kosarevsky, 2005-2010
 * \author support@linderdaum.com http://www.linderdaum.com
 */

#include "Macro.h"

string ReplaceAllSubStr( string Str, string OldSubStr, string NewSubStr )
{
   string Result = Str;

   for ( size_t Pos = Result.find( OldSubStr ); Pos != string::npos; Pos = Result.find( OldSubStr ) )
   {
      Result.replace( Pos, OldSubStr.length(), NewSubStr );
   }

   return Result;
}

string clMacroDef::GenerateInstance( const vector<string>& Values ) const
{
   if ( Values.size() != FParams.size() )
   {
      return "ERROR: Invalid number of params for macro " + FName + "\n";
   }

   string Result = FText;

   // FIXME: input kludge
   for ( size_t i = 0; i != FParams.size(); i++ )
   {
      Result = ReplaceAllSubStr( Result, "##" + FParams[i] + "##", Values[i] );
      Result = ReplaceAllSubStr( Result, "#" + FParams[i], "\"" + Values[i] + "\"" );
      Result = ReplaceAllSubStr( Result, " " + FParams[i] + " ", Values[i] );
      Result = ReplaceAllSubStr( Result, " " + FParams[i], Values[i] );
      Result = ReplaceAllSubStr( Result, FParams[i] + " ", Values[i] );
      Result = ReplaceAllSubStr( Result, FParams[i], Values[i] );
   }

   // TODO : recursive macro expansion (implement Aho–Corasick search in macro list)

   // second pass : search for every available macro

   // return final result
   return Result;
}

/*
 * 08/10/2010
     It's here
*/
