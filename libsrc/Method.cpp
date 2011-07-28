/**
 * \file Method.cpp
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

#include "Method.h"
#include "Class.h"
#include "Utils.h"

void clMethod::GenerateMethodStub( const clClass* OriginalClass, buffered_stream& Out ) const
{
   string OverridenInClass;

   bool MethodOverriden = OriginalClass->FindOverridenMethod( FMethodName, OverridenInClass );

   // generate method proto
   Out << "   virtual " << FReturnType << "    " << FMethodName << "(";

   for ( size_t i = 0; i != FArgTypes.size(); ++i )
   {
      Out << FArgTypes[i] << " P" << static_cast<int>( i );

      if ( !FArgDefaults[i].empty() )
      {
         Out << " = " << FArgDefaults[i];
      }

      Out << ( ( i + 1 != FArgTypes.size() ) ? ", " : "" );
   }

   Out << ")" << ( FConst ? " const" : "" ) << endl;

   // generate method body
   Out << "   {" << endl;

   Out << "      if ( !iObject::FInheritedCall && iObject::IsMethodOverriden( \"" << FMethodName << "\" ) )" << endl;
   Out << "      {" << endl;

   Out << "         clParametersList Params;" << endl;
   Out << endl;

   for ( size_t i = 0; i != FArgTypes.size(); ++i )
   {
      Out << "         ParameterType<" << FArgTypes[i] << ">::Type Param" << static_cast<int>( i ) << ";" << endl;

      if ( i == FArgTypes.size() - 1 ) { Out << endl; }
   }

   for ( size_t i = 0; i != FArgTypes.size(); ++i )
   {
      Out << "         Param" << static_cast<int>( i ) << ".ReadValue( &P" << static_cast<int>( i ) << " );" << endl;

      if ( i == FArgTypes.size() - 1 ) { Out << endl; }
   }

   for ( size_t i = 0; i != FArgTypes.size(); ++i )
   {
      Out << "         Params.push_back( &Param" << static_cast<int>( i ) << " );" << endl;

      if ( i == FArgTypes.size() - 1 ) { Out << endl; }
   }

   Out << "         bool MethodCalled = iObject::CallMethod( \"" << FMethodName << "\", Params, iObject::FInheritedCall );" << endl;

   Out << endl;

   Out << "         if ( !MethodCalled ) ";

   if ( MethodOverriden )
   {
      if ( FReturnType != "void" ) { Out << "return "; }

      Out << OverridenInClass << "::" << FMethodName << "(";

      for ( size_t i = 0; i != FArgTypes.size(); ++i )
      {
         Out << "P" << static_cast<int>( i );

         if ( i != FArgTypes.size() - 1 ) { Out << ", "; }
      }

      Out << ");" << endl;
   }
   else
   {
      Out << "FATAL_MSG(\"Abstract method called: " << FClassName << "::" << FMethodName << "()" << "\")" << endl;
   }

   if ( FReturnType != "void" )
   {
      Out << endl;
      Out << "         return *(TypeTraits< " << FReturnType << " >::ReferredType*)iObject::GetReturnValue()->GetNativeBlock();" << endl;
   }
   else
   {
      Out << endl;
      Out << "         return;" << endl;
   }

   Out << "      }" << endl;

//   Out << endl;
//   Out << "      iObject::ResetInheritedCall();" << endl;

   if ( MethodOverriden )
   {
      Out << endl;

      if ( FReturnType != "void" ) { Out << "      return "; }
      else { Out << "      "; }

      Out << OverridenInClass << "::" << FMethodName << "(";

      for ( size_t i = 0; i != FArgTypes.size(); ++i )
      {
         Out << "P" << static_cast<int>( i );

         if ( i != FArgTypes.size() - 1 ) { Out << ", "; }
      }

      Out << ");" << endl;
   }
   else
   {
      Out << endl;
      Out << "      FATAL_MSG(\"Abstract method called: " << FClassName << "::" << FMethodName << "()" << "\")" << endl;

      if ( FReturnType != "void" )
      {
         Out << endl;
         Out << "      return *(TypeTraits< " << FReturnType << " >::ReferredType*)(0);" << endl;
      }
   }

   Out << "   }" << endl;
}

void RemoveMethodModifier( bool& PresenseFlag, string& InLine, const string& Modifier )
{
   size_t WordPos = InLine.find( Modifier );

   PresenseFlag = ( WordPos != -1 );

   if ( PresenseFlag )
   {
      InLine = InLine.substr( WordPos + Modifier.length(), InLine.length() - 1 );
      InLine = TrimSpaces( InLine );
   }
}

void clMethod::FromString( const string& Line )
{
   string CleanLine = Line;

   string Modifier;

   // remove trailing '{'
   size_t BracePos = CleanLine.find( "{" );

   if ( BracePos != -1 )
   {
      CleanLine = CleanLine.substr( 0, BracePos - 1 );
      CleanLine = TrimSpaces( CleanLine );
   }

   bool DummyExported; // dummy variable for Remove...() method call

   RemoveMethodModifier( FVirtual, CleanLine, "virtual " );
   RemoveMethodModifier( FStatic,  CleanLine, "static " );
   RemoveMethodModifier( DummyExported, CleanLine, "scriptmethod " );

   // find args list starting within "(" and ")"
   string ArgsList;

   size_t ArgsPos = CleanLine.find( "(" );

   if ( ArgsPos != -1 )
   {
      ArgsList = TrimSpaces( CleanLine.substr( ArgsPos + 1, CleanLine.length() - ArgsPos ) );
      CleanLine = CleanLine.substr( 0, ArgsPos );
      CleanLine = TrimSpaces( CleanLine );
   }

   ArgsPos = ArgsList.find_last_of( ")" ); // was find(")"), but now we support MarshalAs() macros

   if ( ArgsPos != -1 )
   {
      Modifier = TrimSpaces( ArgsList.substr( ArgsPos + 1, ArgsList.length() - 1 ) );
      ArgsList = TrimSpaces( ArgsList.substr( 0, ArgsPos ) );

      FConst = ( Modifier.find( "const" ) != -1 );
      FAbstract = ( Modifier.find( "=" ) != -1 && Modifier.find( "0" ) != -1 );
   }

   // extract method name
   size_t SpacePos = CleanLine.find_last_of( ' ' );

   // tabs support
   if ( SpacePos == -1 ) { SpacePos = CleanLine.find_last_of( '\t' ); }

   if ( SpacePos != -1 )
   {
      FMethodName = TrimSpaces( CleanLine.substr( SpacePos, CleanLine.length() - SpacePos ) );
      CleanLine = CleanLine.substr( 0, SpacePos );
      CleanLine = TrimSpaces( CleanLine );
   }

   FReturnType = CleanLine;      // return type

   ParseArgsList( ArgsList );
}

void clMethod::ParseArgsList( string& ArgsList )
{
   FArgs.resize( 0 );

   int CurrentParameterIdx = 0;

   // extract parameters
   while ( !ArgsList.empty() )
   {
      string CurrentArg;

      size_t CommaPos = ArgsList.find( "," );

      if ( CommaPos == -1 )
      {
         CurrentArg = ArgsList;
         ArgsList = "";
      }
      else
      {
         CurrentArg = ArgsList.substr( 0, CommaPos );
         ArgsList = TrimSpaces( ArgsList.substr( CommaPos + 1, ArgsList.length() - 1 ) );
      }

      CurrentArg = TrimSpaces( CurrentArg );

      clMethodArg NewArg;
      NewArg.FromString( CurrentArg );

      if ( NewArg.Name.empty() )
      {
         NewArg.Name = "P" + Int2Str( CurrentParameterIdx );
      }

      // fill old string lists with name/type/defaults
//      AddOldStyleArgument( NewArg );
      FArgNames.push_back( NewArg.Name );
      FArgTypes.push_back( NewArg.Type );
      FArgDefaults.push_back( NewArg.Defaults );

      CurrentParameterIdx++;

      // uncomment to get the list
//      FArgs.push_back(NewArg);
   }
}
/*
void clMethod::AddOldStyleArgument( const clMethodArg& Arg )
{
   FArgNames.push_back(Arg.Name);
   FArgTypes.push_back(Arg.Type);
   FArgDefaults.push_back(Arg.Defaults);
}
*/
/*
 * 27/07/2007
     It's here
*/
