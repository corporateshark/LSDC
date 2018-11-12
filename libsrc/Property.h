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

struct clDatabase;

struct clProperty
{
	clProperty():SmartPointer(false) {};

	clDatabase* FDatabase;

	/// where it resides
	std::string  FClassName;

	/// to-string converter, can be determined automatically from property type
	std::string  ToStringConverter;
	/// from-string converter, can be determined automatically from property type
	std::string  FromStringConverter;

	/// type of the property - if the FieldName is specified, then it is autodetected
	std::string  Type;

	/// name of the property
	std::string  Name;

	/// textual description of the property
	std::string  Description;

	/// category of the property
	std::string  Category;

	/// if it is a get-property, then this method must be specified
	std::string  Getter;
	/// if it is a set-property, then this method must be specified
	std::string  Setter;

	/// 'get' method for indexed property access - C++/CLI specific
	std::string  NetIndexedGetter;
	/// 'set' method for indexed property access - C++/CLI specific
	std::string  NetIndexedSetter;

	/// 'add' function for IList<T> implementation
	std::string  NetAddFunction;
	/// 'remove' function for IList<T> implementation
	std::string  NetRemoveFunction;
	/// 'clear' function for IList<T> implementation
	std::string  NetClearFunction;
	/// 'count' function for IList<T> implementation
	std::string  NetCounterFunction;

	/// Can we declare indexer for this prop
	bool IsIndexSupported() const
	{
		return !(NetIndexedGetter.empty() || NetIndexedSetter.empty() ||
			NetAddFunction.empty() || NetRemoveFunction.empty() || NetCounterFunction.empty() );
	}

	/// if it is a direct field access, then this field accessor is defined
	std::string  FieldName;

	/// if the property is indexed, then this type is specified - int, std::string  or whatever
	std::string  IndexType;

	/// if the property is indexed, then this is the number-of-elements function
	std::string  Counter;

	/// validator function (or <default>)
	std::string  Validator;

	/// non-standart error reporting function (serialization)
	std::string  ErrorLogger;

	/// clPtr<> wrapped
	bool SmartPointer;

	//////// Methods ////

	/// Internal parameter parser
	bool SetParam( const std::string & ParamName, const std::string & ParamValue );

	/// Convert property description to std::string
	std::string  ToString() const;

	/// Convert std::string  to property description, returning error code is something is wrong
	std::string  FromString( const std::string & P );

	/// Check the validity of every parameter and their combinations
	std::string  Validate();

	/// Scripted declaration
	std::string  GetScriptDeclaration() const;

#pragma region Serialization stuff

	std::string  GetSaveCode() const;
	std::string  GetLoadCode() const;

	std::string  GetRegistrationCode(std::string & FullPropertyName) const;
	std::string  GetLoadSaveDeclarations() const;

	bool Saveable() const;
	bool Loadable() const;

	std::string  GetFromStringConverter() const;
	std::string  GetToStringConverter() const;

#pragma endregion

#pragma region Editing stuff

	/// Editor type name (empty string for default editor)
	std::string  EditorType;

	/// Generic string with parameters (e.g., min/max values for floats/vectors etc.)
	std::string  EditorParams;

	/// Is this property editable
	std::string  Editable;

	bool IsEditable() const;

#pragma endregion

#pragma region .NET stuff

	/// Check if it is an indexed property
	inline bool IsIndexed() const { return !IndexType.empty(); }

	/// DEFINE_INDEXER_STUFF
	std::string  GetIndexerStuffDefinition() const;

	/// INIT_INDEXER_STUFF
	std::string  GetIndexerStuffInitialization() const;

	/// DECLARE_PROPERTY std::string  formation
	std::string  DeclareNETProperty() const;

	/// DECLARE_PROPERTY_IMPL for the .cpp
	std::string  DeclareNETProperty_Impl() const;

	std::string  GetBinderMacro( bool IsArray, bool IsScalar, bool Accessor, bool Load, const std::string & Conv ) const;

#pragma endregion
};

#endif
