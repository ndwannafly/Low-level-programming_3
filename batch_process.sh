#!/bin/bash

cat queries.txt |
while read in; do
    echo "$in" | ./CLIENT 12345
done
