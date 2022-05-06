#!/usr/bin/env sh

lookup() {
	files=`ls -A $1`
	for i in $files ; do
		rpath=`realpath $1/$i`
		if test -d $rpath ; then
			lookup $rpath
		fi
		if test -r $rpath ; then
			var=`grep $string $rpath`
			if test $? = 0 ; then
				ls -l $rpath | awk '{print $9, $5}'
			fi
		fi
	done
}

string=$1
dirpath=`realpath $2`

lookup $dirpath
