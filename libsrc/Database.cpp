#include "Database.h"

#include "Package.h"

#include <fstream>
#include <iostream>

using std::ifstream;
using std::cout;

clDatabase::clDatabase(): InheritanceCache( NULL )
{
   LoadPropertyMacros();
}

clDatabase::~clDatabase()
{
   delete( InheritanceCache );
}

string clDatabase::ExpandMacro(const LString& MacroString)
{
	if(!DoExpandMacros) { return MacroString; }

   int FirstBracket = MacroString.find_first_of('(');
   int num = MacroString.find_last_of(')') - FirstBracket;
   string MacroVals = MacroString.substr(FirstBracket+1, num-1);
   string MacroName = MacroString.substr(0, FirstBracket);

   vector<string> MacroValues = SplitLineSep(MacroVals, ',');

   //
   string Comment = "// " + MacroName + "\n";

   for ( size_t i = 0; i != MacroValues.size(); i++ ) { Comment += "// " + MacroValues[i] + "\n"; }

   // commence macro expansion
   std::map<string, clMacroDef>::const_iterator Macro = PropertyMacros.find( MacroName );

   if ( Macro != PropertyMacros.end() )
   {
      return Comment + Macro->second.GenerateInstance( MacroValues );
   }
   else
   {
      return Comment + MacroString;
   }
}

void clDatabase::LoadPropertyMacros()
{
   const string PropertyMacrosFileName = "Src/Linderdaum/Core/RTTI/PropertyMacros.h";

   std::ifstream Macros( PropertyMacrosFileName.c_str() );

   string Line;

   int NumMacros = 0;

   while ( getline( Macros, Line ) )
   {
      Line = TrimSpaces( Line );

      // generate macro
      if ( Line.find( "#define " ) == 0 )
      {
         size_t OpenPos  = Line.find( '(' );
         size_t ClosePos = Line.find( ')' );

         if ( OpenPos == string::npos || ClosePos == string::npos ) { continue; }

         string MacroName       = TrimSpaces( Line.substr( 8, OpenPos - 8 ) );
         string MacroParamsList = TrimSpaces( Line.substr( OpenPos + 1, ClosePos - OpenPos - 1 ) );
         string MacroText       = TrimSpaces( Line.substr( ClosePos + 1, Line.length() ) );

         if ( MacroText[ MacroText.length()-1 ] == '\\' ) { MacroText = MacroText.substr( 0, MacroText.length() - 1 ); }

         while ( getline( Macros, Line ) )
         {
            if ( TrimSpaces( Line ).empty() ) { break; }

            if ( Line[ Line.length()-1 ] == '\\' ) { Line = Line.substr( 0, Line.length() - 1 ); }

            MacroText += "\n" + Line;
         }

         std::vector<string> ParamsList = SplitLineSep( MacroParamsList, ',' );

         clMacroDef NewMacro( MacroName, ParamsList, MacroText + "\n\n" );
		 NewMacro.FDatabase = this;

         PropertyMacros.insert( std::pair<string, clMacroDef>( MacroName, NewMacro ) );
         NumMacros++;
      }
   }

	if ( Verbose )
	{
	   cout << PropertyMacrosFileName << " contains " << NumMacros << " macro definitions" << endl;
	}
}

void clDatabase::AddClass( clClass* Class )
{
   Class->FClassInternalID = static_cast<int>( GlobalClassesList.size() );

   GlobalClassesList.push_back( Class );
}

clPackage* clDatabase::ProcessPackageDirectory( const string& PackageDir )
{
   // 1. create and register the new package
   clPackage* Pack = new clPackage;
   RegisterPackage( Pack );

   // 2. read package contents from a signle directory
   Pack->FPackageInDirectories.resize( 1 );
   Pack->FPackageInDirectories[0] = PackageDir;
   Pack->ProcessPackageInputDirectories();

   return Pack;
}

