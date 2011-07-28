/**
 * \file DotNET.cpp
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

#include <algorithm>
#include <iostream>
using std::cout;
using std::endl;

#include "BufStream.h"

#include "Package.h"

// TODO : support dependant packages through FDependsOn list
void clPackage::GenerateDotNETWrappers()
{
   map<string, clClass>::iterator i;

   string OutDirPrefix = FPackageOutDirectory + "/" + NETDirName + "/";
   string FileNameBase = FPackageNetName;
   string NETHeaderName = FileNameBase + string( ".h" );
   string NETSourceName = FileNameBase + string( ".cpp" );

   // 1. ManagedAPI.h, ref classes
   buffered_stream Out( ( OutDirPrefix + NETHeaderName ).c_str() );

	Out.WriteDoxygenHeader( NETHeaderName, ".NET wrappers header" );

   // some header
   Out << "#ifndef __" << FPackageNetName << "_h__included__" << endl;
   Out << "#define __" << FPackageNetName << "_h__included__" << endl << endl;

   Out << "// Disable the warning caused by inappropriate 'override' flags for some methods";
   Out.endl(); // << endl;
   Out << "#pragma warning(disable:4490)" << endl;

   Out << endl;

   Out << "using namespace System::ComponentModel;" << endl;
   Out << "using namespace System::Globalization;" << endl;

   Out << endl;

   // native includes
   map<string, int> NativeIncludes;

   // avoid duplicates
   Out << "#pragma unmanaged" << endl;

   for ( i = FClasses.begin(); i != FClasses.end(); ++i )
   {
      clClass Class = i->second;

      if ( Class.FNetExportable )
      {
         NativeIncludes[Class.FDeclaredIn] = 1;
      }
   }

   for ( map<string, int>::iterator j = NativeIncludes.begin() ; j != NativeIncludes.end() ; j++ )
   {
      if ( j->first != "" )
      {
         Out << "#include \"" << j->first << "\"" << endl;
      }
   }

   Out << "#pragma managed" << endl;

   Out << endl;

   Out << "#using <mscorlib.dll>" << endl;

   // TODO : custom includes, just like in exports
   Out << "#include \"NET_BasicTypes.h\"" << endl;
   Out << "#include \"NET_TypeConverters.h\"" << endl;
   Out << "#include \"NET_Macros.h\"" << endl;

   Out << endl;

   // TODO : normal namespace name
   Out << "namespace LinderdaumNET" << endl;
   Out << "{" << endl;

   vector<string> SortedClasses;

   // 1. fetch exported class list
   for ( i = FClasses.begin(); i != FClasses.end(); ++i )
   {
      clClass& Class = i->second;
      string name = Class.FClassName;

      if ( name != "" && Class.FNetExportable )
      {
         Class.FNetExportable = true;

         Class.CollectBaseClasses( SortedClasses );
      }
   }

   /*
      cout << "Sorted class list: " << endl;
      for(size_t jj = 0 ; jj < SortedClasses.size() ; jj++)
      {
         cout << SortedClasses[jj] << endl;
      }
      cout << "================== " << endl;
   */

   Out << "\t/// Forward declarations for DECLARE_PROPERTY macros";
   Out.endl();
   Out.endl(); // << endl << endl;
   // forward class declarations
   for ( size_t j = 0 ; j < SortedClasses.size() ; j++ )
   {
      clClass* Class = FDatabase->GetClassPtr( SortedClasses[j] ); //FClasses[SortedClasses[j]];

      if ( Class->FNetExportable )
      {
         Out << "\tref class " << Class->FClassName << ";" << endl;
      }
   }

   Out.endl(); // << endl;

   Out.endl(); // << "\t// .NET wrapper class declarations"; Out.endl(); // << endl << endl;
   // wrapper for each class
   for ( size_t j = 0 ; j < SortedClasses.size() ; j++ )
   {
      clClass* Class = FDatabase->GetClassPtr( SortedClasses[j] );

      if ( Class->FNetExportable )
      {
         Class->GenerateDotNETRefClass( Out );
      }
   }

   // some footer
   Out << endl << endl;
   Out << "}  // namespace LinderdaumNET" << endl;
   Out << endl << endl;
   Out << "#endif  // #ifndef " << FPackageNetName << "_Included" << endl;

