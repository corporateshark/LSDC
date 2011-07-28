/// полный список enum'ов должен быть преобразован в объ€влени€ enum-типов.

/// при маршалировании отдельных элементов типа enum надо проводить преобразование.
/// т.е., дл€ каждого enum'а объ€вить inline-функцию преобразовани€ C++ <-> C++/CLI

/*
enum NativeEnum
{
   NativeVal1, ...
};

/// генерируем
public enum ManagedEnum
{
   ManagedVal1, ...
};
*/


void GenerateManagedEnumDeclaration( buffered_stream& Out, const string& offset, const clEnum& Enum )
{
   Out << offset << "// Wrapper enum for " << Enum.NativeName << endl;
   Out << offset << "public enum " << Enum.ManagedName << endl;
   Out << offset << "{" << endl;

   for ( size_t i = 0 ; i < Enum.Items.size() ; i++ )
   {
      Out << offset << "\t" << Enum.Items[i].Name;

      if ( Enum.Items[i].Value != "" )
      {
         Out << " = " << Enum.Items[i].Value;
      }

      if ( i != Enum.Items.size() - 1 ) { Out << ","; }

      Out << endl;
   }

   Out << offset << "}" << endl;
};

void GenerateConversionCode_NativeToNET( const clEnum& Enum )
{
   Out << "/*inline*/ " << Enum.ManagedName << " Native_" << Enum.Name << "_To_Managed(" << Enum.NativeName << " V)" << endl;
   Out << "{" << endl;

   for ( size_t i = 0 ; i < Enum.Items.size() ; i++ )
   {
      Out << "\tif (" << Enum.Items[i].Name << " == V) return " << Enum.ManagedName << "::" << Enum.Items[i].Name << ";" << endl;
   }

   Out << ""
       Out << "\t" << endl;
   Out << "\t// Default value" << endl;
   Out << "\treturn " << Enum.ManagedName << "::" << Enum.Items[0].Name << ";" << endl;
   Out << "}" << endl << endl;
}

void GenerateConversionCode_NETToNative( const clEnum& Enum )
{
   Out << "/*inline*/ " << Enum.NativeName << " Managed_" << Enum.ManagedName << "_To_Native(" << Enum.ManagedName << " V)" << endl;
   Out << "{" << endl;

   for ( size_t i = 0 ; i < Enum.Items.size() ; i++ )
   {
      Out << "\tif (" Enum.ManagedName << "::" << Enum.Items[i].Name << " == V) return " << Enum.Items[i].Name << ";" << endl;
   }

   Out << ""
       Out << "\t" << endl;
   Out << "\t// Default value" << endl;
   Out << "\treturn " << Enum.Items[0].Name << ";" << endl;
   Out << "}" << endl << endl;
}

/**
inline ManagedEnum <NativeEnum>_To_<ManagedEnum>(NativeEnum V)
{
   for_each value in NativeEnum:

   if (NativeVal_i == V) return ManagedVal_i;

   return ... default ...;
}

inline NativeEnum <ManagedEnum>_To_<NativeEnum>()
{
   for_each value in ManagedEnum:

   if (ManagedVal_i == V) return NativeVal_i;

   return ... default ...;
}
*/

/// “акже придЄтс€ добавить в GetAppropriateNETType() проверки IsEnum

/// и при маршалировании вставл€ть не explicit-cast в int, а соответствующий .NET-enum
