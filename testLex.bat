@REM 构建词法分析器
g++ lexTest.cpp ./src/preprocessor.cpp ./src/lex.cpp -Iinclude -o bin/lexTest.exe 
cd bin
@REM 词法分析器测试 正确样例
.\lexTest ../test/sourceProgram1.txt