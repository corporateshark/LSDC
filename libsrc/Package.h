#ifndef __package__h__included__
#define __package__h__included__

#include "Database.h"

#include "Class.h"
#include "Enums.h"
#include "Method.h"
#include "Consts.h"

struct clDatabase;

// some directory names for code generation
const string StatisticsDirName    = "Stats";
const string SerializationDirName = "Serialization";
const string ScriptDirName        = "Script";
const string ExportDirName        = "Export";
const string NETDirName           = "NET";

/**
   Package is a collection of exported items:
      - classes
      - static methods
      - enums and consts
      - static symbols (variables)

   Package description also contains dependency information and some
   code generation parameters (.NET package name, .cpp-prefixes etc.).

   The LSDC tool processes specified directories, extracts
   metainformation about each item and then generates
   required source code:
      - metainfo (RTTI) registration for classes etc.
      - script binding (.rs)
      - .NET C++/CLI wrappers for each class
      - standart (binary and property-based) serialization code
      - code statistics

   Command-line invocation of LSDC simply specifies the directory
   with the source code. Specific setup (export names, type mappings)
   are embedded in the C++ .h-files

   Every serializable class is marked with 'serializable' attribute,
   properties are specified inside the class definition,
   .NET-exported classes are marked with 'netexportable' attribute etc.

   See user manual for further information.

   // Code generation issues:

   The amount of generated code for a particular class library can be quite large,
   so the generated code is written strictly to the OutputDirectory specified
   in FPackageOutDirectory field.

   Generated code is also subdivided into logical parts:
     - external language bindings (subdirectories "NET", "Java", etc.)
     - script export ("Script" subdirectory with "_Tunneller" files)
     - native metaclass registration ("Export" subdirectory)
     - serialization code ("Serialization" subdirectory)
     - statistics ("Stats") - not really the code, but just some information

    // Major TODO:
   Split code generation and actual containers to support plugin-based architecture.
   The package class must not containt methods like GenerateNETWrappers etc.
*/

struct clPackage
{
public:
   /// Link to global items database
   clDatabase* FDatabase;

   clPackage() : FPackagesProcsCounter( 0 ) {}
   virtual ~clPackage() {};

   /// write debug package information (serializable/netexportable lists, directories, file names)
   void DumpPackageStats( const string& fname );

   /// Generate the directory name for script export (tunneller classes)
   string GetScriptExportDir() const;

   /// Total number of methods registered in this package (for code statistics)
   int FPackagesProcsCounter;

   /// The name of the package
   string FPackageName;

   /// The name of .NET namespace for the package
   string FPackageNetName;

   /// The prefix of C++ export files
   string FPackageCPPPrefix;

   /// Additional include file for the package
   string FPackageCustomIncludeName;

   /// C++-compatible name of the package
   string FPackageCPPName;

   /// The directory(ies) of the package
   vector<string> FPackageInDirectories;

   /// Output for the package registration source code
   string FPackageOutDirectory;

   /// Custom include files for this package. Use CUSTOM_INCLUDE keyword in the source code
   vector<string> FPackageCustomIncludes;

   /// List of external dependencies
   vector<string> FDependsOn;

   /// List of native final classes
   vector<string>    FNeverOverrideMethodsOf;

#pragma region Item collections

public:

   /// Local class database (list of package's classes)
   map<string, clClass>    FClasses;

   /// Exported Enums in this package
   vector<clEnum> FEnums;

   /// Exported global Consts in this package
   vector<clConst> FConsts;

   /// Static methods
   vector<clMethod> FScriptExportMethods;

   /// Exported global variables ?

#pragma endregion

#pragma region Item management

public:

   /// Direct access to class information
   clClass* GetClassPtr( const string& ClassName ) { if ( !ClassExists( ClassName ) ) { return NULL; } return &FClasses[ClassName]; };

