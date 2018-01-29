/*
禁忌搜索解决图着色问题
update-time: 9:35 2018/1/23
*/
/*
禁忌搜索解决图着色问题
*/
#include<time.h>
#include<fstream>
#include<stdlib.h>
#include<stack>
#include<vector>
#include<climits>
#include<string>
#include<iostream>
using namespace std;

int maxC = 60;//最大颜色
int N;//图的顶点数目
int **g;//邻接图
int *v_edge;//每个顶点的边数

//---
//读取图

//按空格切分每行
void split(const string& src, const string& delim, vector<string>& dest)
{
    dest.clear();
    string str = src;
    string::size_type start = 0, index;
    string substr;
    index = str.find(delim, start);    //在str中查找(起始：start) delim的任意字符的第一次出现的位置  
    while (index != string::npos)
    {
        substr = str.substr(start, index - start);
        dest.push_back(substr);
        start = index + 1;
        index = str.find(delim, start);    //在str中查找(起始：index) 第一个不属于delim的字符出现的位置  
    }
    substr = str.substr(start, index);
    dest.push_back(substr);
}

//初始化图
void init_graph() {
    try {
        g = new int*[N];//初始化图
        v_edge = new int[N];
        for (int i = 0; i < N; i++) {
            g[i] = new int[N];
            v_edge[i] = 0;
        }

        for (int i = 0; i<N; i++)
            for (int j = 0; j<N; j++)
                g[i][j] = 0;
    }
    catch (const bad_alloc & e) {
        std::cout << "图内存分配失败" << e.what() << endl;
        init_graph();//分配失败重新分配
    }
}

//读取文件数据，创建图
void create_graph() {
    ifstream infile("C:\\Users\\hujing\\Desktop\\TopSup\\GraphColoring\\DSJC500.5.col.txt", ios::in);
    vector<string> data;
    string delim(" ");
    string textline;
    bool start = false;
    int v1, v2;
    int tmp;
    if (infile.good())
    {
        while (!infile.fail())
        {
            getline(infile, textline);
            if (start && textline.find("e", 0) != string::npos) {
                split(textline, delim, data);
                v1 = stoi(data[1]) - 1;
                v2 = stoi(data[2]) - 1;
                tmp = ++v_edge[v1];
                g[v1][tmp - 1] = v2;
                tmp = ++v_edge[v2];
                g[v2][tmp - 1] = v1;

            }
            else {
                if (textline.find("p edge", 0) != string::npos) {
                    split(textline, delim, data);
                    N = stoi(data[2]);
                    init_graph();
                    start = true;
                }
            }
        }
    }
    infile.close();
}


//---
//禁忌算法
int *sol;//结点对应颜色
int f;//冲突值
int **tabutenure;//禁忌表
int **adj_color_table;//邻接颜色表
int K;//颜色数量
int delt;//移动增量
int best_f;//历史最好的冲突值
int node;//每次移动的结点
int color;//每次移动的颜色
int iter;//迭代次数

         //初始化内存分配

void initalloc() {
    try {
        sol = new int[N];
        adj_color_table = new int*[N];
        tabutenure = new int*[N];

        for (int i = 0; i<N; i++) {
            adj_color_table[i] = new int[K];
            tabutenure[i] = new int[K];
        }

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < K; j++) {
                adj_color_table[i][j] = 0;
                tabutenure[i][j] = 0;
            }
        }
    }
    catch (const bad_alloc &e) {
        std::cout << "初始化内存分配失败:" << e.what() << endl;
        initalloc();
    }
}

//释放内存
void delete_alloc() {
    for (int i = 0; i < N; i++) {
        delete[]tabutenure[i];
        delete[]adj_color_table[i];
        delete[]g[i];
    }
    delete[]sol;
    delete[]tabutenure;
    delete[]adj_color_table;
    delete[]g;
}

