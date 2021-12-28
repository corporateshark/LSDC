/**
 * \file LSDC.cpp
 * \brief Main driver file for the database compiler
 *
 * LinderScript Database Compiler
 *
 * \version 0.9.80
 * \date 28/12/2021
 * \author Sergey Kosarevsky, 2010-2021
 * \author Viktor Latypov, 2010-2014
 * \author support@linderdaum.com, viktor@linderdaum.com http://www.linderdaum.com
 */

#include "Database.h"
#include <iostream>

#include "MethodBinding.h"
#include "FunctorGeneration.h"

#include "Utils.h"

void CheckArgs( int i, int args, std::string Message )
{
   if ( i >= args )
   {
      std::cout << Message << std::endl;

      exit( 255 );
   }
}

/// no statistics by default
bool DumpStatistics = true;

/// Declare the list of package directories to be processed
vector<string> PackageInDirs;

/// The package to be processed (if not empty, then only this package is processed)
string OnlyPackage = "";

void Help()
{
   cout << "LSDC [-v|-s] [-stats] [-log] [-no-methods] -p <path> [-p <path>...] [--exclude <filename>...] [--exclude <dirname>...]" << endl;
   cout << "LSDC --generate-opcodes" << endl;
   cout << "LSDC --generate-functors" << endl;
   cout << "LSDC --generate-binders" << endl;
   cout << "LSDC --generate-capsules" << endl;
   cout << endl;
   cout << " -v          --verbose" << endl;
   cout << " -s          --silent" << endl;
   cout << " -stats      --statistics" << endl;
   cout << " -no-methods --disable-method-export" << endl;
   cout << "             --no-export-shortcuts" << endl;
   cout << "             --exclude" << endl;
   cout << "             --excludedir" << endl;
   cout << " -log        --enable-logging" << endl;
   cout << " -gen-func   --generate-functors" << endl;
   cout << " -gen-bind   --generate-binders" << endl;
   cout << " -gen-cap    --generate-capsules" << endl;
   cout << " -p          --package" << endl;
   cout << endl;
}

