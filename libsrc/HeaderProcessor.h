/**
 * \file HeaderProcessor.h
 * \brief
 *
 * LinderScript Database Compiler
 *
 * \version 0.9.13
 * \date 08/10/2010
 * \author Sergey Kosarevsky, 2005-2010
 * \author Viktor Latypov, 2007-2010
 * \author support@linderdaum.com http://www.linderdaum.com
 */

#ifndef __HeaderProcessor__h__included__
#define __HeaderProcessor__h__included__

#include "Package.h"

#include <fstream>

// TODO : track namespaces for every declaration
//        (now everything is ready and the parsing logic is really simple)

/**
   \brief Ad-hoc C++ header file processor

   Encapsulates header file parsing:
      - file I/O, source line / file tracking, error reporting
      - reading description of class, enum, const, static/class methods
      - comments skipping

      - FUTURE : namespace tracking
      - FUTURE : rudimentary postprocessor interaction : avoid reading "#if 0 ... #endif" etc., process "#include" directives

   The class provides a single method: ProcessFile.
*/
class clHeaderProcessor
{
public:
	clHeaderProcessor() : FLastError( "" ) {}

	/**
	\brief Process C++ header file and return true if no errors occured.

	If an error occurs, the information is stored
	in FLastError field and the last line number is the CurrentLine.
	*/
	bool ProcessFile( const string& FileName );

public:
	/// Link to current package
	clPackage*  FPackage;

	/// Link to classes database
	clDatabase* FDatabase;

	/// Last error message
	string      FLastError;

	/// Current header file name (reported by caller)
	string IncFileName;

	/// Source line counter (for error reporting)
	int CurrentLine;

private:
	/// Source file handle
	std::ifstream In;

	/// Current line read from input file
	std::string Line;

	/// Read next line from the input file (into the Line field) and increment the line counter
	bool GetNextLine( std::string& L );

	/// Read next valid line and track/skip C-style comments
	bool SkipComments();

	/// Accumulate file lines in Line until '{'
	bool ReadTillNextCurlyBracket();

	/// Accumulate method prototype parameters in Line until ')'
	bool ReadMultilineMethodProto();

	/// Try to find any of the special package parameters in the current code line
	bool ProcessSpecialParameters();

	bool TryParseLSDCCommand();
	bool TryParseConst();
	bool TryParseEnum ();
	bool TryParseStaticMethod();

	/// Try to parse one more line of class definition. Returns false if an error has occured and tru if the line is OK (but not necessary a class definition)
	bool ParseClass();

	/// Process class definition (TheClass must have the FClassName field assigned)
	bool ParseClassBody( clClass* TheClass );

	/// Read the list of base classes, class attributes and extract class name
	bool ReadClassNamesAndAttributes( int ClassNameOffset, clClass* TheClass );

	/// Parse class method prototype
	bool ParseMethodProto( clClass* TheClass );

	/// temporary storage for class modifiers
	std::vector<std::string> ClassAttribs;

	/// Comment flag
	bool InsideComment;

	/// Current class item's access modifier (private, public or protected)
	std::string CurrentModifier;

	bool SkipNextLine;
	int  SkipClass;

	/// Check if S is an access modifier and store it in CurrentModifier
	bool SaveAccessModifier( const string& S );
};

#endif
