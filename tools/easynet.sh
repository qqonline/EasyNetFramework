#!/bin/bash
TEMPLATEDIR=/usr/include/easynet/template

Usage()
{
	echo -e "\neasynet -p [-t all|bin|text] ClassName"
	echo -e "easynet -a ClassName [-m]\n"
	echo "  -p : Create ProtocolFactory. -t : optional."
	echo "  -a : Create Application Instance. -m : optional. gen server main"
}

function GenProtocolFactory()
{
	local type=$(echo $1|tr '[a-z]' '[A-Z]')
	local ClassName=$2
	local Subfix=$3
	local Filename=""

	if [ "$Subfix" == "h" ];then
		cp ${TEMPLATEDIR}/TemplateProtocolFactory.th $ClassName.tmp
		Filename=$ClassName.h
		
		if [ "$type" == "A" ];then
			echo -e "\tprotocol type:Bin And Text\n"
		elif [ "$type" == "B" ];then
				echo -e "\tprotocol type:Bin...\n"
		else
			echo -e "\tprotocol type:Text...\n"
		fi
	elif [ "$Subfix" == "cpp" ];then
		cp ${TEMPLATEDIR}/TemplateProtocolFactory.tpp $ClassName.tmp	
		Filename=$ClassName.cpp
	fi

	sed -i "s/TemplateProtocolFactory/$ClassName/g" $ClassName.tmp
	local UP_ClassName=$(echo $ClassName|tr '[a-z]' '[A-Z]')
	sed -i "s/TEMPLATEPROTOCOLFACTORY/${UP_ClassName}/g" $ClassName.tmp

	awk -F"<<TP>>" 'BEGIN{pattern_flag=0;match_flag=0;}
	{
		if($2 == "")
		{
			if(pattern_flag==0 || match_flag==1)
			{
				#print pattern_flag, match_flag, "##################"
				print
			}
		}
		else
		{
			if(pattern_flag==1)
			{
				pattern_flag=0;
				#print "pattern off @@@@@@@@@@"			
			}
			else
			{
				#print "pattern on @@@@@@@@@@"
				pattern_flag=1;
			}

			if(match($2,t) > 0)
			{
				match_flag=1;
				#print "match************"
			}
			else
			{
				match_flag=0;
				#print "no match*********"
			}
		}
	}' t=$type $ClassName.tmp > $Filename

	rm $ClassName.tmp
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

if [ "$1" == "-p" ];then
	type="all"
	ClassName=""
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
	
	echo "Gen ProtocolFactory class file: ${ClassName}.h..."
	GenProtocolFactory ${type:0:1} ${ClassName} h;
	
	echo "Gen ProtocolFactory class file: ${ClassName}.cpp..."
	GenProtocolFactory ${type:0:1} ${ClassName} cpp;

elif [ "$1" == "-a" ];then
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

