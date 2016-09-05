/**
 * \file Class.cpp
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

#include "Utils.h"

#include "Class.h"

#include "Package.h"

#include <algorithm>
#include <iostream>
#include <fstream>

bool clClass::HasScriptfinalAncestors() const
{
   /// iterate base classes and check for 'scriptfinal' modifier
   for ( clBaseClassesList::const_iterator i = FBaseClasses.begin(); i != FBaseClasses.end(); ++i )
   {
      clClass* Base = FDatabase->GetClassPtr( i->FBaseClassName );

      if ( Base )
      {
         if ( Base->FScriptFinal ) { return true; }

         if ( Base->HasScriptfinalAncestors() ) { return true; }
      }
   }

   return false;
}

bool clClass::IsDestructorDefinition( const string& S ) const
{
   string DestructorSign = "~" + FClassName + "(";
   return ( CollapseSpaces( S ).find( DestructorSign ) != -1 );
}

bool clClass::IsConstructorDefinition( const string& S ) const
{
   string ConstructorSign = FClassName + "(";

   return ( S.find( ConstructorSign ) != -1 ) || ( S.find( "explicit " ) != -1 );
}

void clClass::AddProperty( const clProperty& Prop )
{
   FProperties.push_back( Prop );
   FProperties[FProperties.size() - 1].FClassName = FClassName;
   FProperties[FProperties.size() - 1].FDatabase = FDatabase;
}

void clClass::AddConstructor( const string& Access, const clMethod& TheConstructor )
{
   FConstructors.push_back( TheConstructor );
   clMethod& LastConstructor = FConstructors[FConstructors.size() - 1];
   LastConstructor.FClassName = FClassName;
   LastConstructor.FDatabase = FDatabase;
   LastConstructor.FAccess  = Access;
}

void clClass::AddMethod( const string& Access, const clMethod& TheMethod )
{
   for ( clMethodsList::const_iterator i = FMethods.begin(); i != FMethods.end(); ++i )
   {
      if ( ( *i ).FMethodName == TheMethod.FMethodName )
      {
         cout << endl;
         cout << "ERROR: Class " << FClassName << " contains overloaded method: '" << TheMethod.FMethodName << "'" << endl;
         exit( 255 );
      }
   }

   FMethods.push_back( TheMethod );
   clMethod& LastMethod = FMethods[FMethods.size() - 1];
   LastMethod.FDatabase = FDatabase;
   LastMethod.FClassName = FClassName;
   LastMethod.FAccess = Access;
}

void clClass::AddFieldA( const string& Access, const clField& Field )
{
   for ( clFieldsList::const_iterator i = FFields.begin(); i != FFields.end(); ++i )
   {
      if ( ( *i ).FFieldName == Field.FFieldName )
      {
         cout << endl;
         cout << "ERROR: Class " << FClassName << " contains duplicate field: '" << Field.FFieldName << "'" << endl;
         exit( 255 );
      }
   }

   FFields.push_back( Field );
   clField& LastField = FFields[FFields.size() - 1];
   LastField.FDatabase = FDatabase;
   LastField.FClassName = FClassName;
   LastField.FAccess = Access;
}

void clClass::AssignClassAttributes( const vector<string>& Attribs )
{
   // default values go here
   FSerializable = false;
   FScriptFinal = false;
   FNetExportable = false;

   if ( Attribs.empty() ) { return; }

   for ( size_t idx = 0 ; idx < Attribs.size() - 1 ; idx++ )
   {
      if ( Attribs[idx] == "serializable" )
      {
         FSerializable = true;
      }
      else if ( Attribs[idx] == "scriptfinal" )
      {
         FScriptFinal = true;
      }
      else if ( Attribs[idx] == "netexportable" )
      {
         FNetExportable = true;
      }
   }
}

bool clClass::ExtractBaseClassList( const string& InBaseClasses, string& Error )
{
   string BaseClasses = InBaseClasses;

   // extract base classes list
   while ( !BaseClasses.empty() )
   {
      size_t CommaPos = BaseClasses.find( "," );

      string BaseClass;

      if ( CommaPos == -1 )
      {
         BaseClass = TrimSpaces( BaseClasses );
         BaseClasses = "";
      }
      else
      {
         BaseClass = TrimSpaces( BaseClasses.substr( 0, CommaPos ) );
         BaseClasses = TrimSpaces( BaseClasses.substr( CommaPos + 1, BaseClasses.length() - 1 ) );
      }

      // check virtual inheritance
      size_t VirtualPos = BaseClass.find( "virtual" );

      if ( VirtualPos != -1 )
      {
         BaseClass = TrimSpaces( BaseClass.substr( VirtualPos + 8, BaseClass.length() - 1 ) );
      }

      // extract base class modifier
      string Modifier( "public" );

      size_t SpacePos = BaseClass.find( " " );

      if ( SpacePos != -1 )
      {
         Modifier = TrimSpaces( BaseClass.substr( 0, SpacePos ) );
         BaseClass = TrimSpaces( BaseClass.substr( SpacePos + 1, BaseClass.length() - 1 ) );
      }

      if ( BaseClass.find( " " ) != -1 && BaseClass.find( "<" ) == -1 && BaseClass.find( ">" ) == -1 )
      {
         Error = string( "ERROR: invalid base class name: '" ) + BaseClass + string( "'" );
         return false;
      }

      if ( FClassName.empty() )
      {
         Error = "ERROR: empty class name";
         return false;
      }

      if ( BaseClass.empty() ) { return true; }

      // build inheritance hierarchy
      clBaseClassLink BaseClassLink;

      BaseClassLink.FBaseClassName   = BaseClass;
      BaseClassLink.FInheritanceType = Modifier;
      BaseClassLink.FVirtualBase     = ( VirtualPos != -1 );
      BaseClassLink.FTemplateClass   = false;

      FBaseClasses.push_back( BaseClassLink );
   }

   return true;
}

bool clClass::HasDefaultConstructor() const
{
   if ( FConstructors.empty() ) { return true; }

   for ( clMethodsList::const_iterator i = FConstructors.begin(); i != FConstructors.end(); ++i )
   {
      if ( i->FArgTypes.empty() ) { return true; }
   }

   return false;
}

void clClass::CollectBaseClasses( vector<string>& OutList ) const
{
   for ( clBaseClassesList::const_iterator i = FBaseClasses.begin(); i != FBaseClasses.end(); ++i )
   {
      clClass* Base = FDatabase->GetClassPtr( i->FBaseClassName ); //(*FClassesDatabase)[ i->FBaseClassName ];

      if ( Base )
      {
         Base->CollectBaseClasses( OutList );
      }
   }

   AddUnique( OutList, FClassName );
}

bool clClass::FindOverridenMethod( const string& MethodName, string& OverridenInClass ) const
{
   for ( clMethodsList::const_iterator i = FMethods.begin(); i != FMethods.end(); ++i )
   {
      if ( !i->FAbstract )
      {
         if ( i->FMethodName == MethodName )
         {
            OverridenInClass = FClassName;

            return true;
         }
      }
   }

   if ( FBaseClassesCache.empty() )
   {
      for ( clBaseClassesList::const_iterator i = FBaseClasses.begin(); i != FBaseClasses.end(); ++i )
      {
         clClass* Base = FDatabase->GetClassPtr( i->FBaseClassName ); // (*FClassesDatabase)[ i->FBaseClassName ];

         if ( Base )
         {
            FBaseClassesCache.push_back( *Base );
         }
      }
   }

   for ( clClassesCache::const_iterator i = FBaseClassesCache.begin(); i != FBaseClassesCache.end(); ++i )
   {
      if ( i->FindOverridenMethod( MethodName, OverridenInClass ) ) { return true; }
   }

   return false;
}

void clClass::CollectAbstractMethods( buffered_stream& Stub, clStringsList& AbstractMethods ) const
{
   for ( clBaseClassesList::const_iterator i = FBaseClasses.begin(); i != FBaseClasses.end(); ++i )
   {
      clClass* Base = FDatabase->GetClassPtr( i->FBaseClassName ); //(*FClassesDatabase)[ i->FBaseClassName ];

      if ( Base )
      {
         Base->CollectAbstractMethods( Stub, AbstractMethods );
      }
   }

   if ( g_EnableLogging )
   {
      Stub << "         // Class: " << FClassName << endl;
   }

   for ( clMethodsList::const_iterator i = FMethods.begin(); i != FMethods.end(); ++i )
   {
      if ( i->FAbstract )
      {
         // add new abstract
         clStringsList::iterator j = std::find( AbstractMethods.begin(), AbstractMethods.end(), i->FMethodName );

         if ( j == AbstractMethods.end() )
         {
            if ( g_ExportMethods && g_EnableLogging )
            {
               Stub << "            // Adding abstract: " << i->FMethodName << endl;
            }

            AbstractMethods.push_back( i->FMethodName );
         }
      }
      else
      {
         // override abstract
         clStringsList::iterator j = std::find( AbstractMethods.begin(), AbstractMethods.end(), i->FMethodName );

         if ( j != AbstractMethods.end() )
         {
            if ( g_ExportMethods && g_EnableLogging )
            {
               Stub << "            // Overriding abstract: " << i->FMethodName << endl;
            }

            AbstractMethods.erase( j );
         }
      }
   }

}

bool clClass::IsAbstract( buffered_stream& Stub ) const
{
   clStringsList AbstractMethods;

   CollectAbstractMethods( Stub, AbstractMethods );

   for ( clStringsList::const_iterator i = AbstractMethods.begin();
         i != AbstractMethods.end();
         ++i )
   {
      if ( g_ExportMethods && g_EnableLogging )
      {
         Stub << "      // Abstract method: " << ( *i ) << endl;
      }
   }

   return !AbstractMethods.empty();
}


// SaveToXLML for linear param writing
bool clClass::GenerateStdSaveCode( buffered_stream& Out )
{
   Out << "#if 0" << endl;

   string SaveName = "SaveToXLMLStream";

   Out << "void " << FClassName << "::" << SaveName << "(iXLMLWriter* XLMLStream)" << endl;
   Out << "{" << endl;

   Out << "   guard(\"" << FClassName << "::SaveToXLMLStream()\" );" << endl << endl;

   // first - the base class. TODO : check, if we are succeeding from iParametrizable
   string ovClass;

   if ( FindOverridenMethod( SaveName, ovClass ) )
   {
      // предварительное решение проблем с предками ?
//    if (ovClass != FClassName)
      {
         // get the direct base class
         string Base = FBaseClasses[0].FBaseClassName;

         Out << "\t" << Base << "::" << SaveName << "(XLMLStream);" << endl << endl;
      }
   }

   for ( size_t i = 0 ; i < FProperties.size() ; ++i )
   {
      string SaveCode = FProperties[i].GetSaveCode();

      if ( !SaveCode.empty() )
      {
         Out << "\t" << "XLMLStream->" << SaveCode << ";" << endl;
      }
      else
      {
         Out << "\t// NOTE: no getter for property \"" << FProperties[i].Name << "\"" << endl;
      }
   }

   Out << endl;
   Out << "   unguard();" << endl;

   Out << "}" << endl;

   Out << "#endif" << endl;

   return true;
}

// AcceptParameter for linear param reading
bool clClass::GenerateStdLoadCode( buffered_stream& Out )
{
   Out << "#if 0" << endl;

   Out << "void " << FClassName << "::AcceptParameter(const LString& ParamName, const LString& ParamValue)" << endl;
   Out << "{" << endl;

   Out << "   guard(\"" << FClassName << "::AcceptParameter(\" + ParamName + \", \" + ParamValue + \")\" );" << endl << endl;

   for ( size_t i = 0 ; i < FProperties.size() ; ++i )
   {
      string LoadCode = FProperties[i].GetLoadCode();

      if ( !LoadCode.empty() )
      {
         Out << "\t" << "if ( ParamName == LStr::GetUpper(\"" << FProperties[i].Name << "\"))" << endl;
         Out << "\t{" << endl;
         Out << "\t\t" << LoadCode << ";" << endl;
         Out << "\t} else" << endl;
      }
   }

   Out << "\t{" << endl;

   // at last, check the base class. TODO : check, if we are succeeding from iParametrizable
   string ovClass;
   bool WriteFatal = true;

   if ( FindOverridenMethod( "AcceptParameter", ovClass ) || FDatabase->InheritsFrom( FClassName, "iObject" ) )
   {
      if ( FBaseClasses.size() > 0 )
      {
         // get direct base class
         string Base = FBaseClasses[0].FBaseClassName;

         Out << "\t\t" << Base << "::AcceptParameter(ParamName, ParamValue);" << endl << endl;
         WriteFatal = false;
      }

      if ( FProperties.size() == 0 )
      {
         //
         WriteFatal = false;
      }
   }

   if ( WriteFatal )
   {
      // fatal section ?
      Out << "\t\tUnknownToken(ParamName, ParamValue);" << endl;
   }

   Out << "\t}" << endl << endl;

   Out << "   unguard();" << endl;

   Out << "}" << endl;

   Out << "#endif" << endl;

   return true;
}

bool clClass::IsDerivedFromNativeBase() const
{
   string TheRootestClass = FDatabase->GetRootestClassFor( FClassName );

   if ( TheRootestClass != "" )
   {
      return FDatabase->InheritsFrom( FClassName, TheRootestClass );
   }

   return false;
}

void clClass::WriteMethodRegistration( buffered_stream& Out, bool Tunneller ) const
{
   if ( !FMethods.empty() && g_ExportMethods )
   {
      // if FMethods.size() > this const, then we define a macro to reduce the code size
      const int MinNumOfMethodsForMacro = 3;

      bool WithMacro = static_cast<int>( FMethods.size() ) > MinNumOfMethodsForMacro;

      if ( WithMacro  && g_UseExportShortcuts )
      {
         Out << "   #define _RM__(Mtd) REG_CLS_MTD(" << FClassName << ",Mtd)" << endl;
      }

      for ( clMethodsList::const_iterator i = FMethods.begin(); i != FMethods.end(); ++i )
      {
         // static methods are also exported
         if ( /*(!i->FStatic) &&*/ ( i->FAccess == "public:" ) )
         {
            if ( !g_UseExportShortcuts )
            {
               // old code was the direct invocation
               Out << "   StaticClass->RegisterMethod( BindNativeMethod( &" << FClassName << ( Tunneller ? "_Tunneller" : "" ) << "::" << ( i->FMethodName ) << ", \"" << ( i->FMethodName ) << "\" ) );" << endl;
            }
            else
            {
               // now we use macros to reduce C++ file size
               if ( WithMacro )
               {
                  Out << "   _RM__(" << i->FMethodName << ")" << endl;
               }
               else
               {
                  // there are not much methods to define additional macro
                  Out << "   REG_CLS_MTD( " << FClassName << "," << i->FMethodName << " )" << endl;
               }
            }
         }
      }

      if ( WithMacro && g_UseExportShortcuts )
      {
         Out << "   #undef _RM__" << endl;
      }
   }
}

