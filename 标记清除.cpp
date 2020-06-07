//标记清除
//新建对象
new():
	  //尝试给对象分配内存
    ref <— allocate()  
    //分配失败
    if ref = null
       //开始收集
       collect()
       //再次尝试分配       
       ref <— allocate()
       //再次分配失败
       if ref = null
       		//抛异常
          error "Out of memory" s
    //如果成功分配到内存，则返回地址引用
    return ref

//分配过程
atomic collect():
		//标记对象
    markFromRoots()
    //开始清理，传入参数为HeapStart=堆开始位置，HeapEnd=堆结束位置
    sweep(HeapStart, HeapEnd)
    
   
//标记过程
markFromRoots():
		//初始化工作列表，这个工作列表基于栈实现
   	worklist <— empty
   	//遍历根，采用深度优先遍历(depth-first traversal)
		for each fid in Roots
			//获取当前根对象
			ref <- *fld
			//如果根对象不为空，且未被标记
			if ref != null && not isMarked(ref)
				//标记根对象
				setMarked(ref)
				//将根对象压入栈
				add(worklist, ref)
				//开始遍历&标记根对象下面的对象
				mark()
	

//遍历&标记根对象下面的对象过程
mark():
	//循环遍历工作列表，直接工作列表为空
	while not isEmpty(worklist)
		//弹出栈顶，并获取弹出的对象
		ref <— remove(worklist)
		//获取当前对象的所有引用者 
		for each fid in Pointers(ref)
			//获取引用者Z对象
			child <— *fld
			//如果对象非空，且未被标记
			if child != null && not isMarked(child)
				//标记对象
				setMarked(child)
				//压入栈
				add(worklist, child)
				
/**
清理过程
@Param start 起始位置
@Param end 结束位置
**/
sweep(start, end): 
		//获取堆第一个对象开始位置
		scan <— start
		//判断当前位置是否已经到最后
		while scan < end
			//判断当前对象是否被标记
			if isMarked(scan)
				//如果已经被标记，则取消标记
				unsetMarked(scan)
			else
				//如果未被标记，则释放对象 
				free(scan)
			//获取下个对象位置
			scan <— nextObject(scan)
			
			
			
			
			

//Printezis和Detlefs的位图标记
mark()
    //获取下个位图标记
    cur <- nextInBitmap()
	//循环判断是否超过堆
    while cur < HeapEnd
        //当前位，加入worklist	
        add(worklist, cur)
		//标记当前位
        markStep(cur)
		//获取下个位
        cur <- nextInBitmap()
//标记 位
markStep(start):
    //循环遍历worklist，直到空
    while not isEmpty(worklist)
	    //弹出worklist元素
        ref <- remove(worklist)
		//遍历弹出元素的子节点
        for each fld in Pointers(ref)
            child <- *fld
			//判断子节点是否不为空且未被标记
            if child != null && not isMarked(child)
			    //子节点不为空且未被标记，标记子节点
                setMarked(child)
				//如果子节点所在位置小于start，则加入当前worklist循环
                if child < start
                    add(worklist, child)

//惰性
//收集
atomic collect():
    //标记根
    markFromRoots()
    //遍历内存块
    for each block in Blocks
        //判断块是否标记
        if not isMarked(block)
        	  //将未被标记的块归还给分配器
            add(blockAllocator, block)
        else
        	  //将标记标记的块，加入待清理队列
            add(reclaimList, block)

//分配
//sz：指定的分配空间
atomic allocate(sz):
	  //尝试分配
    result <— remove(sz)
    //判断是否分配成功
    if result = null
    	  //执行清理
        lazySweep(sz)
        //再次尝试分配
        result <- remove(sz)
    //返回分配结果
    return result
//惰性清理
lazySweep(sz) :
	  //do while
    repeat
        //从标记块列表弹出一个符合sz大小的块
        block <- nextBlock(reclaimList, sz)
        if block != null
        	  //清理块
            sweep(start(block), end(block))
            if spaceFound(block)
                return
    until block = null
    //尝试分配空间
    allocSlow(sz)

allocSlow(sz) :
	  //分配块
    block <— allocateBlock()
    if block != null
    	  //分配成功，初始化块
        initialise(block, sz)

		
		
//基于先进先出预取缓冲区的标记过程		
add(worklist, item):
    markStack <r- getStack(worklist)
    push(markStack, item)

remove(worklist):
    markStack <— getStack(worklist)
    addr <— pop(markStack)
    prefetch(addr)
    fifo <— getFifo(worklist)
    prepend(fifo, addr)
    return remove(fifo)
	

//标记对象图的边二非节点
mark():
    while not isEmpty(worklist)
        obj <— remove(worklist)
        if not isMarked(obj)
            setMarked(obj)
            for each fid in Pointers(obj)
                child <- *fld
                if child != null
                    add(worklist, child)