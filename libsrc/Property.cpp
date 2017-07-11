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

/**

   Property syntax:

      Property(FieldName = "", )

      IndexedProperty
*/

inline void AddParam_NoValue( string& ParamList, const string& ParamName )
{
   if ( !ParamList.empty() ) { ParamList += string( ", " ); }

   ParamList += ParamName;
}

inline void AddParam( string& ParamList, const string& ParamName, const string& ParamValue )
{
   if ( ParamValue.empty() ) { return; }

   AddParam_NoValue( ParamList, ParamName );

   ParamList += string ( " = " ) + ParamValue;
}

string clProperty::ToString() const
{
   string P = "Property";

   string ParamList = "";

   AddParam( ParamList, "Name",        TrimQuotes( Name ) );
   AddParam( ParamList, "Type",        SmartPointer ? Type + "^" : Type );
   AddParam( ParamList, "Description", Description );
   AddParam( ParamList, "Category",    Category );
   AddParam( ParamList, "Getter",      Getter );
   AddParam( ParamList, "Setter",      Setter );
   AddParam( ParamList, "FieldName",   FieldName );
   AddParam( ParamList, "IndexType",   IndexType );
   AddParam( ParamList, "Counter",     Counter );
   AddParam( ParamList, "ToStringConverter",   ToStringConverter );
   AddParam( ParamList, "FromStringConverter", FromStringConverter );
   AddParam( ParamList, "Validator",           Validator );
   AddParam( ParamList, "ErrorLogger",         ErrorLogger );
   AddParam( ParamList, "NetIndexedGetter",    NetIndexedGetter );
   AddParam( ParamList, "NetIndexedSetter",    NetIndexedSetter );
   AddParam( ParamList, "NetAddFunction",      NetAddFunction );
   AddParam( ParamList, "NetRemoveFunction",   NetRemoveFunction );
   AddParam( ParamList, "NetClearFunction",    NetClearFunction );
   AddParam( ParamList, "NetCounterFunction",  NetCounterFunction );
   AddParam( ParamList, "EditorType",  EditorType );
   AddParam( ParamList, "Editable",    Editable);

   return P + string( "(" ) + ParamList + string( ")" );
}

inline bool AssignPrm( string& _Name, const string& ActualName, const string& PName, const string& Val )
{
   if ( PName == ActualName )
   {
      _Name = /*TrimQuotes(*/Val/*)*/;
      return true;
   }

   return false;
}

#define AssignP(_Name) res = res || AssignPrm(_Name, #_Name, ParamName, ParamValue);

bool clProperty::SetParam( const string& ParamName, const string& ParamValue )
{
	bool res = false;
   AssignP( NetIndexedGetter )
   AssignP( NetIndexedSetter )
   AssignP( NetAddFunction )
   AssignP( NetRemoveFunction )
   AssignP( NetClearFunction )
   AssignP( NetCounterFunction )
   AssignP( Getter )
   AssignP( Setter )
   AssignP( Description )
   AssignP( Category )
   AssignP( ToStringConverter )
   AssignP( FromStringConverter )
   AssignP( Name )
   AssignP( Type )
   AssignP( Validator )
   AssignP( ErrorLogger )
   AssignP( FieldName )
   AssignP( IndexType )
   AssignP( Counter )
   AssignP( Editable )
   AssignP( EditorType )

	if ( ParamName == "Type" )
	{
		res = true;
		SmartPointer = ( !ParamValue.empty() ) && ( ParamValue[ ParamValue.length() - 1 ] == '^' );
	}

	/// error in syntax ?
	return res;
}

#undef AssignP

