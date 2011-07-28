/**
 * \file test.cpp
 * \brief
 *
 * LinderScript Database Compiler
 *
 * \version 0.8.11
 * \date 20/01/2011
 * \author Sergey Kosarevsky, 2005-2011
 * \author Viktor Latypov, 2007-2011
 * \author support@linderdaum.com http://www.linderdaum.com
 */

#include "Property.h"
#include "MethodArg.h"

#include "MethodBinding.h"
#include "Macro.h"

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#ifdef _WIN32
#include <conio.h>
#endif

#include "Package.h"
#include "BufStream.h"

#define _TRACE(X) cout << X << endl; cout.flush();

void test_package( const string& InDirName )
{
// _TRACE("Dumping converter table contents")
// DB.DumpNETTypeMap("Debug_NETTypes.list");
// DB.DumpStringConverters("Debug_StringCvt.list");

   clDatabase DB;

   clPackage* Pack = DB.ProcessPackageDirectory( InDirName );

   Pack->GenerateStatistics();

   Pack->GenerateStuff();
}

void test_property( const string& s )
{
   clProperty p;

   p.SetParam( "Description", "test  test" );

   p.FromString( s );

   cout << p.ToString() << endl;

   cout << endl << endl;

// cout << p.GetIndexerStuffDefinition() << endl;
   cout << p.GetLoadCode() << endl << endl;
   cout << p.GetSaveCode() << endl << endl;
}

void test_method_arg( const string& s )
{
   clMethodArg arg;

   string _s = s;

   if ( arg.FromString( _s ) )
   {
      cout << arg.ToString() << endl;
   }
   else
   {
      cout << "Error while parsing argument: " << s << endl;
   }
}

void test_properties()
{
   string s = "// Property(Name=Shader, Type=string, Validator = \"SomeValidator\", Getter=GetShader, Setter=SetShader, Description=\"Test description\", Category=\"Category of the property\")";
   test_property( s );

   string s2 = "// Property(Name=\"Visible\", Type=bool, Validator = \"<default>\", Getter=GetVisible, Setter=SetVisible, Description=\"Test description\", Category=\"Category of the property\")";
   test_property( s2 );

   //test_method_arg("MarshalAsArray(NumBytes) BYTE* Bytes");
}

void test_packages()
{
   // read main package
   test_package( "../../Src" );
}

void test_binders()
{
   std::ofstream Out( "MethodBind.h" );
   GenerateMethodBinders( Out, 10 );
}

void test_macros()
{
   string Name = "SCALAR_PROPERTY_SAVE__GETTER";
   std::vector<string> Params;

   Params.push_back( "TheFieldName" );
   Params.push_back( "TheGetter"    );
   Params.push_back( "TheClassName" );
   Params.push_back( "ToStringConverter" );

   string Text = "mlNode* SaveScalarField_##TheClassName##_##TheFieldName##_GETTER(iObject* Obj)   \n\
{  \n\
   LString Value = ToStringConverter(dynamic_cast< TheClassName *>(Obj)-> TheGetter () ); \n\
   mlParamNode* Result = new mlParamNode(#TheFieldName , Value ); \n\
   /*Result->setValue( Value );*/   \n\
   return Result; \n\
}";

   clMacroDef Macro( Name, Params, Text );

   std::vector<string> Values;

   Values.push_back( "Vec3" );
   Values.push_back( "GetVector3" );
   Values.push_back( "clCVar" );
   Values.push_back( "LStr::Vec3ToString" );

   string Res = Macro.GenerateInstance( Values );

   std::ofstream Out( "MacroTestOut.h" );

   Out << Res;
}

int main( int argc, char** argv )
{
   test_macros();

   //test_properties();

// test_packages();

   //test_binders();

#ifdef _WIN32
// while(!_kbhit());
#endif

   return 0;
}