void clDatabase::RegisterPackage( clPackage* Pack )
{
   Pack->FDatabase = this;
   FPackages.push_back( Pack );
}

char clDatabase::GetCacheValue( int Class, int Base )
{
   int Index = Class * GlobalClassesList.size() + Base;

   if ( Index < 0 ||
        Index > static_cast<int>( GlobalClassesList.size() * GlobalClassesList.size() ) ) { return CACHE_NONE; }

   return InheritanceCache[ Index ];
}

void clDatabase::SetCacheValue( int Class, int Base, char Value )
{
   int Index = Class * GlobalClassesList.size() + Base;

   if ( Index < 0 ||
        Index > static_cast<int>( GlobalClassesList.size() * GlobalClassesList.size() ) ) { return; }

   InheritanceCache[ Index ] = Value;
}

void clDatabase::AllocateInheritanceCache()
{
    // preallocate inheritance cache
	int Size = GlobalClassesList.size() * GlobalClassesList.size();
	
	if ( Verbose )
	{
		cout << "Allocating " << Size << " bytes for inheritance cache (" << GlobalClassesList.size() << " classes in global list)" << endl;
	}
	
	InheritanceCache = new char[ Size ];
	
	for ( int i = 0; i != Size; i++ ) { InheritanceCache[i] = CACHE_NONE; }
}

void clDatabase::GenerateStuffForPackage(const std::string& PkgName)
{
	AllocateInheritanceCache();
	for ( size_t i = 0 ; i < FPackages.size() ; i++ )
	{
		// don't generate anything for packages without setup scripts,
		// just display a warning instead
		if ( FPackages[i]->FPackageName == PkgName )
		{
			cout << "Generating package: " << FPackages[i]->FPackageName << endl;
			FPackages[i]->GenerateStuff();
			break;
		}
	}
}

void clDatabase::GenerateStuff()
{
	AllocateInheritanceCache();

   for ( size_t i = 0 ; i < FPackages.size() ; i++ )
   {
      // don't generate anything for packages without setup scripts,
      // just display a warning instead
      if ( FPackages[i]->FPackageName.empty() )
      {
         cout << "WARNING: empty package found - bypassing (possibly no project setup file)" << endl;

         continue;
      }

      cout << "Generating package: " << FPackages[i]->FPackageName << endl;

      FPackages[i]->GenerateStuff();
   }
}

void clDatabase::GenerateStatistics()
{
   // 1. dump package statistics and dependencies
   for ( size_t i = 0 ; i < FPackages.size() ; i++ )
   {
      FPackages[i]->GenerateStatistics();
   }

   // 2. dump global statistics
// _TRACE("Dumping converter table contents")
   DumpNETTypeMap( "Debug_NETTypes.list" );
   DumpStringConverters( "Debug_StringCvt.list" );
}

bool clDatabase::IsWrappedClass( const string& ClassName )
{
   // call ProgramDatabase from LSDC.h
   for ( size_t i = 0 ; i < FPackages.size() ; i++ )
   {
      if ( FPackages[i]->IsWrappedClass( ClassName ) ) { return true; }
   }

   return false;
}
/*
bool clDatabase::IsSerializableClass( const string& ClassName )
{
   // call ProgramDatabase from LSDC.h
   for ( size_t i = 0 ; i < FPackages.size() ; i++ )
   {
      if ( FPackages[i]->IsSerializableClass( ClassName ) ) { return true; }
   }

   return false;
}
*/
// TODO : iterate packages or just pick Classes[ClassName]->Package->GetRootClassFor()
string clDatabase::GetRootestClassFor( const string& ClassName )
{
   // TODO : trace base classes up to the native root
   // clClass& cl = FDatabase[ClassName];
   string TheRootestClass = "iObject";
// if (NeverOverrideMethodsOf.size() > 0)
// {
//       TheRootestClass = NeverOverrideMethodsOf[0];
// }

   return TheRootestClass;
}

