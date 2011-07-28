/**
 * \file MethodArg.h
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

#ifndef __method_arg__h__included__
#define __method_arg__h__included__

#include <string>
using std::string;

// actually, this is a pair<Name,Type> and Type class should contain every routine (marshalling properties etc.)
class clMethodArg
{
public:

   /// argument name
   string Name;

   /// argument type
   string Type;

   /// default value or something else
   string Defaults;

   /// unstructured list of modifiers (const, array etc.)
   string Modifiers;

   /// is it a C-style array
   bool   IsArray;

   /// is it a vector<> type
   bool   IsVector;

   /// is it a constant parameter
   bool   IsConst;

   /// is it an Out parameter like "int* Num" or "int& var"
   bool   IsOut;

   /// the type of a vector's item
   string VectorItemType;

   /// which parameter is used as an array size
   string ArraySizeParamName;

public:
   /// serialization
   virtual string ToString() const;
   /// deserialization
   virtual bool FromString( string& CurrentArg );

   /// vectors, scalars, arrays, exported class pointers are allowed
   bool CanMarshalToNET() const;

   /// arrays, vectors, scalars, object pointers are allowed
   bool CanMarshalToScript() const;
   /*
      /// FromNET marshalling code for C++/CLI  ([type] [Name] = MarshallingCode...)
      virtual string DeclareNativeParam(const string& Name);
      /// ToNET marshalling  [ NetParam = SomeCode([Name]) ]
      virtual string GetNETValue(const string& Name);
   */
};

#endif
