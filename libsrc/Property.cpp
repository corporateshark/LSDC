/**
 * \file Property.cpp
 * \brief
 *
 * LinderScript Database Compiler
 *
 * \version 0.9.22
 * \date 09/08/2011
 * \author Sergey Kosarevsky, 2005-2011
 * \author Viktor Latypov, 2007-2011
 * \author support@linderdaum.com http://www.linderdaum.com
 */

#include "Utils.h"
#include "Property.h"
#include "Package.h"

#include <iostream>
using namespace std;

/**

   Property syntax:

      Property(FieldName = "", )

      IndexedProperty
*/

inline void AddParam_NoValue(std::string& ParamList, const std::string& ParamName)
{
	if (!ParamList.empty()) { ParamList += std::string(", "); }

	ParamList += ParamName;
}

inline void AddParam(std::string& ParamList, const std::string& ParamName, const std::string& ParamValue)
{
	if (ParamValue.empty()) { return; }

	AddParam_NoValue(ParamList, ParamName);

	ParamList += std::string(" = ") + ParamValue;
}

std::string clProperty::ToString() const
{
	std::string P = "Property";

	std::string ParamList = "";

	AddParam(ParamList, "Name", TrimQuotes(Name));
	AddParam(ParamList, "Type", SmartPointer ? Type + "^" : Type);
	AddParam(ParamList, "Description", Description);
	AddParam(ParamList, "Category", Category);
	AddParam(ParamList, "Getter", Getter);
	AddParam(ParamList, "Setter", Setter);
	AddParam(ParamList, "FieldName", FieldName);
	AddParam(ParamList, "IndexType", IndexType);
	AddParam(ParamList, "Counter", Counter);
	AddParam(ParamList, "ToStringConverter", ToStringConverter);
	AddParam(ParamList, "FromStringConverter", FromStringConverter);
	AddParam(ParamList, "Validator", Validator);
	AddParam(ParamList, "ErrorLogger", ErrorLogger);
	AddParam(ParamList, "NetIndexedGetter", NetIndexedGetter);
	AddParam(ParamList, "NetIndexedSetter", NetIndexedSetter);
	AddParam(ParamList, "NetAddFunction", NetAddFunction);
	AddParam(ParamList, "NetRemoveFunction", NetRemoveFunction);
	AddParam(ParamList, "NetClearFunction", NetClearFunction);
	AddParam(ParamList, "NetCounterFunction", NetCounterFunction);
	AddParam(ParamList, "EditorFile", EditorFile);
	AddParam(ParamList, "EditorType", EditorType);
	AddParam(ParamList, "Editable", Editable);

	return P + std::string("(") + ParamList + std::string(")");
}

inline bool AssignPrm(std::string& _Name, const std::string& ActualName, const std::string& PName, const std::string& Val)
{
	if (PName == ActualName)
	{
		_Name = /*TrimQuotes(*/Val/*)*/;
		return true;
	}

	return false;
}

#define AssignP(_Name) res = res || AssignPrm(_Name, #_Name, ParamName, ParamValue);

bool clProperty::SetParam(const std::string& ParamName, const std::string& ParamValue)
{
	bool res = false;
	AssignP(NetIndexedGetter)
	AssignP(NetIndexedSetter)
	AssignP(NetAddFunction)
	AssignP(NetRemoveFunction)
	AssignP(NetClearFunction)
	AssignP(NetCounterFunction)
	AssignP(Getter)
	AssignP(Setter)
	AssignP(Description)
	AssignP(Category)
	AssignP(ToStringConverter)
	AssignP(FromStringConverter)
	AssignP(Name)
	AssignP(Type)
	AssignP(Validator)
	AssignP(ErrorLogger)
	AssignP(FieldName)
	AssignP(IndexType)
	AssignP(Counter)
	AssignP(Editable)
	AssignP(EditorType)
	AssignP(EditorFile)

	if (ParamName == "Type")
	{
		res = true;
		SmartPointer = (!ParamValue.empty()) && (ParamValue[ParamValue.length() - 1] == '^');
	}

	/// error in syntax ?
	return res;
}

