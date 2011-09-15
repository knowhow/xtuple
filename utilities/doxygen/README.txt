Dependencies:
- You must have Doxygen 1.6 or later.
- You must have the environment variable QTDIR set to the Qt installed directory
    and exported.
- You must have the Qt docs installed in QTDIR/doc/html.

Steps to run:

  1 $ svn co the-right-xtuple-code-url
  2 $ cd .../xtuple[/tag/tag_name]
  3 $ qmake ; make
  4 $ edit utilities/doxygen/Doxyfile.public
      # set PROJECT_NUMBER to match version.cpp
      # fix the URL in TAGFILES to match the Qt version we're using
  5 $ doxygen utilities/doxygen/Doxyfile.public

Then copy the resulting html directory to the web server to publish the docs.

Note: The build in step 3 is required to get the list of widgets
      from the .ui files.

First pass at usage guidelines; comments welcome.
- document in the .cpp file instead of the .h as much as possible
  (e.g. describe methods in the .cpp but enums in the .h)
- if the only items in the .cpp are class members, start the .cpp with \class;
  if there are globals or statics in the .cpp, start with \file and continue
  with \class?
- use /** to start doxygen comments, not /*!
- use \ style directives, not @
- use \brief instead of relying on doxygen to guess for you.
- put % in front of words that you're using casually that happen to be
  names of classes/functions/... so they don't get converted to links
- tag order seems to be important. put the tags you use in the following order:
    /** \brief

        long description

        \dontinclude samplefile.js
          bits of samplefile.js using \skip, \until, ...

        more long description

        \param

        \return

        \see
        \see samplefile.js
     */
    the method/function definition
    /** \example */


Futures:

* Figure out how to make examples work (see scripttoolbox.cpp for more info)
* How deep should we go into how the internals work?
* Should we restrict the docs to simply what the class and its components do?
* How rich should the See Also tagging be?

Look at turning on HAVE_DOT and using the various graph options.
Some of particular are listed below. However, even if we don't use
dot, we should look at turning on CLASS_DIAGRAMS. The inheritance
hierarchy is pretty flat right now but should get deeper over time.
If so, CLASS_DIAGRAMS would be helpful. It would be really nice if
we could figure out how to build diagrams of JavaScript object
inheritance if we ever start writing JS code that way.

Look at http://ww.mcternan.me.uk/mscgen and Doxygen's MSCGEN_PATH
option.  We could use Doxygen calling mscgen to document the default
signal/slot connections so scripters know what connections they
can/should break to accomplish a desired goal.

Look at turning on COLLABORATION_GRAPH. This would be more useful
for people working in the core. One problem with it, though, is
that the diagrams are really hairy - we have a lot of interdependencies.
On the other hand, this would provide good data on ways we could
simplify the code.

Look at turning off EXTRACT_ALL once we've documented everything.

Look into documenting stored procedures. We would have to write a
parser for PL/PGSQL and a filter to convert -- comments to C- or
C++-style comments. The filter could be run on all source files
with a .sql suffix.  We might be able to do the same with COMMENTS
on tables and their columns and/or views and their columns. Maybe
we should create COMMENTS on functions, too, so everyone gets them
in the db.
