#! /bin/awk

BEGIN{
	crtFileName	="";				# 当前文件名
	stMbrCnt	= 0;
	stDefLineNo	="";				# 结构体定义所在行
	crtSt		= "";
	crtStName	="";				# 当前处理的结构名
	crtReadSt	="";				# 当前行解读出的 st
	bInNormalStDef	=0;			# 是否处在常规结构体定义中
	bInAnonStDef	=0;				# 是否处理匿名结构体定义中
	
	mapMbr[""] = "";	# 成员 map

	# 成员列表
	listMbr[1, "name"] = "";
	listMbr[1, "line"] = "";
	listMbr[1, "depth"] = 0;
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
    match(strLine,/line:([0-9]+)/,arr);
    return arr[1];
}

# 获取 mbrName_ 的路径，需要将匿名结构解除引用
function getMbrPath2(mbrPath, mbrName_)
{
    lSts = split(mbrPath, mbrArr, "::");
    sRes = "";
    
    for (maI in mbrArr)    {
	if (match(mbrArr[maI], "__anon"))	{
	    mbrArr[maI] = mapMbr[mbrArr[maI]];
	}
    }

    for (maI in mbrArr)    {
	sRes = sRes "" mbrArr[maI] ".";
    }

    sRes = sRes "" mbrName_;
    return sRes;
}

function endLastSt()
{
    printf("printf(\"sizeof %s: %zu def at %s:%s\\n\", sizeof(%s));\n", crtStName, crtFileName, stDefLineNo, crtStName);

	for(k=1; k<=stMbrCnt; k++)	{
	    crtMbrName		= listMbr[k, "name"];
	    crtMbrDefLine	= listMbr[k, "line"];
	    strPath		= listMbr[k, "path"];
	    mbrPath		= getMbrPath2(strPath, crtMbrName);
	    
	    printf("printf(\"\toffsetof %s.%s: %zu def at %s:%s\\n\", offsetof(%s, %s));\n", \
		       crtStName, mbrPath, crtFileName, crtMbrDefLine, crtStName, mbrPath);
	}
}

function startNewNormalSt(stName)
{
	# 新的开始
	bInNormalStDef	= 1;
	bInAnonStDef	= 0;
	
	stMbrCnt	= 0;
	crtSt		= stName;				# 更新当前结构体
	crtStName	= "struct " stName;
}

function startNewAnonSt()
{
	bInNormalStDef	= 0;
	bInAnonStDef	= 1;
	
	stMbrCnt	= 0;
	crtSt		= "";
	crtStName	= "";
}

# 匹配到结构体定义, q
/;"[ \t]+s/ && $0 !~ /line:[0-9]+[ \t]+(struct|union):([^ \t:]+)/{
	if (1 == bInNormalStDef)	{	# 终结上一个结构体
		endLastSt();
	}

	stDefLineNo = getLineNo($0);
	startNewNormalSt($1);	# ----------- 新结构体定义开始
}	

# 匹配到结构体成员定义
/;"[ \t]+m/{
	crtFileName	= basename($2);	# 更新当前文件名
	mbrName		= $1;					# 当前成员名
	typeref		= "";
	path		= "";

	# 获取当前成员的 top 父，与路径（不计 top 父）
	lSts = match($0, /line:[0-9]+[ \t]*(struct|union):([^ \t:]+)(::)?([^ \t]*)/, arr);
	if (lSts > 0)	{
	    crtReadSt	= arr[2];		# 当前行 top 父
	    path	= arr[4];		# 获取路径，top 父不计入
	}

	if (crtReadSt != crtSt)	{	# 新开始了一个匿名结构定义，结束上一个
		if ("" != crtSt)	       {
		    	endLastSt();
		}

		# 开始一个新结构的处理
		startNewAnonSt();
		crtSt = crtReadSt;		# ----------- 新结构体定义开始
	}

	# 可能是一个 typeref
	lSts = match($0, /typeref:(struct|union):(.*)$/, arr);
	if (lSts > 0)	{	# 获取最后的分量，为其建立映射
	    split(arr[2], subArr, "::");
	    last = subArr[length(subArr)];
	    mapMbr[last] = mbrName;
	}

	# 加入到成员列表中
	stMbrCnt++;
	listMbr[stMbrCnt, "name"] = mbrName;
	listMbr[stMbrCnt, "line"] = getLineNo($0);
	listMbr[stMbrCnt, "path"] = path;
}

# 匹配到匿名结构体定义结尾，则匿名结构体定义结束
/;"[ \t]+t/ && FNR > 6{
	bInAnonStDef	= 0;				# 不再在匿名结构体定义中
	bInNormalStDef	= 0;
	stDefLineNo	= getLineNo($0);		# 获取其中的行号

	crtSt = $1;
	crtStName = $1;
	lSts = match($0, /typeref:(struct|union):(.+)$/, arr);
	if (lSts > 0)	{
	    mapMbr[arr[2]] = $1;
	}
	
	endLastSt();
}

# 
END{
	if (1 == bInNormalStDef)	{	# 终结上一个结构体
	    endLastSt();
	}
}
