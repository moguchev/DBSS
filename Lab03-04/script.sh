#!/bin/bash
# EXAMPLES:
# ./script.sh -f 1.txt --out 2.txt --err 3.txt ./hello [args...]
# ./script.sh --out 1.txt --err 2.txt ./hello [args...]
# ./script.sh --out 1.txt --err 1.txt ./hello [args...]
# ./script.sh --out 1.txt ./hello [args...]
# ./script.sh --err 1.txt ./hello [args...]
# ./script.sh ./hello [args...]

while [ -n "$1" ]
do
case "$1" in
    --out) out="$2"
        shift ;;
    --err) err="$2"
        shift ;;
    -f) file="$2"
        shift ;;
    *) prog="$1"
    
        break ;;
esac
shift
done

exec 6>&1

if [[ -n $out ]] && [[ -n $err ]]
then
    if [ $out != $err ]
    then
        exec 1> $out                                                                      
        exec 2> $err
    else
        exec 1> $out                                                                      
        exec 2>&1
    fi
elif [[ -n $out ]] && [[ -z $err ]]
then
    exec 1> $out
elif [[ -z $out ]] && [[ -n $err ]]
then
    exec 2> $err
fi

if [[ -n $file ]]
then
    exec < $file 
fi

echo $($prog $@)
code=$?

echo "Exit code: $code" >&6

if [[ -n $out ]]
then
    echo "Exit code: $code" >&1
fi
if [[ -n $err ]]
then
    if [[ $out != $err ]]
    then
        echo "Exit code: $code" >&2
    fi
fi

exec 6>&-