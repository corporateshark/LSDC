/**
   Array marshalling:

   Managed to native
      - .NET array<type>^ to vector<native_type>
      - .NET array<type>^ to (native_type*, ArraySize)

   Native to managed:
      - vector<int> to .NET array<type>^
      - (type* Array, int ArraySize) to
*/

string ConvertElementToNative( const string& VarName, const string& ManagedVar, const string& ManagedType, const string& NativeType )
{
   /// кусок кода с преобразованием ManagedVar типа ManagedType в родную переменную VarName типа NativeType

   /// ...
   return VarName + string( " = Some_NETToNative_Conversion(" ) + ManagedVar + string( ")" );
}

void ConvertArrayBody_ToNative( buffered_stream& Out, const string& offset, const string& NativeVarName, const string& ManagedVarName, const string& NativeElementType, const string& ManagedElementType )
{
   string TempVarName = NativeVarName + string( "_Element" );

   // declare temporary native array element
   Out << offset << NativeElementType << " " << TempVarName << ";" << endl << endl;

   // iterate over the elements
   Out << offset << "for(int i = 0 ; i < " << ManagedVarName << "->Length ; i++)" << endl;
   Out << offset << "{" << endl;

   // convert each element, just like the parameter
   ConvertElementToNative( TempVarName, ManagedVarName + string( "[i]" ), ManagedElementType, NativeElementType );

   Out << offset << "\t" << NativeVarName << "[i] = " << ConvertedElementName << ";" << endl;
   Out << offset << "}" << endl;
}

void ConvertArrayBody_ToManaged()
{
   string TempVarName = ManagedVarName + string( "_Element" );

   // 1.2. declare temporary native array element
   Out << offset << ManagedElementType << " " << TempVarName << ";" << endl << endl;

   // iterate over the elements
   Out << offset << "for(int i = 0 ; i < " << SizeExpression << "; i++)" << endl;
   Out << offset << "{" << endl;

   // convert each element, just like the parameter
   Out << ConvertElementToManaged( TempVarName, ManagedVarName + string( "[i]" ), ManagedElementType, NativeElementType ) << ";" << endl;

   Out << offset << "\t" << ManagedVarName << "[i] = " << ConvertedElementName << ";" << endl;
   Out << offset << "}" << endl;
}

/**
   ManagedVarName, ManagedElementType
   NativeVarName, NativeElementType
*/
void Array_MarshallingCode_NET_to_Vector( buffered_stream& Out, const string& offset )
{
   // 1. declare native variable
   Out << offset << "vector<" << NativeElementType << "> " << NativeVarName << ";" << endl;

   // 1.1. resize vector<> to array<>::Length
   Out << offset << NativeVarName << ".resize(" << ManagedVarName << "->Length" << ");" << endl << endl;

   // 2. convert each element
   ConvertArrayBody_ToNative();
}

/**
   ManagedVarName, ManagedElementType
   NativeVarName, NativeElementType, NativeArraySizeVar
*/
void Array_MarshallingCode_NET_to_Array( buffered_stream& Out, const string& offset )
{
   // 1. declare native variable
   Out << offset << NativeElementType << "* " << NativeVarName;

   // 1.1. allocate array with array<>::Length elements
   Out << " = new " << NativeElementType << "[" << ManagedVarName << "->Length" << "];" << endl << endl;

   // 2. convert each element
   ConvertArrayBody_ToNative();
}

void Array_MarshalingCode_Array_to_NET()
{
   string SizeExpression = NativeArraySizeVar; // or "NativeVarName".size() for vector<>

   // 1. declare managed variable
   Out << offset << "array<" << ManagedElementType << ">^ " << ManagedVarName << " = gcnew array<" << ManagedElementType << ">^( " << SizeExpression << " );" << endl;

   // 2. convert each element
   ConvertArrayBody_ToManaged();
}
