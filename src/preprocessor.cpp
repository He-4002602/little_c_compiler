#include "preprocessor.h"
#define DEFAULT 0  // 默认状态
#define SINGLE 1  // 单行注释
#define MULTI 2  // 多行注释
#define TABLE 3  // 制表符
using namespace std;
// 通过stat结构体 获得文件大小，单位字节
size_t get_file_size(const char *fileName) {
	if (fileName == NULL) {
		return 0;
	}
	struct stat statbuf;
	stat(fileName, &statbuf);
	size_t filesize = statbuf.st_size;
	return filesize;
}
string preprocess(string fileName){
    int state=0,nextstate=0;
    unsigned long long srow=0,row=1;
    ifstream fin(fileName);
    if(!fin.is_open()){
        fin.close();
        stringstream ss;
        ss<<"Failed to open file "<<fileName;
        throw string(ss.str());
    }
    string content="";
    if(get_file_size(fileName.data())>content.max_size()){
        fin.close();
        stringstream ss;
        ss<<"The size of the file "<<fileName<<" is "<<get_file_size(fileName.data())
            <<" bytes, which exceeds the limit of "<<content.max_size();
        throw string(ss.str());
    }
    string buf;
    while(getline(fin,buf))
        content+=(buf+'\n');
    fin.close();
    for(unsigned long long i=0;i<content.size();++i){
        state=nextstate;
        switch(state){
            case DEFAULT:
                if(content[i]=='/'){
                    if(i==content.size()-1)
                        break;
                    if(content[i+1]=='/'){
                        nextstate=SINGLE;
                        content[i]=content[i+1]=' ';
                        ++i;
                        break;
                    }
                    if(content[i+1]=='*'){
                        nextstate=MULTI;
                        content[i]=content[i+1]=' ';
                        srow=row;
                        ++i;
                        break;
                    }
                }
                if(content[i]=='\t'){
                    nextstate=TABLE;
                    i--;
                    break;
                }
                if(content[i]=='\n')
                    ++row;
                break;
            case SINGLE:
                if(content[i]=='\n'){
                    nextstate=DEFAULT;
                    ++row;
                    break;
                }
                content[i]=' ';
                break;
            case MULTI:
                if(i==content.size()-1){
                    stringstream ss;
                    ss<<"The multi-line comment started at line "<<srow<<" is not closed";
                    throw string(ss.str());
                }
                if(content[i]=='*'){
                    if(content[i+1]=='/'){
                        nextstate=DEFAULT;
                        content[i]=content[i+1]=' ';
                        ++i;
                        break;
                    }
                }
                if(content[i]=='\n')
                    ++row;
                else content[i]=' ';
                break;
            case TABLE:
                if(content[i]=='\t'){
                    content[i]=' ';
                    break;
                }
                if(content[i]=='/'){
                    if(i==content.size()-1){
                        nextstate=DEFAULT;
                        break;
                    }
                    if(content[i+1]=='/'){
                        nextstate=SINGLE;
                        content[i]=content[i+1]=' ';
                        ++i;
                        break;
                    }
                    if(content[i+1]=='*'){
                        nextstate=MULTI;
                        content[i]=content[i+1]=' ';
                        srow=row;
                        ++i;
                        break;
                    }
                }
                nextstate=DEFAULT;  
        }
    }
    return content;
}