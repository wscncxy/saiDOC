createSemispaces():
    tospace <— HeapStart
    extent <- (HeapEnd - HeapStart) / 2 
    top <— fromspace <— HeapStart + extent
    free <— tospace
    
atomic allocate(size): 
    result <— free
    newfree <— result + size
    if newfree > top
        return null
    free <— newfree
    return result





atomic collect(): 
    flip()
    initialise(worklist)
    for each fid in Roots
        process(fld)
    while not isEmpty(worklist) 
        ref <— remove(worklist)
        scan(ref)
/*
*/

flip():
    fromspace, tospace <— tospace, fromspace
    top <- tospace + extent
    free <— tospace


scan(ref):
    for each fld in Pointers(ref)
        process(fld)

process(fld): 
    fromRef <- *fld
    if fromRef != null
        *fld <— forward(fromRef)


forward(fromRef):
    toRef <— forwardingAddress(fromRef)
    if toRef = null
        toRef <— copy(fromRef) 
    return toRef
   
   
copy(fromRef):
    toRef <— free
    free <— free + size(fromRef)
    move(fromRef, toRef)
    forwardingAddress(fromRef) <- toRef 
    add(worklist, toRef)
    return toRef
    
    
    
    
//Cheney's work list
initialise(worklist):
    scan <- free
    
isEmpty(worklist):
    return scan = free

remove(worklist):
    ref <- scan
    scan <— scan + size(scan)
    return ref

add(worklist, ref): /* nop */
	
	
	
	
//Approximately depth-first copying 
initialise(worklist):
    scan <- free
    parrtialScan <— free
    
isEmpty(worklist):
    return scan = free

remove(worklist):
    if(partialScan < free)
        ref <— partialScan 
        partialScan <— partialScan + size(partialScan)
    else
        ref <— scan 
        scan <- scan + size(scan)
    return ref


add(worklist, ref):
	partialScan <— max(partialScan, startOfPage(ref))
	
	
//Online object reordering
atomic collect(): 
    flip()
    initialise(hotList, coldList)
    for each fld in Roots
        adviceProcess(fld)
    repeat
        while not isEmpty(hotList)
            adviceScan(remove(hotList))
        while not isEmpty(coldList)
            adviceProcess(remove(coldList))
    until isEmpty(hotList)

initialise(hotList, coldList): 
    hotList <— empty
    coldList <— empty

adviceProcess(fld):
    fromRef <— *fld
    if fromRef != null
        *fld <— forward(fromRef)

adviceScan(obj) :
    for each fld in Pointers(obj)
        if isHot(fld)
            adviceProcess(fld)
        else
            add(coldList, fld)
