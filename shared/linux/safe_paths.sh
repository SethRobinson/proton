# Source this file; all destructive callers must supply a checked absolute root
# and a literal child name whenever possible. No implicit working-directory root.
proton_child_path() (
    proton_root=${1:?Cleanup root is required}
    proton_child=${2:?Cleanup child is required}
    case "$proton_root" in /|//|''|*[\*\?\[]*) return 1;; /*) ;; *) return 1;; esac
    case "$proton_child" in ''|/*|*\\*|*:*|*[\*\?\[]*) return 1;; esac
    case "/$proton_child/" in *'/../'*|*'/./'*|*'//'*) return 1;; esac
    # A physical root and no linked intermediate child directories keep cleanup
    # within this tree. Reject links even when their current destination is safe.
    proton_physical=$(CDPATH= cd -- "$proton_root" && pwd -P) || return 1
    [ "$proton_physical" = "$proton_root" ] || return 1
    proton_probe=$proton_root
    proton_rest=$proton_child
    while :; do
        proton_part=${proton_rest%%/*}
        proton_probe=$proton_probe/$proton_part
        [ ! -L "$proton_probe" ] || return 1
        case "$proton_rest" in */*) proton_rest=${proton_rest#*/};; *) break;; esac
    done
    printf '%s/%s\n' "$proton_root" "$proton_child"
)

proton_remove_tree() (
    proton_root=${1:?Cleanup root is required}
    proton_child=${2:?Cleanup child is required}
    proton_child_path "$proton_root" "$proton_child" >/dev/null || return 1
    rm -rf -- "${proton_root:?}/${proton_child:?}"
)
