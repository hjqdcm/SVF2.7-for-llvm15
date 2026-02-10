在原SVF的基础上修改了opaque pointer问题，使得SVF2.7能够在llvm15中正常运行。

注：一开始想复杂了，试了很久一直都有问题，后来直接修改了getPtrElementType函数就直接可以跑起来了，至于是否正确，还需另行测试。