clClass* clDatabase::GetClassPtr( const string& ClassName )
{
   for ( vector< clClass* >::const_iterator i = GlobalClassesList.begin();
         i != GlobalClassesList.begin(); ++i )
   {
      if ( ( *i )->FClassName == ClassName ) { return ( *i ); }
   }

   int idx = GetPackageForClass( ClassName );

   if ( idx == -1 ) { return NULL; }

   return FPackages[idx]->GetClassPtr( ClassName );
}

int clDatabase::GetPackageForClass( const string& ClassName )
{
   for ( size_t i = 0 ; i < FPackages.size() ; i++ )
   {
      if ( FPackages[i]->ClassExists( ClassName ) ) { return static_cast<int>( i ); }
   }

   return -1;
}

///// Type information retrieval
bool clDatabase::InheritsFrom( const string& Class, const string& BaseClass )
{
   if ( Class == BaseClass ) { return true; }

   clClass* TargetClassPtr = GetClassPtr( Class );
   clClass* BaseClassPtr   = GetClassPtr( BaseClass );

   // not found
   if ( !TargetClassPtr || !BaseClassPtr ) { return false; }

   int TIndex = TargetClassPtr->FClassInternalID;
   int BIndex = BaseClassPtr->FClassInternalID;

   char Cache = GetCacheValue( TIndex, BIndex );

   if ( Cache != CACHE_NONE )
   {
      if ( Cache == CACHE_TRUE  ) { return true; }

      if ( Cache == CACHE_FALSE ) { return false; }
   }

   for ( clBaseClassesList::const_iterator i = TargetClassPtr->FBaseClasses.begin(); i != TargetClassPtr->FBaseClasses.end(); ++i )
   {
      if ( ( *i ).FBaseClassName == BaseClass )
      {
         SetCacheValue( TIndex, BIndex, CACHE_TRUE );
         return true;
      }
   }

   for ( clBaseClassesList::const_iterator i = TargetClassPtr->FBaseClasses.begin(); i != TargetClassPtr->FBaseClasses.end(); ++i )
   {
      if ( InheritsFrom( ( *i ).FBaseClassName, BaseClass ) )
      {
         SetCacheValue( TIndex, BIndex, CACHE_TRUE );
         return true;
      }
   }

   SetCacheValue( TIndex, BIndex, CACHE_FALSE );

   return false;
}

bool clDatabase::IsEnumType( const string& TypeName )
{
	if(IsEnum.count(TypeName) > 0) { return IsEnum[TypeName];}

	return false;
}

bool clDatabase::IsScalarType( const string& type )
{
   if ( IsScalar.count( type ) > 0 ) { return IsScalar[type]; }

   return false;

// return type == "void" || type == "bool" || type == "int" || type == "float" || type == "double" || type == "Lfloat" || type == "Ldouble" || type == "size_t";
}

bool clDatabase::IsPODType( const string& type )
{
   if ( IsPOD.count( type ) > 0 ) { return IsPOD[type]; }

   return false;
// return (type == "vec2" || type == "LVector2" || type == "vec3" || type == "LVector3" || type == "LVector4" || type == "vec4" || type == "mtx3" || type == "LMatrix3" || type == "LMatrix4" || type == "mtx4");
}

