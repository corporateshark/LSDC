/**
 * \file Macro.h
 * \brief
 *
 * LinderScript Database Compiler
 *
 * \version 0.9.10
 * \date 08/10/2010
 * \author Sergey Kosarevsky, 2005-2010
 * \author support@linderdaum.com http://www.linderdaum.com
 */

#ifndef __Macro_h__
#define __Macro_h__

#include <string>
#include <vector>

using std::string;
using std::vector;

struct clDatabase;

class clMacroDef
{
public:
   explicit clMacroDef( const string& Name,
                        const vector<string>& Params,
                        const string& Text )
      : FName( Name ),
        FParams( Params ),
        FText( Text ) {};
   string GenerateInstance( const std::vector<string>& Values ) const;
public:
   clDatabase*            FDatabase;
   string                 FName;
   std::vector<string>    FParams;
   string                 FText;
};

#endif

/*
 * 08/10/2010
     It's here
*/