/**
  Here we read each command line argument

  0) "-log" or "--enable-logging" enables internal on-screen logging
     "--statistics" or "-stats" enables the debug statistics generation for every loaded package
     "--no-export-shortcuts" disables the usage of macros for method registration
     "--disable-method-export" or "-no-methods" disables export of class methods

  // TODO : "--no-serialization-shortcuts" to disable macros for serialized properties

  LSDC can produce some supplementary autogenerated code:

  1) "-gen-func" or "--generate-functors" switch produces Alexandrescu-style functor definitions
    Next parameter specifies the maximum number of functor's parameters

  2) "-gen-bind" or "--generate-binders" switch produces method binders used in RTTI/Script
    Next parameter specifies the maximum number of the method to bind

  3) "-gen-cap" or "--generate-capsules" switch produces async capsules
    Next parameter specifies the maximum number of the method to bind

  4) "--package" or "-p" specifies another input directory with a native (C++) package
*/
void ProcessCommandLine( int argc, char** argv )
{
   int i = 1;

   while ( i < argc )
   {
      string OptionName( argv[i] );

      if ( OptionName == "-v" || OptionName == "--verbose" )
      {
         g_Verbose = true;
      }
      else if ( OptionName == "-s" || OptionName == "--silent" )
      {
         g_Verbose = false;
      }
		else if ( OptionName == "-stats" || OptionName == "--statistics" )
      {
         DumpStatistics = true;
      }
      else if ( OptionName == "--disable-method-export" || OptionName == "-no-methods" )
      {
         g_ExportMethods = false;
      }
      else if ( OptionName == "--no-export-shortcuts" )
      {
         g_UseExportShortcuts = false;
      }
      else if ( OptionName == "--exclude" )
      {
         CheckArgs( i + 1, argc, "Expecting file name to exclude" );

         i++;

         ExcludeFile( argv[i] );
      }
      else if ( OptionName == "--excludedir" )
      {
         CheckArgs( i + 1, argc, "Expecting dir name to exclude" );

         i++;

         ExcludeDir( argv[i] );
      }
      else if ( OptionName == "--enable-logging" || OptionName == "-log" )
      {
         g_EnableLogging = true;
      }
      else if ( OptionName == "-gen-func" || OptionName == "--generate-functors" )
      {
         CheckArgs( i + 1, argc, "Maximum number of parameters expected for functors" );

         i++;

         int NumParams = atoi( argv[i] );

         if ( NumParams < 1 || NumParams > 20 )
         {
            cout << "Invalid number of method parameters for functors: " << NumParams << endl;
            exit( 255 );
         }

         std::ofstream Out( "Functors.h" );

         GenerateFunctors( Out, NumParams );

         exit( 0 );
      }
      else if ( OptionName == "-gen-bind" || OptionName == "--generate-binders" )
      {
         CheckArgs( i + 1, argc, "Maximum number of parameters expected for method binders" );

         i++;

         int NumParams = atoi( argv[i] );

         if ( NumParams < 1 || NumParams > 30 )
         {
            cout << "Invalid number of method parameters for binders: " << NumParams << endl;
            exit( 255 );
         }

         std::ofstream Out( "MethodBind.h" );

         GenerateMethodBinders( Out, NumParams );

         exit( 0 );
      }
      else if ( OptionName == "-gen-cap" || OptionName == "--generate-capsules" )
      {
         CheckArgs( i + 1, argc, "Maximum number of parameters expected for capsules" );

         i++;

         int NumParams = atoi( argv[i] );

         if ( NumParams < 1 || NumParams > 30 )
         {
            cout << "Invalid number of method parameters for capsules: " << NumParams << endl;
            exit( 255 );
         }

         std::ofstream Out( "AsyncCapsule.h" );

         GenerateCapsules( Out, NumParams );

         exit( 0 );
      }
      else if ( OptionName == "--onlypackage" || OptionName == "-op")
      {
		  CheckArgs( i + 1, argc, "Package Name expected for option --onlypackage" );
		  string Pkg = string( argv[i+1] );
         // store the "only" package
		  OnlyPackage = Pkg;

		  if ( g_Verbose ) cout << "Generating code only for Package named \"" << Pkg << "\" " << endl;
		  
		  i++;
      }
      else if ( OptionName == "-p" || OptionName == "--package" )
      {
         CheckArgs( i + 1, argc, "Package directory name expected for option --package" );
         string Dir = string( argv[i+1] );

         PackageInDirs.push_back( Dir );

			if ( g_Verbose ) cout << "Processing package directory: " << Dir << endl;

         i++;
      }
      else
      {
         CheckArgs( 0, 0, "Invalid option: " + OptionName );
      }

      i++;
   }
}

void Banner()
{
   cout << "LinderScript Database Compiler" << endl;
   cout << "Version " << LSDCVersion << " (" << LSDCDate << ")" << endl;
   cout << "(C) Sergey Kosarevsky, 2005-2021" << endl;
   cout << "(C) Viktor Latypov, 2007-2014" << endl << endl;
}

int main( int argc, char** argv )
{
	/*
	std::ofstream Out( "C:/trunk.git/Src/Generated/AsyncCapsule.h" );

	GenerateCapsules( Out, 11 );
*/
   Banner();

   /// Local package database
   clDatabase DB;

	if ( argc <= 1 )
	{
		Help();
		exit( 255 );
	}

   ProcessCommandLine( argc, argv );

   // TODO : debug only, reads a single package
   if ( PackageInDirs.empty() ) { PackageInDirs.push_back( "../../Src" ); }
	
   // Read each specified package
   for ( size_t i = 0; i != PackageInDirs.size(); ++i )
   {
      DB.ProcessPackageDirectory( PackageInDirs[i] );
   }

	if(OnlyPackage.empty())
	{
		// process all packages
		DB.GenerateStuff();
	} else
	{
		// process single package [useful for non-destructive update of a single package]
		DB.GenerateStuffForPackage(OnlyPackage);
	}

   // statistics, if required
   if ( DumpStatistics ) { DB.GenerateStatistics(); }

   return 0;
}

/*
 * 20/09/2010
     Simple driver program for LSDCLib
*/
