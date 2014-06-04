#include "Package.h"

#include <iostream>
using std::cout;
using std::endl;

#if (defined(_WIN32) || defined(_WIN64))
#include <direct.h>
#include <windows.h>
//#endif
#else
//#ifdef __linux__
#include <time.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#endif

#include "Utils.h"

#include "HeaderProcessor.h"

#include "FileWalker.h"

class Package_FileWalker : public FileWalker
{
public:
   clPackage* FPackage;

   virtual void ProcessDirectory( const string& dirName, const string& shortName )
   {
      if ( shortName != ".svn" && shortName != ".cvs" )
      {
         Scan( dirName );
      }
   }
   virtual void ProcessFile( const string& fileName, const string& shortName )
   {
      if ( shortName.find( ".h" ) != -1 && !IsFileExcluded( shortName ) )
      {
         FPackage->ParseHeaderFile( fileName );
      }
   }
};


/*
// this is the base class to start from
const string Default_BaseStart        = "iObject"; // Scriptable
const string Default_BaseStartExports = "iObject";

string BaseStart = Default_BaseStart;
string BaseStartExports = Default_BaseStartExports;
*/

const char PATH_SEPARATOR = '\\';

// TODO : move to utils, if it is used
void CreateDirsPhys( const string& DirectoryName )
{
   string Path = ReplaceAll( DirectoryName + "/", '/', PATH_SEPARATOR );

   for ( size_t i = 0; i < Path.length(); i++ )
   {
      if ( Path[i] == PATH_SEPARATOR )
      {
         Path[i] = '\0';
#if (defined(_WIN32) || defined(_WIN64))
         _mkdir( Path.c_str() );
#else
         mkdir( ReplaceAll( Path, '\\', '/' ).c_str(), 0777 ); // second ARG ?
#endif
         Path[i] = PATH_SEPARATOR;
      }
   }
}

void clPackage::CreatePackageDirectories()
{
   // statistics
   CreateDirsPhys( FPackageOutDirectory + string( "/" ) + StatisticsDirName );
   // script export
   CreateDirsPhys( FPackageOutDirectory + string( "/" ) + ScriptDirName );
   // native registration
   CreateDirsPhys( FPackageOutDirectory + string( "/" ) + ExportDirName );
   // .NET binding
   CreateDirsPhys( FPackageOutDirectory + string( "/" ) + NETDirName );
}

string clPackage::GetScriptExportDir() const
{
	return FPackageOutDirectory + "/" + ScriptDirName;
}

void clPackage::GenerateStuff()
{
   // just in case, create the directories
   CreatePackageDirectories();

   if ( FGenerateExports ) { GenerateNativeFramework(); }

   // TODO : BaseClassForTunnellers and BaseClassForExports should be specified in config
   if ( FGenerateTunnellers ) { GenerateStubs( "iObject", "iObject" ); }

   if ( FGenerateNETExport ) { GenerateDotNETWrappers(); }

   if ( FGenerateScriptExports ) { GenerateScriptExport(); }

   if ( FGenerateEnums ) { GenerateEnums(); }

   if ( FGenerateConsts ) { GenerateConsts(); }
}

void clPackage::GenerateStatistics()
{
	string Dir = FPackageOutDirectory + string( "/" ) + StatisticsDirName;

	DumpPackageStats( Dir + string( "/Debug_PackageStats.txt" ) );

	buffered_stream OutList( string( Dir + string( "/Package1_ClassesList.Test" ) ).c_str() );
	GenerateClassesList( OutList );
	OutList.write();
}

vector<string> StdSerializationHeaders;

void clPackage::GenerateEnumConverterHeaders( const string& FileName )
{
	buffered_stream Out( FileName.c_str() );

	string IncludeGuard = string( "__" ) + FPackageName + string( "__EnumConversions__h__included__" );

	// std include guard
	Out << "#ifndef " << IncludeGuard << endl;
	Out << "#define " << IncludeGuard << endl;

	// forward definitions and function prototypes

	if ( FEnums.size() > 0 )
	{
		Out << endl << "#include \"LString.h\"" << endl << endl;

		Out << endl;

		for ( size_t i = 0 ; i < FEnums.size() ; i++ )
		{
			if(FEnums[i].FExported)
			{
				// for C++11 we may use forward defs
				// Out << "enum " << FEnums[i].FEnumName << ";" << endl << endl;
				Out << "#include \"" << FEnums[i].FDeclaredIn << "\"" << endl << endl;

				FEnums[i].GenerateConverterHeaders( Out );
				Out << endl;
				Out << endl;
			}
		}
	}

	// end of std_include_guard
	Out << "#endif" << "  // " << IncludeGuard << endl;
}

