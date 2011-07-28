/**
 * \file BufStream.cpp
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

#include "BufStream.h"

#include "Utils.h"

#include <fstream>

bool buffered_stream::CompareToReference( const char* InFileName )
{
   if ( !FileExists( InFileName ) ) { return false; }

   size_t line = 0;
   ifstream f( InFileName );

   string s;

   while ( getline( f, s ) )
   {
      if ( line >= ReferenceBuffer.size() ) { return false; }

      if ( s != ReferenceBuffer[line] ) { return false; }

      line++;
   }

   return true;
}

void buffered_stream::write()
{
   if ( ReferenceFile != "" )
   {
      if ( !CompareToReference( ReferenceFile.c_str() ) )
      {
         ofstream f( ReferenceFile.c_str() );

         for ( vector<string>::iterator i = ReferenceBuffer.begin() ; i != ReferenceBuffer.end() ; i++ )
         {
            f << *i << std::endl;
         }
      }
   }
}

buffered_stream& endl( buffered_stream& stream )
{
   stream.endl();

   return stream;
}

void buffered_stream::WriteDoxygenHeader( const string& FileName, const string& Brief )
{
	( *this ) << "/**" << ::endl;
	( *this ) << " * \\file " << FileName << ::endl;
	( *this ) << " * \\brief " << Brief << ::endl;
	( *this ) << " * \\version " << GetCurrentVersion() << ::endl;
	( *this ) << " * \\date " << GetCurrentDate() << ::endl;
	( *this ) << " * \\author Sergey Kosarevsky, 2005-2011" << ::endl;
	( *this ) << " * \\author Viktor Latypov, 2007-2011" << ::endl;
	( *this ) << " * \\author support@linderdaum.com http://www.linderdaum.com" << ::endl;
	( *this ) << " */" << ::endl << ::endl;
}
