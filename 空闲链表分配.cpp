sequentialAllocate(n): 
    //开始分配，result和free在统一起点
    result <— free
    //result增加指定空间，作为作为新起始地址
    newFree <— result + n 
    //判断newFree是否超过了界限指针
    if newFree > limit
        //如果超过了，则返回空，分配失败
        return null
    //则更新free指针
    free <— newFree
    return result
    
    
    


//首次适应分配
//假设每个单元本身都记录了自身大小和下一个空闲内存单元的地址
firstFitAllocate(n):
		//获取头对象
    prev <- addressOf(head)
 		//死循环
 		loop
 				//获取下个单元地址
 		    curr <— next(prev)
        if curr = null
        		//如果当前单元为空，则分配失败
            return null 
        else if size(curr) < n
        		//如果当前单元小于所需空间，则继续遍历下个单元
            prev <- curr
        else
        		//如果当前单元符合所需空间，则开始分配内存
            return listAllocate(prev, curr, n)

//策略1
//prev：上个单元地址，curr：当前单元空间地址，n：所需空间大小
listAllocate(prev, curr, n):
		//获取当前单元起始地址
    result <— curr
    //判断是否需要判断
    if shouldSplit(size(curr), n) 
    		//如果需要分裂单元
    		//获取剩余部分起始地址
        remainder <- result + n
        //更新remainder下个对象起始地址
        next(remainder) <— next(curr)
        //更新remainder大小
        size(remainder) <- size(curr) — n
        //将prev单元的下个对象地址指向remainder
        next(prev) <- remainder 
    else
    		//如果不需要分裂单元，则更新链表中单元指向，将已分配的空间抛离链表
        next(prev) <- next(curr)
    return result

//策略2
//将单元的尾部分割出来
//return the portion at the end of the cell being split
//该方案不足之处在于对象对齐方式有所不同
//A possible disadvantage of this approach is the different alignment of objects
listAllocateAlt(prev, curr, n):
		//判断是否需要分裂
    if shouldSplit(size(curr), n)
    	  //当前单元减去所需大小，重新设定当前单元大小
        size(curr) <— size(curr) — n;
        //将当前单元加上新的单元大小，获取分配空间的起始地址
        result <— curr + size(curr) 
    else
    		//如果不需要分裂则当前单元即分配单元
        next(prev) <- next(curr)
        result <— curr
    //返回分配单元
    return result
    
    
    
//循环首次适应分配
nextFitAllocate(n):
		//获取上次单元地址
    start <— prev
    //死循环
    loop
    		//获取下个单元地址
        curr <— next(prev)
        //判断下个单元是否为空
        if curr = null
        	 	//如果下个单元为空，则获取重新定位到头地址
            prev <— addressOf (head)
            //再次获取下个单元地址
            curr <- next(prev)
        //判断上次单元和开始单元是哦符相同
        if prev = start
        		//如果相同，则说明已经轮训一遍都未找到合适的单元，返回空
            return null
        //判断当前单元是否符合容量需求
        else if size(curr) < n
        	  //如果不符合容量要求，则进入下个单元
            prev <— curr
        else
        	  //如果符合分配需求，则
            return listAllocate(prev, curr, n)
            
//最佳适应分配
bestFitAllocate(n):
	  //初始化单元最佳位置
    best <- null
    //初始化最佳单元的大小为无限大
    bestSize <— oo
    //获取头地址
    prev <— addressOf (head)
    loop
    		//获取下个单元地址
        curr <— next(prev)
        //判断当前是否为空，单元大小是否符合需求
        if curr = null || size(curr) = n
        		//判断当前单元是否为空
            if curr != null
            		//如果当前单元非空，则获取最佳单元上个单元地址
                bestPrev <— prev
                //当前单元即为最佳单元
                best <— curr
            else if best = null
            	  //如果最佳单元为空，则返回null，分配失败
                return null
            //分配内存
            return listAllocate(bestPrev, best, n)
        //判断当前单元是否小于需求或大于最佳大小
        else if size(curr) < n || bestSize < size(curr)
        	  //如果当前单元小于需求或大于最佳大小，则进入下个循环
            prev <— curr
        else
        	  //如果如果当前单元大于需求且小于等于最佳大小
        	  //设置当前单元为最佳单元
            best <— curr
            bestPrev <— prev
            //当前单元大小为最佳大小单元
            bestSize <— size(curr)
            
            
//笛卡尔树--基于首次适应分配
firstFitAllocateCartesian(n):
    parent <— null
    //从根开始遍历
    curr <— root
    loop
        //判断左节点非空且左节点最大单元大小是否满足需求
        if left(curr) != null && max(left(curr)) >= n
        	  //如果左节点非空且最大单元大小满足需求，则从左节点遍历
            parent <— curr
            curr <— left(curr)    
        else if prev < curr && size(curr) >= n
        	  //如果当前节点是在上个扫描到节点之后且符合需求大小
            prev <— curr
            //分配资源，并增删树
            return treeAllocate(curr, parent, n)
        else if right(curr) != null && max(right(curr)) > n
        	  //如果右节点非空且最大单元大小满足需求，则从右节点遍历
        	  parent <— curr
            curr <— right(curr)
        else
        	  //否则OOM
            return null