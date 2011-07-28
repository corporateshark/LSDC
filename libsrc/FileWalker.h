/**
 * \file FileWalker.h
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

#ifndef __FileWalker__h__included__
#define __FileWalker__h__included__

#include <string>
using namespace std;

class FileWalker
{
public:
   int NestLevel;

   void Begin( int _Level = 2 ) { NestLevel = _Level; }
public:
   void Scan( const string& dirName );

   // override these to define custom behavior
   virtual void ProcessFile( const string& fileName, const string& shortName ) = 0;
   virtual void ProcessDirectory( const string& dirName, const string& shortName ) = 0;
};

#endif
