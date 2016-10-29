#! /bin/awk

BEGIN{
	crtFileName="";				# 当前文件名

	stDefLineNo="";				# 结构体定义所在行
	stMbrCnt=0;					# 结构体成员个数
	crtStName="";				# 当前处理的结构名
	crtReadSt="";				# 当前行解读出的 st
	bInNormalStDef=0;			# 是否处在常规结构体定义中
	bInAnonStDef=0;				# 是否处理匿名结构体定义中
	
	# 成员 map
	mapMbr["keyNone", "name"]="";
	mapMbr["keyNone", "father"]="";
	mapMbr["keynone", "line"]="";
	mapMbr["keynone", "file"]="";

	# 成员列表
	listMbr[1]="";
}

# 获取路径中的最后一个分量
function basename(strPath)
{
	n=split(strPath, arr, "/");
	return arr[n];
}

# 提取出一tag 行中记录的行号
function getLineNo(strLine)
{
	match(strLine, /line:([0-9]+)/, arr);
	return arr[1];
}

function getMbrPath(mbr)
{
	# 路径 stack
	stackPath[1]="";
	i = 1;
	crtMbr = mbr;
	mbrPath = "";
	
	for(i = 1; mapMbr[crtMbr, "father"] != ""; i++)
	{
		stackPath[i] = mapMbr[crtMbr, "name"];
		crtMbr = mapMbr[crtMbr, "father"];
	}

	# 拼接成由 . 访问路径
	for (j = i; j > 1; j--)
	{
		mbrPath = stackPath[j] "" ".";
	}
	mbrPath "" stackPath[1];

	return mbrPath;
}

# 
function endLastSt()
{
	# 进行打印
	bInNormalStDef = 0;
	bInAnonStDef = 0;
	mbrPath = "";
	crtStShowName=mapMbr[crtStName, "name"];
	crtMbrName = "";
	
	# 先输出结构体大小
	printf("printf(\"sizeof %s: %zu def at %s:%s\\n\", sizeof(%s));\n", crtStShowName, crtFileName, stDefLineNo, crtStShowName);

	print "is " stMbrCnt;
	#依次输出各个成员的 offset
	for(k=1; k<=stMbrCnt; k++)
	{
		crtMbrName = listMbr[k];
		mbrPath = getMbrPath(crtMbrName);
		crtMbrDefLine = mapMbr[crtMbrName, "line"];
		
		printf("printf(\"\toffsetof %s: %zu def at %s:%s\\n\", offsetof(%s));\n", \
			   mbrPath, crtFileName, crtMbrDefLine, mbrPath);
	}
}

function startNewNormalSt(stName)
{
	# 新的开始
	bInNormalStDef = 1;
	bInAnonStDef = 0;

	stMbrCnt = 0;
	
	crtStName=stName;				# 更新当前结构体

	mapMbr[stName, "name"] = "struct " "" stName;
	mapMbr[stName, "father"] = "";

}

function startNewAnonSt()
{
	bInNormalStDef = 0;
	bInAnonStDef = 1;
	
	stMbrCnt = 0;

}

# 匹配到结构体定义
/;"[ \t]+s/{

	print "匹配到结构体定义 at line: " FNR;
	
	if (1 == bInNormalStDef)
	{	# 终结上一个结构体
		endLastSt();
	}

	stDefLineNo= getLineNo($0);
	startNewNormalSt($1);	# ----------- 新结构体定义开始
}	

# 匹配到结构体成员定义
/;"[ \t]+m/{

#	print "匹配到结构体成员定义, at line: " FNR;
	
	crtFileName = basename($2);	# 更新当前文件名
	mbrName=$1;					# 当前成员名
	typeref="";
	father="";

	# 获取当前成员的父（考虑多重嵌套结构）
	arr[1] = "";
	lSts = match($0, /(struct|union):([^ \t]+)/, arr);
#	print "lsts: " lSts "res: " arr[2];
	if (lSts > 0)
	{
		father=arr[2];		# 获取当前成员的父
		split(father, arr, ":");
		crtReadSt=arr[1];		# 获取当前成员所属的 st

#		print "father:" father " real father : " crtReadSt;
	}

	if (crtReadSt !=crtStName)
	{	# 新开始了一个匿名结构定义，结束上一个
		if ("" != crtStName)
		{
			endLastSt();
		}

		# 开始一个新结构的处理
		startNewAnonSt();
		crtStName = crtReadSt;		# ----------- 新结构体定义开始
	}

	# 可能是一个 typeref
	lSts = match($0, /typeref:(struct|union):(.+)$/, arr);
	if (lSts > 0)
	{
		mapMbr[arr[1], "name"]=mbrName;
	}

	# 加入到成员列表中
	listMbr[stMbrCnt++]=mbrName;
	mapMbr[mbrName, "name"]=mbrName;
	mapMbr[mbrName, "father"]=father;
	mapMbr[mbrName, "line"]=getLineNo($0);
}

# 匹配到匿名结构体定义结尾，则匿名结构体定义结束
/;"[ \t]+t/ && FNR > 6{

	print "匹配到结构体 typedef, at line: " FNR;
	
	bInAnonStDef=0;				# 不再在匿名结构体定义中
	bInNormalStDef=0;
	stDefLineNo=getLineNo($0);		# 获取其中的行号

	lSts = match($0, /typeref:(struct|union):(.+)$/, arr);
	if (lSts > 0)
	{
		typeref=arr[1];
		mapMbr[typeref, "name"]=$1;
		mapMbr[typeref, "father"]="";
	}

	endLastSt();
}

# 
END{
	if (1 == bInNormalStDef)
	{	# 终结上一个结构体
		endLastSt();
	}
}
