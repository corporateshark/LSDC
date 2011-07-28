/**
 * \file MethodBinding.h
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

#include "Utils.h"

#include <iostream>
#include <string>

using std::string;

const string BinderClassName = "iMethod"; // "clNativeMethod";

void WriteItemList( std::ostream& Out, const string& NonZeroPrefix, const string& ItemPrefix, bool NewLine, int ItemCount )
{
   for ( int i = 0; i != ItemCount; ++i )
   {
      if ( i != 0 ) { Out << NonZeroPrefix; }

      Out << ItemPrefix << "P" << i;

      if ( i != ItemCount - 1 )
		{
			Out << ","; 
			if ( NewLine ) { Out << endl; }
		};
   }
}

void GenerateBinder( std::ostream& Out, bool Static, bool Function, int ParamsCount, bool Const, bool Volatile )
{
   Out << endl;
   Out << endl;
   Out << "//////////////////" << endl;
   Out << "// Params count: " << ParamsCount << endl;
   Out << "//////////////////" << endl;

   int SpaceCount = 19;

   if ( Function ) { SpaceCount = 40; }

   if ( Static ) { SpaceCount -= 9; }

   // generate template header
   if ( !( Static && ParamsCount == 0 && !Function ) )
   {

      Out << "template <" << ( ( Static ) ? "" : "class T" );

      if ( Function ) { Out << ( ( Static ) ? "" : ", " ) << "typename ReturnType"; }

      if ( !Static || Function ) { Out << ( ( ParamsCount > 0 ) ? ", " : "" ); }

      WriteItemList( Out, MultiSpace( SpaceCount ), "typename ", true, ParamsCount );

      Out << ">" << endl;

   }

   //
   if ( ( !Function ) && ( Static ) && ( ParamsCount == 0 ) )
   {
      Out << "template <>" << endl;
   }

   Out << "class clNative" << ( ( Static ) ? "Static" : "" ) << "MethodParams" << ParamsCount << "_Func" << ( ( Const ) ? "_Const" : "" ) << ( ( Volatile ) ? "_Volatile" : "" );

   if ( !Function )
   {
      Out << "<";
      Out << ( ( Static ) ? "" : "T, " );

      Out << "void";

      Out << ( ( ParamsCount > 0 ) ? ", " : "" );

      WriteItemList( Out, MultiSpace( SpaceCount ), "", true, ParamsCount );

      Out << ">";
   }

   Out << ": public " << BinderClassName << endl;
   Out << "{" << endl;
   Out << "private:" << endl;

   // generate method prototype
   Out << "   typedef " << ( ( Function ) ? "ReturnType" : "void" ) << " (" << ( ( Static ) ? "" : "T::" ) << "*MethodPtr)(";

   WriteItemList( Out, "", "", false, ParamsCount );

   Out << ")" << ( ( Const ) ? " const" : "" );
   Out << ( ( Volatile ) ? " volatile" : "" ) << ";" << endl;

   if ( Function )
   {
      Out << "   typedef typename TypeTraits<ReturnType>::ReferredType ReturnValueType;" << endl;
      Out << "   typedef typename TypeTraits<ReturnValueType>::NonConstType ReturnNonConstType;" << endl;
      Out << "   typedef typename TypeTraits<ReturnValueType>::UnqualifiedType ReturnUnqualifiedType;" << endl;
   }

   //
   Out << "   MethodPtr    FMethodPtr;" << endl;
   Out << "public:" << endl;
   Out << "   clNative" << ( ( Static ) ? "Static" : "" ) << "MethodParams" << ParamsCount << "_Func" << ( ( Const ) ? "_Const" : "" ) << ( ( Volatile ) ? "_Volatile" : "" ) << "(MethodPtr Ptr):FMethodPtr(Ptr)" << endl;
   Out << "   {" << endl;

   if ( Function )
   {
      Out << "      SetReturnValue( CreateNativeParameter<ReturnType>() );" << endl;
   }

   Out << "   }" << endl;
   Out << "   virtual void        Invoke(void* ObjectAddr, clParametersList& Params)" << endl;
   Out << "   {" << endl;

   if ( ParamsCount > 0 )
   {
      Out << "      FATAL( Params.size() != " << ParamsCount << ", \"Not enough parameters passed in clParametersList\" );" << endl;
      Out << endl;
   }

   // generate invocation
   Out << "      ";

   int InvSpaces = 39;

   if ( Function )
   {
      InvSpaces = 83;
      Out << "*(ReturnUnqualifiedType*)GetReturnValuePtr() = ";
   }

   if ( Static ) { InvSpaces -= 18; }

   Out << "(" << ( ( Static ) ? "" : "((T*)ObjectAddr)->" ) << "*FMethodPtr)(";

   for ( int i = 0; i != ParamsCount; ++i )
   {
      if ( i != 0 ) { Out << endl; }

      Out << ( ( i == 0 ) ? " " : MultiSpace( InvSpaces ) ) << "*(typename TypeTraits<P" << i << ">::ReferredType*)( Params[" << i << "]->GetNativeBlock() )";

      if ( i != ParamsCount - 1 ) { Out << ","; }
      else { Out << " "; }
   }

   Out << ");" << endl;
   //

   Out << "   }" << endl;
   Out << "   virtual int         GetParamsCount() const" << endl;
   Out << "   {" << endl;
   Out << "      return " << ParamsCount << ";" << endl;
   Out << "   }" << endl;
   Out << "   virtual int         GetParamSize(int Index)" << endl;
   Out << "   {" << endl;

   for ( int i = 0; i != ParamsCount; ++i )
   {
      Out << "      if ( Index == " << i << " ) return sizeof(P" << i << ");" << endl;
   }

   Out << endl;
   Out << "      return 0;" << endl;

   Out << "   }" << endl;
   Out << "   virtual iParameter* CreateParameter(int Index, void* InitialValue)" << endl;
   Out << "   {" << endl;
   Out << "      iParameter* Param = NULL;" << endl;
   Out << endl;

   // generate parameters creation
   for ( int i = 0; i != ParamsCount; ++i )
   {
      Out << "      if ( Index == " << i << " ) Param = CreateNativeParameter<P" << i << ">();" << endl;
   }

   if ( ParamsCount > 0 ) { Out << "" << endl; }

   //

   Out << "      if ( Param && InitialValue ) Param->ReadValue( InitialValue );" << endl;
   Out << "" << endl;
   Out << "      return Param;" << endl;
   Out << "   }" << endl;
   Out << "};" << endl;

   //
   // Generating binder
   //
   if ( !Function ) { return; }

   Out << endl;

   if ( !( Static && ParamsCount == 0 && !Function ) )
   {

      Out << "template <" << ( ( Static ) ? "" : "class T" );

      if ( Function ) { Out << ( ( Static ) ? "" : ", " ) << "typename ReturnType"; }

      if ( !Static || Function ) { Out << ( ( ParamsCount > 0 ) ? ", " : "" ); }

      WriteItemList( Out, MultiSpace( SpaceCount ), "typename ", true, ParamsCount );

      Out << ">" << endl;

   }

   Out << "inline " << BinderClassName << "* BindNativeMethod( " << ( ( Function ) ? "ReturnType" : "void" ) << " (" << ( ( Static ) ? "" : "T::" ) << "*MethodPtr)(";

   WriteItemList( Out, "", "", false, ParamsCount );

   Out << ") " << ( ( Const ) ? "const " : "" )  << ( ( Volatile ) ? "volatile " : "" ) << ", const LString& MethodName)" << endl;

   Out << "{" << endl;
   Out << "   " << BinderClassName << "* Method = new clNative" << ( ( Static ) ? "Static" : "" ) << "MethodParams" << ParamsCount << "_Func" << ( ( Const ) ? "_Const" : "" ) << ( ( Volatile ) ? "_Volatile" : "" );

   if ( !( Static && ParamsCount == 0 && !Function ) )
   {

      Out << "<" << ( ( Static ) ? "" : "T" );

      Out << ( ( !Static && Function ) ? ", " : "" );

      Out << ( ( Function ) ? "ReturnType" : "" );


      if ( ParamsCount > 0 )
      {
         Out << ( ( Function || !Static ) ? ", " : "" );

         WriteItemList( Out, "", "", false, ParamsCount );
      }

      Out << ">";

   }

   Out << "(MethodPtr);" << endl;
   Out << endl;
   Out << "   Method->SetMethodName( MethodName );" << endl;
   Out << endl;
   Out << "   return Method;" << endl;

   Out << "}" << endl;
   Out << endl;
}

void GenerateMethodBinders( std::ostream& Out, int NumParams )
{
   Out << "/**" << endl;
   Out << " * \\file MethodBind.h" << endl;
   Out << " * \\brief Native methods binders" << endl;
   Out << " * \\version 0.8.0" << endl;
   Out << " * \\date 22/01/2011" << endl;
   Out << " * \\author Sergey Kosarevsky, 2005-2011" << endl;
   Out << " * \\author support@linderdaum.com http://www.linderdaum.com" << endl;
   Out << " * \\author Viktor Latypov, 2007-2011" << endl;
   Out << " * \\author viktor@linderdaum.com http://www.linderdaum.com" << endl;
   Out << " */" << endl;
   Out << endl;
   Out << "#ifndef _iMethodBind_" << endl;
   Out << "#define _iMethodBind_" << endl;
   Out << endl;
   Out << "#include \"Core/RTTI/Method.h\"" << endl;
   Out << "#include \"Core/RTTI/Parameters.h\"" << endl;
   Out << endl;
   //
   Out << "/// \\cond" << endl;
   Out << endl;

   for ( int i = 0; i < NumParams + 1 ; i++ )
   {
      // non-const methods
      GenerateBinder( Out, false, true,  i, false, false );
      GenerateBinder( Out, false, false, i, false, false );
      GenerateBinder( Out, true,  true,  i, false, false );
      GenerateBinder( Out, true,  false, i, false, false );
/*
      GenerateBinder( Out, false, true,  i, false, true );
      GenerateBinder( Out, false, false, i, false, true );
      GenerateBinder( Out, true,  true,  i, false, true );
      GenerateBinder( Out, true,  false, i, false, true );
*/
      // const methods (only non-static allowed)
      GenerateBinder( Out, false, true,  i, true, false );
      GenerateBinder( Out, false, false, i, true, false );
      // const volatile methods (only non-static allowed)
      GenerateBinder( Out, false, true,  i, true, true );
      GenerateBinder( Out, false, false, i, true, true );
      // volatile methods (only non-static allowed)
      GenerateBinder( Out, false, true,  i, false, true );
      GenerateBinder( Out, false, false, i, false, true );
   }

   //
   Out << "#endif" << endl;
   Out << endl;
   Out << "/// \\endcond" << endl;
   Out << endl;
   Out << "/*" << endl;
   Out << " * 08/07/2010" << endl;
   Out << "     Autogenerated via LSDC" << endl;
   Out << "*/" << endl;
   Out << endl;
}
