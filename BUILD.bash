#!/bin/bash

logfile="squared7.log"

cmd="${0}"
cmdr="$(realpath "${cmd}")"
wd="$(dirname "$(realpath "${0}")")"

run=0

[ "${1}" == '-r' ] || [ "${1}" == '--run' ] && run=1

(cd "${wd}" && gcc -o squared7 main.c -lncurses -lpanel -lm) && {
    [ ${run} -gt 0 ] && {
        (cd "${wd}" && ./squared7 2>"${logfile}")
        # FIXME: Shouldn't need to reset, echo() and nocbreak() should have fixed this :(
        reset
    }
}