// Out.write(); -- this is done automatically in the destructor

   // 2. ManagedAPI.cpp, ref classes method stubs
   buffered_stream Out2( ( OutDirPrefix + NETSourceName ).c_str() );

   // some header
   Out2 << "#include \"" << NETHeaderName << "\"" << endl << endl;

   Out2 << "#using <mscorlib.dll>" << endl << endl;

   Out2 << "using namespace System;" << endl << endl;

   // TODO : insert references to the other packages (dependencies)

   Out2 << "namespace LinderdaumNET" << endl;
   Out2 << "{" << endl;
   Out2 << endl;

   // first some code for downcasters
   Out2 << "#define CLASS_NAME_CHECKER(TheClass) \\" << endl;
   Out2 << "\tif (LocalClassName == #TheClass)\\" << endl;
   Out2 << "\t{\\" << endl;
   Out2 << "\t\tif (this->GetStaticClass()->InheritsFrom( #TheClass))	\\" << endl;
// Out2 << "\t\t\tres = Activator::CreateInstance< TheClass >();\\" << endl;
   Out2 << "\t\t\tres = gcnew TheClass ( dynamic_cast<:: TheClass *> (this->GetNativeObject()) ); \\" << endl;
   Out2 << "\t}" << endl << endl;

   // then we export each class
   for ( i = FClasses.begin(); i != FClasses.end(); ++i )
   {
      clClass& Class = i->second;

      if ( Class.FNetExportable )
      {
         Class.GenerateDotNETRefClassImpl( Out2 );

//       cout << "Generating .NET wrapper for class " << Class.FClassName << endl;

         /// downcaster
         if ( InheritsFrom( Class.FClassName, "iObject" ) )
         {
            GenerateNETDowncasterImpl( Class, Out2 );
            Out2 << endl;
         }
      }
      else
      {
//       cout << "Class " << Class.FClassName << " is not .NET-exportable" << endl;
      }
   }

   Out2 << "#undef CLASS_NAME_CHECKER" << endl;

   // and then undeclare the macro for downcaster

   // some footer
   Out2 << "} // namespace LinderdaumNET" << endl << endl;
}

void clPackage::GenerateNETDowncasterImpl( clClass& C, buffered_stream& Out )
{
   Out << "System::Object^ " << C.FClassName << "::TotalDowncast()" << endl;

   Out << "{" << endl;

   vector<string> Checkers;

   Out << "\tLString LocalClassName = GetNativeObject()->NativeClassName();" << endl << endl;

   // iterate all classes from DB
   for ( map<string, clClass>::iterator i = FClasses.begin(); i != FClasses.end(); ++i )
   {
      clClass& C2 = i->second;

      if ( !C2.FNetExportable ) { continue; }

      if ( C2.FClassName == C.FClassName ) { continue; }

      if ( InheritsFrom( C2.FClassName, C.FClassName ) )
      {
         string Ch = string( "\tCLASS_NAME_CHECKER(" ) + C2.FClassName + string( ")" );

         Checkers.push_back( Ch );
      }
   }

   if ( !Checkers.empty() )
   {
      Out << "\t// 1. empty new object" << endl;
      Out << "\tSystem::Object^ res = nullptr;" << endl << endl;

      Out << "\t// 2. Comparison" << endl;

      for ( size_t j = 0 ; j < Checkers.size() ; j++ )
      {
         Out << Checkers[j] << endl;
      }

      Out << endl;
      Out << "\t// 3. assign native object" << endl;
      Out << "\tif (res != nullptr)" << endl;
      Out << "\t{" << endl;
      // Out << "\t\t((iObject)res)->FNativeObject = this->GetNativeObject();" << endl << endl;
      Out << "\t\t(cli::safe_cast<iObject^>(res))->FNativeObject = this->GetNativeObject();" << endl << endl;
      Out << "\t\treturn res;" << endl;
      Out   << "\t}" << endl << endl;
   }

   Out << "\treturn this;" << endl;

   Out << "}" << endl;
}

