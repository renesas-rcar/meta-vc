#!/bin/bash

# header and footer size of jic file
header=159
footer=69

usage()
{
    echo "usage: jic2bin -i inputfile [-o outputfile]"
}

while getopts i:o:h flag
do
    case "${flag}" in
	i) inputfile=${OPTARG};;
	o) outputfile=${OPTARG};;
	h) usage; exit;;
    esac
done

if [[ ! -v inputfile ]]; then
    echo "Please provide the input filename with the -i option!"
    usage
    exit 1
fi

if [[ ! -f ${inputfile} ]]; then
    echo "Input file does not exist!"
    exit 1
fi

tmpfile=$(mktemp /tmp/stripped-jic.XXXXX)
outputsize=$(($(stat -c '%s' ${inputfile}) - ${header} - ${footer}))

# strip header and footer from input file
dd if="${inputfile}" of="${tmpfile}" iflag=count_bytes,skip_bytes skip=${header} count=${outputsize}

# reverse bit order
if [[ -v outputfile ]]; then
    reverse-bits "${tmpfile}" > ${outputfile}
else
    reverse-bits "${tmpfile}"
fi

rm "${tmpfile}"

