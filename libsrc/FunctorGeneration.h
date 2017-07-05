/**
 * \file FunctorGeneration.h
 * \brief FunctorList generator
 *
 * LinderScript Database Compiler
 *
 * \version 0.8.10c
 * \date 30/07/2010
 * \author Sergey Kosarevsky, 2005-2010
 * \author Viktor Latypov, 2007-2010
 * \author support@linderdaum.com http://www.linderdaum.com
 */

#include <iostream>
#include <fstream>
#include <string>

#include <stdlib.h>
#include <string.h>

#define File_Head "#pragma once\n"\
"#include <memory>\n\n"\
"#include \"Utils/TypeLists.h\"\n\n"\
"#include \"Platform/Platform.h\"\n\n"\
"#define IMPLEMENT_CLONE_METHOD(Cls) virtual Cls* Clone() const { return new Cls(*this); };\n\n"\
"namespace Linderdaum\n"\
"{\n"\
"   namespace Utils\n"\
"   {\n"

#define File_End "   }\n"\
"}\n\n"\

using namespace std;

#include "ItemListsGenerator.h"

void generate_functor_impl_template( std::ostream& Out, int num_args )
{
   Out << "      // " << num_args << " param" << ( num_args != 1 ? "s" : "" ) << "\n      ";

   if ( num_args > 0 )
   {
      make_item_list( Out, ", ", "template <class R, ", ">", "class ", "P%d", num_args );
   }
   else
   {
      Out << "template <class R>";
   }

   Out << endl << "      class clFunctorImpl<R,";

   if ( num_args > 0 )
   {
      Out << " TYPELIST_" << num_args;
      make_item_list( Out, ", ", "(", ")>", "", "P%d", num_args );
   }
   else
   {
      Out << "NullType>";
   }

   Out << endl << "      {" << endl << "      public:" << endl;
   make_item_list( Out, ",", "         virtual R operator()(", ") = 0;", "", "P%d", num_args, false, false, " " );

   Out   << endl
         << "         virtual clFunctorImpl* Clone() const = 0;" << endl
         << "         virtual long long GetUID() const = 0;" << endl
         << "         virtual ~clFunctorImpl() {};" << endl
         << "      };" << endl;
}

void generate_functor_operator( std::ostream& Out, int num_args, const string& callSemantics, const string& paramTemplate )
{
   make_item_list( Out, ", ", "         ResultType operator()(", ")", paramTemplate, "P%d", num_args );
   Out << "\n         {\n";
   Out << "            return " << callSemantics;
   make_item_list( Out, ", ", "(", ");", "", "P%d", num_args );
   Out << "\n         }\n";
}

void generate_typename_list( std::ostream& Out, int num )
{
   make_item_list( Out, "\n", "", "\n", "         typedef typename TypeAtNonStrict<TList, %d, EmptyType>::Result ", "Param%d;", num, true, false );
}

void generate_functor_operators( std::ostream& Out, int num, const string& callSemantics, const string& paramTemplate )
{
   for ( int i = 0 ; i < num ; i++ )
   {
      generate_functor_operator( Out, i, callSemantics, paramTemplate );
   }
}

void generate_partial_specs( std::ostream& Out, int num )
{
   Out << "      // partial specialization\n";

   for ( int i = 0 ; i < num ; i++ )
   {
      generate_functor_impl_template( Out, i );
   }
}

