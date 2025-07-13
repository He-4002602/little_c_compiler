#include "bnf.h"
#define WLH  0
#define RLH  1
#define WRH  2
#define RRH  3
#define RVN  4
#define RVT1 5
#define RVT2 6
using namespace std;
CFG read_BNF(string fileName){
    vector<production> P;
    unordered_set<string> VN,VT;
    string S;
    P.clear();
    VN.clear();
    VT.clear();
    S="";
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
    int state=0,nextstate=0;
    for(int i=0;i<content.size();++i){
        state=nextstate;
        switch(state){
            case WLH:
                if(content[i]=='<'){
                    P.push_back(*(new production()));
                    P.back().LH="";
                    P.back().RH.clear();
                    nextstate=RLH;
                }else nextstate=WLH;
                break;
            case RLH:
                if(content[i]=='>'){
                    if(S.empty())
                        S=P.back().LH;
                    if(VN.find(P.back().LH)==VN.end())
                        VN.insert(P.back().LH);
                    nextstate=WRH;
                }
                else{
                    P.back().LH+=content[i];
                    nextstate=RLH;
                }
                break;
            case WRH:
                if(content.substr(i,3)=="::="){
                    nextstate=RRH;
                    i+=2;
                }else{
                    nextstate=WRH;
                }
                break;
            case RRH:
                if(content[i]=='<'){
                    P.back().RH.push_back("");
                    nextstate=RVN;
                    break;
                }
                if(content[i]=='\"'){
                    P.back().RH.push_back("");
                    nextstate=RVT1;
                    break;
                }
                if(content[i]>='A'&&content[i]<='Z'){
                    P.back().RH.push_back(content.substr(i,1));
                    nextstate=RVT2;
                    break;
                }
                if(content[i]=='|'){
                    P.push_back(*(new production()));
                    P.back().LH=P[P.size()-2].LH;
                    P.back().RH.clear();
                    nextstate=RRH;
                    break;
                }
                if(content[i]==';'){
                    nextstate=WLH;
                    break;
                }
                nextstate=RRH;
                break;
            case RVN:
                if(content[i]=='>'){
                    nextstate=RRH;
                }else{
                    P.back().RH.back()+=content[i];
                    nextstate=RVN;
                }
                break;
            case RVT1:
                if(content[i]=='\"'){
                    if(VT.find(P.back().RH.back())==VT.end())
                        VT.insert(P.back().RH.back());
                    nextstate=RRH;
                }else{
                    P.back().RH.back()+=content[i];
                    nextstate=RVT1;
                }
                break;
            case RVT2:
                if(content[i]>='A'&&content[i]<='Z'){
                    P.back().RH.back()+=content[i];
                    nextstate=RVT2;
                }else{
                    if(VT.find(P.back().RH.back())==VT.end())
                        VT.insert(P.back().RH.back());
                    nextstate=RRH;
                    --i; // 回退防止吞字符
                }
                break;
        }
    }
    return {P,VN,VT,S};
}
void print_BNF(const CFG& G){
    cout<<"G(VN,VT,S,P) is defined as:"<<endl;
    cout<<"VN: { ";
    for(auto& it:G.VN){
        if(it==*G.VN.begin())
            cout<<it;
        else
            cout<<" ,"<<it;
    }cout<<" }"<<endl;
    cout<<"VT: { ";
    for(auto& it:G.VT){
        if(it==*G.VT.begin())
            cout<<it;
        else
            cout<<" ,"<<it;
    }cout<<" }"<<endl;
    cout<<"S: "<<G.S<<endl;
    cout<<"P: {"<<endl;
    for(auto& it:G.P){
        cout<<"\t"<<it.LH<<" --> ";
        for(auto& subit:it.RH)
            cout<<subit<<" ";
        cout<<endl;
    }cout<<"}"<<endl;
}
