/**
 * \file HeaderProcessor.cpp
 * \brief
 *
 * LinderScript Database Compiler
 *
 * \version 0.9.20
 * \date 21/01/2011
 * \author Sergey Kosarevsky, 2005-2011
 * \author Viktor Latypov, 2007-2011
 * \author support@linderdaum.com http://www.linderdaum.com
 */

#include "HeaderProcessor.h"

#include "Database.h"
#include "Package.h"

#include <iostream>
#include <fstream>
using std::cout;
using std::endl;
using std::ifstream;

const string NATIVE_FINAL_CLASS_STR     = "NATIVE_FINAL(";

const string EXCLUDE_FILE_STR           = "EXCLUDE_FILE(";
const string PACKAGE_CUSTOM_INCLUDE_STR = "PACKAGE_CUSTOM_INCLUDE(";
const string PACKAGE_NET_NAME_STR       = "PACKAGE_NET_NAME(";
const string PACKAGE_NAME_STR           = "PACKAGE_NAME(";
const string PACKAGE_CPP_PREFIX_STR     = "PACKAGE_CPP_PREFIX(";
const string PACKAGE_CPP_NAME_STR       = "PACKAGE_CPP_NAME(";
const string PACKAGE_OUTPUT_DIR_STR     = "PACKAGE_OUTPUT_DIRECTORY(";
const string PACKAGE_DEPENDS_ON_STR     = "DEPENDS_ON(";

const string PACKAGE_GEN_SERIALIZATION  = "GENERATE_SERIALIZATION(";

const string PACKAGE_GEN_TUNNELLERS     = "GENERATE_TUNNELLERS(";
const string PACKAGE_GEN_EXPORTS        = "GENERATE_EXPORTS(";
const string PACKAGE_GEN_SCRIPT_EXPORTS = "GENERATE_SCRIPT_EXPORTS(";
const string PACKAGE_GEN_ENUMS          = "GENERATE_ENUM_EXPORTS(";
const string PACKAGE_GEN_CONSTS         = "GENERATE_CONST_EXPORTS(";
const string PACKAGE_GEN_NET_EXPORT     = "GENERATE_NET_EXPORT(";

// optional defines (non-preprocessor based) are specified in the command line or in the LSDC options
// The following markers try to check if the code between them should be processed
const string LSDC_OPTIONAL_BEGIN = "LSDC_OPTIONAL_BEGIN(";
const string LSDC_OPTIONAL_END   = "LSDC_OPTIONAL_END(";

#pragma region Inline parsing helper functions

/// Что-то про кривые скобки
inline bool FindCharOutOfBraces( const string& Line, char ch )
{
   bool InsideBraces = false;

   for ( size_t i = 0; i != Line.size(); ++i )
   {
      if ( Line[i] == '\"' ) { InsideBraces = !InsideBraces; }

      if ( Line[i] == ch && !InsideBraces ) { return true; }
   }

   return false;
}

/// Is it an internal LSDC declaration of class property ?
inline bool IsPropertyDescriptionLine( const string& S )
{
	string S1 = TrimSpaces( S );

   int PropertyPos = S1.find( "PROPERTY(" );

   return ( PropertyPos == 0 );
}

/// Remove stuff from Constructor
inline void StripConstructorInitializers( string& S )
{
   // sometimes, in a constuctor definition there might be ": [initializer list]" construction
   int FirstColon = S.find_first_of( ":" );

   if ( FirstColon != -1 ) { S = S.substr( 0, FirstColon ); }
}

/// Recheck if it's not a predeclaration or a template or a macro or a function proto
inline bool IsValidClass( const string& Line )
{
   return ( Line.find( ";" )  == -1 ) && ( Line.find( "\\" ) == -1 ) &&
          ( Line.find( "<" )  == -1 ) && ( Line.find( ">" )  == -1 ) &&
          ( Line.find( "(" )  == -1 ) && ( Line.find( ")" )  == -1 );
}

/// Extract "Parameter" (without quotes) from MACRO_NAME("Parameter")
inline string ExtractBracketedParameter( const string& L )
{
	int Start = L.find_first_of( "(" );
	int End   = L.find_first_of( ")" );

	string Param = L.substr( Start + 1, End - Start - 1 );

	return TrimQuotes( Param );
}