LString clPackage::GetEnumConvertersIncludeFile() const
{
	return FPackageName + "_EnumConverters.h";
}

void clPackage::GenerateEnumConverters( const string& FileNameBase )
{
	buffered_stream Out( string( FileNameBase + string( ".cpp" ) ).c_str() );

	Out << "#include \"Generated/" << GetEnumConvertersIncludeFile() << "\"" << endl;

	// include every file with type definitions and declare converters
	if ( FEnums.size() > 0 )
	{
		Out << endl;

		for ( size_t i = 0 ; i < FEnums.size() ; i++ )
		{
			if(FEnums[i].FExported)
			{
				// std::cout << "Enum gen(source): " << FEnums[i].FEnumName << std::endl;
				// fflush(stdout);

				Out << "#include \"" << FEnums[i].FDeclaredIn << "\"" << endl << endl;

				// std::cout << "Enum gen(tostr): " << FEnums[i].FEnumName << std::endl;
				// fflush(stdout);

				FEnums[i].GenerateToStringConverter( Out );
				Out << endl;

				// std::cout << "Enum gen(fromstr): " << FEnums[i].FEnumName << std::endl;
				// fflush(stdout);

				FEnums[i].GenerateFromStringConverter( Out );
				Out << endl;

				// std::cout << "Enum gen(ok): " << FEnums[i].FEnumName << std::endl;
				// fflush(stdout);
			}
		}
	}
}

// delete one of the PackageIncludeDirs from file name
string clPackage::RemovePackageDirectoryFromFile( const LString& InName ) const
{
	for ( size_t i = 0 ; i < FPackageInDirectories.size() ; i++ )
	{
		string Dir = FPackageInDirectories[i];

		if ( InName.find( Dir ) != std::string::npos )
		{
			return TrimSpaces( InName.substr( Dir.length() + 1, InName.length() - 1 ) );
		}
	}

	return InName;
}

void clPackage::ParseHeaderFile( const string& FileName )
{
	clHeaderProcessor P;

	P.FDatabase = FDatabase;
	P.FPackage  = this;

	P.IncFileName = RemovePackageDirectoryFromFile( FileName );

	if ( !P.ProcessFile( FileName ) )
	{
		cout << "Error parsing " << FileName << endl;
		cout << P.FLastError << endl;
		// some error
		exit( 255 );
	}
}

string clPackage::CreateExportRegFileName( int Index ) const
{
	return FPackageOutDirectory + string( "/" ) + ExportDirName + string( "/ExpReg" ) + FPackageCPPPrefix + Int2Str( Index ) + ".cpp";
}

string clPackage::CreateExportRegFileNameWithoutPath( int Index ) const
{
	return string( "ExpReg" ) + FPackageCPPPrefix + Int2Str( Index ) + ".cpp";
}

// write some stuff to shut up stupid WhatsNew/Scheduler tool
void WriteFileFooter( buffered_stream& OutFile )
{
   OutFile << endl;
   OutFile << "/*" << endl;
   OutFile << " * " << LSDCDate << endl;
   OutFile << "     Autogenerated via " << LSDCName << endl;
   OutFile << "*/" << endl;
}

void clPackage::EndExportsFile( buffered_stream* OutExportsI ) const
{
	if ( UseExportShortcuts )
	{
		// undefined utility macro
		( *OutExportsI ) << endl << "#undef REG_CLS_MTD" << endl;
	}

	WriteFileFooter( *OutExportsI );

	// delete file stream here
	delete OutExportsI;
}

