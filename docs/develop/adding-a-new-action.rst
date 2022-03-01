Addding a New Action
====================

Trigger actions can be added in your Cloe plugin in the ``enroll`` method:

.. code-block:: cpp

   class YourPlugin : public cloe::Controller {
      // ...

      void enroll(cloe::Registrar& r) override {
         // This is where you add your calls to register various things with
         // the cloe engine, including actions.
         r.register_action<YourActionFactory>();
      }
   }

And it looks like what ``register_action`` wants is a ``cloe::ActionFactoryPtr``,
but why? Well, trigger actions are most often created in stackfiles, as
something compatible with JSON. The ``ActionFactory`` takes a JSON object or
a string, and makes an ``Action`` out of it.

Defining an Action
------------------

Let's look at a simple action defined in the runtime package in the file
``cloe/trigger/example_actions.hpp``: ``Log``.

We can see there is an include for ``<cloe/trigger.hpp>``. This file can also
be found in the runtime package, and you should have a look at the
documentation there for the ``Action`` interface.

Then we come to the source for the ``Log`` action:

.. code-block:: cpp

   class Log : public Action {
    public:
     Log(const std::string& name, LogLevel level, const std::string& msg)
         : Action(name), level_(level), msg_(msg) {}
     ActionPtr clone() const override { return std::make_unique<Log>(name(), level_, msg_); }
     void operator()(const Sync&, TriggerRegistrar&) override {
       logger()->log(level_, msg_.c_str());
     }
     bool is_significant() const override { return false; }

    protected:
     void to_json(Json& j) const override {
       j = Json{
           {"level", logger::to_string(level_)},
           {"msg", msg_},
       };
     }

    private:
     LogLevel level_;
     std::string msg_;
   };

The constructor is only really relevant for the ``LogFactory``, so we'll ignore
that for now.

Then there is the ``clone()`` method. This should return another new instance
of this action that can do the same thing again. This is important because the
user can request that actions be repeated, which results in clones occurring.

Now we come to the ``operator()`` method. This is where the real work of the
action occurs. It takes two arguments, which you will probably never use. If
you need them, you'll know.
Inside the body of this function you can do what the action says it will do.

For the ``is_significant()`` method, we tell Cloe whether this action can
affect the simulation. If in doubt, answer yes. The ``Log`` action is one of
the few actions that is not significant.

Finally, we need to implement the ``to_json()`` method. This should provide the
extra fields in the JSON representation that are needed to recreate this object
with the ``LogFactory``. Note that there is no name of this action. That's
because that is added automatically by Cloe, in order to prevent errors from
slipping in.

Defining the ActionFactory
--------------------------

Defining the ``ActionFactory`` is often more work than the action itself. We
need to think about how we want to allow the action to be created.
At first glance, the implmentation looks simple:

.. code-block:: cpp

   class LogFactory : public ActionFactory {
    public:
     using ActionType = Log;
     LogFactory() : ActionFactory("log", "log a message with a severity") {}
     TriggerSchema schema() const override;
     ActionPtr make(const Conf& c) const override;
     ActionPtr make(const std::string& s) const override;
   };

But then we see that the implementation for three methods is not inline. We'll
have a look at each of these in turn, but first: the using statement.
This statement defines what the output type of this action factory is, and is
necessary for properly registering the action factory.

The constructor of the action factory simply calls the super-constructor and
supplies the *default* name and the description of the *action*.

The ``schema()`` method is used to define the trigger schema, which among other
things, lets Cloe validate input and also document the action.

.. code-block:: cpp

   TriggerSchema LogFactory::schema() const {
     return TriggerSchema{
         this->name(),
         this->description(),
         InlineSchema("level and message to send", "[level:] msg", true),
         Schema{
             {"level", make_prototype<std::string>("logging level to use")},
             {"msg", make_prototype<std::string>("message to send").require()},
         },
     };
   }

The ``make(const Conf&)`` method takes the object configuration, and reads the
variables that are necessary for configuration.

.. code-block:: cpp

   ActionPtr LogFactory::make(const Conf& c) const {
     auto level = logger::into_level(c.get_or<std::string>("level", "info"));
     return std::make_unique<Log>(name(), level, c.get<std::string>("msg"));
   }

The ``make(const std::string&)`` method takes a string, and tries to parse this
into something that it can fill into a Conf and pass on to the method above.
This is often the most complex function, but it makes using triggers by
hand much much easier. This just takes a string in the format: ``level:
message`` and packs it into the JSON object structure the simpler ``make()``
method needs.

.. code-block:: cpp

   ActionPtr LogFactory::make(const std::string& s) const {
     auto level = spdlog::level::info;
     auto pos = s.find(":");
     std::string msg;
     if (pos != std::string::npos) {
       try {
         level = logger::into_level(s.substr(0, pos));
         if (s[++pos] == ' ') {
           ++pos;
         }
         msg = s.substr(pos);
       } catch (...) {
         msg = s;
       }
     } else {
       msg = s;
     }

     auto c = Conf{Json{
         {"level", logger::to_string(level)},
         {"msg", msg},
     }};
     if (msg.size() == 0) {
       throw TriggerInvalid(c, "cannot log an empty message");
     }
     return make(c);
   }