inline bool ParseParam( const string& PName, string& Value, const string& Line )
{
	if ( Line.find( PName ) != -1 )
	{
		Value = ExtractBracketedParameter( Line );
		return true;
	}

	return false;
}

inline bool ParseParamItem( const string& PName, vector<string>& Values, const string& Line )
{
   if ( Line.find( PName ) != -1 )
   {
      string P = ExtractBracketedParameter( Line );

      if ( !P.empty() ) { Values.push_back( P ); }

      return true;
   }

   return false;
}

#pragma endregion

bool ToBool( const string& S )
{
   return ( S == "True" || S == "true" || S == "TRUE" );
}

bool clHeaderProcessor::ProcessSpecialParameters()
{
#define P_PROCESS(Name,FieldName) if (ParseParam(PACKAGE_##Name, FPackage-> FieldName, Line)) return true;
#define P_PROCESSB(Name,FieldName) { string SS; if (ParseParam(PACKAGE_##Name, SS, Line)) { FPackage-> FieldName = ToBool(SS); return true; } }

   P_PROCESSB( GEN_SERIALIZATION, FGenerateSerialization )
   P_PROCESSB( GEN_TUNNELLERS, FGenerateTunnellers )
   P_PROCESSB( GEN_SCRIPT_EXPORTS, FGenerateScriptExports )
   P_PROCESSB( GEN_EXPORTS, FGenerateExports )
   P_PROCESSB( GEN_NET_EXPORT, FGenerateNETExport )
   P_PROCESSB( GEN_ENUMS, FGenerateEnums )
   P_PROCESSB( GEN_CONSTS, FGenerateConsts )

   P_PROCESS( CPP_PREFIX_STR, FPackageCPPPrefix )
   P_PROCESS( CPP_NAME_STR, FPackageCPPName )
   P_PROCESS( NAME_STR, FPackageName )
   P_PROCESS( CUSTOM_INCLUDE_STR, FPackageCustomIncludeName )
   P_PROCESS( NET_NAME_STR, FPackageNetName )

#undef P_PROCESS

   /*
      if (ParseParam(PACKAGE_CPP_PREFIX_STR, FPackage->FPackageCPPPrefix, Line)) return true;
      if (ParseParam(PACKAGE_CPP_NAME_STR, FPackage->FPackageCPPName, Line)) return true;
      if (ParseParam(PACKAGE_NAME_STR, FPackage->FPackageName, Line)) return true;
      if (ParseParam(PACKAGE_CUSTOM_INCLUDE_STR, FPackage->FPackageCustomIncludeName, Line)) return true;
      if (ParseParam(PACKAGE_NET_NAME_STR, FPackage->FPackageNetName, Line)) return true;
   */
   if ( ParseParam( PACKAGE_OUTPUT_DIR_STR, FPackage->FPackageOutDirectory, Line ) ) { return true; }

   if ( ParseParamItem( PACKAGE_DEPENDS_ON_STR, FPackage->FDependsOn, Line ) ) { return true; }

   if ( ParseParamItem( NATIVE_FINAL_CLASS_STR, FPackage->FNeverOverrideMethodsOf, Line ) ) { return true; }

	vector<string> Excludes;

   if ( ParseParamItem( EXCLUDE_FILE_STR, Excludes, Line ) )
	{
		ExcludeFiles( Excludes );
		return true;
	}

   /// Try to read the type map
   if ( Line.find( "BEGIN_DOTNET_TYPE_MAP()" ) != -1 )
   {
      string s;

      while ( GetNextLine( s ) )
      {
         if ( s.find( "END_DOTNET_TYPE_MAP()" ) != -1 ) { break; }

         FDatabase->ParseNETTypeMapString( s );
      }

      return true;
   }

   /// Try to read to/from string converters and validators
   if ( Line.find( "BEGIN_STRING_CONVERTERS" ) != -1 )
   {
      string s;

      while ( GetNextLine( s ) )
      {
         if ( s.find( "END_STRING_CONVERTERS()" ) != -1 ) { break; }

         FDatabase->ParseStringConverterDescription( s );
      }

      return true;
   }

   return false;
}

