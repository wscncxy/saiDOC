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
			
			
			
			
			






