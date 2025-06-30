#!/bin/bash
sed -i '/Request::/ {
  /^[[:space:]]*inline/ ! {
    /(\|);$/ s/^[[:space:]]*/&inline /
  }
}' ./header_only/curling.hpp
