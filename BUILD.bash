#!/bin/bash

logfile="7curses.log"

cmd="${0}"
cmdr="$(realpath "${cmd}")"
wd="$(dirname "$(realpath "${0}")")"

run=0

[ "${1}" == '-r' ] || [ "${1}" == '--run' ] && run=1

(cd "${wd}" && gcc -o 7curses main.c -lncurses -lpanel) && {
    [ ${run} -gt 0 ] && {
        (cd "${wd}" && ./7curses 2>"${logfile}")
    }
}