#undef AssignP

// convert std::string to property description, returning error code if something is wrong
std::string clProperty::FromString(const std::string& P)
{
	// 1. extract param list
	size_t Pos1 = P.find_first_of('(');
	size_t Pos2 = P.find_last_of(')');

	if (Pos1 == std::string::npos) { return std::string("Expected ( in property description"); }
	if (Pos2 == std::string::npos) { return std::string("Expected ) in property description"); }

	std::string ParamList = TrimSpaces(P.substr(Pos1 + 1, Pos2 - Pos1 - 1));

	if (ParamList.empty()) { return std::string("Incomplete property description"); }

	std::vector<std::string> Params;
	{
		std::string CurrentToken;
		bool InQuotes = false;

		for (size_t i = 0; i != ParamList.size(); i++)
		{
			if (ParamList[i] == '"')
			{
				InQuotes = !InQuotes;
			}
			else if (ParamList[i] == ',')
			{
				//				std::cout << CurrentToken << std::endl;
				Params.push_back(CurrentToken);
				CurrentToken.clear();
			}
			else
			{
				if (((ParamList[i] == ' ') || (ParamList[i] == '\t')) && !InQuotes) continue;

				CurrentToken += ParamList[i];
			}
		}

		if (!CurrentToken.empty()) Params.push_back(CurrentToken);
	}

	/*
		// 2. split it using ',' as the delimiter
		std::vector<std::string> Params;
		std::string Remainder = ParamList;

		while ( !Remainder.empty() )
		{
			std::string NextParam = TrimSpaces( BeforeChar( Remainder, "," ) );
			Params.push_back( NextParam );

			if ( Remainder.find( ',' ) == std::string::npos ) { break; }

			Remainder = TrimSpaces( AfterChar( Remainder, "," ) );
		}

		// SplitLine(ParamList, Params, ',');
	*/
	// 3. split each parameter to <ParamName, ParamValue>
	for (size_t i = 0; i < Params.size(); i++)
	{
		std::string& PP = Params[i];

		if (PP.find_first_of('=') == std::string::npos) return std::string("Expected = in property description: " + PP);

		std::string _Name = TrimSpaces(BeforeChar(PP, "="));
		std::string _Val = TrimSpaces(AfterChar(PP, "="));

		if (!IsStrAlphanumeric(_Name))
		{
			return std::string("Property param name should be alphanumeric: " + PP);
		}

		if (_Name != "Name" && !IsStrAlphanumeric(_Name))
		{
			return std::string("Property param value should be alphanumeric: " + PP);
		}

		// if ( _Name == "Name" ) { _Val = TrimQuotes( _Val ); }

		if (!SetParam(_Name, _Val))
		{
			return "Error in property syntax, parameter name = " + _Name;
		}
	}

	if (SmartPointer)
	{
		// drop the last ^ from the type name
		Type = TrimSpaces(Type.substr(0, Type.length() - 1));

		if (g_Verbose) cout << "Smartpointer property: " << Name << " " << Type << endl;
	}

	// validity check
	return Validate();
}

std::string clProperty::GetSaveCode() const
{
	if (!IndexType.empty())
	{
		// we do not yet serialize indexed properties;
		return "";
	}

	if (FDatabase->IsWrappedClass(Type))
	{
		// we do not serialize iObject-based classes in an old visitor-based scheme
		return "";
	}

	std::string res = "";

	std::string ActualToStringConverter = GetToStringConverter();

	std::string Accessor = "";

	if (!FieldName.empty())
	{
		Accessor = FieldName;
	}

	if (!Getter.empty())
	{
		Accessor = Getter + "()";
	}

	if (!Accessor.empty())
	{
		res += std::string("WriteParameter(\"") + Name + std::string("\", ");
		res += ActualToStringConverter + std::string("(") + Accessor + std::string(") )");
	}

	return res;
}