   /// Check if class exists in this package
   bool     ClassExists( const string& ClassName ) { return FClasses.count( ClassName ) > 0; }

   /// Check if this class should be exported to .NET
   bool IsWrappedClass( const string& ClassName );

   /// Check if this class is marked SERIALIZABLE_CLASS()
   bool IsSerializableClass( const string& ClassName );

   /// Check inheritance relation between two classes
   bool InheritsFrom( const string& Class, const string& BaseClass ) const;

   ///  Get the highest class in the hierarchy which is the ancestor of ClassName ('rootest' is a strange word)
   string   GetRootestClassFor( const string& ClassName );

#pragma endregion

#pragma region Code generation

public:

   /// Flags for code/export generation

   /// Serialization code
   bool FGenerateSerialization;

   bool FGenerateExports;
   bool FGenerateScriptExports;
   bool FGenerateNETExport;
   bool FGenerateTunnellers;
   bool FGenerateEnums;
   bool FGenerateConsts;

   /// Generate everything specified by boolean flags
   void GenerateStuff();

   /// Generate some debug information in separate files
   void GenerateStatistics();

private:

   /// Create every required package directory for generated code output
   void CreatePackageDirectories();

   /// .NET C++/CLI binding code
   void GenerateDotNETWrappers();

   /// Complete class list for debugging information
   void GenerateClassesList( buffered_stream& Out ) const;

   /// Script tunnellers and metaclass registration code
   void GenerateStubs( const string& BaseClass, const string& BaseClassExports ) const;

   /// Script language definitions
   void GenerateNativeFramework() const;

   /// .NET method declaration (debug info ?)
   void GenerateScriptExport();

   // TODO : consts export to Script
   //void GenerateScriptSymbols();

   /// Debug list of enums
   void GenerateEnums();

   /// Debug list of consts
   void GenerateConsts();

private:
   /**
     Declare ToString and FromString converter prototypes for every enum in this package

    FileNameBase is the C++ source file name (where the declarations reside) without .h/.cpp extension
   */
   void GenerateEnumConverterHeaders( const string& FileNameBase );

   /// Declare ToString and FromString converter prototypes for every enum in this package
   void GenerateEnumConverters( const string& FileName );

	LString GetEnumConvertersIncludeFile() const;

#pragma endregion

#pragma region C++ parsing

public:
   /// Process every directory in FPackageInDirs list
   bool ProcessPackageInputDirectories();

   /// Process header file and extract each definition
   void ParseHeaderFile( const string& FileName );

   /// Last parsing error
   string FLastError;
#pragma endregion

private:
   /**
     Each exported class/method/symbol in the package must be registered.

     For the MSVS.2002/2003 compiler we have to split
    class/method registration into multiple files
    because the linker does not allow too much symbol declarations
   */

   /// Generate standart starting part for the class exports file
   buffered_stream* BeginExportsFile( const string& ExpFileName, const clStringsList& Includes, int Index ) const;

   /// Generate standart ending trailer for the class exports file
   void             EndExportsFile( buffered_stream* OutExportsI ) const;

   /// Make a unique file name for the next class registration file
   string    CreateExportRegFileName( int Index ) const;
   string    CreateExportRegFileNameWithoutPath( int Index ) const;
   void      GenerateExportsRegHeader( buffered_stream& Out ) const;

   void      GenerateExportsHeader( buffered_stream& Out ) const;
   void      GenerateExportsFooter( buffered_stream& Out, const string& BaseClass, const string& BaseClassExports,
                                    const clStringsList& Includes, const clStringsList& ClassNames ) const;

   void      GenerateExportsH( buffered_stream& Out ) const;

   /// The function to get a .NET object lying at the bottom of class hierarchy
   void GenerateNETDowncasterImpl( clClass& Class, buffered_stream& Out );

private:
   // Remove any of PackageInDirs from file name
   string RemovePackageDirectoryFromFile( const LString& InName ) const;
};

#endif