void clClass::WriteFieldHandlers( buffered_stream& Out ) const
{
   // do not generate field binders for Tunnellers

   if ( FFields.empty() ) { return; }

// Out << "/**" << endl;

   for ( clFieldsList::const_iterator i = FFields.begin(); i != FFields.end(); ++i )
   {
      if ( i->FAccess == "public:" )
      {
         if ( !i->FExcludeFromExport )
         {
            string ShortTypeName = FDatabase->CollapseTypeName( i->FFieldType );

            bool ClassExists = FDatabase->ClassExists( ShortTypeName );

            if ( ClassExists )
            {
//             string DeclaredIn = ProgramDatabase.GetClassRef(ShortTypeName).FDeclaredIn;
               clClass* Ptr = FDatabase->GetClassPtr( ShortTypeName );

               if ( !Ptr )
               {
                  cout << "Class not found in database: " << ShortTypeName << endl;
                  exit( 255 );
               }

               string DeclaredIn = Ptr->FDeclaredIn;
               // add the include
               Out << "#include \"" << DeclaredIn << "\"" << endl;
            }

            if ( ClassExists || FDatabase->IsScalarType( ShortTypeName ) || FDatabase->IsPODType( ShortTypeName ) )
            {
               Out << "DEFINE_NATIVE_POD_FIELD_BINDER(" << FClassName << ", " << i->FFieldName << ", " << ShortTypeName << ")" << endl;
            }
         }
         else
         {
            Out << "// Skipping excluded property: \"" << i->FFieldName << "\"" << endl;
         }
      }
      else if ( i->FAccess == "protected:" )
      {
         Out << "   // bypassing 'protected' property: \"" + i->FFieldName +  "\", \"" + i->FPropertyName + "\", \"" + i->FFieldType + "\"" << endl;
      }
      else if ( i->FAccess == "private:" )
      {
         Out << "   // bypassing 'private' property: \"" + i->FFieldName +  "\", \"" + i->FPropertyName + "\", \"" + i->FFieldType + "\"" << endl;
      }
   }

// Out << "*/" << endl;
}