std::string clProperty::GetLoadCode() const
{
	if (!IndexType.empty())
	{
		// we do not yet serialize indexed properties;
		return "";
	}

	if (FDatabase->IsWrappedClass(Type))
	{
		// we do not serialize iObject-based classes in an old visitor-based scheme
		return "";
	}

	std::string res = "";

	std::string ActualFromStringConverter = GetFromStringConverter();

	std::string ValueValidatorFunction = "";

	// first, validate the value
	if (!Validator.empty())
	{
		if (Validator == "<default>")
		{
			std::string DefaultValidator = FDatabase->GetDefaultValidator(Type);

			if (DefaultValidator.empty())
			{
				//          cout << "No default value validator for type [" << Type << "]" << endl;
				return ""; // no serialization code !
			}

			ValueValidatorFunction = DefaultValidator;
		}
		else
		{
			ValueValidatorFunction = Validator;
		}
	}

	// finally, assign the value

	if (!FieldName.empty())
	{
		// Field = ToStringConverter [ ParamValue ]
		res += FieldName + std::string(" = ");
		res += ActualFromStringConverter + std::string("(ParamValue)");
	}

	if (!Setter.empty())
	{
		res += Setter + std::string("(") + ActualFromStringConverter + std::string(" (ParamValue) )");
	}

	if (!ValueValidatorFunction.empty())
	{
		res = std::string("bool Valid = ") + ValueValidatorFunction + std::string("(ParamValue);\n\n") +
			std::string("\t\tif ( Valid ) ") + res;

		if (!ErrorLogger.empty())
		{
			res += std::string(" else\n{") + ErrorLogger + std::string("(ParamValue); }");
		}
		else
		{
			// default error message
			res += std::string("; else ERROR_MSG( \"Valid '" + Type + "' expected for property '" + Name + "'\" )");
		}
	}

	return res;
}

/// DEFINE_INDEXER_STUFF
std::string clProperty::GetIndexerStuffDefinition() const
{
	if (IndexType.empty()) { return ""; }

	/*
		std::string res = "DEFINE_INDEXER_STUFF(";

		res += Name + std::string( ", " );

		res += FDatabase->NativeToNet[IndexType] + std::string( ", " );
		res += FDatabase->AddNETReferenceModifierIfNeeded( Type ) + std::string( ")" );*/

		// "ManagedType, NativeType, ";
	std::string res = "DEFINE_INDEXED_PROPERTY";

	if (!IsIndexSupported()) { res += "_NO_LIST"; }

	res += "(";

	res += Type + std::string(", "); // ManagedType ?
	res += Type + std::string(", "); // NativeType ? do it carefully ?

	res += Name + std::string(", ");

	res += FDatabase->NativeToNet[IndexType] + std::string(", ");
	res += FDatabase->AddNETReferenceModifierIfNeeded(Type) + std::string(")");

	return res;
}

/// INIT_INDEXER_STUFF
/**
	To be able to access indexed properties in C# or any other .NET language,
	we declare special arrays of get/set functions (see getter/setter templates in NET_Macros)
*/
std::string clProperty::GetIndexerStuffInitialization() const
{
	if (IndexType.empty()) { return ""; }

	// TODO : if there is no indexed getter and indexed setter, we can use direct access with converters
	// This requires careful handling of POD/Object types and To/From Netconverters

	std::string res = "INIT_INDEXER_STUFF";

	if (!IsIndexSupported()) { res += "_NO_LIST"; }

	res += "(";

	res += Name + std::string(", ");
	res += FClassName + std::string(", ");
	res += NetIndexedGetter + std::string(", ");
	res += NetIndexedSetter + std::string(")");

	return res;
}