buffered_stream* clPackage::BeginExportsFile( const string& ExpFileName, const clStringsList& Includes, int Index ) const
{
   buffered_stream* OutExportsI = new buffered_stream( ExpFileName.c_str() );

   OutExportsI->WriteDoxygenHeader( CreateExportRegFileNameWithoutPath(Index), "Autogenerated via " + LSDCName );

   GenerateExportsRegHeader( *OutExportsI );

   // if we use custom includes we include single file
   if ( FPackageCustomIncludeName.empty() )
   {
      // Old code : dump every include (slows down the compilation)
      for ( size_t ii = 0; ii != Includes.size(); ++ii )
      {
         OutExportsI->Include( Includes[ii] );
      }
   }
   else
   {
      // new code : insert reference to custom include file
      //            which resolves all difficulties
      ( *OutExportsI ) << "// Custom include file" << endl;
      OutExportsI->Include( FPackageCustomIncludeName );
   }

   if ( UseExportShortcuts )
   {
      // define utility macro to reduce code size
      ( *OutExportsI ) << endl << "#define REG_CLS_MTD(ClsName, MtdName) \\" << endl;
      ( *OutExportsI ) << "StaticClass->RegisterMethod( BindNativeMethod( & ClsName :: MtdName, #MtdName ) );" << endl;
   }

   return OutExportsI;
}

void clPackage::GenerateExportsHeader( buffered_stream& Out ) const
{
	Out.Include( "Exports_" + FPackageCPPName + string( ".h" ) );
	Out << endl;

	Out.Include( "Core/Linker.h" );
	Out.Include( "Core/RTTI/iStaticClass.h" );
	Out.Include( "Environment.h" );
}

void clPackage::GenerateExportsRegHeader( buffered_stream& Out ) const
{
	Out.Include( "Generated/MethodBind.h" );
	Out << endl;
	Out.Include( "Core/Linker.h" );
	Out.Include( "Core/RTTI/iStaticClass.h" );
	Out.Include( "Core/RTTI/FieldBinding.h" );
	Out.Include( "Core/RTTI/PropertyMacros.h" );
	Out << endl;

	Out.Include( "Core/VFS/ML.h" );
	Out << endl;

	Out.Include( "Generated/" + this->GetEnumConvertersIncludeFile() );
	Out << endl;
}

void clPackage::GenerateExportsH( buffered_stream& Out ) const
{
   string IncludeGuard = string( "_Package" ) + FPackageName + string( "Exports_" );

   Out << "#ifndef " << IncludeGuard << endl;
   Out << "#define " << IncludeGuard << endl << endl;

   Out.Include( "Core/Linker.h" );
   Out << endl;

   for ( int i = 0; i != FPackagesProcsCounter; ++i )
   {
      Out << "void RegisterPackage" << FPackageName << "Class" << i << "(sEnvironment* Env);" << endl;
   }

   Out << endl;

   Out << "void RegisterPackage" << FPackageName << "Tunnellers(sEnvironment* Env);" << endl;
   Out << endl;
   Out << "#endif" << " // " << IncludeGuard << endl;
   Out << endl;
   Out << "/*" << endl;
   Out << " * " << LSDCDate << endl;
   Out << "     Autogenerated via " << LSDCName << endl;
   Out << "*/" << endl;
}