void generate_functor_class( std::ostream& Out, int max_args )
{
   Out   << "      // forwards\n";
   Out   << "      template <class R, class TList> class clFunctorImpl;\n"
         << "      template <class ParentFunctor, class Fun> class clFunctionHandler;\n"
         << "      template <class ParentFunctor, class Object, class Method> class clMethodHandler;\n";

   Out   << "      // clFunctor class definition\n"
         << "      template <class ResultType, class TList>\n"
         << "      class clFunctor\n"
         << "      {\n"
         << "      private:\n"
         << "         typedef clFunctorImpl<ResultType, TList> Impl;\n"
         << "      public:\n"
         << "         typedef ResultType                                            ParentResultType;\n"
         << "         typedef TList                                                 ParamList;\n";

   generate_typename_list( Out, max_args/*+1*/ );

   Out   << "      public:\n"
         << "         clFunctor():FImpl(0) {}\n"
         << "         clFunctor(const clFunctor& Functor):FImpl(clFunctor::Clone(Functor.FImpl.get())) {}\n"
         << "         template <class Fun> explicit clFunctor(Fun fun):FImpl(new clFunctionHandler<clFunctor, Fun>(fun)) {}\n"
         << "         template <class Object, class Method> explicit clFunctor(const Object& Obj, const Method Meth):FImpl(new clMethodHandler<clFunctor, Object, Method>(Obj, Meth)) {}\n"
         << "         virtual ~clFunctor() {};\n"
         << "         virtual bool IsEqual(const clFunctor& Functor) const\n"
         << "         {\n"
         << "            return FImpl.get()->GetUID() == Functor.FImpl.get()->GetUID();\n"
         << "         }\n"
         << "         inline bool IsNull() const\n"
         << "         {\n"
         << "            return FImpl.get() == NULL;\n"
         << "         }\n"
         << "         inline bool operator == ( const clFunctor& Other ) const\n"
         << "         {\n"
         << "            return IsEqual(Other);\n"
         << "         }\n"
         << "         inline bool IsEqual( const clFunctor& Functor ) const\n"
         << "         {\n"
         << "             return FImpl.get()->GetUID() == Functor.FImpl.get()->GetUID();\n"
         << "         }\n"
         << "         inline bool IsObject( void* Obj ) const\n"
         << "         {\n"
         << "             return reinterpret_cast<void*>( FImpl.get()->GetUID() ) == Obj;\n"
         << "         }\n"
         << "         inline void* GetObjectPtr() const\n"
         << "         {\n"
         << "             return FImpl.get()->GetObjectPtr();\n"
         << "         }\n"
         << "         template <class U>\n"
         << "         static U* Clone(U* pObj)\n"
         << "         {\n"
         << "            if (!pObj) return 0;\n"
         << "            U* pClone = static_cast<U*>(pObj->Clone());\n"
//    << "            if (typeid(*pClone) != typeid(*pObj)) throw LString(\"Invalid functors\");\n"
         << "            if (typeid(*pClone) != typeid(*pObj)) exit(255);\n"
         << "            return pClone;\n"
         << "         }\n"
         << "         clFunctor& operator = (const clFunctor& Functor)\n"
         << "         {\n"
         << "            clFunctor copy(Functor);\n"
         << "            // swap auto_ptrs by hand\n"
         << "            Impl* p = FImpl.release();\n"
         << "            FImpl.reset(copy.FImpl.release());\n"
         << "            copy.FImpl.reset(p);\n"
         << "            return *this;\n"
         << "         };\n";

   generate_functor_operators( Out, max_args + 1, "(*FImpl)", "Param%d " );

   Out   << "      private:\n"
         << "         std::auto_ptr<Impl>    FImpl;\n"
         << "      };\n";
}

void generate_function_functors( std::ostream& Out, int count )
{
   Out   << "      //\n"
         << "      // For ordinary functions\n"
         << "      //\n"
         << "      template <class ParentFunctor, class Fun> class clFunctionHandler: public clFunctorImpl<typename ParentFunctor::ParentResultType, typename ParentFunctor::ParamList>\n"
         << "      {\n"
         << "      public:\n"
         << "         typedef typename ParentFunctor::ParentResultType ResultType;\n"
         << "         explicit clFunctionHandler(const Fun& fun):FFun(fun) {}\n"
         << "         IMPLEMENT_CLONE_METHOD(clFunctionHandler)\n"
         << "         virtual long long GetUID() const\n"
         << "         {\n"
         << "#ifdef _WIN64\n"
         << "             return (long long)FFun;\n"
         << "#else\n"
         << "             return (long)FFun;\n"
         << "#endif\n"
         << "         };\n";

   generate_functor_operators( Out, count + 1, "FFun", "typename ParentFunctor::Param%d " );

   Out   << "      private:\n"
         << "         Fun FFun;\n"
         << "      };\n";
}