std::string clProperty::DeclareNETProperty() const
{
	/*	if ( SmartPointer )
		{
			if ( Verbose ) cout << "Skipping smartpointer property .NET declaration: " << Name << " " << Type << endl;

			// TODO: declare smart pointer property to a wrapped class
			return "";
		}*/

	if (!IndexType.empty()) { return GetIndexerStuffDefinition(); }

	bool AddNativeConverters = false;

	// non-indexed property

	std::string AccessModifier = "";

	if (!Getter.empty()) { AccessModifier += std::string("_GET"); }

	if (!Setter.empty()) { AccessModifier += std::string("_SET"); }

	std::string PropertyDeclarator = "";

	bool Wrapped = false;

	bool AddNativeTypeToDescription = false;

	if (FDatabase->IsScalarType(Type))
	{
		// declare scalar property with direct field access
		PropertyDeclarator = "DECLARE_SCALAR_PROPERTY";

		AddNativeTypeToDescription = true;
	}
	else
	{
		Wrapped = FDatabase->IsWrappedClass(Type);

		// if it is one of the wrapped classes - search in database
		if (Wrapped)
		{
			// it is some class
			PropertyDeclarator = "DECLARE_WRAPPER_PROPERTY";

			if (Getter.empty() && Setter.empty())
			{
				PropertyDeclarator += "_DIRECT"; // direct access to native field
			}
		}
		else
		{
			// it is a POD type like vec3
			PropertyDeclarator = "DECLARE_POD_PROPERTY";

			AddNativeConverters = true;

			AddNativeTypeToDescription = true;
		}
	}

	PropertyDeclarator += AccessModifier;

	std::string PropertyParams = "";

	AddParam_NoValue(PropertyParams, Description == "" ? std::string("\"\"") : AddQuotesIfNone(Description));
	AddParam_NoValue(PropertyParams, Category == "" ? std::string("\"\"") : AddQuotesIfNone(Category));

	if (AddNativeTypeToDescription) { AddParam_NoValue(PropertyParams, Type); }

	// add ^ to the end, if it is a .NET class, not POD
	std::string ModifiedType = FDatabase->AddNETReferenceModifierIfNeeded(Type);

	if (Wrapped)
	{
		// do not use '^' sign on wrapped properties
		ModifiedType = Type;
	}

	AddParam_NoValue(PropertyParams, ModifiedType);
	AddParam_NoValue(PropertyParams, Name);

	// direct field access
	/*if ( Wrapped )*/ if (!FieldName.empty()) { AddParam_NoValue(PropertyParams, FieldName); }

	/// add native field name

	if (!Getter.empty()) { AddParam_NoValue(PropertyParams, Getter); }

	/*if ( !Wrapped )*/ if (!Setter.empty()) { AddParam_NoValue(PropertyParams, Setter); }

	if (AddNativeConverters)
	{
		if (!Getter.empty() || !FieldName.empty())
		{
			std::string NetConverter = FDatabase->ToNetConverter[Type];

			AddParam_NoValue(PropertyParams, NetConverter);
		}

		if (!Setter.empty() || !FieldName.empty())
		{
			std::string NetConverter = FDatabase->FromNetConverter[Type];

			AddParam_NoValue(PropertyParams, NetConverter);
		}
	}

	return PropertyDeclarator + std::string("(") + PropertyParams + std::string(")");
}

/// Return empty string if no implementation is assumed

// #define DECLARE_WRAPPER_PROPERTY_DIRECT_IMPL(TheClassName, ManagedType, ManagedName, NativeName)

