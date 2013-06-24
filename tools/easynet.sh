#!/bin/bash
TEMPLATEDIR=/usr/include/easynet/template

DATE=`date +"%Y-%m-%d"`
AUTHOR=`whoami`

Usage()
{
	echo -e "easynet -a ClassName [-m]\n"
	echo "  -a : Generate Application Instance Class."
	echo "  -m : Optional. gen server main framework."
	echo "  -p : Generate ProtocolFactory Class."
}

function GenAppInstance()
{
	local ClassName=$1
	local Subfix=$2
	local Filename=""

	if [ "$Subfix" == "h" ];then
		if [ -f "$ClassName.h" ];then
			echo "ERROR: target file already exists: $ClassName.h ... [exit]"
			exit
		fi

		cp ${TEMPLATEDIR}/TemplateAppInterface.th $ClassName.h
		Filename=$ClassName.h
	elif [ "$Subfix" == "cpp" ];then
		if [ -f "$ClassName.cpp" ];then
			echo "ERROR: target file already exists: $ClassName.cpp ... [exit]"
			exit
		fi

		cp ${TEMPLATEDIR}/TemplateAppInterface.tpp $ClassName.cpp
		Filename=$ClassName.cpp
	fi
	sed -i "s/TemplateAppInterface/$ClassName/g" $Filename
	local UP_ClassName=$(echo $ClassName|tr '[a-z]' '[A-Z]')
	sed -i "s/TEMPLATEAPPINTERFACE/${UP_ClassName}/g" $Filename
	sed -i "s/_#CreateDate#_/${DATE}/g" $Filename
	sed -i "s/_#AuthorName#_/${AUTHOR}/g" $Filename
}

function GenAppMain()
{
	local ClassName=$1

	if [ -f "${ClassName}Main.cpp" ];then
		echo "ERROR: target file already exists: ${ClassName}Main.cpp ... [exit]"
		exit
	fi

	cp ${TEMPLATEDIR}/TemplateAppInterfaceMain.tpp ${ClassName}Main.cpp
	sed -i "s/TemplateAppInterface/$ClassName/g" ${ClassName}Main.cpp
	sed -i "s/_#CreateDate#_/${DATE}/g" ${ClassName}Main.cpp
	sed -i "s/_#AuthorName#_/${AUTHOR}/g" ${ClassName}Main.cpp
}

function GenProtocolFactory()
{
	local ClassName=$1

	### h
	cp ${TEMPLATEDIR}/TemplateProtocolFactory.th $ClassName.h
	sed -i "s/TemplateProtocolFactory/$ClassName/g" $ClassName.h

	local UP_ClassName=$(echo $ClassName|tr '[a-z]' '[A-Z]')
	sed -i "s/TEMPLATEPROTOCOLFACTORY/${UP_ClassName}/g" $ClassName.h
	sed -i "s/_#CreateDate#_/${DATE}/g" $ClassName.h
	sed -i "s/_#AuthorName#_/${AUTHOR}/g" $ClassName.h

	### cpp
	cp ${TEMPLATEDIR}/TemplateProtocolFactory.tpp $ClassName.cpp
	sed -i "s/TemplateProtocolFactory/$ClassName/g" $ClassName.cpp
	sed -i "s/_#CreateDate#_/${DATE}/g" $ClassName.cpp
	sed -i "s/_#AuthorName#_/${AUTHOR}/g" $ClassName.cpp	
}


##########################  main  ##########################

if [ $# -lt 2 ];then
	Usage;
	exit;
fi

if [ "$1" == "-a" ];then
	if [ $# -lt 2 ];then
		Usage;
		exit;	
	fi
	echo "Generate Application instance class file: $2.h ..."
	GenAppInstance $2 h;

	echo "Generate Application instance class file: $2.cpp ..."
	GenAppInstance $2 cpp;
	
	if [ $# -gt 2 ] && [ "$3" == "-m" ];then
		echo "Generate ApplicationMain class file: $2Main.cpp ..."
		GenAppMain $2;
	fi

	echo -e "\nCompile Using:\n\t-I/usr/include/easynet -leasynet -lpthread -llog4cplus\n\nSee temp makefile: Makefile.tmp"
	echo "INCLUDE=-I/usr/include/easynet"> Makefile.tmp
	echo "LIBS=-leasynet -lpthread -llog4cplus" >> Makefile.tmp
elif [ "$1" == "-p" ];then
	echo -e "Generate ProtocolFactory class file:\n\t$2.h ...\n\t$2.cpp ..."
	if [ -f "$2.h" ];then
		echo "ERROR: ProtocolFactory class file: $2.h already exists ... [exit]"
		exit;
	elif [ -f "$2.cpp" ];then
		echo "ERROR: ProtocolFactory class file: $2.cpp already exists ... [exit]"
		exit;
	fi
	GenProtocolFactory $2;
	
	echo "Generate ProtocolFactory ... [OK]"
fi

