atomic collect(): 
    markFromRoots()
    compact()


//双指针
compact():
    //整理
    relocate(HeapStart, HeapEnd)
    //重新定位
    updateReferences(HeapStart, free)
    
relocate(start, end): 
    //free指向开始地址
    free <— start
    //scan指向结束地址
    scan <— end
    //开始移动
    while free < scan
    		//移动free指针，直到发现可收回对象
        while isMarked(free)
        		//如果发现活对象，则移动free指针
            unsetMarked(free)
            free <— free + size(free)
        //移动scan，直到发现活对象或越过了free指针
        while not isMarked(scan) && scan > free
        		//移动scan指针
            scan <— scan — size(scan) 
        //这时free指向未分配空间起始地址
        //scan指向一个活对象
        //判断scan是否越过free
        if scan > free
        	  //取消scan对象的标记
            unsetMarked(scan) 
            //将scan指向的对象移动到free
            move(scan, free)
            //在当前*scan位置上记录对象被转发到哪个地址上了
            *scan <— free  
            //移动free，距离为移入对象的大小
            free <— free + size(free)
            //移动scan，距离为移出对象的大小
            scan <- scan — size(scan)

//更新对象引用
updateReferences(start, end):
	  //遍历根集合
    for each fid in Roots
    		//获取根对象最初地址
        ref <- *fld
        //判断根对象最初地址是否在end后
        if ref >= end
        	  //如果根对象在end后，说明根对象被移动过
        	  //获取根对象最新地址，更新引用位置
            *fld <— *ref
    //遍历所有对象
    scan <— start
    while scan<end
    		//遍历当前对象地址集合
        for each fld in Pointers(scan)
            //更新对象地址
            ref <- *fld
            if ref >= end
                *fld <- *ref
        //获取下个对象
        scan <- scan + size(scan)
        





//Lisp 2
compact():
	  //计算整理
    computeLocations(HeapStart, HeapEnd, HeapStart)
    //更新引用
    updateReferences(HeapStart, HeapEnd)
    //移动对象
    relocate(HeapStart, HeapEnd) 

//start：待整理起始位置, end：待整理结束位置, toRegion；整理目标区域起始位置(the start of the region into which the compacted objects are to be moved.)
computeLocations(start, end, toRegion):
    scan <- start
    free <- toRegion
    //遍历对象
    while scan < end
        //判断是否活对象
        if isMarked(scan)
        	  //如果是活对象，则在对象头记录转发地址
            forwardingAddress(scan) <- free
            //将目标区域预留指定大小
            free <- free + size(scan)
        //遍历下个对象
        scan <- scan + size(scan)

//更新引用
updateReferences(start, end):
		//遍历根集合
    for each fid in Roots
    		//获取根对象最初地址
        ref <- *fld
        //根对象非空
        if ref != null
        	  //更新根对象引用地址
            *fld <— forwardingAddress(ref)
    //遍历活对象        
    scan <- start 
    while scan < end
        //判断是否活对象
        if isMarked(scan)
        	  //如果是活对象，则遍历对象地址集合
            for each fld in Pointers(scan)
                //如果非空
                if *fld != null
                	  //更新地址
                    *fld <— forwardingAddress(*fld)
        //获取下个对象
        scan <- scan + size(scan)

//移动对象
relocate(start, end):
    scan <- start
    //开始遍历
    while scan < end
        //判断是否活对象
        if isMarked(scan)
        	  //获取新地址
            dest <- forwardingAddress(scan)
            //移动对象到新地址
            move(scan, dest)
            //取消对象标记
            unsetMarked(dest)
        //下个对象
        scan <- scan + size(scan)
        
 
//Jonkers引线算法
compact():
    updateForwardReferences()
    updateBackwardReferences()

thread(ref): 
    if *ref != null
        *ref, **ref <- **ref, ref

update(ref, addr): 
    tmp <- *ref
    while isReference(tmp)
        *tmp, tmp <- addr, *tmp
    *ref <- tmp 
    
updateForwardReferences(): 
    for each fid in Roots
        thread(*fld)
        
    free <— HeapStart
    scan <— HeapStart
    while scan < HeapEnd
        if isMarked(scan)
            update(scan, free)
            for each fid in Pointers(scan)
                thread(fld)
            free <- free + size(scan)
        scan <r- scan + size(scan) 28

updateBackwardReferences():
    free <- HeapStart
    scan <- HeapStart
    while scan < HeapEnd
        if isMarked(scan)
            update(scan, free)
            move(scan, free)
            free <- free + size(scan)
        scan <- scan + size(scan)
        
        
        
//单次遍历----Compressor遍历
compact():
	  //计算位置
    computeLocations(HeapStart, HeapEnd, HeapStart)
    //更新引用
    updateReferencesRelocate(HeapStart, HeapEnd) 

//计算存活对象偏移量
computeLocations(start, end, toRegion):
		//获取新对象存放空间的开始下标
    loc <— toRegion
    //获取起始位置块下标
    block <— getBlockNum(start)
    //遍历位图，0 to 开始结束位置之间bit数量-1
    for b <— 0 to numBits(start, end) —1
        //判断当前bit下标是否超过一个块的大小
        if b % BITS_IN_BLOCK = 0
        	  //记录当前块对应的起始地址
            offset[block] <— loc
            //指向下个块
            block <— block + 1
        //判断当前bit对应的对象是否存活   
        if bitmap[b] = MARKED
        	  //增加位移，进量为每bit对应的大小
            loc <- loc + BYTES_PER_BIT

//通过块偏移量和块内容偏移量计算新地址
newAddress(old):
	  //获取对象所在的块
    block <— getBlockNum(old)
    //块地址偏移量+对象在块内的偏移量，获取对象新地址
    return offset[block] + offsetInBlock(old)

//更新引用
updateReferencesRelocate(start, end): 
	  //遍历根路径
    for each fld in Roots
        //获取旧的引用
        ref <— *fld
        //判断是否有存活对象
        if ref != null
        	  //更新引用地址
            *fld <— newAddress(ref)
    //开始移动对象
    scan <— start
    while scan < end
        //获取下个活对象
        scan <— nextMarkedObject(scan)
        //遍历活对象的引用指针
        for each fid in Pointers(scan)
            //更新引用指针
            ref <— *fld
            if ref != null
                *fld <— newAddress(ref)
        //获取对象本身的新地址位置
        dest <— newAddress(scan)
        //移动对象至新位置
        move(scan, dest)
            
            