bool clHeaderProcessor::GetNextLine( string& L )
{
	getline( In, L );

	bool res = !In.eof();

	if ( res ) { CurrentLine++; }

	return res;
}

bool clHeaderProcessor::TryParseEnum()
{
	size_t EnumPos = Line.find( "enum " );

	if ( EnumPos != 0 ) { return false; }

	// parse only named enums
	Line = TrimSpaces( TrimComments( Line.substr( 5, Line.length() - 5 ) ) );

	// bypass unnamed enums
	if ( Line.empty() ) { return false; }

	if ( Line[0] == '{' ) { return false; }

	while ( ( Line.find( '}' ) == -1 ) && ( Line.find( ';' ) == -1 ) )
	{
		// its a multiline declaration
		string NextLine;

		if ( !GetNextLine( NextLine ) )
		{
			cout << endl;
			cout << "   ERROR: expected } but EOF found while parsing 'enum': " << Line << endl;

			exit( 255 );
		}

		Line += TrimSpaces( TrimComments( NextLine ) );
	}

	size_t BraceOpenPos  = Line.find( '{' );
	size_t BraceClosePos = Line.find( '}' );

	// bypass forward declarations
	if ( ( BraceOpenPos == -1 ) && ( BraceClosePos == -1 ) ) { return false; }

	string EnumParamsAndName = TrimSpaces( TrimComments( Line.substr( 0, BraceOpenPos ) ) );

	Line = TrimSpaces( Line.substr( EnumParamsAndName.length(), Line.length() - EnumParamsAndName.length() ) );

	// it's something strange
	if ( Line[0] != '{' ) { return false; }

	BraceOpenPos  = Line.find( '{' );
	BraceClosePos = Line.find( '}' );

	// it's something strange
	if ( BraceClosePos == std::string::npos ) { return false; }

	Line = TrimSpaces( TrimComments( Line.substr( BraceOpenPos + 1, BraceClosePos - BraceOpenPos - 1 ) ) );

	clEnum Enum( FPackage, IncFileName );

	if(!Enum.ParseNameAndParams(EnumParamsAndName))
	{
		cout << endl << "   ERROR: could not parse 'enum' parameters: " << EnumParamsAndName << endl;
		exit( 255 );
		return false;
	}

	// parse enum items
	while ( !Line.empty() )
	{
		clEnumItem Item;

		Item.FItemName  = Line;
		Item.FItemValue = "";

		size_t CommaPos  = Line.find( ',' );

		if ( CommaPos != -1 )
		{
			Item.FItemName = TrimSpaces( TrimComments( Line.substr( 0, CommaPos ) ) );
			Line = TrimSpaces( TrimComments( Line.substr( CommaPos + 1, Line.length() - CommaPos ) ) );
		}
		else
		{
			Line = "";
		}

		size_t AssignPos = Item.FItemName.find( '=' );

		if ( AssignPos != -1 )
		{
			Item.FItemValue = TrimSpaces( TrimComments( Item.FItemName.substr( AssignPos + 1, Item.FItemName.length() - AssignPos ) ) );
			Item.FItemName  = TrimSpaces( TrimComments( Item.FItemName.substr( 0, AssignPos ) ) );
		}

		if ( Item.FItemName.find( "#pragma" ) == 0 ) continue;

		Item.ParseParamsAndValue();

		Enum.AddItem( Item );
	}

	FPackage->FEnums.push_back( Enum );
	// parse exported static symbol name and type.
	// todo : If we are in Enum, then use 'int' as a symbol type

	return true;
}