std::string clProperty::DeclareNETProperty_Impl() const
{
	if (!IndexType.empty())
	{
		// scalar types are not here yet
		if (!FDatabase->IsScalarType(Type))
		{
			if (!IsIndexSupported())
			{
				return "";
			}

			std::string Result("IMPLEMENT_LIST_PROPERTY_ACCESSOR_");

			if (SmartPointer) { Result += "SMARTPTR"; }
			else { Result += "PTR"; }

			Result += "(";

			Result += Type + ", "; // ManagedType
			Result += Type + ", "; // NativeType
			Result += Name + ", "; // PropName
			Result += NetIndexedGetter + ", ";
			Result += NetIndexedSetter + ", ";
			Result += NetAddFunction + ", ";
			Result += NetRemoveFunction + ", ";
			Result += NetCounterFunction; // add Remover function and ClearFunction

			Result += ")";

			//(clSceneNode, clSceneNode, SubNodes, GetSubNode, SetSubNode, Add, Remove, GetTotalSubNodes)

			//IMPLEMENT_LIST_PROPERTY_ACCESSOR_PTR(iGUIView, iGUIView, ChildViews, GetSubView, SetSubView, AddView, RemoveView, GetNumSubViews)

			return Result;
		}

		return "";
	}

	std::string AccessModifier = "";

	if (!Getter.empty()) { AccessModifier += std::string("_GET"); }

	if (!Setter.empty()) { AccessModifier += std::string("_SET"); }

	if (!FDatabase->IsScalarType(Type))
	{
		// if it is one of the wrapped classes - search in database
		if (FDatabase->IsWrappedClass(Type))
		{
			// it is some class
			if (Getter.empty() && Setter.empty())
			{
				// direct access to native field
				std::string Result("DECLARE_WRAPPER_PROPERTY_DIRECT_IMPL");

				if (SmartPointer) { Result += "_SMARTPTR"; }

				Result += "(";

				Result += FClassName + std::string(", ");
				Result += Type + std::string(", ");
				Result += Name + std::string(", ");
				Result += FieldName;

				Result += std::string(")");

				return Result;
			}
			else
				if (!AccessModifier.empty())
				{
					std::string Result("DECLARE_WRAPPER_PROPERTY");
					Result += AccessModifier + std::string("_IMPL");

					if (SmartPointer) { Result += "_SMARTPTR"; }

					Result += std::string("(");

					Result += FClassName + std::string(", ");
					Result += Type + std::string(", ");
					Result += Name + std::string(", ");

					bool NeedComma = false;

					if (!Getter.empty()) { Result += Getter; NeedComma = true; }
					if (!Setter.empty())
					{
						if (NeedComma) { Result += std::string(", "); }
						Result += Setter;
					}

					Result += std::string(")");

					return Result;
				}
		}
	}

	return "";
}

std::string clProperty::GetScriptDeclaration() const
{
	std::string res = std::string("// property ");

	res += Name;

	if (FieldName.empty())
	{
		if (!Getter.empty())
		{
			res += std::string(" get(") + Getter + std::string(")");
		}

		if (!Setter.empty())
		{
			res += std::string(" set(") + Setter + std::string(")");
		}
	}
	else
	{
		res += std::string(" get() set()");
	}

	res += std::string(";");

	res += std::string(" // ");

	if (!Validator.empty())
	{
		res += std::string("Validator = ") + Validator;
	}
	else
	{
		res += std::string("No validator");
	}

	return res;
}

bool clProperty::IsEditable() const
{
	return (Editable == "True") || (Editable == "true") || (Editable == "TRUE");
}

/////// New serialization scheme
bool clProperty::Saveable() const
{
	if (!IndexType.empty())
	{
		if (IndexType == "int") { return true; }
		if (IndexType == "size_t") { return true; }
	}

	if (!FieldName.empty()) { return true; }

	if (!Setter.empty()) { return true; }

	return false;
}

bool clProperty::Loadable() const
{
	if (!IndexType.empty())
	{
		if (IndexType == "int") { return true; }
		if (IndexType == "size_t") { return true; }
	}

	if (!FieldName.empty()) { return true; }

	if (!Getter.empty()) { return true; }

	return false;
}

std::string clProperty::GetFromStringConverter() const
{
	std::string Conv = FromStringConverter;

	if (FromStringConverter.empty())
	{
		Conv = FDatabase->GetAppropriateFromStringConverter(Type);
	}

	return Conv;
}

std::string clProperty::GetToStringConverter() const
{
	std::string Conv = ToStringConverter;

	if (ToStringConverter.empty())
	{
		Conv = FDatabase->GetAppropriateToStringConverter(Type);
	}

	return Conv;
}

std::string CompressSerializedName(const std::string& InName)
{
	std::string Res = InName;

	for (size_t i = 0; i < Res.length(); i++)
	{
		if (InName[i] == '.' ||
			InName[i] == '[' ||
			InName[i] == ']' ||
			InName[i] == '(' ||
			InName[i] == ')' ||
			InName[i] == ' ' ||
			InName[i] == '\t')
		{
			Res[i] = '_';
		}
		else
		{
			Res[i] = InName[i];
		}
	}

	return Res;
}

