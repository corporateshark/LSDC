/**
 * \file Class.h
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

#ifndef _Class_h_
#define _Class_h_

#include "Method.h"
#include "Property.h"

#include "BufStream.h"

using namespace std;

typedef vector<clMethod>    clMethodsList;

struct clPackage;
struct clDatabase;

struct clBaseClassLink
{
   string    FBaseClassName;
   string    FInheritanceType;
   bool      FVirtualBase;
   bool      FTemplateClass;
};

struct clField
{
   string           FClassName;
   string           FFieldName;
   string           FPropertyName;
   clDatabase*      FDatabase;
   string           FFieldType;
   bool             FConst;
   string           FAccess;

   bool             FExcludeFromExport;
#pragma region Used only in "scriptsymbol"/"nativefield" generation
   string           FDeclaredIn;
#pragma endregion

   clField() {}

   string FromString( const string& F );
   string ToString() const;

   string GetScriptDeclaration() const;
};

struct clClass;

typedef vector<clClass>            clClassesCache;
typedef vector<clBaseClassLink>    clBaseClassesList;
typedef vector<clField>            clFieldsList;
typedef vector<clProperty>         clPropertiesList;

// some attribute attached to class, method or something else
struct clAttribute
{
   string FName;
   vector<string> FParameters;
};

/**
   Class is the main exported item. It corresponds to a single C++ class header.

   This type contains methods for class item definitions and for the generation of
     - serialization code
     - script exports
     - .NET bindings
     - code statistics/information
*/
struct clClass
{
   clClass(): FClassInternalID( -1 ) {};

   /// Link to class package which this class belongs to
   clPackage*             FPackage;
   /// Link to package/type database
   clDatabase*            FDatabase;

   /// File where this class is defined
   string                 FDeclaredIn;

   /// Name of this class
   string                 FClassName;

   /// List of ancestors
   clBaseClassesList      FBaseClasses;

   /// ID of a class (index in FDatabase->GlobalClassesList)
   int                    FClassInternalID;

   /// Temporary list of ancestor class handles (to avoid map<> lookups)
   mutable clClassesCache FBaseClassesCache;

#pragma region Class parts: methods, fields, etc.
   clMethodsList          FMethods;
   clMethodsList          FConstructors;
   clFieldsList           FFields;
   clPropertiesList       FProperties;
#pragma endregion

#pragma region Predefined Class attributes (defined in the C++ source code using LSDC metalanguage)

   /// Can this class be inherited in script
   bool                   FScriptFinal;
   /// Is this class exported to .NET
   bool                   FNetExportable;
   /// Is this class serializable (i.e., serialization code is generated for this class)
   bool                   FSerializable;

   /// Process a list of class attributes
   void AssignClassAttributes( const vector<string>& Attribs );
#pragma endregion

   /// Process .h-file with class definition
   void ParseClassBody( ifstream& In, bool Struct );

   void AddConstructor( const string& Access, const clMethod& TheConstructor );

   /// Add method to the list and check if there are no overloads
   void AddMethod( const string& Access, const clMethod& TheMethod );

   /// Add field to the list and check for duplicates
   void AddFieldA( const string& Access, const clField& Field );

   void AddProperty( const clProperty& Prop );

   /// Parse a string (it can be multiline) with base class list
   bool ExtractBaseClassList( const string& InBaseClasses, string& Error );

   /// Check if the line is a constructor header for this class
   bool IsConstructorDefinition( const string& S ) const;

   /// Check if the line is a destructor header for this class
   bool IsDestructorDefinition( const string& S ) const;

#pragma region Type information retrieval

   /// Check if the class contains some abstract (pure virtual) methods
   bool    IsAbstract( buffered_stream& Stub ) const;

   /// Check if the class has a parameterless contstructor (or the constructor list is empty)
   bool    HasDefaultConstructor() const;

   /// Try to find the class where MethodName is declared
   bool    FindOverridenMethod( const string& MethodName, string& OverridenInClass ) const;

   /// Get complete list of base classes
   void    CollectBaseClasses( vector<string>& OutList ) const;

   /// Get string list with every abstract method
   void    CollectAbstractMethods( buffered_stream& Stub, clStringsList& AbstractMethods ) const;

   /// Check if this class has a native ancestor
   bool    IsDerivedFromNativeBase() const;

   /// Check if this class is derived from some 'scriptfinal' class (to avoid Tunneller generation)
   bool    HasScriptfinalAncestors() const;

#pragma endregion

   void    GenerateInterfaceStub( const clClass* OriginalClass, buffered_stream& Stub, string& Access, clStringsList& AlreadyGeneratedMethods, const vector<string> NeverOverride ) const;

   void    GenerateClassInterface( const clClass* OriginalClass, buffered_stream& Stub, string& Access, clStringsList& AlreadyGeneratedMethods, const vector<string> NeverOverride ) const;

   /**
      Generates engine's class export code
      Tunneller flag automatically adds "_Tunneller" suffix to the class

      Returns boolean flag indicating whether the actual export code was generated.

     We need to track the number of generated exports because some linkers do not allow
     too much definitions in a single source file
   **/
   bool    GenerateEngineExports( buffered_stream& Stub, bool Tunneller ) const;

   ///   Create tunneller class for Script <-> NativeCode data marshalling
   bool    GenerateClassStub( const vector<string>& NeverOverride ) const;

   /// 'native class' declaration for the script
   void WriteScriptDeclaration( buffered_stream& Out ) const;

   /// .NET interop classes generation:  .h-part
   bool GenerateDotNETRefClass( buffered_stream& Out );

   /// .NET interop classes generation:  .cpp-part
   bool GenerateDotNETRefClassImpl( buffered_stream& Out );

   // Serialization : [to be removed right after the property mechanism is up and running]

   /// Serialization code for each property
   bool GenerateStdSaveCode( buffered_stream& Out );

   /// Deserialization code for each property
   bool GenerateStdLoadCode( buffered_stream& Out );

private:

#pragma region Registration of methods, fields and properties in C++ code

   /**
      Internal methods used in 'ExpReg.cpp' files
   */

   /// Code for method registration in current metaclass
   void WriteMethodRegistration( buffered_stream& Out, bool Tunneller ) const;

   /// Helper macros with field binder declarations
   void WriteFieldHandlers( buffered_stream& Out ) const;

   /// Code for field registration in current metaclass
   void WriteFieldRegistration( buffered_stream& Out ) const;

   /// Generation of helper macros with property loaders/savers
   void WritePropertyHandlers( buffered_stream& Out ) const;

   /// Code for property registration in current metaclass
   void WritePropertyRegistration( buffered_stream& Out ) const;

#pragma endregion

#pragma region Script Export

   /**
      Internal methods used for .rs class declaration
   */

   void WriteScriptConstructors( buffered_stream& Out ) const;
   void WriteScriptMethods( buffered_stream& Out ) const;
   void WriteScriptProperties( buffered_stream& Out ) const;
   void WriteScriptFields( buffered_stream& Out ) const;

#pragma endregion
};

#endif

/*
 * 12/12/2007
     Adopted for new Environment
 * 04/10/1007
     Fixed bug in basses classes caching schema
 * 30/07/2007
     Improved filtering of "native final" methods
 * 27/07/2007
     It's here
*/