// convert string to property description, returning error code if something is wrong
string clProperty::FromString( const string& P )
{
   // 1. extract param list
   size_t Pos1 = P.find_first_of( '(' );
   size_t Pos2 = P.find_last_of( ')' );

   if ( Pos1 == std::string::npos ) { return string( "Expected ( in property description" ); }
   if ( Pos2 == std::string::npos ) { return string( "Expected ) in property description" ); }

   string ParamList = TrimSpaces( P.substr( Pos1 + 1, Pos2 - Pos1 - 1 ) );

   if ( ParamList.empty() ) { return string( "Incomplete property description" ); }

	vector<string> Params;
	{
		string CurrentToken;
		bool InQuotes = false;

		for ( size_t i = 0; i != ParamList.size(); i++ )
		{
			if ( ParamList[i] =='"' )
			{
				InQuotes = !InQuotes;
			}
			else if ( ParamList[i] == ',' )
			{
//				std::cout << CurrentToken << std::endl;
				Params.push_back( CurrentToken );
				CurrentToken.clear();
			}
			else
			{
				if ( ( ( ParamList[i] == ' ' ) || ( ParamList[i] == '\t' ) ) && !InQuotes ) continue;

				CurrentToken += ParamList[i];
			}
		}

		if ( !CurrentToken.empty() ) Params.push_back( CurrentToken );
	}

/*
   // 2. split it using ',' as the delimiter
   vector<string> Params;
   string Remainder = ParamList;

   while ( !Remainder.empty() )
   {
      string NextParam = TrimSpaces( BeforeChar( Remainder, "," ) );
      Params.push_back( NextParam );

      if ( Remainder.find( ',' ) == std::string::npos ) { break; }

      Remainder = TrimSpaces( AfterChar( Remainder, "," ) );
   }

// SplitLine(ParamList, Params, ',');
*/
   // 3. split each parameter to <ParamName, ParamValue>
   for ( size_t i = 0 ; i < Params.size() ; i++ )
   {
      string& PP = Params[i];

	   if ( PP.find_first_of( '=' ) == std::string::npos ) return string( "Expected = in property description: " + PP );

      string _Name = TrimSpaces( BeforeChar( PP, "=" ) );
      string _Val  = TrimSpaces( AfterChar( PP, "=" ) );

		if ( !IsStrAlphanumeric( _Name ) )
		{
			return string( "Property param name should be alphanumeric: " + PP );
		}

		if ( _Name != "Name" && !IsStrAlphanumeric( _Name ) )
		{
			return string( "Property param value should be alphanumeric: " + PP );
		}

//      if ( _Name == "Name" ) { _Val = TrimQuotes( _Val ); }

      if(!SetParam( _Name, _Val ))
	{
		return "Error in property syntax, parameter name = " + _Name;
	}
   }

	if ( SmartPointer )
	{
		// drop the last ^ from the type name
		Type = TrimSpaces( Type.substr( 0, Type.length() - 1 ) );

		if ( g_Verbose ) cout << "Smartpointer property: " << Name << " " << Type << endl;
	}

   // validity check
   return Validate();
}

string clProperty::GetSaveCode() const
{
   if ( !IndexType.empty() )
   {
      // we do not yet serialize indexed properties;
      return "";
   }

   if ( FDatabase->IsWrappedClass( Type ) )
   {
      // we do not serialize iObject-based classes in an old visitor-based scheme
      return "";
   }

   string res = "";

   string ActualToStringConverter = GetToStringConverter();

   string Accessor = "";

   if ( !FieldName.empty() )
   {
      Accessor = FieldName;
   }

   if ( !Getter.empty() )
   {
      Accessor = Getter + "()";
   }

   if ( !Accessor.empty() )
   {
      res += string( "WriteParameter(\"" ) + Name + string( "\", " );
      res += ActualToStringConverter + string( "(" ) + Accessor + string( ") )" );
   }

   return res;
}

