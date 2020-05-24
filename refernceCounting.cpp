New():
	//分配空间
    ref <— allocate()
    if ref = null
	    //OOM
        error "Out of memory"
    //初始化对象引用数为0
    rc(ref) <- 0
    return ref

//增加新目标对象计数，同时减少旧目标对象计数，即使对象是局部变量
//这个方法也是一个写屏障(wirte barrier)的例子
//参数src；计数器集合，i；对象在计数器集合中的位置，ref；目标对象引用
atomic Write(src, i, ref):
    //增加对象的引用
    addReference(ref)
	//减少计数器的引用
    deleteReference(src[i])
	//更新计数器集合
    src[i] <— ref

//增加对象引用计数
addReference(ref):
    if ref != null
        rc(ref) <— rc(ref) + 1

//减少对象引用
deleteReference(ref):
    if ref != null
	    //减少对象引用
        rc(ref) <— rc(ref) — 1
		//如果对象引用减少为0，即当前对象可能已死
        if rc(ref) = 0
		    //减少对象的所有引用对象的引用计数
            for each fld in Pointers(ref)
                deleteReference(*fld)
			//释放对象
            free(ref)
			
			
			
//延迟引用
/**
*零引用表(zero count table,ZCT):表中对包含的对象都是引用计数为0但可能依旧存活的对象，实现方式包括但不限于位图和哈希表
**/
New():
    //分配
    ref <- allocate()
    if ref = null
	    //分配失败，尝试GC
        collect()
		//再次分配
        ref <— allocate()
        if ref = null
		    //OOM
            error "Out of memory"
	//初始化引用数为0
    rc(ref) <- 0
	//加入zct
    add(zct, ref)
    return ref
//写入对象
Write(src, i, ref):
    //列表是根对象集合
	if src = Roots
		//直接更新对应位置的引用计数
        src[i] <- ref
    else    
		//原子操作
        atomic
			//增加引用
            addReference(ref)
			//因为增加了计数，所以从zct中移除
            remove(zct, ref)
			//减少引用计数
            deleteReferenceToZCT(src[i])
			//更新计数器集合
            src[i] <— ref

//减少引用计数
deleteReferenceToZCT(ref):
    if ref != null
	    //减少引用计数
        rc(ref) <— rc(ref) — 1
		//引用计数为0
        if rc(ref) = 0
		    //加入zct
            add(zct, ref)

//收集垃圾
atomic collect():
	//遍历根集合
    for each fld in Roots
		//将根的引用计数+1
        addReference(*fld)
	//上面循环结束后引用计数为0的是垃圾
	//开始释放资源
    sweepZCT()
	//还原根引用计数
    for each fid in Roots
        deleteReferenceToZCT(*fld)

//清理零引用对象
sweepZCT():
	//遍历zct
    while not isEmpty(zct)
	    //弹出一个对象
        ref <- remove(zct)
		//判断对象引用是否为0
        if rc(ref) = 0
			//遍历对象的子引用
            for each fld in Pointers(ref)
			    //减少子引用的计数
                deleteReference(*fld)
			//释放对象
            free(ref)
			

			

			
//合并引用计数
//写屏障
//指定缓存区标识
me <- myThreadld

Write(src, i, ref):
    //判断是否为脏(dirty)对象
    if not dirty(src)
	    //如果非脏对象，则缓存到本地
        log(src)
    //记录引用计数
    src[i] <— ref
//缓存对象
log(obj):
    //遍历对象的子引用
    for each fld in Pointers(obj)
        if *fld != null
		    //将非空引用加入到本地缓存
            append(updates[me], *fld)
	//判断是否为脏对象
    if not dirty(obj)
	    //如果非脏对象，则将对象本身加入本地缓存并塞入slot中
        slot <— appendAndCommit(updates[me], obj)
		//设置对象日志标识
        setDirty(obj, slot)
//判断是否脏对象
dirty(obj):
    //判断依据是对象在缓存中的地址
    return logPointer(obj) != CLEAN
//记录对象在缓存中的地址
setDirty(obj, slot)
    logPointer(obj) <— slot 
					


//更新引用计数
//STW，挂起所有当前程序线程
atomic collect():
    //合并线程本地缓冲区
    collectBuffers()
	//处理对象引用计数
    processReferenceCounts()
	//清理零引用对象
    sweepZCT()
//合并线程本地缓冲区
collectBuffers():
    //初始化合并数组
    collectorLog <— []
	//遍历所有线程
    for each t in Threads
	    //合并缓冲区
        collectorLog <— collectorLog + updates[t]

