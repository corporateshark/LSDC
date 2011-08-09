/**
 * \file Utils.h
 * \brief
 *
 * LinderScript Database Compiler
 *
 * \version 0.9.22
 * \date 09/08/2011
 * \author Sergey Kosarevsky, 2005-2011
 * \author Viktor Latypov, 2007-2011
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

const char TAB_CHAR = 0x9;

const string EngineVersion = "0.6.02";
const string LSDCVersion = "0.9.22";
const string LSDCDate = "09/08/2011";
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

   if ( CommentStart != -1 )
	{
		return Line.substr( 0, CommentStart );
	}
   else
	{ 
		return Line;
	}
}

void ExcludeFile( const string& Name );
void ExcludeFiles( const vector<string>& Names );
bool IsFileExcluded( const string& Name );
string GetCurrentDate();
string GetCurrentVersion();

#endif

/*
 * 27/07/2007
     It's here
*/

