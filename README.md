LinderScript Database Compiler
==============================

Copyright (c) Sergey Kosarevsky, 2005-2019

Copyright (c) Viktor Latypov, 2007-2019

https://github.com/corporateshark/LSDC

lsdc@linderdaum.com

=============================

LSDC is a C++ header parsing tool that supports Linderdaum Engine reflection system and metadata.

=============================

Features:
---------

* Parse C++ headers and collect metadata
* Classes and their inheritence graph
* Enums
* Global constants
* Properties (custom syntax)

=============================

Build instructions:
------

```
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
```

=============================

Limitations (TODO):
------------

* Ad hoc C++ parser (full of crutches and does not support templates)
* Code generator is currently targeting only Linderdaum Engine reflection system

=============================

License:
--------

The source code is available under the terms of the modified GPL3 License: https://github.com/corporateshark/LSDC/blob/master/COPYING.txt