void clClass::WriteFieldRegistration( buffered_stream& Out ) const
{
   if ( FFields.empty() ) { return; }

// Out << "/**" << endl;

   for ( clFieldsList::const_iterator i = FFields.begin(); i != FFields.end(); ++i )
   {
      if ( i->FAccess == "public:" )
      {
         if ( !i->FExcludeFromExport )
         {
            Out << "   BIND_NATIVE_FIELD(" << FClassName << ", " << i->FFieldName << ");" << endl;
         }
      }
   }

// Out << "*/" << endl;
}

/// called from 'GenerateEngineExports', after fields' registration
void clClass::WritePropertyRegistration( buffered_stream& Out ) const
{
   if ( FProperties.empty() ) { return; }

// Out << "/**" << endl;
   for ( clPropertiesList::const_iterator i = FProperties.begin(); i != FProperties.end(); ++i )
   {
      bool Save = i->Saveable();
      bool Load = i->Loadable();

      if (  Save && !Load && g_Verbose ) { std::cout << "NOTE: Property " << i->FClassName << "::" << i->Name << " has no Getter (write-only)" << std::endl; }

      if ( !Save &&  Load && g_Verbose ) { std::cout << "NOTE: Property " << i->FClassName << "::" << i->Name << " has no Setter (read-only)"  << std::endl; }

      if ( Save && Load )
      {
         Out << "   " << i->GetRegistrationCode()  << endl;
      }
   }

// Out << "*/" << endl;
}

