#!/bin/bash
# Script: inline.sh
# Purpose: Modifies a C++ header file by inserting the `inline` keyword before each
# member function definition of the `curling::Request` class.
# Notes: This is useful when using header-only libraries to prevent multiple definition errors
# during the linking phase when including the header in multiple translation units.

sed -i '/Request::/ {
  /^[[:space:]]*inline/ ! {
    /(\|);$/ s/^[[:space:]]*/&inline /
  }
}' ./header_only/curling.hpp
