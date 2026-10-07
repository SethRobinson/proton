#!/bin/sh
# Sent to the web host over stdin. Keep the remote deletion suffix literal.
set -eu
: "${HOME:?Remote HOME must be set}"
case "$HOME" in /|//) exit 1;; /*) ;; *) exit 1;; esac
[ "$(CDPATH= cd -- "$HOME" && pwd -P)" = "$HOME" ] || exit 1
case "${1-}" in
    rtbarebones|rtconsole|rtshader|rtsimpleapp) ;;
    *) echo 'Unknown web project; refusing cleanup' >&2; exit 1;;
esac
# rm does not traverse links inside the deleted tree, but ancestors must not
# redirect the named project outside the web tree.
for parent in "$HOME/www" "$HOME/www/web" "$HOME/www/web/$1" "$HOME/www/web/$1/WebLoaderData"; do
    [ ! -L "$parent" ] || exit 1
done
case "$1" in
    rtbarebones) rm -rf -- "${HOME:?}/www/web/rtbarebones/WebLoaderData";;
    rtconsole) rm -rf -- "${HOME:?}/www/web/rtconsole/WebLoaderData";;
    rtshader) rm -rf -- "${HOME:?}/www/web/rtshader/WebLoaderData";;
    rtsimpleapp) rm -rf -- "${HOME:?}/www/web/rtsimpleapp/WebLoaderData";;
esac
