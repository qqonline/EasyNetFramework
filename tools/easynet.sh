#!/bin/bash
TEMPLATEDIR=/usr/include/easynet/template

Usage()
{
	echo -e "easynet -a ClassName [-m]\n"
	echo "  -a : Create Application Instance. -m : optional. gen server main"
}

function GenAppInstance()
{
	local ClassName=$1
	local Subfix=$2
	local Filename=""

	if [ "$Subfix" == "h" ];then
		cp ${TEMPLATEDIR}/TemplateAppInterface.th $ClassName.h
		Filename=$ClassName.h
	elif [ "$Subfix" == "cpp" ];then
		cp ${TEMPLATEDIR}/TemplateAppInterface.tpp $ClassName.cpp
		Filename=$ClassName.cpp
	fi
	sed -i "s/TemplateAppInterface/$ClassName/g" $Filename
	local UP_ClassName=$(echo $ClassName|tr '[a-z]' '[A-Z]')
	sed -i "s/TEMPLATEAPPINTERFACE/${UP_ClassName}/g" $Filename

}

function GenAppMain()
{
	local ClassName=$1
	
	cp ${TEMPLATEDIR}/TemplateAppInterfaceMain.tpp ${ClassName}Main.cpp
	sed -i "s/TemplateAppInterface/$ClassName/g" ${ClassName}Main.cpp
}

if [ $# -lt 2 ];then
	Usage;
	exit;
fi

if [ "$1" == "-a" ];then
	if [ $# -lt 2 ];then
		Usage;
		exit;	
	fi
	echo "Gen Application instance class file: $2.h..."
	GenAppInstance $2 h;

	echo "Gen Application instance class file: $2.cpp..."
	GenAppInstance $2 cpp;
	
	if [ $# -gt 2 ] && [ "$3" == "-m" ];then
		echo "Gen ApplicationMain class file: $2Main.cpp..."
		GenAppMain $2;
	fi
fi