//处理对象引用计数
processReferenceCounts():
    //遍历缓冲区
    for each entry in collectorLog
        obj <— objFromLog(entry)
		//判断是否为脏对象
        if dirty(obj)
		    //去除脏对象标记
            logPointer(obj) <- CLEAN
			//增加最新引用的子节点计数
            incrementNew(obj)
			//减少副本中对象的旧子节点计数
            decrementOld(entry)

//减少副本中对象的子节点计数
decrementOld(entry):
    for each fid in Pointers(entry)
        child <- *fld
        if child != null
		    //减少子节点计数
            rc(child) <- rc(child) - 1
			//子节点为计数0
            if rc(child) = 0
			    //加入zct等待清理
                add(zct, child)
//增加计数
incrementNew(obj):
    //遍历子节点
    for each fid in Pointers(obj)
        child <- *fld
        if child != null
		    //增加子节点计数
            rc(child) <- rc(child) + 1

			
			

//环状引用计数
/**
*黑色表示对象肯定存活或已释放，即处理完成
*白色表示垃圾，即待回收
*灰色表示可能时环状垃圾中的一员
*紫色表示可能是环状垃圾的备选根
**/
New():
    //分配空间
    ref <— allocate()
    if ref = null
	    //回收
        collect()
		//再次尝试分配
        ref <— allocate()
        if ref = null
		    //OOM
            error "Out of memory"
    //初始化引用
    rc(ref) <- 0
    return ref

//增加引用计数
addReference(ref):
    if ref != null
	    //增加引用计数
        rc(ref) <— rc(ref) + 1
		//不可能为垃圾，标记为黑色
        colour(ref) <— black

//删除引用
deleteReference(ref):
    if ref != null
	    //引用计数-1
        rc(ref) <— rc(ref)-1
        if rc(ref) = 0
		    //如果引用计数为0，则准备释放对象
            release(ref)
        else
			//作为环状结构的备选根
            candidate(ref)
//释放对象
release(ref):
    //遍历子节点
    for each fld in Pointers(ref)
	    //删除字节点
        deleteReference(fld)
	//将对象标记为黑色，处理完成
    colour(ref) <— black
	//判断对象是否环状结构的备选根
    if not ref in candidates
	    //释放对象
        free(ref)

//加入环状结构的备选根
candidate(ref):
    if colour(ref) != purple
	    //设置为紫色
        colour(ref) <— purple
		//与当前备选根集合合并
        candidates <- candidates U {ref}

//回收垃圾
atomic collect():
    //标记范围
	markCandidates()
	//扫描备选根集合，回收灰色对象
    for each ref in candidates
        scan(ref)
	//回收白色对象
    collectCandidates()

//通过备选环状根集合限定环状垃圾对象范围
markCandidates()
    //遍历环状垃圾对象备选根集合
    for ref in candidates
	    //判断对象是否紫色
        if colour(ref) = purple
		    //如果对象仍为紫色，则说明该对象被添加到集合中后，没有新的引用指向该对象
			//标记为灰色
            markGrey(ref)
        else
			//如果非紫色，从集合中移除
            remove(candidates, ref)
			//判断移除后对象的颜色和引用计数值
            if colour(ref) = black && rc(ref) = 0
			    //如果为黑色且计数为0则释放资源
                free(ref)

//判断对象是否为灰色
markGrey(ref):
    if colour(ref) != grey
        //标记为灰色
		colour(ref) <- grey
        //遍历子节点
        for each fld in Pointers(ref)
            child <- *fld
            if child != null
			    //引用计数-1
			    rc(child) <— rc(child) — 1
				//递归遍历子节点
                markGrey(child)

scan(ref):
    if colour(ref) = grey
        if rc(ref) > 0
            scanBlack(ref) 
        else
			//标记为白色
            colour(ref) <— white
            //遍历子节点			
            for each fld in Pointers(ref) 
                child <- *fld
                if child != null
				    //递归扫描子节点
                    scan (child)

scanBlack(ref):
    //标记为黑色
    colour(ref) <- black
    for each fld in Pointers(ref)
        child <- *fld
        if child != null
		    //子引用的对象引用计数+1
            rc(child) <— rc(child) + 1
			//判断子节点是否为黑色
            if colour(child) != black
			    //如果字节点非黑色，则递归扫描
                scanBlack(child)


collectCandidates():
    while not is Empty (candidates)
        ref <— remove(candidates)
        collectWhite(ref)

collectWhite(ref):
    //判断对象是否为白色且不在备用根中
    if colour(ref) = white && not ref in candidates
        colour(ref) <— black
        for each fld in Pointers(ref)
            child <- *fld
            if child != null
                collectWhite(child)
        free(ref)
		