string clProperty::GetLoadCode() const
{
   if ( !IndexType.empty() )
   {
      // we do not yet serialize indexed properties;
      return "";
   }

   if ( FDatabase->IsWrappedClass( Type ) )
   {
      // we do not serialize iObject-based classes in an old visitor-based scheme
      return "";
   }

   string res = "";

   string ActualFromStringConverter = GetFromStringConverter();

   string ValueValidatorFunction = "";

   // first, validate the value
   if ( !Validator.empty() )
   {
      if ( Validator == "<default>" )
      {
         string DefaultValidator = FDatabase->GetDefaultValidator( Type );

         if ( DefaultValidator.empty() )
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

   if ( !FieldName.empty() )
   {
      // Field = ToStringConverter [ ParamValue ]
      res += FieldName + string( " = " );
      res += ActualFromStringConverter + string( "(ParamValue)" );
   }

   if ( !Setter.empty() )
   {
      res += Setter + string( "(" ) + ActualFromStringConverter + string( " (ParamValue) )" );
   }

   if ( !ValueValidatorFunction.empty() )
   {
      res = string( "bool Valid = " ) + ValueValidatorFunction + string( "(ParamValue);\n\n" ) +
            string( "\t\tif ( Valid ) " ) + res;

      if ( !ErrorLogger.empty() )
      {
         res += string( " else\n{" ) + ErrorLogger + string( "(ParamValue); }" );
      }
      else
      {
         // default error message
         res += string( "; else ERROR_MSG( \"Valid '" + Type + "' expected for property '" + Name + "'\" )" );
      }
   }

   return res;
}

/// DEFINE_INDEXER_STUFF
string clProperty::GetIndexerStuffDefinition() const
{
	if ( IndexType.empty() ) { return ""; }

/*
	string res = "DEFINE_INDEXER_STUFF(";

	res += Name + string( ", " );

	res += FDatabase->NativeToNet[IndexType] + string( ", " );
	res += FDatabase->AddNETReferenceModifierIfNeeded( Type ) + string( ")" );*/

	// "ManagedType, NativeType, ";
	string res = "DEFINE_INDEXED_PROPERTY";

	if( !IsIndexSupported()) { res += "_NO_LIST"; }

	res += "(";

	res += Type + string(", "); // ManagedType ?
	res += Type + string(", "); // NativeType ? do it carefully ?

	res += Name + string(", ");

	res += FDatabase->NativeToNet[IndexType] + string( ", " );
	res += FDatabase->AddNETReferenceModifierIfNeeded( Type ) + string( ")" );

	return res;
}

/// INIT_INDEXER_STUFF
/**
   To be able to access indexed properties in C# or any other .NET language,
   we declare special arrays of get/set functions (see getter/setter templates in NET_Macros)
*/
string clProperty::GetIndexerStuffInitialization() const
{
	if ( IndexType.empty() ) { return ""; }

	// TODO : if there is no indexed getter and indexed setter, we can use direct access with converters
	// This requires careful handling of POD/Object types and To/From Netconverters

	string res = "INIT_INDEXER_STUFF";

	if( !IsIndexSupported() ) { res += "_NO_LIST"; }

	res += "(";

	res += Name + string( ", " );
	res += FClassName + string( ", " );
	res += NetIndexedGetter + string( ", " );
	res += NetIndexedSetter + string( ")" );

	return res;
}

string clProperty::DeclareNETProperty() const
{
/*	if ( SmartPointer )
   {
		if ( Verbose ) cout << "Skipping smartpointer property .NET declaration: " << Name << " " << Type << endl;

      // TODO: declare smart pointer property to a wrapped class
		return "";
   }*/

   if ( !IndexType.empty() ) { return GetIndexerStuffDefinition(); }

   bool AddNativeConverters = false;

   // non-indexed property

   string AccessModifier = "";

   if ( !Getter.empty() ) { AccessModifier += string( "_GET" ); }

   if ( !Setter.empty() ) { AccessModifier += string( "_SET" ); }

   string PropertyDeclarator = "";

   bool Wrapped = false;

   bool AddNativeTypeToDescription = false;

   if ( FDatabase->IsScalarType( Type ) )
   {
      // declare scalar property with direct field access
      PropertyDeclarator = "DECLARE_SCALAR_PROPERTY";

      AddNativeTypeToDescription = true;
   }
   else
   {
      Wrapped = FDatabase->IsWrappedClass( Type );

      // if it is one of the wrapped classes - search in database
      if ( Wrapped )
      {
         // it is some class
         PropertyDeclarator = "DECLARE_WRAPPER_PROPERTY";

		 if ( Getter.empty() && Setter.empty() )
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

   string PropertyParams = "";

   AddParam_NoValue( PropertyParams, Description == "" ? string( "\"\"" ) : AddQuotesIfNone( Description ) );
   AddParam_NoValue( PropertyParams, Category == "" ? string( "\"\"" ) : AddQuotesIfNone( Category ) );

   if(AddNativeTypeToDescription) { AddParam_NoValue( PropertyParams, Type); }

   // add ^ to the end, if it is a .NET class, not POD
   string ModifiedType = FDatabase->AddNETReferenceModifierIfNeeded( Type );

   if ( Wrapped )
   {
      // do not use '^' sign on wrapped properties
      ModifiedType = Type;
   }

   AddParam_NoValue( PropertyParams, ModifiedType );
   AddParam_NoValue( PropertyParams, Name );

   // direct field access
   /*if ( Wrapped )*/ if ( !FieldName.empty() ) { AddParam_NoValue( PropertyParams, FieldName ); }

   /// add native field name

   if ( !Getter.empty() ) { AddParam_NoValue( PropertyParams, Getter ); }

   /*if ( !Wrapped )*/ if ( !Setter.empty() ) { AddParam_NoValue( PropertyParams, Setter ); }

   if ( AddNativeConverters )
   {
      if ( !Getter.empty() || !FieldName.empty() )
      {
         string NetConverter = FDatabase->ToNetConverter[Type];

         AddParam_NoValue( PropertyParams, NetConverter );
      }

	  if ( !Setter.empty() || !FieldName.empty() )
      {
         string NetConverter = FDatabase->FromNetConverter[Type];

         AddParam_NoValue( PropertyParams, NetConverter );
      }
   }

   return PropertyDeclarator + string( "(" ) + PropertyParams + string( ")" );
}

/// Return empty string if no implementation is assumed

// #define DECLARE_WRAPPER_PROPERTY_DIRECT_IMPL(TheClassName, ManagedType, ManagedName, NativeName)

string clProperty::DeclareNETProperty_Impl() const
{
	if ( !IndexType.empty() )
	{
		// scalar types are not here yet
		if ( !FDatabase->IsScalarType( Type ) )
		{
			if( !IsIndexSupported() )
			{
				return "";
			}

			string Result("IMPLEMENT_LIST_PROPERTY_ACCESSOR_");

			if(SmartPointer) { Result += "SMARTPTR"; } else { Result += "PTR"; }

			Result += "(";

			Result += Type + ", "; // ManagedType
			Result += Type + ", "; // NativeType
			Result += Name + ", "; // PropName
			Result += NetIndexedGetter  + ", ";
			Result += NetIndexedSetter  + ", ";
			Result += NetAddFunction    + ", ";
			Result += NetRemoveFunction + ", ";
			Result += NetCounterFunction; // add Remover function and ClearFunction

			Result += ")";

			//(clSceneNode, clSceneNode, SubNodes, GetSubNode, SetSubNode, Add, Remove, GetTotalSubNodes)

			//IMPLEMENT_LIST_PROPERTY_ACCESSOR_PTR(iGUIView, iGUIView, ChildViews, GetSubView, SetSubView, AddView, RemoveView, GetNumSubViews)

	        	return Result;
		}

		return "";
	}

   string AccessModifier = "";

   if ( !Getter.empty() ) { AccessModifier += string( "_GET" ); }

   if ( !Setter.empty() ) { AccessModifier += string( "_SET" ); }

   if ( !FDatabase->IsScalarType( Type ) )
   {
      // if it is one of the wrapped classes - search in database
      if ( FDatabase->IsWrappedClass( Type ) )
      {
         // it is some class
         if ( Getter.empty() && Setter.empty() )
         {
            // direct access to native field
            string Result("DECLARE_WRAPPER_PROPERTY_DIRECT_IMPL");

            if(SmartPointer) { Result += "_SMARTPTR"; }

            Result += "(";

            Result += FClassName + string(", ");
            Result += Type + string(", ");
            Result += Name + string(", ");
            Result += FieldName;

            Result += string(")");

            return Result;
         } else
         if(!AccessModifier.empty())
         {
            string Result("DECLARE_WRAPPER_PROPERTY");
            Result += AccessModifier + string("_IMPL");

            if(SmartPointer) { Result += "_SMARTPTR"; }

            Result += string("(");

            Result += FClassName + string(", ");
            Result += Type + string(", ");
            Result += Name + string(", ");

            bool NeedComma = false;

            if(!Getter.empty()) { Result += Getter; NeedComma = true; }
            if(!Setter.empty())
            {
               if(NeedComma) { Result += string(", "); }
               Result += Setter;
            }

            Result += string(")");

            return Result;
         }
      }
   }

   return "";
}

string clProperty::GetScriptDeclaration() const
{
	string res = string( "// property " );

	res += Name;

	if ( FieldName.empty() )
	{
		if ( !Getter.empty() )
		{
			res += string( " get(" ) + Getter + string( ")" );
		}

		if ( !Setter.empty() )
		{
			res += string( " set(" ) + Setter + string( ")" );
		}
	}
	else
	{
		res += string( " get() set()" );
	}

	res += string( ";" );

	res += string( " // " );

	if ( !Validator.empty() )
	{
		res += string( "Validator = " ) + Validator;
	}
	else
	{
		res += string( "No validator" );
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
	if ( !IndexType.empty() )
	{
		if ( IndexType == "int" ) { return true; }
		if ( IndexType == "size_t" ) { return true; }
	}

	if ( !FieldName.empty() ) { return true; }

	if ( !Setter.empty() ) { return true; }

	return false;
}

bool clProperty::Loadable() const
{
	if ( !IndexType.empty() )
	{
		if ( IndexType == "int" ) { return true; }
		if ( IndexType == "size_t" ) { return true; }
	}

	if ( !FieldName.empty() ) { return true; }

	if ( !Getter.empty() ) { return true; }

	return false;
}

string clProperty::GetFromStringConverter() const
{
	string Conv = FromStringConverter;

	if ( FromStringConverter.empty() )
	{
		Conv = FDatabase->GetAppropriateFromStringConverter( Type );
	}

	return Conv;
}

string clProperty::GetToStringConverter() const
{
	string Conv = ToStringConverter;

	if ( ToStringConverter.empty() )
	{
		Conv = FDatabase->GetAppropriateToStringConverter( Type );
	}

	return Conv;
}

string CompressSerializedName( const string& InName )
{
   string Res = InName;

   for ( size_t i = 0 ; i < Res.length() ; i++ )
   {
      if ( InName[i] == '.' ||
           InName[i] == '[' ||
           InName[i] == ']' ||
           InName[i] == '(' ||
           InName[i] == ')' ||
           InName[i] == ' ' ||
           InName[i] == '\t' )
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
string clProperty::GetBinderMacro( bool IsArray, bool IsScalar, bool Accessor, bool Load, const string& Conv ) const
{
// add ',' at the end if required
#define FIX_TRAILING_COMMA() \
	if ( res[res.length() - 2] != ',') res += string(", ");

	string res = "";

	// <MacroName> ( PropertyName, FieldName, ClassName, AccessName or Getter/Setter, Type for arrays, ToStringConverter )

	res  = string( ( IsScalar ? "SCALAR" : "OBJECT" ) );
	res += string( ( IsArray ? "_ARRAY" : "" ) );

	res += string( "_PROPERTY_" );

	if(Accessor)
	{
		res += string( Load ? "SET" : "GET" );
	} else
	{
		res += string( Load ? "LOAD" : "SAVE" );
	}

	res += string( "__" );

	bool EmptyFlag = Load ? Setter.empty() : Getter.empty();
	bool NeedType = false;

	if ( !EmptyFlag )
	{
		res += string( Load ? "SETTER" : "GETTER" );
	}
	else
	{
		// compressed name is used for the loader function name
		res += string( "FIELD" );
	}

	if(SmartPointer && ( (Accessor && !Load && !IsArray) || (!Accessor && Load && IsArray) ) && !IsScalar)
	{
		/// Accessor/Getter must decypher smart pointers
		res += string( "_SMARTPTR");
	}
	else
	if(SmartPointer && !Accessor && Load && EmptyFlag)
	{
		/// Field loader must decypher smart pointers
		res += string( "_SMARTPTR");
		NeedType = true;
	}

	res += string("(");

	string MacroName = res;

	res += Name + string(", ");

	res += (FieldName.empty() ? string("\"\"") : FieldName) + string( ", " );

	res += FClassName;

	if ( !EmptyFlag)
	{
		FIX_TRAILING_COMMA()
		res += ( Load ? Setter : Getter );
	}
	else
	{
		// arrays do not yet support indirect referencing !
		if ( !IsArray )
		{
			FIX_TRAILING_COMMA();
			res += CompressSerializedName( FieldName );
		}
	}

	if ( IsArray )
	{
		if ( !IsScalar )
		{
			if ( Load )
			{
				FIX_TRAILING_COMMA()
				res += Type;
			}
		}
	}

	if ( IsScalar && !Accessor )
	{
		string Conv = Load ? GetFromStringConverter() : GetToStringConverter();

		if ( Conv.empty() ) { Conv = "EMPTY_CONVERTER"; }

		FIX_TRAILING_COMMA()
		res += Conv;
	}

	if ( ( !IsArray && ((!IsScalar && !EmptyFlag && Load) || Accessor) ) || NeedType )
	{
		// HACK with strings
		if(Type == "string")
		{
			res += ", LString";
		}
		else
		{
			res += ", " + Type;
		}
	}

	if( IsScalar && Accessor )
	{
		// accessor needs both ToString/FromString methods
		res += ", " + GetToStringConverter() + ", " + GetFromStringConverter();
	}

	res += string( ")\n" );

	return res; //FDatabase->ExpandMacro(res);
}

string clProperty::GetLoadSaveDeclarations() const
{
	bool isArray = ( IndexType == "int" ) || ( IndexType == "size_t" );

	if ( !IndexType.empty() ) { if ( !isArray ) { return ""; } } // only 'int' is supported

	bool isScalar = !FDatabase->IsWrappedClass( Type ); //!IsClassType(Type);

	string res;

	// Get/Set property has a different binding code implementation
	// loader/saver for a scalar/pod/object field

	// Text loader/saver
	// Loader
	res += FDatabase->ExpandMacro( GetBinderMacro( isArray, isScalar, false,   true, GetFromStringConverter() ) );
	// Saver
	res += FDatabase->ExpandMacro( GetBinderMacro( isArray, isScalar, false,  false, GetToStringConverter() ) );

	// Binary loader/saver
	// Loader
//	res += FDatabase->ExpandMacro( std::string("BIN_") + GetBinderMacro( isArray, isScalar, false,   true, GetFromStringConverter() ) );
	// Saver
//	res += FDatabase->ExpandMacro( std::string("BIN_") + GetBinderMacro( isArray, isScalar, false,  false, GetToStringConverter() ) );

	if(!isArray)
	{
		// TODO: support scalars and strings
		if(!isScalar)
		{
			// Get-accessor
			res += FDatabase->ExpandMacro( GetBinderMacro( isArray, isScalar, true,   true, GetToStringConverter() ) );
			// Set-accessor
			res += FDatabase->ExpandMacro( GetBinderMacro( isArray, isScalar, true,  false, GetToStringConverter() ) );
		} else
		{
			// Get-accessor
			res += FDatabase->ExpandMacro( GetBinderMacro( isArray, isScalar, true,   true, GetToStringConverter() ) );
			// Set-accessor
			res += FDatabase->ExpandMacro( GetBinderMacro( isArray, isScalar, true,  false, GetToStringConverter() ) );
		}
	}
	else
	{
		//    Items, SerializableClass)
		res += "\n";

		string res1 = string( "ARRAY_PROPERTY_FUNCTIONS__FIELD(" );
		res1 += FieldName;
		res1 += string( ", " ) + FClassName;
		res1 += string( ")" );

		res += FDatabase->ExpandMacro( res1 );

		if ( !isScalar )
		{
			res += string( "\n" );
         
			string res2 = string( "ARRAY_PROPERTY_DELETE_FUNCTION__FIELD" );
			if(SmartPointer) { res2 += string("_SMARTPTR"); }
			res2 += string("(");
			res2 += FieldName;
			res2 += string( ", " ) + FClassName;
			res2 += string( ")" );

			res += FDatabase->ExpandMacro( res2 );

			res += string( "\n" );

			/// array getter
			res2 = string("ARRAY_PROPERTY_GETOBJECT_FUNCTION__FIELD");
			if(SmartPointer) { res2 += string("_SMARTPTR"); }
			res2 += string("(");
			res2 += FieldName;
			res2 += string( ", ") + FClassName;
			res2 += string( ")" );

			res += FDatabase->ExpandMacro( res2 );

			res += string( "\n" );

			/// array setter
			res2 = string("ARRAY_PROPERTY_SETOBJECT_FUNCTION__FIELD");
			if(SmartPointer) { res2 += string("_SMARTPTR"); }
			res2 += string("(");
			res2 += FieldName;
			res2 += string( ", ") + FClassName;
			res2 += string( ", ") + Type;
			res2 += string( ")" );

			res += FDatabase->ExpandMacro( res2 );

			res += string( "\n" );

			res2 = string("ARRAY_PROPERTY_INSERTOBJECT_FUNCTION__FIELD");
			if(SmartPointer) { res2 += string("_SMARTPTR"); }
			res2 += string("(");
			res2 += FieldName;
			res2 += string( ", ") + FClassName;
			res2 += string( ", ") + Type;
			res2 += string( ")" );

			res += FDatabase->ExpandMacro( res2 );

			res += string( "\n" );

			res2 = string("ARRAY_PROPERTY_REMOVEOBJECT_FUNCTION__FIELD");
			if(SmartPointer) { res2 += string("_SMARTPTR"); }
			res2 += string("(");
			res2 += FieldName;
			res2 += string( ", ") + FClassName;
			res2 += string( ")" );

			res += FDatabase->ExpandMacro( res2 );
		} else
		{
			res += string( "\n" );
			// POD values

			/// array getter
			string res2  ("ARRAY_PROPERTY_GETITEM_FUNCTION__FIELD(");
			res2 += FieldName;
			res2 += string( ", ") + FClassName;
			res2 += string( ", ") + GetToStringConverter();
			res2 += string( ")" );

			res += FDatabase->ExpandMacro( res2 );

			res += string( "\n" );

			/// array setter
			res2 = string("ARRAY_PROPERTY_SETITEM_FUNCTION__FIELD(");
			res2 += FieldName;
			res2 += string( ", ") + FClassName;
			res2 += string( ", ") + GetFromStringConverter();
			res2 += string( ")" );

			res += FDatabase->ExpandMacro( res2 );

			res += string( "\n" );

			res2 = string("ARRAY_PROPERTY_INSERTITEM_FUNCTION__FIELD(");
			res2 += FieldName;
			res2 += string( ", ") + FClassName;
			res2 += string( ", ") + GetFromStringConverter();
			res2 += string( ")" );

			res += FDatabase->ExpandMacro( res2 );

			res += string( "\n" );

			res2 = string("ARRAY_PROPERTY_REMOVEITEM_FUNCTION__FIELD(");
			res2 += FieldName;
			res2 += string( ", ") + FClassName;
			res2 += string( ")" );

			res += FDatabase->ExpandMacro( res2 );
		}

		res += string( "\n" );
	}

	// TODO : handle non-resizable array and Getter/Setter arrays
	// if (isArray && Setter.)

	return res;
}

string clProperty::GetRegistrationCode(string& FullPropertyName) const
{
   bool isArray = ( IndexType == "int" ) || ( IndexType == "size_t" );

   if ( !IndexType.empty() ) { if ( !isArray ) { return ""; } } // only 'int' is supported

// bool isScalar = !FDatabase->IsSerializableClass(Type);
   bool isScalar = !FDatabase->IsWrappedClass( Type );

   string res = string( "REGISTER_PROPERTY__" );

   string add_res = string( "REGISTER_PROPERTY_ACCESSORS__");

   string ObjPrefix = ( isScalar ? "SCALAR_" : "OBJECT_" );

// std::cout << FieldName << " Getter: '" << Getter << "' Setter: '" << Setter << "'" << std::endl;

   if ( isArray )
   {
      FullPropertyName = FieldName;
      // todo : support getter/setter
      res += ObjPrefix + string( "ARRAY_FIELD(" ) + FullPropertyName;
   }
   else
   {
      if ( Setter.empty() && Getter.empty() )
      {
         string CommonPart = ObjPrefix + string( "FIELD");

//         if(SmartPointer) { CommonPart += string("_SMARTPTR"); }

         FullPropertyName = CompressSerializedName( FieldName );
         CommonPart += string("(" ) + FullPropertyName;

         res += CommonPart;

         add_res += CommonPart;
      }
      else
      {
         FullPropertyName = Name;

         string CommonPart = ObjPrefix + string( "GETTER_SETTER(" ) + FullPropertyName;

         res += CommonPart;
         //+ Getter + string(", ") + Setter;

         add_res += CommonPart;
      }
   }

   FullPropertyName = "Prop_" + FullPropertyName;

   string RegCommon = string( ", " ) + FClassName + ", " + Name;

   res += RegCommon + ", " + Type + ")";
   add_res += RegCommon + ")";

   if ( isArray )
   {
      if ( !Counter.empty() )
      {
         // ....
      }
   } else
   {
      // Now we add the code for field contents access (i.e., iObject* GetValue(iObject* Obj) will get the Obj->FFieldForProperty)
      if(isScalar)
      {
         // strings are handled differently and PODs require more params to the macro
      } else
      {
         // iObject props are accessed easily

         res += string( "\n" );
    
         /// PROPERTY_REGISTER_ACCESSORS__*
         res += add_res;

         res += string( "\n" );
      }
   }

   return res;
}

string clProperty::Validate()
{
	if ( Type.empty() ) { return "Property type is not specified. Check definition syntax"; }

	if ( Name.empty() ) { return "Property name is not specified. Check definition syntax"; }

	if ( FieldName.empty() )
	{
		if ( Getter.empty() && Setter.empty() )
		{
			// it is not an array property ?
			return "No field, getter or setter";
		}
	}

	return "";
}