/*
  Read a list of Native <-> .NET mappings

 Token[1] == NetName
 Token[2] == NativeName
 Token[3] == ToNetConverter
 Token[4] == FromNetConverter
*/
void clDatabase::ParseNETTypeMapString( const string& In )
{
   string s = TrimSpaces( In );

   vector<string> Parts;

   if ( s == "" ) { return; }

   if ( s[0] == ';' ) { return; }

   if ( s.length() > 2 ) if ( s[0] == '/' && s[1] == '/' ) { return; }

// cout << "Type: " << s << endl;

   SplitLine( s, Parts, true );

   string NetName    = Parts[1];
   string NativeName = Parts[0];

// cout << "NetName    = " << NetName << endl;
// cout << "NativeName = " << NativeName << endl;

   NativeToNet[NativeName] = NetName;

   IsClass[NativeName]  = false;
   IsPOD[NativeName]    = false;
   IsEnum[NativeName]   = false;
   IsScalar[NativeName] = false;

   if ( Parts.size() > 2 )
   {
      if ( Parts[2] == "class" )
      {
//       cout << NativeName << " is a class" << endl;
         IsClass[NativeName]  = true;
      }
      else if ( Parts[2] == "scalar" )
      {
//       cout << NativeName << " is a scalar" << endl;
         IsScalar[NativeName] = true;
      }
      else if ( Parts[2] == "pod" )
      {
//       cout << NativeName << " is a pod" << endl;
         IsPOD[NativeName]    = true;
      }
      else if ( Parts[2] == "enum" )
      {
//       cout << NativeName << " is an enum" << endl;
         IsEnum[NativeName]    = true;
      }

      if ( Parts.size() > 3 )
      {
//       cout << "ToNetConverter   = [" << Parts[3] << "]" << endl;
         ToNetConverter[NativeName]   = Parts[3];
      }

      if ( Parts.size() > 4 )
      {
//       cout << "FromNetConverter[" << NativeName << "] = [" << Parts[4] << "]" << endl;
         FromNetConverter[NativeName] = Parts[4];
      }
   }
}

// supports multiple equivalent type names, string-LString, vec3-LVector3 etc.
void clDatabase::MakeTypeMap( const string& ToFname )
{
   ifstream ToNetTypes( ToFname.c_str() );

   string s;

   while ( getline( ToNetTypes, s ) )
   {
      ParseNETTypeMapString( s );
   }

// cout << "Done reading" << endl;

   ToNetTypes.close();
}

string clDatabase::GetAppropriateNetTypeForParameter( const string& type )
{
   if ( !MapsToNET( type ) )
   {
		cout << "Failed checking[" << type << "]" << endl;
      return "";
   }

   if ( IsScalarType( type ) )
   {
      string res = NativeToNet[type];
//    cout << "Scalar type " << type << " maps to << " << res << endl;
      return res;
   }
	else if ( IsSmartPointer( type ) )
	{
		string res = type;
//		cout << "Smartpointer type " << type << endl;
		return ExtractSmartPointerType( type ) + string( "^" );
	}
   else if ( IsPODType( type ) )
   {
      string res = NativeToNet[type] + string( "^" );
//    cout << "POD type " << type << " maps to << " << res << endl;
      return res;
   }

   string res = type + string( "^" );
// cout << "Class type " << type << " maps to << " << res << endl;

   return res;
}