/// generate Save and Load functions for the properties of this class
/// These function go right before the RegisterClass<i>() procedure
void clClass:: WritePropertyHandlers( buffered_stream& Out ) const
{
   if ( FProperties.empty() ) { return; }

// Out << "/**" << endl;

   for ( clPropertiesList::const_iterator i = FProperties.begin(); i != FProperties.end(); ++i )
   {
      if ( i->Saveable() && i->Loadable() )
      {
         Out << i->GetLoadSaveDeclarations() << endl;
      }
   }

// Out << "*/" << endl;

   Out << endl;
}

void clClass::WriteScriptConstructors( buffered_stream& Out ) const
{
   if ( FConstructors.empty() )
   {
      Out << "   // Autogenerated default constructor: " << FClassName << "()" << endl;
      return;
   }

   Out << "   // Constructors" << endl;

   for ( clMethodsList::const_iterator j = FConstructors.begin(); j != FConstructors.end(); ++j )
   {
      clMethod Method = ( *j );

      string Constructor = Method.FArgTypes.empty() ? "Default constructor: " : "Constructor: ";

      Out << "   // " << Constructor << Method.FMethodName << "(";

      for ( size_t i = 0; i != Method.FArgTypes.size(); ++i )
      {
         string TypeName = FDatabase->CollapseTypeName( Method.FArgTypes[i] );

         Out << TypeName << " P" << static_cast<int>( i );

         Out << ( ( i + 1 != Method.FArgTypes.size() ) ? ", " : "" );
      }

      Out << ")" << endl;
   }
}

