#!/bin/bash

pdflatex -interaction=batchmode minsky
if [ $? -ne 0 ]; then exit 1; fi

pdflatex -interaction=batchmode minsky
latex2html -local_icons -info "" minsky
if [ $? -ne 0 ]; then exit 1; fi

# copy images, and mutate table of contents after latex2html suitable
# for our in house help file style

cp -f images/* minsky
sed -e 's/^\(  HREF=[^>]*\)>/\1 target="content">/' <minsky/minsky.html >tmp
mv tmp minsky/minsky.html

#check for any broken links
linkchecker -f ../linkcheckerrc minsky.html

