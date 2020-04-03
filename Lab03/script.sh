#!/bin/bash
# ./script.sh --out 1.txt --err 2.txt
while [ -n "$1" ]
do
case "$1" in
--out) out="$2"
echo "stdout: $out"
shift ;;
--err) err="$2"
echo "stderr: $err"
shift ;;
*) prog="$1"
echo "program: $prog"
break ;;

esac
shift
done