string clDatabase::GetNetToNativeConversion( const string& FullType, const string& VarName, const string& NetVarName, const string& NativeType )
{
   if ( IsScalarType( NativeType ) )
   {
      // make a "safety" cast, to insure everything is ok
      return NativeType + string( " " ) + VarName + string( " = (" ) + NativeType + string( ")" ) + NetVarName;
   }

// cout << "Converting.NET [" << NativeType << "]" << endl;
   string TrimmedType = TrimSpaces( StripTypeName( TrimSpaces( FullType ) ) );

   if ( ( FromNetConverter.count( NativeType ) > 0 ) || IsPODType( NativeType ) )
   {
      string cvt = FromNetConverter[NativeType];

//    cout << "Using FromNetConverter: " << cvt << endl;

      string PtrSign = "";

      if ( IsPointer( FullType ) ) { PtrSign = "*"; }

      string res = NativeType + PtrSign + string( " " ) + VarName + string( " = " ) + cvt + string( "(" ) + NetVarName + string( ")" );

      return res;
   }
   else if ( IsSmartPointer( TrimmedType /*FullType*/ /*NativeType*/ ) )
	{
//      cout << "using smart_ptr = " << TrimmedType << endl;

		TrimmedType = ExtractSmartPointerType( TrimmedType );

//      cout << "extracted smart_ptr = " << TrimmedType << endl;

      clClass* C = GetClassPtr( TrimmedType );

      if ( !C ) { /*cout << "no class: " << TrimmedType << endl;*/ return ""; }

      if ( !C->FNetExportable ) { /*cout << "not an net-exportable class: " << TrimmedType << endl;*/ return ""; }

//    cout << "Using FROM_NET_OBJ" << endl;

      string TheNamespace = "::"; // TODO : get some map NativeNamespaces[<TypeName>]

      string FullNativeType = TheNamespace + TrimmedType;
      return FullNativeType + string( "* " ) + VarName + string( " = (" ) + NetVarName + ( " == nullptr) ? NULL : (" ) + NetVarName + string( "->GetNativeObject()" ) + string( ")" );
	}
   else if ( IsPointer( FullType/*NativeType*/ ) )
   {
      clClass* C = GetClassPtr( TrimmedType );

      if ( !C ) { return ""; }

      if ( !C->FNetExportable ) { return ""; }

//    cout << "Using FROM_NET_OBJ" << endl;

      string TheNamespace = "::"; // TODO : get some map NativeNamespaces[<TypeName>]

      string FullNativeType = TheNamespace + TrimmedType;
      return FullNativeType + string( "* " ) + VarName + string( " = (" ) + NetVarName + ( " == nullptr) ? NULL : (" ) + NetVarName + string( "->GetNativeObject()" ) + string( ")" );
   }

// cout << "Conversion is not found" << endl;

   return "";
}

string clDatabase::GetNativeToNetConversion( const string& NativeVarName, const string& NativeType )
{
   if ( IsScalarType( NativeType ) )
   {
      // no conversion, just return the variable
      return NativeVarName;
   }

   /**
   // Enum handling :
      if (IsEnumType(NativeType))
      {
         // use NativeToManaged converter function for this enumeration type
         string EnumConverter = "";

         return EnumConverter;
      }
   */

   // the type is not scalar, but it might be POD
   if ( ToNetConverter.count( NativeType ) > 0 )
   {
      return ToNetConverter[NativeType] + string( "(" ) + NativeVarName + string( ")" );
   }
   else
   {
      string TheNamespace = "";

      if ( IsWrappedClass( NativeType ) ) // avoid confusion of .NET and Native types
      {
         TheNamespace = "::"; // TODO : get some map NativeNamespaces[<TypeName>]
      }

		bool IsSmartPtr = IsSmartPointer( NativeType );
		
		string InnerType = ExtractSmartPointerType( NativeType );

      // it is a wrapped class
//    return string("TO_NET_OBJECT(") + NativeVarName + string(", ") + TheNamespace + NativeType + string(")");
      return string( "(" ) + NativeVarName + string( " == NULL) ? (nullptr) : ( gcnew " ) + InnerType + string( "(" ) + NativeVarName + string( IsSmartPtr ? ".GetInternalPtr()) )" : ") )" );
   }
}

bool clDatabase::MapsToNET( const string& type )
{
   bool res = ( ToNetConverter.count( type ) > 0 ) || IsWrappedClass( type ) || IsScalarType( type );

	if ( IsSmartPointer( type ) )
	{
//		if ( Verbose ) { std::cout << "MapsToNET: " << type << std::endl; }

		string InnerType = ExtractSmartPointerType( type );

//		if ( Verbose ) { std::cout << "InnerType: " << InnerType << std::endl; }

		res = IsWrappedClass( InnerType );
	}

// cout << "type[" << type << "] " << ( res ? "maps" : "does not map") << " to .NET" << endl;

   return res;
}

bool clDatabase::IsReference( const string& TypeName )
{
   return ( TypeName[ TypeName.size()-1 ] == '&' );
}

bool clDatabase::IsPointer( const string& TypeName )
{
   return ( TypeName[ TypeName.size()-1 ] == '*' );
}

