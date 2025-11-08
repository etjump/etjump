#!/bin/bash

for i in $(find src -name 'etj_*.cpp' -o -name 'etj_*.h' -o -name 'etj_*.hpp'); do
  if ! grep -q Copyright "$i"; then
    echo "/*" >>"$i.new"
    cat LICENSE | while read line; do echo " * $line"; done >>"$i.new"
    echo $' */\n' >>"$i.new"
    cat "$i" >>"$i.new"
    mv "$i.new" "$i"
  fi
done
