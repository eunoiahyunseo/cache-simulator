#!/bin/bash -e

# set -- -a -bc hello world
# echo "$@"

# echo $OPTIND

# getopts abc opt "$@"
# echo $opt, $OPTIND # 다음 옵션 "b"의 index값은 2가 된다.

# getopts abc opt "$@"
# echo $opt, $OPTIND # 다음 옵션 "b"의 index값은 2가 된다.

# getopts abc opt "$@"
# echo $opt, $OPTIND # 다음 옵션 "b"의 index값은 2가 된다.

# echo "$@"

# shift $((OPTIND - 1))

# echo "$@"

# OPTIND=1
# set -- -a xyz -b -c hello world

# getopts a:bc opt
# echo $opt, $OPTARG, $OPTIND

# getopts a:bc opt
# echo $opt, $OPTARG, $OPTIND

# getopts a:bc opt
# echo $opt, $OPTARG, $OPTIND

usage() {
        err_msg "Usage: $(basename "$0") -s <45|90> -p string"
        exit 1
}

err_msg() { echo "$@" ;}

while getopts ":s:p:" opt; do
        case $opt in
                s)
                        s=$OPTARG
                        ["$s" = 45 ] || ["$s" = 90] || { err_msg_s; usage(); }
                        ;;
                p)
                        p=$OPTARG
                        ;;
                
done