bool clDatabase::IsSmartPointer( const string& TypeName )
{
	return ( TypeName.find( "clPtr<" ) == 0 );
}

string clDatabase::ExtractSmartPointerType( const string& TypeName )
{
	if ( IsSmartPointer( TypeName ) )
	{
		return TrimSpaces( TypeName.substr( 6, TypeName.size() - 7 ) );
	}
	
	return TypeName;
}

bool clDatabase::IsConstType( const string& TypeName )
{
   return ( TypeName.find( "const " ) != -1 );
}

bool clDatabase::IsOutParameter( const string& Type )
{
   return ( !IsConstType( Type ) && IsReference( TrimSpaces( Type ) ) );
}

string clDatabase::StripTypeName( const string& Type )
{
   string TypeName = Type;

   if ( IsReference( TypeName ) || IsPointer( TypeName ) )
   {
      TypeName = TypeName.substr( 0, TypeName.size() - 1 );
   }

   if ( IsConstType( TypeName ) )
   {
      TypeName = TypeName.substr( 6, TypeName.size() - 6 );
   }

   return TypeName;
}

string clDatabase::CollapseTypeName( const string& Name )
{
   string TypeName = StripTypeName( TrimSpaces( Name ) );

   while ( TypeName.find( "::" ) != -1 )
   {
      size_t DPos = TypeName.find( "::" );

      TypeName[DPos+0] = '_';
      TypeName[DPos+1] = '_';
   }

   return TypeName;
}

bool clDatabase::IsTemplateType( const string& Name )
{
   bool Template  = Name.find( "<" ) != -1;
   bool Namespace = Name.find( "::" ) != -1;

   return Template || Namespace;
}

string clDatabase::GetAppropriateToStringConverter( const string& type )
{
   string TheType = StripTypeName( type );
   return ToStringConverters[TheType];
}

string clDatabase::GetAppropriateFromStringConverter( const string& type )
{
   string TheType = StripTypeName( type );
   return FromStringConverters[TheType];
}

string clDatabase::GetDefaultValidator( const string& type )
{
   string TheType = StripTypeName( type );

   if ( TheType.empty() ) { return ""; }

   if ( DefaultValidators.count( TheType ) <= 0 ) { return ""; }

   return DefaultValidators[TheType];
}

void clDatabase::ParseStringConverterDescription( const string& In )
{
   vector<string> Parts;

   string s = TrimSpaces( In );

   if ( s == "" ) { return; }

   if ( s[0] == ';' ) { return; }

   if ( s.length() > 2 ) if ( s[0] == '/' && s[1] == '/' ) { return; }

   Parts.resize( 0 );
   SplitLine( s, Parts, true );

   if ( Parts.size() < 1 ) { return; }

   string _to = "";
   string _from = "";
   string _defaultValidator = "";
   string _name = Parts[0];

   if ( Parts.size() > 2 )
   {
      _to = Parts[1];
      _from = Parts[2];
   }

   if ( Parts.size() > 3 ) { _defaultValidator = Parts[3]; }

   ToStringConverters[_name] = _to;
   FromStringConverters[_name] = _from;
   DefaultValidators[_name] = _defaultValidator;
}

void clDatabase::ReadTypeConvertersAndValidators( const string& fname )
{
   ifstream f( fname.c_str() );

   string s;

   while ( getline( f, s ) )
   {
      ParseStringConverterDescription( s );
   }
}

string clDatabase::AddNETReferenceModifierIfNeeded( const string& NativeType )
{
   string res = NativeToNet[NativeType];

   // class name coincides with the native name
   if ( res.empty() ) { res = NativeType; }

   if ( !IsScalarType( NativeType ) ) { res += string( "^" ); }

   return res;
}

/// Increase MaxLen if it is less than s.length()
inline void CompareMax( const string& s, int& MaxLen )
{
   int l = static_cast<int>( s.length() );

   if ( MaxLen < l ) { MaxLen = l; }
}

