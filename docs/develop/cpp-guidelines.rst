C++ Guidelines
==============

C++ is a powerful language, but you can easily shoot yourself in the foot with
it if you're not careful. Thankfully, there are many resources out there that
help with writing clean and effective C++ code.

Coding Style
------------

The first rule is: be consistent.
The importance of consistency increases as we move deeper within a scope:
in a library, in a file, in a namespace, in a class, in a function, and finally
in a block.

This project uses the `Google C++ Style Guide`_ with the following exceptions
and clarifications:

- Line length: 80 characters if possible, 100 characters if necessary
- Naming files: ``my_useful_class.h`` and ``my_useful_class.cpp``
- Naming variables: ``price_count_reader`` and ``num_dns_connections``
- Naming member variables: ``price_count_reader_`` and ``socket_``
- Naming types: ``UrlTable`` and ``RosController``
- Naming functions: ``url_table`` and ``ros_controller``
- Naming constants and enum values: ``MY_DEFINITION`` and ``GET, POST, PUT, DELETE``

Code Formatting
---------------

For code formatting like indentation, whitespaces, line wrapping and so on
we use the tool `ClangFormat <https://clang.llvm.org/docs/ClangFormat.html>`__.
The definition file ``.clang-format`` is located in the cloe root directory.
Thus it is automatically applied for any source file in the cloe repository you
run clang-format on.

You can guard code sections which are less readable after formatting enclosing
them like:

.. code-block:: cpp

    // clang-format off
    Code i = "would rather format myself"
             "because it's more readable";
    // clang-format on

It is also good practice to configure your editor to format on save.
In Visual Studio Code your config file should then contain lines like:

.. code-block:: json

   {
      "C_Cpp.clang_format_path": "/usr/bin/clang-format-6.0",
      "editor.formatOnSave": true
   }


Modern C++
----------

C++ is a huge language, which we want to use as effectively as possible.
Modern C++, such as C++11 and C++14 provide tools and idioms that solve
and simplify certain constructs and processes in C++.
There are several classic C++ books/references available that are helpful to
read:

- *EC3*:  Effective C++ Third Edition (Scott Meyers)
- *ESTL*: Effective STL (Meyers)
- *EMC*:  Effective Modern C++ (Meyers)
- *CCS*:  C++ Coding Standards (Herb Sutter, Alexandrescu)
- *CSL*:  The C++ Standard Library (Josuttis)

These authors are all professionals in C++, and their books are top-notch.
It is valuable to learn from and take advice from the books into account.
If you come across constructs that you don't know, consider having a look
there. If you find that you learned something new from the books that you want
to contribute to the code, and it is not there already, consider
trickling the information learned into our code with a special comment:

.. code-block:: cpp

    // NOTE(C++/EMC, #5, p.37): I use the auto keyword as described in Item 5.
    for (const auto& p : list) {
        // ...
    }

By using ``TODO(C++/<ref>)`` and ``NOTE(C++/<ref>)`` as the initial part of the
comment, we can easily search for it in the code. The ``<ref>`` part refers to an
abbreviation of the book, e.g. *EC3* or *CCS*. That way, when we see one of
these and are puzzled, we know where to look it up. This should enable a gentle
journey towards writing modern C++! :-D

.. _Google C++ Style Guide: https://google.github.io/styleguide/cppguide.html