/// Generate some code for property serialization
/// If Accessor is true, then Load/Save is replaced by Get/Set
std::string clProperty::GetBinderMacro(bool IsArray, bool IsScalar, bool Accessor, bool Load, const std::string& Conv) const
{
	// add ',' at the end if required
#define FIX_TRAILING_COMMA() \
	if ( res[res.length() - 2] != ',') res += std::string(", ");

	std::string res = "";

	// <MacroName> ( PropertyName, FieldName, ClassName, AccessName or Getter/Setter, Type for arrays, ToStringConverter )

	res = std::string((IsScalar ? "SCALAR" : "OBJECT"));
	res += std::string((IsArray ? "_ARRAY" : ""));

	res += std::string("_PROPERTY_");

	if (Accessor)
	{
		res += std::string(Load ? "SET" : "GET");
	}
	else
	{
		res += std::string(Load ? "LOAD" : "SAVE");
	}

	res += std::string("__");

	bool EmptyFlag = Load ? Setter.empty() : Getter.empty();
	bool NeedType = false;

	if (!EmptyFlag)
	{
		res += std::string(Load ? "SETTER" : "GETTER");
	}
	else
	{
		// compressed name is used for the loader function name
		res += std::string("FIELD");
	}

	if (SmartPointer && ((Accessor && !Load && !IsArray) || (!Accessor && Load && IsArray)) && !IsScalar)
	{
		/// Accessor/Getter must decypher smart pointers
		res += std::string("_SMARTPTR");
	}
	else
		if (SmartPointer && !Accessor && Load && EmptyFlag)
		{
			/// Field loader must decypher smart pointers
			res += std::string("_SMARTPTR");
			NeedType = true;
		}

	res += std::string("(");

	std::string MacroName = res;

	res += Name + std::string(", ");

	res += (FieldName.empty() ? std::string("\"\"") : FieldName) + std::string(", ");

	res += FClassName;

	if (!EmptyFlag)
	{
		FIX_TRAILING_COMMA()
			res += (Load ? Setter : Getter);
	}
	else
	{
		// arrays do not yet support indirect referencing !
		if (!IsArray)
		{
			FIX_TRAILING_COMMA();
			res += CompressSerializedName(FieldName);
		}
	}

	if (IsArray)
	{
		if (!IsScalar)
		{
			if (Load)
			{
				FIX_TRAILING_COMMA()
					res += Type;
			}
		}
	}

	if (IsScalar && !Accessor)
	{
		std::string Conv = Load ? GetFromStringConverter() : GetToStringConverter();

		if (Conv.empty()) { Conv = "EMPTY_CONVERTER"; }

		FIX_TRAILING_COMMA()
			res += Conv;
	}

	if ((!IsArray && ((!IsScalar && !EmptyFlag && Load) || Accessor)) || NeedType)
	{
		// HACK with strings
		if (Type == "std::string")
		{
			res += ", LString";
		}
		else
		{
			res += ", " + Type;
		}
	}

	if (IsScalar && Accessor)
	{
		// accessor needs both ToString/FromString methods
		res += ", " + GetToStringConverter() + ", " + GetFromStringConverter();
	}

	res += std::string(")\n");

	return res; //FDatabase->ExpandMacro(res);
}