//初始化，分组顶点颜色，计算初始冲突值，初始化邻接颜色表
void initialization(int numofcolor) {
    K = numofcolor;
    f = 0;
    initalloc();//初始化内存分配
    for (int i = 0; i<N; i++)
        sol[i] = rand() % K;//初始化颜色
    int num_edge;
    int *h_graph;
    int adj_color;
    int c_color;
    for (int i = 0; i < N; i++) {
        num_edge = v_edge[i];
        h_graph = g[i];
        c_color = sol[i];
        for (int u = 0; u < num_edge; u++) {
            adj_color = sol[h_graph[u]];
            if (c_color == adj_color) f++;
            adj_color_table[i][adj_color]++;//初始化邻接颜色表
        }
    }
    f = f / 2;
    best_f = f;
    std::cout << "init number of confilcts:" << f << endl;
}


int equ_delt[2000][2];//非禁忌相同delt值
int equ_tabudelt[2000][2];//禁忌相同delt值

//找最佳禁忌或者非禁忌移动
void findmove() {
    delt = 10000;//初始为最大整数
    int tmp;//临时变量
    int tabu_delt = 10000;
    int count = 0, tabu_count = 0;
    int A = best_f - f;
    int c_color;//当前结点颜色
    int *h_color;//邻接颜色表行首指针
    int *h_tabu;//禁忌表行首指针
    int c_color_table;//当前结点颜色表的值
    for (int i = 0; i<N; i++) {//11.3
        c_color = sol[i];//6.1%
        h_color = adj_color_table[i];
        c_color_table = h_color[c_color];
        if (h_color[c_color] > 0) {//17.6
            h_tabu = tabutenure[i];
            for (int j = 0; j < K; j++) {
                if (c_color != j) {//cpu流水线
                    //非禁忌移动
                    tmp = h_color[j] - c_color_table;
                    if (h_tabu[j] <= iter) {//22.6
                        if (tmp <= delt) {//分支预判惩罚 6.0
                            if (tmp < delt) {
                                count = 0;
                                delt = tmp;
                            }
                            count++;
                            equ_delt[count - 1][0] = i;
                            equ_delt[count - 1][1] = j;
                        }
                    }else {//禁忌移动
                        if (tmp <= tabu_delt) {//6.0
                            if (tmp < tabu_delt) {
                                tabu_delt = tmp;
                                tabu_count = 0;
                            }
                            tabu_count++;
                            equ_tabudelt[tabu_count - 1][0] = i;
                            equ_tabudelt[tabu_count - 1][1] = j;
                        }
                    }
                }
            }
        }
    }
    tmp = 0;
    if (tabu_delt<A && tabu_delt < delt) {
        delt = tabu_delt;
        tmp = rand() % tabu_count;//相等delt随机选择
        node = equ_tabudelt[tmp][0];
        color = equ_tabudelt[tmp][1];
    }
    else {
        tmp = rand() % count;//相等delt随机选择
        node = equ_delt[tmp][0];
        color = equ_delt[tmp][1];
    }
}


//更新值
void makemove() {
    f = delt + f;
    if (f < best_f) best_f = f ;
    int old_color = sol[node];
    sol[node] = color;
    tabutenure[node][old_color] = iter + f + rand() % 10 + 1;
    int *h_graph = g[node];
    int num_edge = v_edge[node];
    int tmp;
    for (int i = 0; i<num_edge; i++) {//12.4%
        tmp = h_graph[i];
        adj_color_table[tmp][old_color]--;
        adj_color_table[tmp][color]++;
    }
}

//禁忌搜索
void tabusearch() {
    create_graph();
    int numofcolor = 13;
    ofstream ofile("C:\\Users\\hujing\\Desktop\\total_O3.txt", ios::out);
    double start_time,end_time;
    double elapsed_time;
    while (cin >> numofcolor)
    {
        srand(clock());
        initialization(numofcolor);
        start_time = clock();
        iter = 0;
        while (f>0) {
            iter++;
            if ((iter % 100000) == 0) ofile << iter << " " << f << " " << K << " " << delt << " " << best_f << endl;
            findmove();
            makemove();
        }
        end_time = clock();
        elapsed_time = (double(end_time - start_time))/CLOCKS_PER_SEC;
        ofile << "成功,迭代次数" << iter << "  迭代时间(s)" << elapsed_time <<"迭代频率"<<double(iter / elapsed_time)<< endl;
        std::cout << "success,iterations:" << iter << "  elapsed_time(s):" << elapsed_time<< "frequency:" << double(iter / elapsed_time)<< endl;

    }
    ofile.close();
}


int main() {
    tabusearch();
    system("pause");
    return 0;
}