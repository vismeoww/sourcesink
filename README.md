
# Building 
follow the instructions below

```bash
mkdir build 
cd build 
cmake  .. \
  -DLLVM_DIR=/path/to/llvm-project/build/lib/cmake/llvm \
  -DMLIR_DIR=/path/to/llvm-project/build/lib/cmake/mlir
cmake --build .
```