std::string clProperty::GetLoadSaveDeclarations() const
{
	bool isArray = (IndexType == "int") || (IndexType == "size_t");

	if (!IndexType.empty()) { if (!isArray) { return ""; } } // only 'int' is supported

	bool isScalar = !FDatabase->IsWrappedClass(Type); //!IsClassType(Type);

	std::string res;

	// Get/Set property has a different binding code implementation
	// loader/saver for a scalar/pod/object field

	// Text loader/saver
	// Loader
	res += FDatabase->ExpandMacro(GetBinderMacro(isArray, isScalar, false, true, GetFromStringConverter()));
	// Saver
	res += FDatabase->ExpandMacro(GetBinderMacro(isArray, isScalar, false, false, GetToStringConverter()));

	// Binary loader/saver
	// Loader
//	res += FDatabase->ExpandMacro( std::string("BIN_") + GetBinderMacro( isArray, isScalar, false,   true, GetFromStringConverter() ) );
	// Saver
//	res += FDatabase->ExpandMacro( std::string("BIN_") + GetBinderMacro( isArray, isScalar, false,  false, GetToStringConverter() ) );

	if (!isArray)
	{
		// TODO: support scalars and strings
		if (!isScalar)
		{
			// Get-accessor
			res += FDatabase->ExpandMacro(GetBinderMacro(isArray, isScalar, true, true, GetToStringConverter()));
			// Set-accessor
			res += FDatabase->ExpandMacro(GetBinderMacro(isArray, isScalar, true, false, GetToStringConverter()));
		}
		else
		{
			// Get-accessor
			res += FDatabase->ExpandMacro(GetBinderMacro(isArray, isScalar, true, true, GetToStringConverter()));
			// Set-accessor
			res += FDatabase->ExpandMacro(GetBinderMacro(isArray, isScalar, true, false, GetToStringConverter()));
		}
	}
	else
	{
		//    Items, SerializableClass)
		res += "\n";

		std::string res1 = std::string("ARRAY_PROPERTY_FUNCTIONS__FIELD(");
		res1 += FieldName;
		res1 += std::string(", ") + FClassName;
		res1 += std::string(")");

		res += FDatabase->ExpandMacro(res1);

		if (!isScalar)
		{
			res += std::string("\n");

			std::string res2 = std::string("ARRAY_PROPERTY_DELETE_FUNCTION__FIELD");
			if (SmartPointer) { res2 += std::string("_SMARTPTR"); }
			res2 += std::string("(");
			res2 += FieldName;
			res2 += std::string(", ") + FClassName;
			res2 += std::string(")");

			res += FDatabase->ExpandMacro(res2);

			res += std::string("\n");

			/// array getter
			res2 = std::string("ARRAY_PROPERTY_GETOBJECT_FUNCTION__FIELD");
			if (SmartPointer) { res2 += std::string("_SMARTPTR"); }
			res2 += std::string("(");
			res2 += FieldName;
			res2 += std::string(", ") + FClassName;
			res2 += std::string(")");

			res += FDatabase->ExpandMacro(res2);

			res += std::string("\n");

			/// array setter
			res2 = std::string("ARRAY_PROPERTY_SETOBJECT_FUNCTION__FIELD");
			if (SmartPointer) { res2 += std::string("_SMARTPTR"); }
			res2 += std::string("(");
			res2 += FieldName;
			res2 += std::string(", ") + FClassName;
			res2 += std::string(", ") + Type;
			res2 += std::string(")");

			res += FDatabase->ExpandMacro(res2);

			res += std::string("\n");

			res2 = std::string("ARRAY_PROPERTY_INSERTOBJECT_FUNCTION__FIELD");
			if (SmartPointer) { res2 += std::string("_SMARTPTR"); }
			res2 += std::string("(");
			res2 += FieldName;
			res2 += std::string(", ") + FClassName;
			res2 += std::string(", ") + Type;
			res2 += std::string(")");

			res += FDatabase->ExpandMacro(res2);

			res += std::string("\n");

			res2 = std::string("ARRAY_PROPERTY_REMOVEOBJECT_FUNCTION__FIELD");
			if (SmartPointer) { res2 += std::string("_SMARTPTR"); }
			res2 += std::string("(");
			res2 += FieldName;
			res2 += std::string(", ") + FClassName;
			res2 += std::string(")");

			res += FDatabase->ExpandMacro(res2);
		}
		else
		{
			res += std::string("\n");
			// POD values

			/// array getter
			std::string res2("ARRAY_PROPERTY_GETITEM_FUNCTION__FIELD(");
			res2 += FieldName;
			res2 += std::string(", ") + FClassName;
			res2 += std::string(", ") + GetToStringConverter();
			res2 += std::string(")");

			res += FDatabase->ExpandMacro(res2);

			res += std::string("\n");

			/// array setter
			res2 = std::string("ARRAY_PROPERTY_SETITEM_FUNCTION__FIELD(");
			res2 += FieldName;
			res2 += std::string(", ") + FClassName;
			res2 += std::string(", ") + GetFromStringConverter();
			res2 += std::string(")");

			res += FDatabase->ExpandMacro(res2);

			res += std::string("\n");

			res2 = std::string("ARRAY_PROPERTY_INSERTITEM_FUNCTION__FIELD(");
			res2 += FieldName;
			res2 += std::string(", ") + FClassName;
			res2 += std::string(", ") + GetFromStringConverter();
			res2 += std::string(")");

			res += FDatabase->ExpandMacro(res2);

			res += std::string("\n");

			res2 = std::string("ARRAY_PROPERTY_REMOVEITEM_FUNCTION__FIELD(");
			res2 += FieldName;
			res2 += std::string(", ") + FClassName;
			res2 += std::string(")");

			res += FDatabase->ExpandMacro(res2);
		}

		res += std::string("\n");
	}

	// TODO : handle non-resizable array and Getter/Setter arrays
	// if (isArray && Setter.)

	return res;
}

