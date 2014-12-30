/**
 * \file Utils.h
 * \brief
 *
 * LinderScript Database Compiler
 *
 * \version 0.9.62
 * \date 30/12/2014
 * \author Sergey Kosarevsky, 2005-2014
 * \author Viktor Latypov, 2007-2014
 * \author support@linderdaum.com http://www.linderdaum.com
 */


#ifndef _Utils_h_
#define _Utils_h_

#include <string>
#include <vector>

#pragma warning(disable:4267)
#pragma warning(disable:4100)

using namespace std;

#define LString string

// output detailed information about method registration for each class
extern bool EnableLogging;
// export methods for each class, not just VirtualConstructor
extern bool ExportMethods;
// reduce generated code size by using tricky macros
extern bool UseExportShortcuts;

// replace macros by their code, for debugging purposes
extern bool DoExpandMacros;

// silence is golden
extern bool Verbose;

const char TAB_CHAR = 0x9;

const string EngineVersion = "0.6.34";
const string LSDCVersion = "0.9.62";
const string LSDCDate = __DATE__;
const string LSDCName = "LinderScript Database Compiler " + LSDCVersion;

typedef vector<string>    clStringsList;

bool IsSeparator( const char ch );
string TrimSpaces( const string& str );
string Int2Str( int FromInt );

string MultiSpace( const int Count );

// extract everything after char s
string AfterChar( const string& P, const string& s );

// extract everything before char s
string BeforeChar( const string& P, const string& s );

void SplitLine( const LString& Str, vector<string>& Components, bool ShouldTrimSpaces );
vector<string> SplitLineSep( const string& Str, char Sep );

/// Remove "" from the string
string TrimQuotes( const string& Str );

string AddQuotesIfNone(const string& S);

bool FileExists( const string& fname );

/// Returns true if the item was added
bool AddUnique( vector<string>& OutList, const string& Item );

/// Add trailing characters to the right to make 's' have length Len
string PadStringRight( const string& s, int Len, char PadSymbol );

/// Remove all spaces from the string
string CollapseSpaces( const string& s );

inline string BoolToStr( bool B ) { return B ? string( "True" ) : string( "False" ); }

string ReplaceAll( const string& s, char Src, char Dest );

/*
    Trim // comments
*/
inline string TrimComments( const string& Line )
{
   size_t CommentStart = Line.find( "//" );

	size_t Comment1 = Line.find( "/*" );
	size_t Comment2 = Line.find( "*/" );

   if ( CommentStart != -1 && CommentStart < Comment1 )
	{
		return Line.substr( 0, CommentStart );
	}
   else
	{ 
		if ( Comment1 != -1 && Comment2 != -1 && Comment2 > Comment1 )
		{
			return Line.substr( 0, Comment1 ) + " " + Line.substr( Comment2+2, Line.length()-1 );
		}
	}

	return Line;
}

void ExcludeFile( const string& Name );
void ExcludeFiles( const vector<string>& Names );
bool IsFileExcluded( const string& Name );
string GetCurrentDate();
string GetCurrentVersion();

inline bool IsDigit( const char Ch )
{
	return ( ( Ch >= '0' ) && ( Ch <= '9' ) );
}

inline bool IsAlpha( char Ch )
{
	return ( ( Ch >= 'a' ) && ( Ch <= 'z' ) ) || ( ( Ch >= 'A' ) && ( Ch <= 'Z' ) );
}

inline bool IsAlphanumeric( char Ch )
{
	return IsDigit( Ch ) || IsAlpha( Ch );
}

inline bool IsStrAlphanumeric( const string& S )
{
	for ( size_t i = 0; i != S.length(); i++ )
	{
		if ( !IsAlphanumeric( S[i] ) ) return false;
	}

	return true;
}

#endif

/*
 * 27/07/2007
     It's here
*/
