#!/bin/sh

autoreconf -i
mkdir -p build
cd build && ./configure
