    这是一个兰州大学信息科学与工程学院2025年春季学期编译原理课程设计的项目，该项目基于LittleC语言构建了一个包含预处理、词法分析、语法分析、语义检查和中间代码生成的编译器前端，并可以使用Clang将生成的中间代码编译成可执行文件。
    LittleC是一个基于C语法设计的源语言，由兰州大学信息科学与工程学院燕昊老师和马浚老师设计并提供，其基本描述如下：
    （1）结构和语义说明：
        a. 该语言的一个程序由且仅由一个块结构组成，该块中包含两部分：可选的声明语句和必须的执行语句。声明语句用于声明变量，执行语句用于表达计算过程，过程中使用这些变量来存储内部数据和计算结果。
        b. 该语言支持两种数据类型：整型和布尔型。整型数据占4个字节，布尔型占1个字节。整型数据的常量是整常数，布尔型数据的常量是 true 和 false 。
        c. 整型数据支持 +、-、*、/ 四种算术运算，运算结果为整型值。整型数据可与>、>=...等六个关系运算符结合，运算结果为布尔型值。
        d. 布尔型数据支持 || 、&& 、！三种逻辑运算，运算结果为布尔常量。
        e. 整型的变量和常量可以用算术运算符组合成算术表达式，其值为整常数；布尔型的变量和常量可以用逻辑运算符组成成布尔表达式，其值为布尔常量；算术表达式可以加上关系运算符构造出布尔表达式。
        f. 整常数和算术表达式可以用于整型变量赋值，布尔常量和布尔表达式可以用于布尔型变量赋值；两者不兼容（文法中使用不同的赋值运算符以示区别）。
        g. 各种运算符的优先级和结合规则参照C语言语法理解。
        h. 控制流语句中的控制条件限定为1个布尔型变量，该量应该在引用之前定值。
        i. 该语言不支持数组、结构体、指针等复杂数据类型。
        j. 该语言不含有子程序结构，也不支持过程调用。
        k. 该语言有简单的I/O功能：使用read指令能够从键盘读入1个整常数，使用write指令能向屏幕打印输出1个整常数。
    （2）文法定义：
        PROG        →    {  DECLS  STMTS  }
        
        DECLS       →    DECLS  DECL    |   empty
        DECL         →    int  NAMES  ;  |  bool  NAMES  ; 
        NAMES     →    NAMES ,  NAME  |  NAME
        NAME       →    id

        STMTS    →    STMTS  STMT  |   STMT          
        STMT      →    id  =  EXPR ;    |   id := BOOL ;
        STMT      →    if  id   then  STMT
        STMT      →    if   id   then  STMT  else STMT
        STMT      →    while   id  do  STMT
        STMT      →    {  STMTS   STMT  }
        STMT      →    read  id  ;
        STMT      →    write  id  ;

        EXPR    →    EXPR  ADD  TERM  |  TERM
        ADD     →    + | -
        TERM    →    TERM  MUL NEGA  |  NEGA
        MUL     →    * | /
        NEGA   →    FACTOR  |  - FACTOR  
        FACTOR→    (  EXPR ) |  id  |  number 

        BOOL    →    BOOL  ||  JOIN    |    JOIN
        JOIN     →    JOIN   &&   NOT  |   NOT
        NOT      →    REL   |  ! REL
        REL       →    EXPR   ROP  EXPR 
        ROP      →     >  |  >=  |  <  |  <=  |  ==  |   !=
    （3）词法规则
        a. 标识符：由字母打头后跟字母、数字任意组合的字符串；长度不超过8；不区分大小写；把下划线看作第27个字母；
        b. 整常数：完全由数字组成的字符串；正数和0前面不加符号，负数在正数前面加-构成；长度不超过8；
        c. 布尔常量：true 和 false 。
        d. 关键字、运算符、分隔符仅包含在文法定义中出现过的终结符。关键字保留。
        e. 字母表定义为上述规则中出现的字符的集合；不在该集合中的符号都以非法字符对待；
        f. 源程序中可以出现单行注释和多行注释，其格式参考C语言设计。
    
    运行compile.bat构建编译器bin/lcc.exe和编译器前端驱动程序bin/semTest.exe，之后可以运行testSem.bat测试语义分析的正确性，其测试一个有语义错误的程序和一个没有错误的程序，后者生成的out.ll位于bin目录下，可以查看。
    此外，testLex.bat和testSyn.bat也可以分别测试词法分析和语法分析模块。
    test目录下还有很多测试用例，如果需要测试他们或者自定义的程序，请使用如下命令：
    ./bin/lcc.exe <sourceFileName> [outputFileName] [--visAST] [--visCFG]
    需要确保测试机器架构为x86_64 win，并且已经配置Clang和python环境。
    LCC(LittleC Compiler)支持将编译生成的lcc.exe文件位置添加到环境变量中，便于在任意位置使用编译器。
    
    本项目系初学者简单实现，发布以便于广大同为初学者的同学借鉴交流。如需获取更多信息或进一步交流，欢迎通过邮箱联系作者：326404210@qq.com（宋成城）。