void generate_method_functors( std::ostream& Out, int count )
{
   Out   << "      //\n"
         << "      // For methods\n"
         << "      //\n"
         << "      template <class ParentFunctor, class Object, class Method> class clMethodHandler: public clFunctorImpl<typename ParentFunctor::ParentResultType, typename ParentFunctor::ParamList>\n"
         << "      {\n"
         << "      public:\n"
         << "         typedef typename ParentFunctor::ParentResultType ResultType;\n"
         << "         explicit clMethodHandler(const Object& Obj, const Method Meth):FObject(Obj), FMethod(Meth) {};\n"
         << "         IMPLEMENT_CLONE_METHOD(clMethodHandler)\n"
         << "         virtual long long GetUID() const\n"
         << "         {\n"
         << "             // NOTE: this is not 64-bit safe\n"
         << "             union\n"
         << "             {\n"
         << "                Object Obj;\n"
         << "                void* UID;\n"
         << "             } Cast;\n"
         << "             Cast.Obj = FObject;\n"
         << "#ifdef _WIN64\n"
         << "             return reinterpret_cast<long long>( Cast.UID );\n"
         << "#else\n"
         << "             return reinterpret_cast<long>( Cast.UID );\n"
         << "#endif\n"
//    << "             return Cast.UID;\n"
         << "         };\n";

   generate_functor_operators( Out, count + 1, "((*FObject).*FMethod)", "typename ParentFunctor::Param%d " );

   Out   << "      private:\n"
         << "         Object    FObject;\n"
         << "         Method    FMethod;\n"
         << "      };\n\n";
}

void generate_binder( std::ostream& Out, int num_args, bool isMethod )
{
   Out << "      template <class ResultType" << ( isMethod ? ", class ClassType, class ObjectType" : "" );

   if ( num_args > 0 )
   {
      Out << ", ";
      make_item_list( Out, ", ", "", "", "class ", "P%d", num_args );
   }

   Out   << ">\n";

   Out   << "      inline clFunctor<ResultType, TYPELIST_" << num_args;
   make_item_list( Out, ", ", "(", ")", "", "P%d", num_args );
   Out   << "> Bind("
         << ( isMethod ? "ResultType(ClassType::*MethodPtr)" : "ResultType(*FuncPtr)" );
   // params
   make_item_list( Out, ", ", "(", ")", "", "P%d", num_args );

   Out   << ( isMethod ? ", const ObjectType& Object" : "" ) << ")\n";
   Out   << "      {\n";

   Out << "         return clFunctor<ResultType, TYPELIST_" << num_args;

   make_item_list( Out, ", ", "(", ")", "", "P%d", num_args );

   Out << ">(" << ( isMethod ? "Object, MethodPtr" : "FuncPtr" ) << ");\n";

   Out   << "      }\n";
}

void GenerateFunctors( std::ostream& Out, int count )
{
   // header

   Out << File_Head;
   generate_functor_class( Out, count );
   generate_partial_specs( Out, count + 1 );
   generate_function_functors( Out, count );
   generate_method_functors( Out, count );

   // 5. Binders
   Out   << "      //////////////////////////\n"
         << "      //        Binders       //\n"
         << "      //////////////////////////\n";

   Out << "      // for functions\n";

   for ( int i = 0 ; i < count + 1 ; i++ )
   {
      generate_binder( Out, i, false );
   }

   Out << "\n      // for methods\n\n";

   for ( int i = 0 ; i < count + 1 ; i++ )
   {
      generate_binder( Out, i, true );
   }


   Out << "#endif" << endl;
   Out << endl;
   Out << "/*" << endl;
   Out << " * 12/04/2007" << endl;
   Out << "     Autogenerated via FunctorGen" << endl;
   Out << " * 23/01/2005" << endl;
   Out << "     IsEqual()" << endl;
   Out << " * 17/01/2005" << endl;
   Out << "     Simple binding semantics added" << endl;
   Out << " * 16/01/2005" << endl;
   Out << "     It's here" << endl;
   Out << "*/" << endl;
   Out << endl;
}