void clClass::WriteScriptMethods( buffered_stream& Out ) const
{
   if ( FMethods.empty() ) { return; }

   Out << "   // Methods" << endl;

   for ( clMethodsList::const_iterator j = FMethods.begin(); j != FMethods.end(); ++j )
   {
      clMethod Method = ( *j );

      bool TemplateMethod = FDatabase->IsTemplateType( Method.FReturnType );

      for ( size_t i = 0; i != Method.FArgTypes.size(); ++i )
      {
         if ( FDatabase->IsTemplateType( Method.FArgTypes[i] ) ) { TemplateMethod = true; }
      }

      if ( TemplateMethod ) { continue; }

      string Modifier = "public";

      if ( Method.FAccess == "protected:" ) { Modifier = "protected"; }

      string TypeName = FDatabase->CollapseTypeName( Method.FReturnType );

      Out << "   " << Modifier;
      Out << ( Method.FAbstract ? " abstract " : "          " ) ;
      Out << ( Method.FStatic ?   "/*static*/" : "          " );
      Out << TypeName << " " << Method.FMethodName << "(";

      for ( size_t i = 0; i != Method.FArgTypes.size(); ++i )
      {
         string TypeName = FDatabase->CollapseTypeName( Method.FArgTypes[i] );

         string ParamName = Method.FArgNames[i];

         Out << TypeName;

         if ( ParamName.empty() )
         {
            Out << " P" << static_cast<int>( i );
         }
         else
         {
            Out << " " << ParamName;
         }

         Out << ( ( i + 1 != Method.FArgTypes.size() ) ? ", " : "" );
      }

      Out << ");" << endl;
   }
}