void clPackage::GenerateExportsFooter( buffered_stream& Out, const string& BaseClass, const string& BaseClassExports,
                                       const clStringsList& Includes, const clStringsList& ClassNames ) const
{
   const int MaxClassesInExport = 50;

   int ExportsIndex          = 1;
   int ClassesExportedInFile = 0;

   // registered classes
   vector<string> RegisteredClasses;

   buffered_stream* OutExportsI = BeginExportsFile( CreateExportRegFileName( ExportsIndex ), Includes, ExportsIndex );

   // generate classes
   for ( map<string, clClass>::const_iterator i = FClasses.begin(); i != FClasses.end(); ++i )
   {
      if ( ClassesExportedInFile > MaxClassesInExport )
      {
         EndExportsFile( OutExportsI );

         ClassesExportedInFile = 0;
         ExportsIndex++;

         OutExportsI = BeginExportsFile( CreateExportRegFileName( ExportsIndex ), Includes, ExportsIndex );
      }

      string CN = i->second.FClassName;

      bool ShouldGenerateClassExport = false;

/*      if ( !i->second.HasDefaultConstructor() )
      {
         if ( EnableLogging )
         {
            Out << "   // Class: " << i->second.FClassName << " has no default constructor - bypassing" << endl;
         }
      }
      else*/ if ( InheritsFrom( CN, BaseClass ) )
      {
         // optimize includes
         for ( size_t j = 0; j != ClassNames.size(); ++j )
         {
            if ( ( ClassNames[j] == CN ) || ( ClassNames[j] == CN + "_Tunneller" ) )
            {
               ( *OutExportsI ) << endl;
               OutExportsI->Include( Includes[j] );
            }
         }

         // do not generate tuneller export code if the class is ScriptFinal
         // or thisclass has some 'scriptfinal' ancestors
         if ( !i->second.FScriptFinal /* && !i->second.HasScriptfinalAncestors() */ )
         {
            // generate exports for tunneller only if it is not excluded
            if ( i->second.GenerateEngineExports( *OutExportsI, true ) )
            {
               // one export for tuneller
               ClassesExportedInFile++;

               RegisteredClasses.push_back( CN + string( "_Tunneller" ) );
            }
         }

//         cout << "Generating engine exports for " << i->second.FClassName << endl;
         ShouldGenerateClassExport = true;
         //
      }
      else if ( InheritsFrom( CN, BaseClassExports ) )
      {
         // optimize includes
         for ( size_t j = 0; j != ClassNames.size(); ++j )
         {
            if ( ClassNames[j] == CN )
            {
               ( *OutExportsI ) << endl;
               OutExportsI->Include( Includes[j] );
               break;
            }
         }

         ShouldGenerateClassExport = true;
      }

//     cout << "Generating " << CN << endl;

      if ( ShouldGenerateClassExport )
      {
         if ( i->second.GenerateEngineExports( *OutExportsI, false ) )
         {
            // another export
            ClassesExportedInFile++;

            // remember this class
            RegisteredClasses.push_back( CN );
         }
      }
   }

   EndExportsFile( OutExportsI );

   Out << endl << endl;

   Out << "void RegisterPackage" << FPackageName << "Tunnellers(sEnvironment* Env)" << endl;
   Out << "{" << endl;

   for ( int i = 0; i != FPackagesProcsCounter; ++i )
   {
      Out << "   RegisterPackage" << FPackageName << "Class" << i << "( Env );" << endl;
   }

   /// Virtual Table building

   /// We have to iterate each registered class and call the BuildVirtualTables() method
   /// To do so, we have to store every class name being registered for this package. Any suggestions ?

   Out << "   // NumClasses : " << RegisteredClasses.size() << endl;

   Out << "#if !defined(_DISABLE_METHODS_) && !defined(_DISABLE_TUNNELLERS)" << endl;

   for ( size_t i = 0 ; i < RegisteredClasses.size() ; i++ )
   {
      Out << "   Env->Linker->FindStaticClass( \"" << RegisteredClasses[i] << "\" )->BuildVirtualTables();" << endl;
   }

   Out << "#endif // _DISABLE_METHODS_" << endl;

   Out << "}" << endl;

   WriteFileFooter( Out );
}

void clPackage::GenerateNativeFramework() const
{
   buffered_stream ScriptStubs( string( FPackageOutDirectory + string( "/Declarations_" ) + FPackageName + string( ".rs" ) ).c_str() );

   ScriptStubs << "// LinderScript Native Framework" << endl;
   ScriptStubs << "// Autogenerated via " << LSDCName << endl;
   ScriptStubs << endl;

   for ( map<string, clClass>::const_iterator i = FClasses.begin(); i != FClasses.end(); ++i )
   {
      clClass Class = i->second;

      if ( Class.FMethods.empty() ) { continue; }

      Class.WriteScriptDeclaration( ScriptStubs );
   }

   ScriptStubs.write();
}