bool clClass::GenerateDotNETRefClass( buffered_stream& Out )
{
   bool NoIndexedProperties = true;

   for ( size_t i = 0 ; i != FProperties.size(); i++ )
   {
      if ( FProperties[i].IsIndexed() ) { NoIndexedProperties = false; }
   }

   if ( FClassName == "" )
   {
//    cout << "Strange empty class name" << endl;
      return false;
   }

   string FullNativeType = string( "::" ) + FClassName; // TODO : get namespace information

   Out << "\tpublic ref class " << FClassName;

   if ( FBaseClasses.size() > 0 )
   {
      string TheRoot = FBaseClasses[0].FBaseClassName;

      if ( FDatabase->IsWrappedClass( TheRoot ) )
      {
         // derive from the first one
         Out << " : " << TheRoot;
      }
   }

   Out.endl(); // << endl;

   Out << "\t{";
   Out.endl(); // << endl;

   string TheRootestClass = FDatabase->GetRootestClassFor( FClassName );

   string WrapperType = "WRAPPER_CLASS_CONSTRUCTOR_HEADER";

   bool IsDerived = true;

   if ( IsDerivedFromNativeBase() && !( TheRootestClass == FClassName ) )
   {
      IsDerived = true;
      WrapperType = "WRAPPER_DERIVED_CLASS_CONSTRUCTOR_HEADER";
   }

   Out << "\t\t" << WrapperType << "(" << FullNativeType << ", " << FClassName << ")";
   Out.endl(); // << endl;

   /*
      // check if we are the root of hierarchy - this is embedded to WRAPPER_CLASS macro
      if (TheRootestClass == FClassName)
      {
         // emit the declaration of NativeObject
         Out << "\tpublic:" << endl;
         Out << "\t\t::" << TheRootestClass << "* FNativeObject;" << endl;
      }
   */
   Out << "\t\tWRAPPER_CLASS_NATIVE_OBJECT(" << FullNativeType << ", " << FClassName << ")";
   Out.endl(); // << endl;
   Out << "\t\tWRAPPER_CLASS_DEFAULT_CONSTRUCTOR(" << FClassName << ")";
   Out.endl(); // << endl;

   if ( NoIndexedProperties )
   {
      Out << "\t\tWRAPPER_CLASS_NO_INDEXERS";
      Out.endl(); // << endl;
   }

   if ( FProperties.size() > 0 )
   {
      Out.endl(); // << endl;
   }

   /// downcaster
   if ( FDatabase->InheritsFrom( FClassName, "iObject" ) )
   {
      Out << "\t\tSystem::Object^    TotalDowncast();" << endl << endl;
   }

   // generate indexed property initializer
   for ( size_t j = 0 ; j < FProperties.size() ; j++ )
   {
      Out << "\t\t" << FProperties[j].DeclareNETProperty();
      Out.endl(); // << endl;
   }

   if ( !NoIndexedProperties )
   {
      // generate InitInternal method
      Out << "\t\tSystem::Void InitInternal()";
      Out.endl(); // << endl;
      Out << "\t\t{";
      Out.endl(); // << endl;

      for ( size_t j = 0; j < FProperties.size() ; j++ )
      {
         if ( FProperties[j].IsIndexed() )
         {
            Out << "\t\t\t" << FProperties[j].GetIndexerStuffInitialization();
            Out.endl(); // << endl;
         }
      }

      Out << "\t\t}";
      Out.endl(); // << endl;
   }

   int MethodCount = 0;
   // determine maximum length of the return type name
   int MaxRetType = 0;

   for ( clMethodsList::iterator i = FMethods.begin(); i != FMethods.end(); ++i )
   {
      i->FExportedToNET = i->CanExportToNET();

      if ( !i->FExportedToNET ) { continue; }

      int len = i->GetNETReturnTypeLength();

      if ( len > MaxRetType ) { MaxRetType = len; }

      MethodCount++;
   }

   if ( MethodCount > 0 )
   {
      Out.endl(); // << endl;
   }

   // now the method list
   for ( clMethodsList::iterator i = FMethods.begin(); i != FMethods.end(); ++i )
   {
//    Out << "\t\t" << endl;
//    i->GenerateMethodDotNETImpl( Out );
      if ( i->FExportedToNET )
      {
         i->GenerateMethodDotNETHeader( Out, MaxRetType );
      }
   }

   // and finally, two helpers

   Out << "\t};";
   Out.endl();
   Out.endl(); // << endl << endl;

   return true;
}

string clMethod::MakeNetArgList( string& Result ) const
{
   Result = "";

   for ( size_t i = 0 ; i != FArgNames.size() ; i++ )
   {
      if ( !Result.empty() ) { Result += string( ", " ); }

      string Modifier = ""; // % is required ?

      string SType = TrimSpaces( FDatabase->StripTypeName( TrimSpaces( FArgTypes[i] ) ) );

      if ( FDatabase->IsOutParameter( FArgTypes[i] ) )
      {
         Modifier = "%";
      }

      string TheType = FDatabase->GetAppropriateNetTypeForParameter( SType );

      if ( TheType == "" )
      {
         Result = "";
         return string( "Cannot map " ) + SType;
      }

      Result += TheType + Modifier + string( " " );

      Result += FArgNames[i];
   }

   return "";
}