std::string clProperty::GetRegistrationCode(std::string& FullPropertyName) const
{
	bool isArray = (IndexType == "int") || (IndexType == "size_t");

	if (!IndexType.empty()) { if (!isArray) { return ""; } } // only 'int' is supported

// bool isScalar = !FDatabase->IsSerializableClass(Type);
	bool isScalar = !FDatabase->IsWrappedClass(Type);

	std::string res = std::string("REGISTER_PROPERTY__");

	std::string add_res = std::string("REGISTER_PROPERTY_ACCESSORS__");

	std::string ObjPrefix = (isScalar ? "SCALAR_" : "OBJECT_");

	// std::cout << FieldName << " Getter: '" << Getter << "' Setter: '" << Setter << "'" << std::endl;

	if (isArray)
	{
		FullPropertyName = FieldName;
		// todo : support getter/setter
		res += ObjPrefix + std::string("ARRAY_FIELD(") + FullPropertyName;
	}
	else
	{
		if (Setter.empty() && Getter.empty())
		{
			std::string CommonPart = ObjPrefix + std::string("FIELD");

			//         if(SmartPointer) { CommonPart += std::string("_SMARTPTR"); }

			FullPropertyName = CompressSerializedName(FieldName);
			CommonPart += std::string("(") + FullPropertyName;

			res += CommonPart;

			add_res += CommonPart;
		}
		else
		{
			FullPropertyName = Name;

			std::string CommonPart = ObjPrefix + std::string("GETTER_SETTER(") + FullPropertyName;

			res += CommonPart;
			//+ Getter + std::string(", ") + Setter;

			add_res += CommonPart;
		}
	}

	FullPropertyName = "Prop_" + FullPropertyName;

	std::string RegCommon = std::string(", ") + FClassName + ", " + Name;

	res += RegCommon + ", " + Type + ")";
	add_res += RegCommon + ")";

	if (isArray)
	{
		if (!Counter.empty())
		{
			// ....
		}
	}
	else
	{
		// Now we add the code for field contents access (i.e., iObject* GetValue(iObject* Obj) will get the Obj->FFieldForProperty)
		if (isScalar)
		{
			// strings are handled differently and PODs require more params to the macro
		}
		else
		{
			// iObject props are accessed easily

			res += std::string("\n");

			/// PROPERTY_REGISTER_ACCESSORS__*
			res += add_res;

			res += std::string("\n");
		}
	}

	return res;
}

std::string clProperty::Validate()
{
	if (Type.empty()) { return "Property type is not specified. Check definition syntax"; }

	if (Name.empty()) { return "Property name is not specified. Check definition syntax"; }

	if (FieldName.empty())
	{
		if (Getter.empty() && Setter.empty())
		{
			// it is not an array property ?
			return "No field, getter or setter";
		}
	}

	return "";
}
