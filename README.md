# ftmalloc
fast malloc. similar to google-tcmalloc.

因为项目需要管理大块的共享内存，之前的方案采用的线性管理方式，内存分配、释放，以及内存碎片的处理都有非常大的局限性，不仅处理速度慢，而且性能不稳定。所以提出优化需求。</br>
因为tcmalloc是进程内存管理，没法直接移植到管理共享内存上，所以需要重新开发。本工程就是针对项目做得工作，后面通过抽取内存管理和分配部分的逻辑，修改成针对进程内存管理，形成本仓库，并参考tcmalloc，提供malloc/free/new/delete等接口，提供和tcmalloc类似的内存库功能。</br>
</br></br>
###设计思路</br>
####freelist和切片规则和tcmalloc一致，这部分没有过多设计，哈希链表管理线程内存切片</br>
####使用红黑树实现页和切片的映射关系</br>
####使用红黑树实现页管理关系</br>
####模仿linux内核的slab实现从系统内存分配内存</br>

</br></br></br>
###使用</br>
####在ftmalloc目录下执行make，会将src目录下的代码编译，生成libftmalloc.so.</br>
####test目录下会编译main.cpp生成test程序</br>
####LD_PRELOAD=../src/libftmalloc.so  ./test 指定使用libtcmalloc.so作为程序的内存库，申请和释放内存都会通过libftmalloc.so完成。</br>