void clClass::WriteScriptProperties( buffered_stream& Out ) const
{
   if ( FProperties.empty() ) { return; }

   Out << "   // Properties" << endl;

   for ( clPropertiesList::const_iterator p = FProperties.begin(); p != FProperties.end(); ++p )
   {
      Out << p->GetScriptDeclaration() << endl;
   }
}

void clClass::WriteScriptFields( buffered_stream& Out ) const
{
   if ( FFields.empty() ) { return; }

   Out << "   // Fields" << endl;

   for ( clFieldsList::const_iterator f = FFields.begin() ; f != FFields.end() ; f++ )
   {
      Out << "   public " << f->GetScriptDeclaration() << endl;
   }
}

void clClass::WriteScriptDeclaration( buffered_stream& Out ) const
{
   string Extends = FBaseClasses.empty() ? "" : " extends " + FBaseClasses[0].FBaseClassName;

   Out << "// Declared within: " << FDeclaredIn << endl;

   if ( !Extends.empty() )
   {
      Out << "// Extends: " << endl;

      for ( clBaseClassesList::const_iterator j = FBaseClasses.begin(); j != FBaseClasses.end(); ++j )
      {
         Out << "//    Base class: " << j->FBaseClassName << "  (" << j->FInheritanceType << ")" << ( j->FVirtualBase ? "  <- virtual base" : "" ) << endl;
      }
   }

   Out << "native class " << FClassName << Extends << endl;
   Out << "{" << endl;

   // generating constructors list
   WriteScriptConstructors( Out );
   // generating methods list
   WriteScriptMethods( Out );
   // generating property list
   WriteScriptProperties( Out );
   // generating field list
   WriteScriptFields( Out );

   Out << "}" << endl;
   Out << endl;
}

void    clClass::GenerateInterfaceStub( const clClass* OriginalClass, buffered_stream& Stub, string& Access, clStringsList& AlreadyGeneratedMethods, const vector<string> NeverOverride ) const
{
   if ( FMethods.size() == 0 ) { return; }

   if ( Access != FMethods.begin()->FAccess )
   {
      Access = FMethods.begin()->FAccess;
      Stub << Access << endl;
   }

   Stub << "   //" << endl;
   Stub << "   // " << FClassName << " interface" << endl;
   Stub << "   //" << endl;

   bool GenerateOnlyPureVirtual = false;

   if ( find( NeverOverride.begin(), NeverOverride.end(), FClassName ) != NeverOverride.end() )
   {
      Stub << "      /*" << endl;
      Stub << "            The interface of this class was marked as \"native final\"." << endl;
      Stub << "            LSDC bypassed generation of stubs for it - only pure virtual methods are generated." << endl;
      Stub << "      */" << endl;

      GenerateOnlyPureVirtual = true;
      //         return;
   }

   for ( clMethodsList::const_iterator i = FMethods.begin();
         i != FMethods.end();
         ++i )
   {
      if ( i->FAccess == "private:" ) { continue; }

      if ( GenerateOnlyPureVirtual )
      {
         string OverridenInClass;

         if ( OriginalClass->FindOverridenMethod( i->FMethodName, OverridenInClass ) ||
              this->FindOverridenMethod( i->FMethodName, OverridenInClass ) )
         {
            AlreadyGeneratedMethods.push_back( i->FMethodName );

            continue;
         }
      }
      else
      {
         if ( Access != i->FAccess )
         {
            Access = i->FAccess;
            Stub << i->FAccess << endl;
         }
      }

      string MethodSignature = i->FMethodName;

      /*
      // DEBUG INFO
      if ( MethodSignature == "AcceptParameter" && OriginalClass->FClassName == "iActor" )
      {
      string t = i->FAccess;

      Stub << t;
      }
      */
      if ( find( AlreadyGeneratedMethods.begin(),
                 AlreadyGeneratedMethods.end(),
                 MethodSignature ) == AlreadyGeneratedMethods.end() )
      {
         if ( !i->FStatic )
         {
            i->GenerateMethodStub( OriginalClass, Stub );
         }

         AlreadyGeneratedMethods.push_back( MethodSignature );
      }
   }
}

