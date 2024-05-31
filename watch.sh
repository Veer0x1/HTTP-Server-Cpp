#!/bin/sh

fswatch -0 src | while read f
do
    make
done

