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

#define AssignP(_Name) AssignPrm(_Name, #_Name, ParamName, ParamValue);

void clProperty::SetParam( const string& ParamName, const string& ParamValue )
{
   AssignP( NetIndexedGetter )
   AssignP( NetIndexedSetter )
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

	if ( ParamName == "Type" )
	{
		SmartPointer = ( !ParamValue.empty() ) && ( ParamValue[ ParamValue.length() - 1 ] == '^' );
	}
}

#undef AssignP

// convert string to property description, returning error code if something is wrong
string clProperty::FromString( const string& P )
{
   // 1. extract param list
   size_t Pos1 = P.find_first_of( '(' );
   size_t Pos2 = P.find_last_of( ')' );

   string ParamList = TrimSpaces( P.substr( Pos1 + 1, Pos2 - Pos1 - 1 ) );

   if ( ParamList.empty() ) { return string( "Incomplete property description" ); }

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

   // 3. split each parameter to <ParamName, ParamValue>
   for ( size_t i = 0 ; i < Params.size() ; i++ )
   {
      string PP = Params[i];

      string _Name = TrimSpaces( BeforeChar( PP, "=" ) );
      string _Val  = TrimSpaces( AfterChar( PP, "=" ) );

      if ( _Name == "Name" ) { _Val = TrimQuotes( _Val ); }

      SetParam( _Name, _Val );
   }

	if ( SmartPointer )
	{
		// drop the last ^ from the type name
		Type = Type.substr( 0, Type.length() - 1 );

		if ( Verbose ) cout << "Smartpointer property: " << Name << " " << Type << endl;
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

   string res = "DEFINE_INDEXER_STUFF(";

   res += Name + string( ", " );

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

   string res = "INIT_INDEXER_STUFF(";

   res += Name + string( ", " );
   res += FClassName + string( ", " );
   res += NetIndexedGetter + string( ", " );
   res += NetIndexedSetter + string( ")" );

   return res;
}

string clProperty::DeclareNETProperty() const
{
	if ( SmartPointer )
   {
		if ( Verbose ) cout << "Skipping smartpointer property .NET declaration: " << Name << " " << Type << endl;

      // TODO: declare smart pointer property to a wrapped class
		return "";
   }

   if ( !IndexType.empty() ) { return GetIndexerStuffDefinition(); }

   bool AddNativeConverters = false;

   // non-indexed property

   string AccessModifier = "";

   if ( !Getter.empty() ) { AccessModifier += string( "_GET" ); }

   if ( !Setter.empty() ) { AccessModifier += string( "_SET" ); }

   string PropertyDeclarator = "";

   bool Wrapped = false;

   if ( FDatabase->IsScalarType( Type ) )
   {
      // declare scalar property with direct field access
      PropertyDeclarator = "DECLARE_SCALAR_PROPERTY";
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
      }
   }

   PropertyDeclarator += AccessModifier;

   string PropertyParams = "";

   AddParam_NoValue( PropertyParams, Description == "" ? string( "\"\"" ) : Description );
   AddParam_NoValue( PropertyParams, Category == "" ? string( "\"\"" ) : Category );

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
	if ( SmartPointer )
   {
		if ( Verbose ) cout << "Skipping smartpointer property .NET implementation: " << Name << " " << Type << endl;

      // TODO: declare smart pointer property to a wrapped class
		return "";
   }

   if ( !IndexType.empty() ) { return ""; }

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
            string Result("DECLARE_WRAPPER_PROPERTY_DIRECT_IMPL(");

            Result += FClassName + string(", ");
            Result += Type + string(", ");
            Result += Name + string(", ");
            Result += FieldName;

            Result += string(")");

            return Result;
         } else
         if(!AccessModifier.empty())
         {
            string Result(string("DECLARE_WRAPPER_PROPERTY") + AccessModifier + string("_IMPL("));

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
string clProperty::GetBinderMacro( bool IsArray, bool IsScalar, bool Load, const string& Conv ) const
{
// add ',' at the end if required
#define FIX_TRAILING_COMMA() \
	if ( res[res.length() - 2] != ',') res += string(", ");

   string res = "";

   // <MacroName> ( PropertyName, FieldName, ClassName, AccessName or Getter/Setter, Type for arrays, ToStringConverter )

   res  = string( ( IsScalar ? "SCALAR" : "OBJECT" ) );
   res += string( ( IsArray ? "_ARRAY" : "" ) );
   res += string( "_PROPERTY_" ) + string( Load ? "LOAD" : "SAVE" ) + string( "__" );

   bool EmptyFlag = Load ? Setter.empty() : Getter.empty();

   if ( !EmptyFlag )
   {
      res += string( Load ? "SETTER" : "GETTER" ) + string("(");
   }
   else
   {
      // compressed name is used for the loader function name
      res += string( "FIELD(" );
   }

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

   if ( IsScalar )
   {
      string Conv = Load ? GetFromStringConverter() : GetToStringConverter();

      if ( Conv.empty() ) { Conv = "EMPTY_CONVERTER"; }

      FIX_TRAILING_COMMA()
	  res += Conv;
   }

	if ( !IsScalar && !IsArray && !EmptyFlag && Load )
	{
		res += ", " + Type;
	}

   res += string( ")\n" );

   return FDatabase->ExpandMacro(res);
}

string clProperty::GetLoadSaveDeclarations() const
{
   bool isArray = ( IndexType == "int" ) || ( IndexType == "size_t" );

   if ( !IndexType.empty() ) { if ( !isArray ) { return ""; } } // only 'int' is supported

   bool isScalar = !FDatabase->IsWrappedClass( Type ); //!IsClassType(Type);

   string res;

   // Get/Set property has a different binding code implementation
   // loader/saver for a scalar/pod/object field

   res += GetBinderMacro( isArray, isScalar,  true, GetFromStringConverter() );

   res += GetBinderMacro( isArray, isScalar, false, GetToStringConverter() );

   if ( isArray )
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
         
		 string res2 = string( "ARRAY_PROPERTY_DELETE_FUNCTION__FIELD(" );
         res2 += FieldName;
         res2 += string( ", " ) + FClassName;
         res2 += string( ")" );

		 res += FDatabase->ExpandMacro( res2 );

         res += string( "\n" );

	/// array getter
	res2 = string("ARRAY_PROPERTY_GETOBJECT_FUNCTION__FIELD(");
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

string clProperty::GetRegistrationCode() const
{
   bool isArray = ( IndexType == "int" ) || ( IndexType == "size_t" );

   if ( !IndexType.empty() ) { if ( !isArray ) { return ""; } } // only 'int' is supported

// bool isScalar = !FDatabase->IsSerializableClass(Type);
   bool isScalar = !FDatabase->IsWrappedClass( Type );

   string res = string( "REGISTER_PROPERTY__" );

   string ObjPrefix = ( isScalar ? "SCALAR_" : "OBJECT_" );

// std::cout << FieldName << " Getter: '" << Getter << "' Setter: '" << Setter << "'" << std::endl;

   if ( isArray )
   {
      // todo : support getter/setter
      res += ObjPrefix + string( "ARRAY_FIELD(" ) + FieldName;
   }
   else
   {
      if ( Setter.empty() && Getter.empty() )
      {
         res += ObjPrefix + string( "FIELD(" ) + CompressSerializedName( FieldName );
      }
      else
      {
         res += ObjPrefix + string( "GETTER_SETTER(" ) + Name;
         //+ Getter + string(", ") + Setter;
      }
   }

   res += string( ", " ) + FClassName + string ( ", " ) + Name + string( ")" );
   res += string( "\n" );

   if ( isArray )
   {
      if ( !Counter.empty() )
      {
         // ....
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
