#!/bin/sh

if [ ! -f ./Doxyfile ]; then
    echo "Doxyfile not found!"
    echo "Are you in the right directory?"
    echo "Run this script from the repo's root directory like this:"
	echo "    scripts/make_docs.sh"
    exit
fi

rm -rf ./docs/html
doxygen
