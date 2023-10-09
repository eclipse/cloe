Cloe-Engine Lua Shell
=====================

Cloe Engine provides a small Lua shell that you can use as a REPL or a way to
run Lua scripts with access to the Cloe API without running a simulation.

It currently has the following features:

- Runs Lua files (passed as arguments)
- Runs Lua strings (passed with `-c` option)
- REPL session (by default or with `-i` flag)
- Session history (press Up/Down in interactive session)
- Multi-line editing (experimental)
- Automatic value printing (experimental)

You can start the Lua REPL with `cloe-engine shell`.

Hello World
-----------

Let's demo the various ways we can print "Hello world!" to the console.

### In the REPL

Start the REPL and enter in the statement `print("Hello world!")`:
```console
$ cloe-engine shell
Cloe 0.22.0 Lua interactive shell
Press [Ctrl+D] or [Ctrl+C] to exit.
> print("Hello world!")
Hello world!
>
```

### Running a command

Pass the string from the previous example to the shell with `-c`:
```console
$ cloe-engine shell -c 'print("Hello world!")'
Hello world!
```
You can pass more than one command with `-c` just by repeating it.


### Running a file

Create a file `hello.lua` with the following contents:
```lua
print("Hello world!")
```
Now run it with `cloe-engine shell`:
```console
$ cloe-engine shell hello.lua
Hello world!
```

Multi-Line Editing
------------------

If the statement entered on a line looks complete, the shell will run it.
If there is an error in parsing indicating that the statement looks incomplete,
the shell will prompt you for more input:
```
> print(
>> "Hello world!"
>> )
Hello world!
```
This isn't so important for the above example, but for loops, functions, and
if-statements, it is:
```
> function a()
>> print(
>> "Hello world!"
>> )
>> end
> a()
Hello world!
```

Whitespace
----------

Lua does not care about whitespace very much. This means you can replace
all newlines with spaces and the code works the same.

Consider the following block of code:
```lua
print("...")
io.write("[")
for _, v in ipairs({1, 2, 3}) do
    io.write(v .. ",")
end
io.write("]\n")
print("---")
```
This can be minified in the following simple ways:

1. Newlines can be replaced with spaces.
2. Parentheses around plain strings and tables can be removed.
3. Spaces before and after commas, quotes, parentheses, and brackets can be removed.

This leads to the following minified code:
```lua
print"..."io.write"["for _,v in ipairs{1,2,3}do io.write(v..",")end print"]"print"---"
```
This means that sending whole blocks of code from the command line or from
another application or from code generation is a lot easier.
```
$ cloe-engine shell -c 'print"..."io.write"["for _,v in ipairs{1,2,3}do io.write(v..",")end print"]"print"---"'
...
[1,2,3,]
---
```
Of course I don't expect you'd really do this kind of crazy minification, but
it demonstrates just how little Lua cares about whitespace.

:::{note}
This one little quirk can provide significant benefits over the Python
scripting language, because it's very easy to compose generated code without
running into syntax errors because of indentation requirements.
:::
