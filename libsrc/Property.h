/**
 * \file Property.h
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

#ifndef __Property__h__included__
#define __Property__h__included__

#include <string>
using namespace std;

struct clDatabase;

struct clProperty
{
	clProperty():SmartPointer(false) {};

   clDatabase* FDatabase;

   /// where it resides
   string FClassName;

   /// to-string converter, can be determined automatically from property type
   string ToStringConverter;
   /// from-string converter, can be determined automatically from property type
   string FromStringConverter;

   /// type of the property - if the FieldName is specified, then it is autodetected
   string Type;

   /// name of the property
   string Name;

   /// textual description of the property
   string Description;

   /// category of the property
   string Category;

   /// if it is a get-property, then this method must be specified
   string Getter;
   /// if it is a set-property, then this method must be specified
   string Setter;

   /// 'get' method for indexed property access - C++/CLI specific
   string NetIndexedGetter;
   /// 'set' method for indexed property access - C++/CLI specific
   string NetIndexedSetter;

   /// 'add' function for IList<T> implementation
   string NetAddFunction;
   /// 'remove' function for IList<T> implementation
   string NetRemoveFunction;
   /// 'clear' function for IList<T> implementation
   string NetClearFunction;
   /// 'count' function for IList<T> implementation
   string NetCounterFunction;

   /// Can we declare indexer for this prop
   bool IsIndexSupported() const
   {
	return !(NetIndexedGetter.empty() || NetIndexedSetter.empty() ||
				NetAddFunction.empty() || NetRemoveFunction.empty() || NetCounterFunction.empty() );
   }

   /// if it is a direct field access, then this field accessor is defined
   string FieldName;

   /// if the property is indexed, then this type is specified - int, string or whatever
   string IndexType;

   /// if the property is indexed, then this is the number-of-elements function
   string Counter;

   /// validator function (or <default>)
   string Validator;

   /// non-standart error reporting function (serialization)
   string ErrorLogger;

	/// clPtr<> wrapped
	bool SmartPointer;

   //////// Methods ////

   /// Internal parameter parser
   bool SetParam( const string& ParamName, const string& ParamValue );

   /// Convert property description to string
   string ToString() const;

   /// Convert string to property description, returning error code is something is wrong
   string FromString( const string& P );

   /// Check the validity of every parameter and their combinations
   string Validate();

   /// Scripted declaration
   string GetScriptDeclaration() const;

#pragma region Serialization stuff

   string GetSaveCode() const;
   string GetLoadCode() const;

   string GetRegistrationCode(string& FullPropertyName) const;
   string GetLoadSaveDeclarations() const;

   bool Saveable() const;
   bool Loadable() const;

   string GetFromStringConverter() const;
   string GetToStringConverter() const;

#pragma endregion

#pragma region Editing stuff

   string EditorType;

   string Editable;

   bool IsEditable() const;

#pragma endregion

#pragma region .NET stuff

   /// Check if it is an indexed property
   inline bool IsIndexed() const { return !IndexType.empty(); }

   /// DEFINE_INDEXER_STUFF
   string GetIndexerStuffDefinition() const;

   /// INIT_INDEXER_STUFF
   string GetIndexerStuffInitialization() const;

   /// DECLARE_PROPERTY string formation
   string DeclareNETProperty() const;

   /// DECLARE_PROPERTY_IMPL for the .cpp
   string DeclareNETProperty_Impl() const;

   string GetBinderMacro( bool IsArray, bool IsScalar, bool Accessor, bool Load, const string& Conv ) const;

#pragma endregion
};

#endif
