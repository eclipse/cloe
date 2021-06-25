Contributing a New Controller
=============================

One of the design goals of Cloe is that adding a new controller should be
a fairly straight-forward process. This is currently not as great as we'd
like it to be, but the more the interfaces stabilize, the easier it should
become.

This document provides a step-by-step process for adding a new controller to
the Cloe project. Controllers live in their own repository and build to
specific versions of Cloe.

Before anything else, a quick note on naming.  When you see ``<controller>``
then replace that segment with your lowercase controller name. In the same way
for ``<Controller>`` and ``<CONTROLLER>``. The main critical variables where
the name plays a big role are:

   - environment root directory variable: ``<CONTROLLER>_ROOT``
   - controller name: ``<controller>``
   - controller namespace: ``<controller>``

For starters, you will need to have the Cloe development files available to
build a controller to Cloe. You can either clone the Cloe repository, or
install the Debian package `cloe`.

.. todo:: This section will be updated as soon as we have time.
