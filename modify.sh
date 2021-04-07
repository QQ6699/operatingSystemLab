#!/bin/bash

#QingqingYan 309598
#VARIABLES
R_MODE=0
SED_MODE=0
MODE=0  #lowercasing:1 uppercasing:2 sed_pattern:3
SED_PATTERN=""
# the name of the script without a path
name=`basename $0`

# function for printing error messages to diagnostic output
error_msg() 
{ 
        echo "$name: error: $1" 1>&2 
}

# function for servicing -r option
with_arg() 
{ 
        echo "second parameter = $1"
	if test -z "$1"
        then
                error_msg "missing argument for -r after -r,parameters should not be empty.Parameters should be -l | -u |<sed pattern>"
		exit
        fi
}


# if no arguments given
if test -z "$1"
then
cat<<EOT 1>&2

usage:
  $name [-r] [-l|-u] <dir/file names...>
  $name [-r] <sed pattern> <dir/file names...>
  $name [-h]

$name correct syntax examples: 
  $name -r -u /home/shell/file1.txt
  $name -l FILE1.txt
  $name -h
  $name -r 's/\([a-z]\+\)\([0-9]\+\)/\1-\2/' file1.txt 

$name incorrect syntax example: 
  $name -u -l test.txt
  $name -r

EOT
fi

modifyName(){
                    dir_name=`dirname $1`
                    ori_fullname=`basename $1`
                    ori_filename=$(echo $ori_fullname | sed 's/\.[^.]*$//')
                    extension=$(echo $ori_fullname | sed 's/^.*\.//')
                    echo "ori_fullname= $ori_fullname ori_filename= $ori_filename extension= $extension"
                    case $MODE in
                            1)new_filename=`echo $ori_filename | tr [:upper:] [:lower:]`>/dev/null
                                   ;;
                            2)new_filename=`echo $ori_filename | tr [:lower:] [:upper:]`>/dev/nulli
                                   ;;
                            3)new_filename=`echo "$ori_filename" |  sed $SED_PATTERN` ;;
                   esac
                    mv $dir_name/$ori_fullname $dir_name/$new_filename.$extension


}

modifyFiles(){
        #check if the file exists
        if [ ! -f "$1" -a ! -d "$1" ] ; then
                        `echo "file or directory  --$1--does not exist" 1>&2`
                        return
        fi

        if [ ! -w "$1" -a -f "$1" ]; then
                `echo "File $1 cannot be modified" 1>&2`
                return
        fi

        if [ $MODE -ne 0 -a $R_MODE -eq 1 ] ; then
                if [ -d "$1" -o -f "$1" ] ; then
                  dir=`dirname "$1"`
                  files=`find $dir -type f`
		  for i in $files 
                  do 
                    modifyName $i 
                  done
                fi
	elif [ $MODE -ne 0 -a $R_MODE -eq 0 ] 
       	then
		modifyName "$1"

        fi
}





# do with command line arguments
while test "x$1" != "x"
do
        case "$1" in
                -r) R_MODE=1
			with_arg "$2"
			;;
                -l) MODE=1;;
                -u) MODE=2;;
		-h)
 cat<<EOT 1>&2

usage:
  $name [-r] [-l|-u] <dir/file names...>
  $name [-r] <sed pattern> <dir/file names...>
  $name [-h]

$name correct syntax examples: 
  $name -r -u /home/shell/file1.txt
  $name -l FILE1.txt
  $name -h
  $name -r 's/\([a-z]\+\)\([0-9]\+\)/\1-\2/' file1.txt 

$name incorrect syntax example: 
  $name -u -l test.txt
  $name -r

EOT
                     exit;;
                    
                -*) error_msg $1
		       	exit 1 ;;
                *) 
			if [ ! -f "$1" -a ! -d "$1" ] 
		       	then
                         MODE=3
                         SED_PATTERN="$1"
			 echo "sed patternxia $1"
			 shift
			 echo "shift1 $1"
			 
		 else
			 echo "please input -l or -u before you input folder or file name or file|folder $1  doesn't exist"

			fi
			;;
        esac
	if [ $MODE -ne 0 ] && [ $MODE -ne 3 ]
	then
		shift
		if [ ! -f "$1" -a ! -d "$1" ] ;then
                           echo "please input file name or folder after -l -u or file name or file|folder $1  doesn't exist"
			   exit
		fi

		modifyFiles "$1"
		exit
	#elif [ $R_MODE -eq 1 ]&&[ $MODE -eq 3 ]
	elif [ $MODE -eq 3 ]
	then
	       if [ ! -f "$1" -a ! -d "$1" ] ;then
                    echo "please input file name or folder after <sed pattern>.Or your file name or file|folder $1  doesn't exist"
                    exit
                fi

		echo "diaoyongqian $1"
		modifyFiles "$1"
		exit
	fi
	shift
done
