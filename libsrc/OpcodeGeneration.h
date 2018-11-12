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
#include <string>
#include <vector>

#define TAB_SIZE 3

class OpcodeGenerator
{
public:

   std::vector<std::string> opcodes;
   int max_len;

   void load_opcodes( const std::string& fname );
   void write_opcode_mappings( const std::string& fname );
   void write_static_executor_array( const std::string& fname );

   void write_execution_method_list( const std::string& fname );

   std::string out_dir;

   std::string class_name;

public:
   int Generate( int offs, int argc, char** argv );
};

inline std::string remove_op( const std::string& s ) { return s.substr( 3, s.length() - 1 ); }

#endif
