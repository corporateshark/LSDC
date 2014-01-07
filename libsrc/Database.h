#ifndef __Database__h__included__
#define __Database__h__included__

#include <vector>
#include <string>
#include <map>
#include "Macro.h"

using std::vector;
using std::string;
using std::map;

struct clPackage;
struct clClass;

const char CACHE_NONE  = 0;
const char CACHE_TRUE  = 1;
const char CACHE_FALSE = 2;

/**
   The source code database is a collection of loaded packages.

   Any number of packages can be read and later any required
   source code/wrappers can be generated for each loaded package.
*/
struct clDatabase
{
public:
   /// Loaded packages
   vector<clPackage*> FPackages;

   clDatabase();
   ~clDatabase();

   /// Read a single source package from the specified directory
   clPackage* ProcessPackageDirectory( const string& PackageDir );

   /// Add new package to the repository and set internal pointer in Pack to this database
   void RegisterPackage( clPackage* Pack );

   /// Generate code for every loaded package
   void GenerateStuff();

   /// Dump all the debug information about loaded packages, their dependancies etc.
   void GenerateStatistics();

public:
   void LoadPropertyMacros();

   /// Extract parameters, expand macro and substitue parameters there
   string ExpandMacro(const string& MacroString);

   /// Add class to the GlobalClassesList and set its ID
   void AddClass( clClass* Class );

   char GetCacheValue( int Class, int Base );
   void SetCacheValue( int Class, int Base, char Value );

   /// Find the class in any of the registered packages (NULL if not found)
   clClass* GetClassPtr( const string& ClassName );

   /// Find the index of the package containg specified ClassName
   int GetPackageForClass( const string& ClassName );

   /// Check if such class existsd in any of the packages
   inline bool ClassExists( const string& Class ) { return ( GetPackageForClass( Class ) != -1 ); }

   /// Check if the class 'Class' inherits from 'BaseClass'
   bool InheritsFrom( const string& Class, const string& BaseClass );

   /// Check if this is the class, which we are marshalling to .NET
   bool IsWrappedClass( const string& ClassName );

   /// Check if this is the class marked as SERIALIZABLE_CLASS()
   bool IsSerializableClass( const string& ClassName );

   /// Get the top-most ancestor for the specified class
   string GetRootestClassFor( const string& ClassName );

#pragma region Type operations
public:

   /// Process single line of to/from string converter description
   void ParseStringConverterDescription( const string& In );

   /// Converts a C++ typename into a LinderScript typename (references and pointers are ommited as long as the 'const' modifier)
   string CollapseTypeName( const string& Name );

   bool IsScalarType( const string& type );
   bool IsPODType( const string& type );
   bool IsReference( const string& TypeName );
   bool IsPointer( const string& TypeName );
   bool IsConstType( const string& TypeName );
   bool IsSmartPointer( const string& TypeName );
	bool IsEnumType( const string& TypeName );

	string ExtractSmartPointerType( const string& TypeName );

   ///  Remove C++ reference/pointer and const modifiers
   string StripTypeName( const string& Type );

   ///  Is it an output reference ?   e.g., vec3& out
   bool IsOutParameter( const string& Type );

   ///  Returns TRUE for template type declarations
   bool IsTemplateType( const string& Name );

   /// Gets the appropriate native ToString converter
   string GetAppropriateToStringConverter( const string& type );

   /// Gets the appropriate native FromString converter
   string GetAppropriateFromStringConverter( const string& type );

   /// Get the default value validator for the type
   string GetDefaultValidator( const string& type );
#pragma endregion

#pragma region .NET type handling
public:

   /// Process single line of .NET <-> Native type coverters
   void ParseNETTypeMapString( const string& In );

   /// Gets the .NET type suitable for the specified native type for the result and parameter
   string GetAppropriateNetTypeForParameter( const string& type );

   /// Add '^' modifier if the NativeType is a class or non-standart POD type
   string AddNETReferenceModifierIfNeeded( const string& NativeType );

   /// Is there any suitable conversion for this type to .NET class
   bool MapsToNET( const string& type );

   /// Get a code line for Native to Net conversion. E.g.,  vec3 Var  gives  LNativeConverter::ToNetVec3(Var)
   string GetNativeToNetConversion( const string& NativeVarName, const string& NativeType );

   /// Get a code line for .Net to Native conversion. E.g.,  vec3 Var  gives  LNativeConverter::FromNetVec3(Var)
   string GetNetToNativeConversion( const string& FullType, const string& VarName, const string& NetVarName, const string& NativeType );

#pragma endregion

#pragma region Old Load/Save code for type maps
public:

   ///  Load Native <-> .NET type map
   void MakeTypeMap( const string& ToFname );

   /// Load the list of type converters/validators
   void ReadTypeConvertersAndValidators( const string& fname );

   /// Save to/from string converters
   void DumpStringConverters( const string& fname );

   /// Save .NET <-> Managed type map
   void DumpNETTypeMap( const string& fname );

#pragma endregion

public:
   /// ToString-conversion functions for the types
   map<string, string> ToStringConverters;

   /// FromString-conversion functions for the types
   map<string, string> FromStringConverters;

   /// Default validators for each types
   map<string, string> DefaultValidators;

   /// Table of managed class names corresponding to native classes (key - native class name)
   map<string, string> NativeToNet;

   /// Static .NET method for NativeToManaged variable conversion (key - native type)
   map<string, string> ToNetConverter;

   /// Static .NET method for ManagedToNative variable conversion (key - managed type)
   map<string, string> FromNetConverter;

   /// A collection of scarry macros used to generate property bindings
   map<string, clMacroDef> PropertyMacros;

   map<string, bool> IsPOD;
   map<string, bool> IsEnum;
   map<string, bool> IsClass;
   map<string, bool> IsScalar;

   // list of all classes within all packages
   vector< clClass* > GlobalClassesList;

   // 2D array
   char* InheritanceCache;
};

#endif
