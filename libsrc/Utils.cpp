/**
 * \file Utils.cpp
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

#include "Utils.h"

#include <algorithm>
#include <fstream>
#include <iostream>

using std::ifstream;

bool EnableLogging      = true;
bool ExportMethods      = true;
bool UseExportShortcuts = true;
bool Verbose            = false;
bool DoExpandMacros     = false;

vector<int> PackagesProcsCounter;

string CollapseSpaces( const string& s )
{
   string res = "";
   string ch = "-";

   for ( size_t i = 0 ; i < s.length() ; i++ )
   {
      if ( s[i] != ' ' )
      {
         ch[0] = s[i];
         res += ch;
      }
   }

   return res;
}

bool IsSeparator( const char ch )
{
   return ( ch == ' ' || ch == TAB_CHAR || ch == 0x0A || ch == 0x0D );
}

string TrimSpaces( const string& str )
{
   if ( str.length() < 1 ) { return str; }

   size_t i = 0;

   while ( i != str.length() && IsSeparator( str.at( i ) ) ) { ++i; }

   size_t j = str.length();

   while ( j != i && IsSeparator( str.at( j - 1 ) ) ) { --j; }

   return str.substr( i, j - i );
}

string Int2Str( int FromInt )
{
   static const int BUFFER = 30;

   char buf[BUFFER];
   char Format[] = "%i\0";

#ifdef _WIN32
#if _MSC_VER >= 1400
   _snprintf_s( buf, BUFFER - 1, Format, FromInt );
#else
   _snprintf( buf, BUFFER - 1, Format, FromInt );
#endif
#else
   sprintf( buf, Format, FromInt );
#endif

   string Str( buf );

   return Str;
}

string AfterChar( const string& P, const string& s )
{
   size_t pos = P.find( s );

   if ( pos != string::npos )
   {
      return P.substr( pos + 1, P.length() - pos );
   }

   return P;
}

string BeforeChar( const string& P, const string& s )
{
   size_t pos = P.find( s );

   if ( pos != string::npos )
   {
      return P.substr( 0, pos );
   }

   return P;
}

string TrimQuotes( const string& Str )
{
   string T = TrimSpaces( Str );

   if ( T.size() > 1 )
   {
      if ( T.at( 0 ) == '"' && T.at( T.length() - 1 ) == '"' )
      {
         T = T.substr( 1, T.length() - 2 );
//      if ( T.find("\"") == T.npos ) T.assign(T);
      }
   }

   return T;
}

vector<string> SplitLineSep( const string& Str, char Sep )
{
   vector<string> Result;

   string Component;

   for ( size_t i = 0; i != Str.size(); i++ )
   {
      if ( Str[i] == Sep )
      {
         Result.push_back( TrimSpaces( Component ) );
         Component.clear();
         continue;
      }

      Component += Str[i];
   }

   if ( !Component.empty() ) { Result.push_back( TrimSpaces( Component ) ); }

   return Result;
}

// get each token to the 'Components' array
void SplitLine( const LString& Str, vector<string>& Components, bool ShouldTrimSpaces )
{
   size_t TokenNum = 0;
   size_t i = 0;
   size_t j = 0;

   while ( i != Str.length() )
   {
      // bypass spaces & delimiting chars
      while ( i != Str.length() && IsSeparator( Str.at( i ) ) ) { ++i; }

      if ( i == Str.length() ) { return; }

      bool InsideQuotes = ( Str.at( i ) == '\"' );

      if ( InsideQuotes )
      {
         // inside quotes
         j = ++i;                     // exclude first " from token

         while ( j != Str.length() && Str.at( j ) != '\"' ) { j++; }
      }
      else
      {
         // outside quotes
         j = i;

         while ( j != Str.length() && !IsSeparator( Str.at( j ) ) ) { j++; }
      }

      // extract token
      if ( i != j )
      {
         TokenNum++;

         // store each token found
         {
            LString TokenFound = Str.substr( i, j - i );

            if ( ShouldTrimSpaces ) { TokenFound = TrimSpaces( TokenFound ); }

            Components.push_back( TokenFound );
            // proceed to next token
         }
         i = j;

         if ( i != Str.length() ) { ++i; }    // exclude last " from token, handle EOL
      }
   }
}

bool FileExists( const string& fname )
{
   ifstream FileL;
   FileL.open( fname.c_str() );

   if ( !FileL )
   {
      FileL.close();
      return false;
   }
   else
   {
      FileL.close();
      return true;
   }
}


string MultiSpace( const int Count )
{
   string Result;

   for ( int i = Count; i != 0; i-- ) { Result.push_back( ' ' ); }

   return Result;
}

bool AddUnique( vector<string>& OutList, const string& Item )
{
   if ( std::find( OutList.begin(), OutList.end(), Item ) == OutList.end() )
   {
      OutList.push_back( Item );
      return true;
   }

   return false;
}

string PadStringRight( const string& s, int Len, char PadSymbol )
{
   string res = s;
   string c = "X";

   while ( res.length() < static_cast<size_t>( Len ) )
   {
      c[0] = PadSymbol;
      res += c;
   }

   return res;
}

string ReplaceAll( const string& s, char Src, char Dest )
{
   string Res = s;

   for ( size_t i = 0 ; i < s.length() ; i++ )
   {
      if ( s[i] == Src ) { Res[i] = Dest; }
   }

   return Res;
}

std::vector<string> ExcludedFiles;

void ExcludeFile( const string& Name )
{
	ExcludedFiles.push_back( Name );

	if ( Verbose )
	{
		std::cout << "Added exclude file: " << Name << std::endl;
	}
}

void ExcludeFiles( const vector<string>& Names )
{
	for ( size_t i = 0 ; i != Names.size() ; i++ )
	{
		ExcludedFiles.push_back( Names[i] );		

		if ( Verbose )
		{
			std::cout << "Added exclude file: " << Names[i] << std::endl;
		}
	}
}

bool IsFileExcluded( const string& Name )
{
	for ( size_t i = 0 ; i != ExcludedFiles.size() ; i++ )
	{
		if ( Name == ExcludedFiles[i] )
		{
			if ( Verbose )
			{
				std::cout << "Excluded file: " << Name << std::endl;
			}

			return true;
		}
	}

	return false;
}

string GetCurrentDate()
{
	return LSDCDate;
}

string GetCurrentVersion()
{
	return EngineVersion;
}

/*
 * 27/07/2007
     It's here
*/
