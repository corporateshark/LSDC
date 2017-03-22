/**
 * \file Consts.h
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

#ifndef _Consts_h_
#define _Consts_h_

#include "Utils.h"
#include "BufStream.h"
#include "MethodArg.h"

using namespace std;

struct clPackage;

struct clConst
{
public:
   clConst( string Name,
            clPackage* PackageIdx,
            string DeclaredIn,
            string Type,
            string Value,
            bool IsArray );
public:
   string             FName;
   clPackage*         FPackage;
   string             FDeclaredIn;
   string             FType;
   string             FValue;
   bool               FIsArray;
};

#endif

/*
 * 19/05/2010
     It's here
*/