bool clHeaderProcessor::TryParseConst()
{
	size_t ConstPos = Line.find( "const " );

	if ( ConstPos != 0 ) { return false; }

	Line = TrimSpaces( Line.substr( 6, Line.length() - 6 ) );

	size_t SemiPos = Line.find( ";" );

	// parse only single-line consts
	if ( SemiPos == -1 ) { return false; }

	size_t AssignPos = Line.find( "=" );

	// probably it's line  "const type name(constructorargs)" - bypass it
	if ( AssignPos == -1 ) { return false; }

	string ConstName  = "";
	string ConstType  = "";
	string ConstValue = TrimSpaces( Line.substr( AssignPos + 1, SemiPos - AssignPos - 1 ) );

	bool IsArray = false;

	Line = TrimSpaces( Line.substr( 0, AssignPos ) );

	size_t BracePos = Line.find( "[" );

	if ( BracePos != -1 )
	{
		// it's an array "const type value[] = ..."
		IsArray = true;

		Line = TrimSpaces( Line.substr( 0, BracePos ) );
	}

	// extract method name
	size_t SpacePos = Line.find_last_of( ' ' );

	// tabs support
	if ( SpacePos == -1 ) { SpacePos = Line.find_last_of( '\t' ); }

	ConstName = TrimSpaces( Line.substr( SpacePos + 1, Line.length() - SpacePos ) );
	ConstType = TrimSpaces( Line.substr( 0, SpacePos ) );

	FPackage->FConsts.push_back( clConst( ConstName, FPackage, IncFileName, ConstType, ConstValue, IsArray ) );

	return true;
}

bool clHeaderProcessor::TryParseStaticMethod()
{
   size_t ScriptExportPos = Line.find( "scriptexport " );
   bool IsScriptExport = ( ScriptExportPos == 0 ) && ( Line.find( "(" ) > 0 );

   // extract "scriptexport" static func
   if ( !IsScriptExport ) { return false; }

   Line = TrimSpaces( Line.substr( 12, Line.length() - 12 ) );

   size_t LeftBrace = Line.find( "(" );
   size_t RightBrace = Line.find( ")" );

   string DotNETClassName = TrimSpaces( Line.substr( LeftBrace + 1, RightBrace - 1 ) );

   Line = TrimSpaces( Line.substr( RightBrace, Line.length() - RightBrace ) );

   if ( !ReadMultilineMethodProto() )
   {
      cout << "Error parsing static method prototypes :" << endl;
      cout << FLastError << endl;
      exit( 255 );
   }

   if ( ( Line.find( "<" ) != -1 ) || ( Line.find( ">" ) != -1 ) )
	{
      if ( Verbose ) cout << "Skipping template: " << Line << endl;

		return false;
	}

   clMethod NewMethod;
   NewMethod.FromString( Line );

   if ( NewMethod.FMethodName.empty() )
   {
      cout << endl;
      cout << "   ERROR: method prototype parsing failed" << endl;
      cout << "    LINE: '" << Line << "'" << endl;

      exit( 255 );
   }

   NewMethod.FClassName   = DotNETClassName;
   NewMethod.FAccess      = "public:";

   // additional fields
   NewMethod.FPackage      = FPackage;
   NewMethod.FDeclaredIn   = ReplaceAll( IncFileName, '\\', '/' );

   FPackage->FScriptExportMethods.push_back( NewMethod );

   return true;
}

bool clHeaderProcessor::ReadTillNextCurlyBracket()
{
   // remove trailing '{'
   size_t BracePos = Line.find( "{" );

   int StoredPos = In.tellg();
   int StoredLine = CurrentLine;

   while ( BracePos == -1 )
   {
      // look ahead
      string L;

      StoredPos = In.tellg();
      StoredLine = CurrentLine;

      if ( !GetNextLine( L ) )
      {
         FLastError = string( "ERROR: Expected '{' but EOF found" );
         return false;
      }

      // or Line.find ?
      if ( L.find( "/*" ) != -1 ) { InsideComment = true; }

      if ( L.find( "*/" ) != -1 ) { InsideComment = false; }

      if ( InsideComment ) { continue; }

      Line += L;

      BracePos = Line.find( "{" );
   }

   Line = TrimSpaces( Line.substr( 0, BracePos ) );

   In.seekg( StoredPos );
   CurrentLine = StoredLine;

   return true;
}

bool clHeaderProcessor::SkipComments()
{
	Line = TrimSpaces( TrimComments( Line ) );

	if ( Line.size() == 0 ) { return true; }

	if ( Line.find( "/*" ) != -1 ) { InsideComment = true; }

	if ( Line.find( "*/" ) != -1 ) { InsideComment = false; }

	if ( InsideComment ) { return true; }

	return false;
}

