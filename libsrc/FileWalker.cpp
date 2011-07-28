/**
 * \file FileWalker.cpp
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

#include "FileWalker.h"

#ifdef _WIN32

// windows-based environment

#include <windows.h>

bool IsUpDir( string Dir )
{
   return ( Dir == "." ) || ( Dir == ".." );
}

void FileWalker::Scan( const string& dirName )
{
   NestLevel++;

   WIN32_FIND_DATA FileData;

   string Mask = dirName;
   Mask.append( "\\*.*" );

   HANDLE hSearch = FindFirstFile( Mask.c_str(), &FileData );

   if ( hSearch == INVALID_HANDLE_VALUE ) { return; }

   for ( ;; )
   {
      string FullName = dirName;
      FullName.push_back( '\\' );
      FullName.append( FileData.cFileName );

      if ( !IsUpDir( FileData.cFileName ) )
      {
         if ( ( FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY )
         {
            ProcessDirectory( FullName, FileData.cFileName );
         }
         else
         {
            ProcessFile( FullName, FileData.cFileName );
         }
      }

      if ( FindNextFile( hSearch, &FileData ) == 0 ) { break; }
   }

   FindClose( hSearch );

   NestLevel--;
}

#else

// some GCC-based environment

#include <stdlib.h>

#include <string>
#include <iostream>

using namespace std;

#include <dirent.h>
#include <sys/stat.h>

int IsDirectory( const char* filename )
{
   struct stat file_stats;

   stat ( filename, &file_stats );

   if ( S_ISDIR( file_stats.st_mode ) ) { return 1; }

   return 0;
}

void FileWalker::Scan( const string& dirName )
{
   NestLevel++;

   string _dirName = dirName;

   char _lastChar = _dirName.at( _dirName.length() - 1 );

   if ( ( _lastChar != '\\' ) || ( _lastChar != '/' ) )
   {
      _dirName += string( "/" );
   }

   cout << "Opening : " << _dirName << endl;
   cout.flush();

   DIR* d = opendir( _dirName.c_str() );
   struct dirent* de;

   if ( d == NULL )
   {
      cout << "null dir" << endl;
      exit( 0 );
   }

   while ( de = readdir( d ) )
   {
      string dname( de->d_name );

      // this is the only possible check (unix-like does not support hidden/system attr)
      if ( dname.at( 0 ) != '.' )
      {
         string fullName = _dirName + dname;

         if ( IsDirectory( fullName.c_str() ) )
         {
            ProcessDirectory( fullName, dname );
         }
         else
         {
            ProcessFile( fullName, dname );
         }
      }
   }

   closedir( d );

   NestLevel--;
}

#endif