void clPackage::GenerateStubs( const string& BaseClass, const string& BaseClassExports ) const
{
   const vector<string>& NeverOverride = FNeverOverrideMethodsOf;
   // collection to store all include files
   // each include file is stored only once
   clStringsList     Includes;
   clStringsList     ClassNames;

   for ( map<string, clClass>::const_iterator i = FClasses.begin(); i != FClasses.end(); ++i )
   {
      if ( InheritsFrom( i->second.FClassName, BaseClass ) )
      {
         if ( !( i->second ).FScriptFinal )
         {
            if ( ( i->second ).GenerateClassStub( NeverOverride ) )
            {
               string TunnellerName = i->second.FClassName + "_Tunneller";

               if ( AddUnique( Includes, TunnellerName + ".h" ) )
               {
                  ClassNames.push_back( TunnellerName );
               }
            }
         }
      }

      if ( InheritsFrom( i->second.FClassName, BaseClassExports ) )
      {
         // remove duplicates
         if ( AddUnique( Includes, i->second.FDeclaredIn ) )
         {
            ClassNames.push_back( ( i->second ).FClassName );
         }
      }
   }

   string FNameBase = FPackageOutDirectory + "/" + ExportDirName + "/Exports_" + FPackageCPPName;

   buffered_stream OutExports( ( FNameBase + ".cpp" ).c_str() );
	OutExports.WriteDoxygenHeader( "Exports_" + FPackageCPPName + ".cpp", "Exports registration for package "+ FPackageName  +". Autogenerated via " + LSDCName );

   GenerateExportsHeader( OutExports );
   GenerateExportsFooter( OutExports, BaseClass, BaseClassExports, Includes, ClassNames );

   buffered_stream OutExportsH( ( FNameBase + ".h" ).c_str() );

	OutExportsH.WriteDoxygenHeader( "Exports_" + FPackageCPPName + ".h", "Exports registration header for package "+ FPackageName  +". Autogenerated via " + LSDCName );

   GenerateExportsH( OutExportsH );
}

void clPackage::GenerateClassesList( buffered_stream& Out ) const
{
   for ( map<string, clClass>::const_iterator i = FClasses.begin();
         i != FClasses.end();
         ++i )
   {
      clClass Class = i->second;

      Out << Class.FClassName << "   ( Declared in: " << Class.FDeclaredIn << " )" << endl;

      for ( clBaseClassesList::const_iterator j = Class.FBaseClasses.begin();
            j != Class.FBaseClasses.end();
            ++j )
      {
         Out << "    Base class: " << j->FBaseClassName << "  (" << j->FInheritanceType << ")" << ( j->FVirtualBase ? "  <- virtual base" : "" ) << endl;
      }
   }
}

///// Type information retrieval
bool clPackage::InheritsFrom( const string& Class, const string& BaseClass ) const
{
   // FIXME: look in global classes database, but we have to do it per-package
   //        considering inter-package relationships
   return FDatabase->InheritsFrom( Class, BaseClass );

   /*
      if ( Class == BaseClass ) return true;

      if ( FClasses.find(Class) == FClasses.end() )
      {
         //cout << endl;
         //cout << "WARNING: Unable to find class " << Class << " while checking base class " << BaseClass << endl;
         return false;
      }

      const clClass& TargetClass = FClasses.find(Class)->second;

      for (clBaseClassesList::const_iterator i = TargetClass.FBaseClasses.begin(); i != TargetClass.FBaseClasses.end(); ++i)
      {
         if ( (*i).FBaseClassName == BaseClass ) return true;
      }

      for (clBaseClassesList::const_iterator i = TargetClass.FBaseClasses.begin(); i != TargetClass.FBaseClasses.end(); ++i)
      {
         if ( InheritsFrom( (*i).FBaseClassName, BaseClass ) ) return true;
      }

      // look in other packages
      for ( vector<clPackage*>::const_iterator i = FDatabase->FPackages.begin(); i != FDatabase->FPackages.end(); ++i )
      {
         // skip this package
         if ( *i == this ) continue;

         bool Inherits = (*i)->InheritsFrom( Class, BaseClass );

         if ( Inherits ) return true;
      }

      cout << "InheritsFrom(" << Class << ", " << BaseClass << ") false" <<endl;

      return false;
   */
}

bool clPackage::IsWrappedClass( const string& ClassName )
{
   if ( FClasses.count( ClassName ) <= 0 ) { return false; }

   return FClasses[ClassName].FNetExportable;
}
/*
bool clPackage::IsSerializableClass( const string& ClassName )
{
   if ( FClasses.count( ClassName ) <= 0 ) { return false; }

   return FClasses[ClassName].FSerializable;
}
*/

