/**
 * \file OpcodeGeneration.h
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

#ifndef __opcode_geneation__h__included__
#define __opcode_geneation__h__included__

/*
 RS VM Opcode generator
 2005-2010
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#define TAB_SIZE 3

class OpcodeGenerator
{
public:

   vector<string> opcodes;
   int max_len;

   void load_opcodes( const string& fname );
   void write_opcode_mappings( const string& fname );
   void write_static_executor_array( const string& fname );

   void write_execution_method_list( const string& fname );

   string out_dir;

   string class_name;

public:
   int Generate( int offs, int argc, char** argv );
};

inline string remove_op( const string& s ) { return s.substr( 3, s.length() - 1 ); }

#endif