// ClassNameOffset is the length of 'struct ' or 'class '
bool clHeaderProcessor::ReadClassNamesAndAttributes( int ClassNameOffset, clClass* TheClass )
{
   ClassAttribs.resize( 0 );

   string BaseClasses;

	Line = TrimComments( Line );

   size_t Pos = Line.find( ":" );
   size_t EndPos = Line.length();

   // contains base class(es)
   if ( Pos != -1 )
   {
      BaseClasses = TrimSpaces( Line.substr( Pos + 1, Line.length() - 1 ) );
      EndPos = Pos;
   }

   string ClassName = Line.substr( ClassNameOffset, EndPos - ClassNameOffset );

   ClassName = TrimSpaces( ClassName );

   if ( ClassName.find( ' ' ) != -1 )
   {
      SplitLine( ClassName, ClassAttribs, true );

      if ( ClassAttribs.size() > 1 )
      {
         ClassName = ClassAttribs[ClassAttribs.size()-1];
      }
   }

   TheClass->FClassName = ClassName;

   TheClass->FDatabase   = FDatabase;
   TheClass->FPackage    = FPackage;

   TheClass->FDeclaredIn = ReplaceAll( IncFileName, '\\', '/' );
   TheClass->AssignClassAttributes( ClassAttribs );

   // empty class name is handled there
   return TheClass->ExtractBaseClassList( BaseClasses, FLastError );
}

bool clHeaderProcessor::ParseClass()
{
   // bypass (multiline) templates
   if ( ( Line.find( "template" ) != -1 ) && ( Line.find( "<" ) != -1 ) )
   {
      SkipNextLine = true;
      return true;
   }

   // look for the end of multiline template
   if ( ( Line.find( ">" ) != -1 ) )
   {
      if ( SkipNextLine ) { SkipClass = 1; }

      SkipNextLine = false;
   }

   // check for 'class'/'struct'
   bool IsClass    = ( Line.find( "class " ) == 0 );
   bool IsStruct   = ( Line.find( "struct " ) == 0 );
   bool IsNoExport = ( Line.find( "noexport " ) == 0 );

	if ( IsNoExport ) { return true;	}

   if ( IsStruct )
   {
      if ( Line.find( "struct {" ) != std::string::npos ) { IsStruct = false; }
   }

   if ( SkipNextLine || ( !IsClass && !IsStruct ) ) { return true; }

   if ( SkipClass > 0 )
   {
      SkipClass--;
      return true; // skip this
   }

   if ( !IsValidClass( Line ) )
   {
	   return true;
   } // skip

   if ( !ReadTillNextCurlyBracket() ) { return false; }

   clClass Class;

   // read class name, attributes and base class list
   if ( !ReadClassNamesAndAttributes( ( IsClass ? 6 : 7 ), &Class ) ) { return false; }

   // Do not parse autogenerated tunneller classes
   if ( Class.FClassName.find( "_Tunnel" ) != std::string::npos ) { return true; }

   // generate stubs and export information
   // if everything is ok, then the 'Class' is added to this package
   CurrentModifier = ( IsStruct ) ? "public:" : "private:";
   return ParseClassBody( &Class );
}

bool clHeaderProcessor::ProcessFile( const string& FileName )
{
   FLastError = "";

   CurrentLine = 0;
   In.open( FileName.c_str(), ios_base::binary );

   InsideComment = false;
   SkipNextLine  = false;
   SkipClass     = 0;

   while ( GetNextLine( Line ) )
   {
      if ( SkipComments() ) { continue; }

      if ( ProcessSpecialParameters() ) { continue; }

      if ( TryParseConst() ) { continue; }

      if ( TryParseEnum() ) { continue; }

      if ( TryParseStaticMethod() ) { continue; }

      if ( !ParseClass() )
	  {
		  return false;
	  }
   }

   return true;
}

bool clHeaderProcessor::SaveAccessModifier( const string& S )
{
	if ( ( S == "public:" ) || ( S == "protected:" ) || ( S == "private:" ) )
	{
		CurrentModifier = S;
		return true;
	}

	return false;
}

