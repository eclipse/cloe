User Cloe Configuration
=======================

In the previous section we saw how multiple stackfiles can be merged into
a single one. Sometimes, a user might want to augment every simulation with
their own configuration. One way to do that is to pass an extra "user
stackfile" to each call to ``cloe-engine``. Another method is to use the user
configuration, which will be introduced in this section.

.. literalinclude:: ../../engine/src/main_usage.hpp
    :start-at: By default, Cloe will include certain discovered system and user configuration
    :end-at: Thus, this behavior can be disabled with the --no-system-confs flag.
    :language: text
