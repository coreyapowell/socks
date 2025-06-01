# socks
socks library for c / quickjs / others

I keep needing and writing the same code! This is a small adaptable library in both C and Javascript to simplify cross scripting of socket project.
This thing is starting small, but I already have an HTTP server that whips up in very few lines of Javascript using QuickJS.

The strategy I've seen other people using makes more sense, to write a c library and then make that plug into javascript. I admit, the Quickjs-ng team have their compiler settings on torture! You have to install 7 or so declarations/functions exactly to just get an class into QJS with no warnings! so there are macros and scripts I'm using. I don't want to submit something full of macros, I want to leave behind a legacy file that works as an example to interfacing QuickJS .... something I found nodeJS lacking over the decade. So tis project is going to use QuickJS heavily to generate and filter code, but the plan is to separate the C portion. 

I've written a skeleton 10 times in my life of the same project, I might as well put it in a library and leave it to my minions.