void clDatabase::DumpStringConverters( const string& fname )
{
   ofstream f( fname.c_str() );

   vector<string> Names;
   vector<string> ToCvt;
   vector<string> FromCvt;
   vector<string> Validators;

   int MaxNameLen  = -1;
   int MaxToLen    = -1;
   int MaxFromLen  = -1;
   int MaxValidLen = -1;

   /// 1. Pass one : calculate maximum lengths of each element

   for ( map<string, string>::iterator i = ToStringConverters.begin() ; i != ToStringConverters.end() ; i++ )
   {
      string TypeName = i->first;
      string ToString = i->second;
      string FromString = FromStringConverters[TypeName];
      string Validator = "";

      if ( DefaultValidators.count( TypeName ) > 0 )
      {
         Validator = DefaultValidators[TypeName];
      }

      Names.push_back( TypeName );
      ToCvt.push_back( ToString );
      FromCvt.push_back( FromString );
      Validators.push_back( Validator );

      CompareMax( TypeName, MaxNameLen );
      CompareMax( ToString, MaxToLen );
      CompareMax( FromString, MaxFromLen );
      CompareMax( Validator, MaxValidLen );
   }

   /// 2. Pass two : dump each converter with space padding
   for ( size_t i = 0 ; i < Names.size() ; i++ )
   {
      string Res = PadStringRight( Names[i], MaxNameLen + 3, ' ' );

      Res += PadStringRight( ToCvt[i], MaxToLen + 3, ' ' );

      Res += PadStringRight( FromCvt[i], MaxFromLen + 3, ' ' );

      Res += PadStringRight( Validators[i], MaxValidLen + 3, ' ' );

      f << Res << endl;
   }

   f.close();
}

void clDatabase::DumpNETTypeMap( const string& fname )
{
   ofstream f( fname.c_str() );

   vector<string> Natives;
   vector<string> NETs;
   vector<string> Types;
   vector<string> ToNETs;
   vector<string> FromNETs;
   int MaxNativeLen = -1;
   int MaxNETLen = -1;
   int MaxTypeLen = -1;
   int MaxToLen = -1;
   int MaxFromLen = -1;

   /// 1. First pass: read each type from NativeToNet
   for ( map<string, string>::iterator i = NativeToNet.begin(); i != NativeToNet.end() ; i++ )
   {
      string NativeName = i->first;
      string NETName = i->second;

      string Type = "class";

      if ( IsPOD[NativeName] ) { Type = "pod"; }

      if ( IsScalar[NativeName] ) { Type = "scalar"; }

      string ToNET = "";

      if ( ToNetConverter.count( NativeName ) > 0 ) { ToNET = ToNetConverter[NativeName]; }

      string FromNET = "";

      if ( FromNetConverter.count( NativeName ) > 0 ) { FromNET = FromNetConverter[NativeName]; }

      CompareMax( NativeName, MaxNativeLen );
      CompareMax( NETName, MaxNETLen );
      CompareMax( Type, MaxTypeLen );
      CompareMax( ToNET, MaxToLen );
      CompareMax( FromNET, MaxFromLen );

      Natives.push_back( NativeName );
      NETs.push_back( NETName );
      ToNETs.push_back( ToNET );
      FromNETs.push_back( FromNET );
      Types.push_back( Type );
   }

   /// 2. Second pass: write everything
   for ( size_t i = 0 ; i < Natives.size() ; i++ )
   {
      string Res = PadStringRight( Natives[i], MaxNativeLen + 3, ' ' );
      Res += PadStringRight( NETs[i], MaxNETLen + 3, ' ' );

      Res += PadStringRight( Types[i], MaxTypeLen + 3, ' ' );

      if ( ToNETs[i].length() > 0 )
      {
         Res += PadStringRight( ToNETs[i], MaxToLen + 3, ' ' );
         Res += PadStringRight( FromNETs[i], MaxFromLen + 3, ' ' );
      }

      f << Res << endl;
   }

   f.close();
}
