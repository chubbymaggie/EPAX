#!/usr/bin/env bash

#set -x verbose
set -e

function make_docs(){
(
    cd ${1}
    doxygen

    if [ "${3}" == "yes" ]; then
        for f in `find . -type f | grep -Pv Doxyfile`; do
            sed --in-place 's/Interface\.h/EPAX\.h/g' ${f}
        done
    fi

    cd latex
    for i in `seq 3`; do
        pdflatex refman.tex
    done
)
    ln -sf ${1}/latex/refman.pdf ${2}
}

make_docs user/   EPAX.pdf        no
make_docs devel/  EPAX_devel.pdf  no
