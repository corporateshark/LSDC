/**
 * \file Method.cpp
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

#ifndef _Method_h_
#define _Method_h_

#include "BufStream.h"
#include "MethodArg.h"
#include "Utils.h"

using namespace std;

struct clClass;
struct clPackage;
struct clDatabase;

struct clMethod
{
   string           FClassName;
   string           FMethodName;
   string           FReturnType;

#pragma region List-based arguments description
   /// List of argument names
   clStringsList    FArgNames;
   /// List of argument types
   clStringsList    FArgTypes;
   /// List of default values for the arguments
   clStringsList    FArgDefaults;
#pragma endregion

#pragma region Standart method modifiers
   bool             FConst;
   bool             FAbstract;
   bool             FVirtual;
   bool             FStatic;
	bool             FVolatile;
   string           FAccess;
#pragma endregion

#pragma region Non-standart (LSDC-defined) method properties

   /// Is it mapped to NET method (assigned in clClass::GenerateNETClassHeader)
   bool             FExportedToNET;

   /// Is it serialized
//   bool             FSerialized;
#pragma endregion

   /// Link to the global item database
   clDatabase*      FDatabase;

#pragma region Used only in "scriptexport" generation

   /// Link to the package this method belongs to
   clPackage*       FPackage;

   /// The class where this method prototype resides
   string           FDeclaredIn;
#pragma endregion

   // new argument containers, unused
   vector<clMethodArg> FArgs;

   clMethod() : FArgs(), FStatic( false ), FConst( false ), FVolatile( false ), FVirtual( false ), FExportedToNET( false ) {}

   /// Parse C++ method definition
   void FromString( const string& Line );

   /// Script stub (tunneller) code for method call (native -> script data marshalling)
   void    GenerateMethodStub( const clClass* OriginalClass, buffered_stream& Out ) const;

   /// .NET (C++/CLI) binder class implementation
   bool    GenerateMethodDotNETImpl( buffered_stream& Out ) const;

   /// .NET (C++/CLI) binder class header
   bool    GenerateMethodDotNETHeader( buffered_stream& Out, int MaxRetType ) const;

   /// check if this method can be exported to .NET (check each argument type)
   bool    CanExportToNET() const;

   /// Get maximum length of method return type. Used for source code formatting
   int     GetNETReturnTypeLength() const;

private:
   /// Form a parameter list for .NET method binder
   string MakeNetArgList( string& Result ) const;

   /// Store argument in a list
//   void AddOldStyleArgument( const clMethodArg& Arg );

   /// Parse argument list
   void ParseArgsList( string& ArgsList );

   bool    GenerateNETMethodSignature( buffered_stream& Out, int MaxRetType, bool UseClassName, bool UseStatic ) const;
};


#endif

/*
 * 08/04/2009
     FArgNames
 * 27/07/2007
     It's here
*/
