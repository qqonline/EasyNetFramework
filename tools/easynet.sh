#!/bin/bash

Usage()
{
	echo -e "\neasynet -p [-t all|bin|text] ClassName\n"
	echo "  -p : Create ProtocolFactory."
}

function GenProtocolFactory()
{
	type=$1
	ClassName=$2
	echo "$type $ClassName"	
}

if [ $# -lt 2 ];then
	Usage;
	exit;
fi

if [ "$1" == "-p" ];then
	type="all"
	if [ "$2" == "-t" ];then
		if [ $# -ne 4 ];then
			echo "ERROR: the number of params is no enough."
			Usage;
			exit;
		elif [ "$3" != "all" ] && [ "$3" != "bin" ] && [ "$3" != "text" ];then
			echo "ERROR: type of protocol factory is invalid."
			Usage;
			exit;
		fi
		type=$3;
		ClassName=$4;				
	
	else
		ClassName=$2;
	fi
	
	echo $type $ClassName;
	GenProtocolFactory $type $ClassName;
fi
