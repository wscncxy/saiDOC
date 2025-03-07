/**
n所需空间
a对齐方式
m 2的整数次幂
*blk 空闲内存
**/
fits(n, a, m, blk):
    //获取对齐还需空间
    z <— blk — a
    //通过位运算 向上取整
    z <— (z+m— l) & ~(m— l)
    //获取对齐后空间大小
    z <— z + a
    //获取需要补偿字节 
    pad <— z — blk
    return n + pad <= size(curr)