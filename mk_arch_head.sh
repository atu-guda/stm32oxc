#!/bin/bash
dstt=$( date '+%Y%m%d' )
proj='oxc'
git archive --format=tar --prefix=${proj}-${dstt}/ -o ../${proj}-${dstt}.tar HEAD
gzip ../${proj}-${dstt}.tar
chmod 640 ../${proj}-${dstt}.tar*


