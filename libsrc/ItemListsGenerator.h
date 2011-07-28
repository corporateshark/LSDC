/**
 * \file ItemListsGenerator.h
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

// генерация списка, параметризованного индексами
void make_item_list( std::ostream& Out, const string& separator,
                     const string& prefix, const string& postfix,
                     const string& itemPrefix, const string& item,
                     int num, bool zero_based_pref = false, bool zero_based_item = false, const string& nonZeroPrefix = "" )
{
   char itemBuf[1024];
   char prefixBuf[1024];

   Out << prefix;

   for ( int i = 0 ; i < num ; i++ )
   {
      int itemIdx = i;
      int prefIdx = i;

      if ( !zero_based_pref ) { ++prefIdx; }

      if ( !zero_based_item ) { ++itemIdx; }

#if (_MSC_VER > 1300)
      sprintf_s( itemBuf, item.c_str(), itemIdx );
      sprintf_s( prefixBuf, itemPrefix.c_str(), prefIdx );
#else
      sprintf( itemBuf, item.c_str(), itemIdx );
      sprintf( prefixBuf, itemPrefix.c_str(), prefIdx );
#endif

      if ( i > 0 ) { Out << nonZeroPrefix; }

      Out << prefixBuf << itemBuf;

      if ( i < num - 1 )
      {
         Out << separator;
      }
   }

   Out << postfix;
}