void    clClass::GenerateClassInterface( const clClass* OriginalClass, buffered_stream& Stub, string& Access, clStringsList& AlreadyGeneratedMethods, const vector<string> NeverOverride ) const
{
   for ( clBaseClassesList::const_iterator j = FBaseClasses.begin(); j != FBaseClasses.end(); ++j )
   {
      clClass* Base = FDatabase->GetClassPtr( j->FBaseClassName ); //(*FClassesDatabase)[ j->FBaseClassName ];

      if ( Base )
      {
         Base->GenerateClassInterface( OriginalClass, Stub, Access, AlreadyGeneratedMethods, NeverOverride );
      }
   }

   GenerateInterfaceStub( OriginalClass, Stub, Access, AlreadyGeneratedMethods, NeverOverride );
}

/**
   Generates engine's class export code
   Tunneller flag automatically adds "_Tunneller" suffix to the class

   Returns boolean flag indicating whether the actual export code was generated
**/
bool    clClass::GenerateEngineExports( buffered_stream& Stub, bool Tunneller ) const
{
   bool Abstract = Tunneller ? false : IsAbstract( Stub );

   // generate field binders
   if ( !Tunneller ) { WriteFieldHandlers( Stub ); }

   // generate property handlers for serialization (save/load functions for each property)
   if ( !Tunneller ) { WritePropertyHandlers( Stub ); }

   // generate registration method
   Stub << endl;

	string ConstructorParams = "";

	if ( !HasDefaultConstructor() )
	{
		for ( size_t i = 0; i != FConstructors[0].FArgTypes.size(); i++ )
		{
			ConstructorParams += ", " + FConstructors[0].FArgTypes[i];
		}
	}

   Stub << "void RegisterPackage" << FPackage->FPackageName << "Class" << FPackage->FPackagesProcsCounter++ << "(sEnvironment* Env)" << endl;
   Stub << "{" << endl;
	if ( Tunneller ) Stub << "#if !defined(_DISABLE_TUNNELLERS_)" << endl;
   Stub << "   iStaticClass* StaticClass = new clNative" << ( Abstract ? "Abstract" : "" ) << "StaticClass";
	if ( !HasDefaultConstructor() ) Stub << FConstructors[0].FArgTypes.size();
	Stub << "<" << FClassName << ( Tunneller ? "_Tunneller" : "" ) << ConstructorParams << " >;" << endl;
   Stub << endl;
   Stub << "   StaticClass->Env = Env;" << endl;
   Stub << endl;

 	Stub << "#if !defined(_DISABLE_METHODS_)" << endl;

   // register methods
   WriteMethodRegistration( Stub, Tunneller );

	Stub << "#endif // _DISABLE_METHODS_" << endl;	

   // register fields
   if ( !Tunneller ) { WriteFieldRegistration( Stub ); }

   // register properties
   if ( !Tunneller ) { WritePropertyRegistration( Stub ); }

   // assign superclass
   if  ( FBaseClasses.begin() != FBaseClasses.end() )
   {
      Stub << "   StaticClass->SetSuperClassName( \"" << FBaseClasses.begin()->FBaseClassName << "\" );" << endl;
      Stub << endl;
   }

   Stub << "   Env->Linker->RegisterStaticClass( StaticClass );" << endl;
	if ( Tunneller ) Stub << "#endif // _DISABLE_TUNNELLERS_" << endl;
   Stub << "}" << endl;

   // yes, some code was generated
   return true;
}

