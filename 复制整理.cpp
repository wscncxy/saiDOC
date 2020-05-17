//初始化空间
createSemispaces():
	  //目标空间起始位置
    tospace <— HeapStart
    //半区大小
    extent <- (HeapEnd - HeapStart) / 2
    //半区最大地址=来源空间=堆开始地址+半区大小
    top <— fromspace <— HeapStart + extent
    //分配游标
    free <— tospace
   
//分配内存
atomic allocate(size):
	  //获取当前可分配内存起始地址 
    result <— free
    //获取新的可分配内存游标地址
    newfree <— result + size
    //如果新的游标地址大于top
    if newfree > top
    	  //分配失败，内存溢出
        return null
    //更新可分配空间起始地址
    free <— newfree
    //返回分配内存的起始地址
    return result


//开始GC
atomic collect(): 
    flip()//翻转半区
    //初始化工作链表
    initialise(worklist)
    //遍历根列表
    for each fld in Roots
        //处理根节点
        process(fld)
    //循环工作链表
    while not isEmpty(worklist) 
        //弹出工作链表中对象
        ref <— remove(worklist)
        //扫描对象
        scan(ref)
//翻转半区
flip():
    fromspace, tospace <— tospace, fromspace
    top <- tospace + extent
    free <— tospace
//扫描对象的所有引用
scan(ref):
	  //遍历指定对象的引用
    for each fld in Pointers(ref)
        //处理节点
        process(fld)
//处理节点
process(fld):
	  //获取对象源地址 
    fromRef <- *fld
    //如果对象非空
    if fromRef != null
    	  //移动对象并更新对象引用
        *fld <— forward(fromRef)
//重定向对象
forward(fromRef):
	  //获取对象新地址
    toRef <— forwardingAddress(fromRef)
    //判断空间是否获取成功
    if toRef = null
    	  //移动对象并获取新地址
        toRef <— copy(fromRef) 
    return toRef
   
   
copy(fromRef):
	  //获取可分配内存起始位置
    toRef <— free
    //更新可分配内存起始位置
    free <— free + size(fromRef)
    //移动
    move(fromRef, toRef)
    //重定向对象地址
    forwardingAddress(fromRef) <- toRef 
    //将对象加入工作队列
    add(worklist, toRef)
    //返回对象新地址
    return toRef
    
//Cheney's work list
//初始化工作列表
//这里画图比较直观
initialise(worklist):
	  //在这里scan=free=topspace
    scan <- free
    
isEmpty(worklist):
	  //当scan地址追上free时，即为空列表
    return scan = free
//弹出要处理的对象
remove(worklist):
    ref <- scan
    scan <— scan + size(scan)
    return ref

add(worklist, ref): /* nop */
	
	
	
	
//Approximately depth-first copying 
//假设对象不跨页
initialise(worklist):
    scan <- free
    parrtialScan <— free
    
isEmpty(worklist):
    return scan = free

remove(worklist):
	  //优先弹出后面的对象
    if(partialScan < free)
        ref <— partialScan 
        partialScan <— partialScan + size(partialScan)
    else
        ref <— scan 
        scan <- scan + size(scan)
    return ref


add(worklist, ref):
	//优先扫描最后面的对象
	partialScan <— max(partialScan, startOfPage(ref))
	
	
//Online object reordering
atomic collect(): 
	  //对调区域
    flip()
    //初始化，冷热队列
    initialise(hotList, coldList)
    //遍历根节点
    for each fld in Roots
        adviceProcess(fld)
		//do while
    repeat
        //遍历热列表
        while not isEmpty(hotList)
            adviceScan(remove(hotList))
        //遍历冷列表
        while not isEmpty(coldList)
            adviceProcess(remove(coldList))
    //直到热列表为空
    until isEmpty(hotList)
//初始化
initialise(hotList, coldList): 
    hotList <— empty
    coldList <— empty
//通知处理
adviceProcess(fld):
	  //获取当前对象旧地址
    fromRef <— *fld
    if fromRef != null
    	  //移动并更新地址
        *fld <— forward(fromRef)
//通知扫描
adviceScan(obj):
	  //遍历对象所有引用
    for each fld in Pointers(obj)
        //判断是否热对象
        if isHot(fld)
            adviceProcess(fld)
        else
            add(coldList, fld)