void clPackage::GenerateScriptExport()
{
   buffered_stream Out( string( FPackageOutDirectory + "/" + StatisticsDirName + string( "/ScriptExport.txt" ) ).c_str() );

   for ( int i = 0; i != FScriptExportMethods.size(); ++i )
   {
      Out << FScriptExportMethods[i].FMethodName << " (exported to .NET class: " << FScriptExportMethods[i].FClassName << ", declared in " << FScriptExportMethods[i].FDeclaredIn << ")\n";
   }
}

void clPackage::GenerateConsts()
{
   buffered_stream Out( string( FPackageOutDirectory + string( "/ConstsOut.h" ) ).c_str() );

   /*
     TODO:
     FIXME:
     PROBLEM:
        Our ad hoc parser cannot see the scope of 'const' so we
        end up with local consts added to this list.

        There is no way to fix it without reworking our current parser.

        Possible workarounds:
          - directly specify .h files for consts extraction (preferred)
          - add keyword
   */
   for ( size_t i = 0; i != FConsts.size(); i++ )
   {
      clConst Const = FConsts[i];

      Out << Const.FType;

      if ( Const.FIsArray ) { Out << "[]"; }

      Out << " | " << Const.FName << " |=| " << Const.FValue << endl;
   }
}

void clPackage::GenerateEnums()
{
   buffered_stream Out( string( FPackageOutDirectory + string( "/EnumsOut.h" ) ).c_str() );

   for ( size_t i = 0; i != FEnums.size(); i++ )
   {
      clEnum Enum = FEnums[i];

      Out << Enum.FEnumName << endl;
      Out << "{" << endl;

      for ( size_t j = 0; j != Enum.FItems.size(); j++ )
      {
         Out << "   " << Enum.FItems[j].FItemName;

         if ( !Enum.FItems[j].FItemValue.empty() )
         {
            Out << " = " << Enum.FItems[j].FItemValue;
         }

         Out << "," << endl;
      }

      Out << "};" << endl;
      Out << endl;
   }

   // Now the converters with default .h/.cpp files
   GenerateEnumConverterHeaders( string( FPackageOutDirectory + string( "/" ) + FPackageName + string( "_EnumConverters.h" ) ) );
   GenerateEnumConverters( string( FPackageOutDirectory + string( "/" ) + FPackageName + string( "_EnumConverters" ) ) );
}

bool clPackage::ProcessPackageInputDirectories()
{
   FLastError = "";

   for ( size_t i = 0; i != FPackageInDirectories.size(); ++i )
   {
		if ( Verbose )
		{
	      cout << "Building classes database for: " << FPackageInDirectories[i] << "\\" << endl;
		}

      Package_FileWalker FW;
      FW.FPackage = this;

      FW.Scan( FPackageInDirectories[i] );

      if ( !FLastError.empty() ) { return false; }
   }

   return true;
}

void clPackage::DumpPackageStats( const string& fname )
{
   ofstream f( fname.c_str() );

   f << "Package statistics" << endl << endl;

   f << "Name = " << FPackageName << endl;

   f << "C++-compatible name of the package = " << FPackageCPPName << endl;

   f << ".NET Package name = " << FPackageNetName << endl;

   f << "CPP Export prefix = " << FPackageCPPPrefix << endl;

   f << "Additional include file for the package = " << FPackageCustomIncludeName << endl;

   f << "Number of procedures in package = " << FPackagesProcsCounter << endl;

   f << "Source directories for the package:" << endl;

   for ( size_t i = 0 ; i < FPackageInDirectories.size() ; i++ )
   {
      f << "Directory[" << i << "] = " << FPackageInDirectories[i] << endl;
   }

   f << "Exports/Tunellers output directory = " << FPackageOutDirectory << endl;

   f << "Package dependencies:" << endl;

   for ( size_t i = 0 ; i < FDependsOn.size() ; i++ )
   {
      f << "Depends on \"" << FDependsOn[i] << "\"" << endl;
   }

   f << "Native finale classes" << endl;

   for ( size_t i = 0 ; i < FNeverOverrideMethodsOf.size() ; i++ )
   {
      f << "NativeFinalClass = " << FNeverOverrideMethodsOf[i] << endl;
   }

   f << "Number of registered classes = " << FClasses.size() << endl;
}
