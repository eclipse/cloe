Developer's Guide
=================

The information in this chapter is primarily relevant for those interested
in building Cloe from source, writing their own controller, or contributing
to the project.

For installation instructions please refer to the :doc:`installation guide <install>`.

In order to contribute to the project, all you need to do is create a branch in
the repository or a fork of the repository, and create a pull request. Before
you do so however, consider the following points:

- Before you work too long on a contribution, please first contact us at our
  developers mailing list `cloe-dev@eclipse.org`_.

- If you do not intend to create a pull request, do not create a branch in our
  repository. Instead, fork Cloe to your own repository.

- When contributing, please try to use the same style as the project and the
  files that you modify. This will make it easier for us to focus on the
  essential changes, and more likely to merge the pull request back.

If this is your first time contributing to the project, consider at least
skimming through all the documents in this section, especially
:doc:`develop/git-guidelines`.

.. rubric:: Writing Quality

Code that we write has two primary audiences: developers and compilers. The
quality of the writing – whether code or documentation – is as important in
software development as it is for publishers of books, magazines, articles, and
news. An argument can be made that it's often more critical in code; you spend
a lot more time with a paragraph of code than you do with a paragraph in
a block of text. Additionally, the conciseness of code means that
a misunderstanding can have more significant consequences than misunderstanding
of some paragraph in a book. As professionals, our code is our resume and we
will strive for excellence in all we do.

When submitting a pull-request, please bear this in mind – we will almost
certainly require some changes before accepting.

----

.. toctree::
   :maxdepth: 2

   develop/building-cloe
   develop/directory-structure
   develop/plugin-structure

   develop/new-controller
   develop/integrating-a-controller
   develop/debugging-a-controller
   develop/debugging-cloe

   develop/recording-with-asciinema
   develop/git-guidelines
   develop/cpp-guidelines
   develop/python-guidelines
   develop/cloe-ui

.. _cloe-dev@eclipse.org: mailto:cloe-dev@eclipse.org?subject=Planning%20a%20contribution%20to%20Cloe
