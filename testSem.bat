@REM 构建前端测试语义分析器
g++ semTest.cpp src/*.cpp -Iinclude -o bin/semTest.exe 
cd bin
@REM 语义分析器测试 错误样例
.\semTest ../test/sourceProgram6.txt
@REM 语义分析器测试 正确样例 输出为bin/out.ll
.\semTest ../test/sourceProgram1.txt
