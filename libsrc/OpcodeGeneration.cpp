/**
 * \file OpcodeGeneration.cpp
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

#include "OpcodeGeneration.h"

#include <fstream>
using namespace std;

int OpcodeGenerator::Generate( int offs, int argc, char** argv )
{
   opcodes.clear();
   max_len = 0;

   std::string opcode_src = "opcodes.list";
   out_dir = ".";

   class_name = "clExecutionThread";
   std::string exec_array_file = "ExecThread_MtdArray.h";
   std::string method_list_file = "ExecThread_MtdList.h";

   for ( int i = offs ; i < argc ; i++ )
   {
      if ( std::string( argv[i] ) == "--opcode_source" )
      {
         opcode_src = std::string( argv[i+1] );
         i++;
      }
      else if ( std::string( argv[i] ) == "--output_dir" )
      {
         out_dir = std::string( argv[i+1] );
         i++;
      }
      else if ( std::string( argv[i] ) == "--method_list" )
      {
         method_list_file = std::string( argv[i+1] );
         i++;
      }
      else if ( std::string( argv[i] ) == "--exec_array" )
      {
         exec_array_file = std::string( argv[i+1] );
         i++;
      }
      else if ( std::string( argv[i] ) == "--class_name" )
      {
         class_name = std::string( argv[i+1] );
         i++;
      }
   }

   load_opcodes( opcode_src );

   write_opcode_mappings( "LOpCodes" );
   write_static_executor_array( out_dir + std::string( "\\" ) + exec_array_file );
   write_execution_method_list( out_dir + std::string( "\\" ) + method_list_file );

   return 0;
}

void OpcodeGenerator::load_opcodes( const std::string& fname )
{
   ifstream f( fname.c_str() );

   while ( !f.eof() )
   {
      std::string l;
      getline( f, l );

      // skip empty lines
      if ( l.length() > 0 )
      {
         // skip comments
         if ( l[0] != ';' )
         {
            if ( static_cast<int>( l.length() ) > max_len ) { max_len = static_cast<int>( l.length() ); }

            opcodes.push_back( l );
         }
      }
   }

   f.close();
}

void OpcodeGenerator::write_execution_method_list( const std::string& fname )
{
   ofstream f( fname.c_str() );
   f << "// autogenerated by op_processor, do not modify !" << endl;
   f << "\t\tpublic:" << endl;

   for ( std::vector<std::string>::iterator _s = opcodes.begin() ; _s != opcodes.end() ; _s++ )
   {
      f << "\t\t\tvoid Opcode_" << *_s << "();" << endl;
   }

   f.close();
}

void OpcodeGenerator::write_static_executor_array( const std::string& fname )
{
   ofstream f( fname.c_str() );

   f << "// autogenerated by op_processor, do not modify !" << endl;
   f << "typedef void (" << class_name << "::*_executor)();" << endl;
   f << "\tstatic _executor executors[] = {" << endl;

   for ( std::vector<std::string>::iterator _s = opcodes.begin() ; _s != opcodes.end() ; _s++ )
   {
      std::string __comma = "";

      if ( _s != opcodes.end() - 1 ) { __comma = ","; }

      f << "\t\t&" << class_name << "::" << "Opcode_" << *_s /* << "_Executor"*/ << __comma << endl;
   }

   f << "\t};" << endl;
   f.close();
}

// opcode2string and string2opcode mapping
void OpcodeGenerator::write_opcode_mappings( const std::string& fname )
{
   // 1. header file
   ofstream f_h( std::string( out_dir + "\\" + fname + ".h" ).c_str() );
 
   f_h << endl << endl;

   f_h << "// LinderScript Virtual Machine Opcodes" << endl;
   f_h << "// (C) Viktor Latypov, 2005-2007" << endl;
   f_h << "// Autogenerated from OpCodes.list " << endl;
   f_h << "// Do not modify !" << endl << endl;

   f_h << "enum " << fname << " {" << endl;
   int _intcode = 0;

   for ( std::vector<std::string>::iterator _s = opcodes.begin() ; _s != opcodes.end() ; _s++ )
   {
      std::string _fill = "";

      // make the string look good - add some tabs
      int _l = static_cast<int>( _s->length() );

      while ( _l <= max_len )
      {
         _fill += "\t";
         // update _l;
         _l += ( TAB_SIZE - ( _l % TAB_SIZE ) );
      }

      std::string __comma = "";

      if ( _s != opcodes.end() - 1 ) { __comma = ","; }

      f_h << "\tOP_" << *_s << _fill << "= " << _intcode <<  __comma << endl;
      ++_intcode;
   }

   f_h << "};" << endl << endl;

   f_h << "const int MAX_OPCODES = (" << opcodes.size() - 1 << ");" << endl << endl;

   f_h << "static const char* SymOpCodes[MAX_OPCODES+1] = {" << endl;

   for ( std::vector<std::string>::iterator _s = opcodes.begin() ; _s != opcodes.end() ; _s++ )
   {
      std::string __comma = "";

      if ( _s != opcodes.end() - 1 ) { __comma = ","; }

      std::string _str = ( *_s );

      // ignor "invalid" opcode
      if ( _str == "INVALID" ) { _str = ""; }

      f_h << "\t\"" << _str << "\"" <<  __comma << endl;
   }

   f_h << "};" << endl;
// f_h << "#endif" << endl;

   f_h.close();
}