// length of the type name
int clMethod::GetNETReturnTypeLength() const
{
   string DotNetReturnType = FDatabase->GetAppropriateNetTypeForParameter( FDatabase->StripTypeName( FReturnType ) );

   if ( DotNetReturnType == "" )
   {
      return 0;
   }

   return static_cast<int>( DotNetReturnType.length() );
}

bool clMethod::GenerateNETMethodSignature( buffered_stream& Out, int MaxRetType, bool UseClassName, bool UseStatic ) const
{
   string NETArgList;

   string Err = MakeNetArgList( NETArgList );

   if ( Err != "" )
   {
      cout << "Error generating header for " << FMethodName << ": " << Err;
      Out.endl(); // << endl;
      return false;
   }

   string DotNetReturnType = FDatabase->GetAppropriateNetTypeForParameter( FDatabase->StripTypeName( FReturnType ) );

   if ( DotNetReturnType == "" )
   {
      return false;
   }

   if ( FAccess == "public:" )
   {
      if ( !UseClassName )
      {
         Out << "\t\t";

         // it is a declaration in the class, so we add 'virtual' at the beginning, if needed

         // static methods support
         if ( UseStatic )
         {
            Out << "static  ";
         }
         else
         {
            Out << "virtual ";
         }
      }

      Out << DotNetReturnType;
      // padding spaces:
      int Len = static_cast<int>( DotNetReturnType.length() );

      if ( MaxRetType > Len )
      {
         for ( int i = 0 ; i < MaxRetType - Len ; i++ )
         {
            Out << " ";
         }
      }

      // some more spaces
      Out << "    ";

      if ( UseClassName )
      {
         Out << FClassName << "::";
      }

      Out << FMethodName << "(" << NETArgList << ")";

      if ( !UseClassName )
      {
         if ( !UseStatic )
         {
            /*
            // it is a declaration in the class, so we add 'override' at the end, if needed
            clClass& cl = ProgramDatabase.GetClassRef(FClassName);

            string ovClass;
            if (cl.FindOverridenMethod(FMethodName, ovClass))
            {
               Out << " override";
            }
            */

            // add override in any case
            Out << " override";
         }
      }

      return true;
   }

   return false;
}

bool clMethod::CanExportToNET() const
{
   // check each argument
   for ( size_t i = 0 ; i < FArgTypes.size() ; i++ )
   {
      string t = TrimSpaces( FDatabase->StripTypeName( TrimSpaces( FArgTypes[i] ) ) );

      if ( FDatabase->IsTemplateType( t ) ) { return false; }

      // the pointer to void
      if ( t == "void" ) { return false; }

      if ( !FDatabase->MapsToNET( t ) ) { return false; }
   }

   string ret = TrimSpaces( FDatabase->StripTypeName( TrimSpaces( FReturnType ) ) );

   if ( FDatabase->IsTemplateType( ret ) ) { return false; }

   if ( ret == "void" && FDatabase->IsPointer( TrimSpaces( FReturnType ) ) ) { return false; }

   if ( !FDatabase->MapsToNET( ret ) ) { return false; }

   return true;
}

bool clMethod::GenerateMethodDotNETHeader( buffered_stream& Out, int MaxRetType ) const
{
   if ( GenerateNETMethodSignature( Out, MaxRetType, false, FStatic ) )
   {
      Out << ";";
      Out.endl(); // << endl;
      return true;
   }

   return false;
}