bool    clClass::GenerateClassStub( const vector<string>& NeverOverride ) const
{
   // check if this class should be excluded
   if ( FScriptFinal /* ExcludeClassTunneller( FClassName )*/ )
   {
      return false;
   }

   // look for default contructor
/*   if ( !HasDefaultConstructor() )
   {
      cout << "WARNING: class " << FClassName << " has no default constructor - bypassing" << endl;

      return false;
   }
*/
	const string StubFileName = FClassName + "_Tunneller.h";
	const string FullFileName = g_PackTunnellers ? 
		FPackage->GetScriptExportDir() + "/" + FPackage->FPackageName + "_Tunnellers.h" :
		FPackage->GetScriptExportDir() + "/" + StubFileName;

   buffered_stream Stub( FullFileName.c_str(), g_PackTunnellers );

   if ( !g_PackTunnellers ) Stub.WriteDoxygenHeader( StubFileName, "Tunneller for class: " + FClassName );

   for ( clBaseClassesList::const_iterator j = FBaseClasses.begin(); j != FBaseClasses.end(); ++j )
   {
      Stub        << "//                             Base class: " << j->FBaseClassName << "  (" << j->FInheritanceType << ")" << ( j->FVirtualBase ? "  <- virtual base" : "" ) << endl;
   }

   Stub << endl;
	if ( !g_PackTunnellers ) Stub << "#pragma once" << endl;
   Stub << endl;
   Stub.Include( FDeclaredIn );
   Stub << endl;
   Stub << "#if !defined( _DISABLE_TUNNELLERS_ )" << endl;
   Stub << endl;
   Stub << "class " << FClassName << "_Tunneller: public " << FClassName << endl;
   Stub << "{" << endl;

	if ( !HasDefaultConstructor() )
	{
		Stub << "public:" << endl;
		Stub << "   " << FClassName << "_Tunneller(";

		for ( size_t i = 0; i != FConstructors[0].FArgTypes.size(); i++ )
		{
			Stub << FConstructors[0].FArgTypes[i] << " P" << i;
			if ( i != FConstructors[0].FArgTypes.size()-1 ) Stub << ", ";
		}

		Stub << ") : " << FClassName << "(";

		for ( size_t i = 0; i != FConstructors[0].FArgTypes.size(); i++ )
		{
			Stub << "P" << i;
			if ( i != FConstructors[0].FArgTypes.size()-1 ) Stub << ", ";
		}

		Stub << ") {};" << endl;
	}

   string Access = "";

   clStringsList AlreadyGeneratedMethods;

   GenerateClassInterface( this, Stub, Access, AlreadyGeneratedMethods, NeverOverride );

   Stub << "};" << endl;
   Stub << endl;
   Stub << "#endif // _DISABLE_TUNNELLERS_" << endl;
   Stub << endl;
   Stub << "/*" << endl;
   Stub << " * " << LSDCDate << endl;
   Stub << "     Autogenerated via " << LSDCName << endl;
   Stub << "*/" << endl;

   return true;
}


string clField::FromString( const string& F )
{
   size_t TypePos = F.find( "nativefield " );

   string CleanLine = TrimSpaces( F.substr( TypePos + 12 ) );

   size_t SpacePos = CleanLine.find_last_of( ' ' );

   FFieldName    = TrimSpaces( CleanLine.substr( SpacePos ) );
   FFieldType    = TrimSpaces( CleanLine.substr( 0, SpacePos ) );
   FConst        = ( CleanLine.find( "const " ) != -1 );

   FExcludeFromExport = false;

   if ( FFieldName.length() > 0 && FFieldName[FFieldName.length()-1] == ';' ) { FFieldName = FFieldName.substr( 0, FFieldName.length() - 1 ); }

   return "";
}

string clField::ToString() const
{
   return string( "nativefield " ) + FFieldType + string( " " ) + FFieldName;
}

string clField::GetScriptDeclaration() const
{
   return FDatabase->CollapseTypeName( FFieldType ) + string( " " ) + FFieldName + string( ";" );
}