bool clHeaderProcessor::ReadMultilineMethodProto()
{
   while ( Line[Line.length()-1] == ',' )
   {
      string NextLine;

      if ( !GetNextLine( NextLine ) )
      {
         FLastError = string( "ERROR: expected method proto but EOF found" );
         return false;
      }

      Line += TrimSpaces( TrimComments( NextLine ) );
   }

   return true;
}

bool clHeaderProcessor::ParseMethodProto( clClass* TheClass )
{
   bool NotDestructor    = !TheClass->IsDestructorDefinition( Line );
   bool ClassConstructor = TheClass->IsConstructorDefinition( Line );

   bool NoExport   = ( Line.find( "noexport " ) != -1 );
   bool MethodSign = ( Line.find( "virtual " ) != -1 ) || ( Line.find( "scriptmethod " ) != -1 );
   bool StaticSign = ( Line.find( "static " ) != -1 );

   int OpenBracketPos = Line.find( "(" );

   if ( StaticSign && ( OpenBracketPos != -1 ) && !NoExport ) { MethodSign = true; }

   // it's a virtual method or a ctor and not a destructor
   if ( ( MethodSign || ClassConstructor ) && NotDestructor && !NoExport )
   {
      if ( !ReadMultilineMethodProto() ) { return false; }

      if ( ClassConstructor ) { StripConstructorInitializers( Line ); }

      // no template methods
      if ( StaticSign )
      {
         if ( ( Line.find( "template " ) != -1 ) || ( Line.find( "<" ) != -1 ) )
         {
				if ( Verbose ) cout << "Skipping template: " << Line << endl;
            return true; // skip and continue;
         }
      }

      if ( ( OpenBracketPos == -1 ) || ( Line.find( ")" ) == -1 ) ) { return true; } // skip and continue;

      clMethod NewMethod;
      NewMethod.FromString( Line );
      NewMethod.FStatic = StaticSign;

      if ( ClassConstructor )
      {
         TheClass->AddConstructor( CurrentModifier, NewMethod );
         return true;
      }

      if ( NewMethod.FMethodName.empty() )
      {
         FLastError  = string( "ERROR: method parsing failed\n" );
         FLastError += string( " LINE: '" ) + Line + string( "'\n" );

         return false;
      }

      TheClass->AddMethod( CurrentModifier, NewMethod );
   }

   return true;
}

/**
   This method pases only 'first-level' methods.
   Inner classes are not supported.
   Nesting tracking is specifically for inner class detection.
*/
bool clHeaderProcessor::ParseClassBody( clClass* TheClass )
{
	int NestingLevel = 1;

	bool ClassHasBody = false;
	bool FirstLine = true;

	while ( GetNextLine( Line ) )
	{
		if ( SkipComments() ) { continue; }

		if ( FindCharOutOfBraces( Line, '{' ) && !FirstLine ) { NestingLevel++; }

		if ( FindCharOutOfBraces( Line, '}' ) ) { NestingLevel--; }

		FirstLine = false;

		if ( ClassHasBody && ( NestingLevel == 0 ) ) { break; }

		// check if we are inside the declaration
		if ( NestingLevel != 1 ) { continue; }

		ClassHasBody = true;

		if ( SaveAccessModifier( Line ) ) { continue; }

		if ( Line.find( "nativefield " ) != -1 ) // it's a field
		{
			// if (!ParseClassField(TheClass))
			clField Field;
			string Error = Field.FromString( Line );

			TheClass->AddFieldA( CurrentModifier, Field );
			continue;
		}

		if ( IsPropertyDescriptionLine( Line ) ) // it is a property
		{
			clProperty prop;
			string ErrorCode = prop.FromString( Line );

			if ( ErrorCode != "" )
			{
				FLastError  = string( "Class name: " ) + TheClass->FClassName + string( "\n" );
				FLastError += string( "Error parsing property descriptor, line: \n" ) + Line;

				return false;
			}

			TheClass->AddProperty( prop );
			continue;
		}

		if ( !ParseMethodProto( TheClass ) ) { return false; }
	}

	FPackage->FClasses[TheClass->FClassName] = *TheClass;

	FDatabase->AddClass( &( FPackage->FClasses[TheClass->FClassName] ) );

	return true;
}