bool clMethod::GenerateMethodDotNETImpl( buffered_stream& Out ) const
{
   /*
      FIXME : if this line is changed to !this->CanExportToNET(),
              then the output is missing methods like clCVar::SetString() etc
            if the line remains, methods like clViewport::GetScreenshot() [with void* parameters] remain
   */
// if (!this->FExportedToNET) return false;
   // 1. method signature

   if ( !GenerateNETMethodSignature( Out, 0, true, false ) ) { return false; }

   // 2. opening bracket
   Out.endl(); // << endl
   Out << "{";
   Out.endl(); // << endl;

   string LocalParamList = "";
// FROM_NET_OBJECT(clVirtualTrackball, Param0, Trackball);

   // 3. parameter conversion code
   for ( size_t i = 0 ; i != FArgNames.size() ; i++ )
   {
      if ( LocalParamList != "" ) { LocalParamList += ", "; }

      string ParamVarName = string( "Param" ) + Int2Str( i );

      string ArgType = TrimSpaces( FDatabase->StripTypeName( TrimSpaces( FArgTypes[i] ) ) );
      string ConversionCode = FDatabase->GetNetToNativeConversion( FArgTypes[i], ParamVarName, FArgNames[i], ArgType );
      /*
            string PtrSign = "";
            if (IsPointer(FArgTypes[i])) PtrSign = "*";

            Out << "\t" << ArgType << PtrSign << " Param" << i << " = " << ConversionCode << ";" << endl;
      */
      Out << "\t" << ConversionCode << ";";
      Out.endl(); // << endl;

      LocalParamList += ParamVarName;
   }

   // 4. trailing end_of_line, if needed
   if ( FArgNames.size() > 0 ) { Out.endl(); } // << endl;

   Out << "\t";

   // 5. result var declaration
   bool VoidResult = ( FReturnType == "void" );

   if ( !VoidResult )
   {
      string TheNamespace = "";

      string ret = TrimSpaces( FDatabase->StripTypeName( TrimSpaces( FReturnType ) ) );

      if ( FDatabase->IsWrappedClass( ret ) ) // avoid confusion of .NET and Native types
      {
         TheNamespace = "::"; // TODO : get some map NativeNamespaces[<TypeName>]
      }

      // TODO : insert namespace of the NativeType(FReturnType)
      Out << /*<< Here comes the namespace << */ TheNamespace << FReturnType << " NativeResult = ";

      // if the type is 'scalar', then insert the explicit cast
      if ( FDatabase->IsScalarType( FReturnType ) )
      {
         Out << "(" << FReturnType << ")";
      }
   }

   // 6. invocation code

   // final variant with precached native object

   if ( FStatic )
   {
      Out << "::" << FClassName << "::";
   }
   else
   {
      Out << "GetNativeObject()->";
   }

   Out << FMethodName << "(" << LocalParamList << ");";
   Out.endl(); // << endl;

   /*
   // second variant
   string ActualNativeType = string("::") + FClassName; // TODO : determine namespace
   Out << "dynamic_cast<" << ActualNativeType << "*>(FNativeObject)->" << FMethodName << "(" << LocalParamList << ");" << endl;

   // first variant:
   Out << "FNativeObject->" << FMethodName << "(" << LocalParamList << ");" << endl;
   */

   bool HasLineBreak = false;

   // 7. If there are any out parameters, convert them back
   for ( size_t i = 0 ; i != FArgNames.size() ; i++ )
   {
      if ( FDatabase->IsOutParameter( FArgTypes[i] ) )
      {
         string ArgType = TrimSpaces( FDatabase->StripTypeName( TrimSpaces( FArgTypes[i] ) ) );

         string ConvCode = FDatabase->GetNativeToNetConversion( string( "Param" ) + Int2Str( i ), ArgType );

         if ( !HasLineBreak )
         {
            HasLineBreak = true;
            Out.endl(); // << endl;
         }

         Out << "\t" << FArgNames[i] << " = " << ConvCode << ";";
         Out.endl(); // << endl;
      }
   }

   // 8. if the result is meaningful, convert it to native and return

   if ( !VoidResult )
   {
      // one more line break after step 7
//    if (!HasLineBreak) Out << endl;

      string RetType = FDatabase->StripTypeName( TrimSpaces( FReturnType ) );

      Out << "\treturn " << FDatabase->GetNativeToNetConversion( "NativeResult", RetType ) << ";";
      Out.endl(); // << endl;
   }

   Out << "}";
   Out.endl();
   Out.endl(); // << endl << endl;

   return true;
}

bool clClass::GenerateDotNETRefClassImpl( buffered_stream& Out )
{
   // methods
   for ( clMethodsList::iterator i = FMethods.begin(); i != FMethods.end(); ++i )
   {
      if ( i->CanExportToNET() ) //i->FExportedToNET)
      {
         i->GenerateMethodDotNETImpl( Out );
      }
   }

   return true;
}

/*
 * 06/04/2010
     Functional .NET exporting mechanism
 * 02/05/2009
     It's here
*/
