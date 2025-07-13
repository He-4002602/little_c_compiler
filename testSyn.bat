@REM 构建语法分析器
g++ synTest.cpp ./src/preprocessor.cpp ./src/lex.cpp ^
./src/bnf.cpp ./src/treetools.cpp ./src/opparser.cpp ./src/ll1parser.cpp ./src/syntax.cpp ^
-Iinclude -o bin/synTest.exe 
cd bin
@REM 语法分析器测试 正确样例
.\synTest ../test/sourceProgram1.txt
python ./vis_ast.py 