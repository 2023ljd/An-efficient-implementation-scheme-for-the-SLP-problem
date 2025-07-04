#include <iostream>
#include <bitset>
#include <unordered_map>
#include <vector>
#include <set>
#include <array>
#include <queue>
#include <algorithm>
using namespace std ;
#define endl '\n'

#define input_file "./input/Whirlwind_M1.txt"
#define output_file "./output2/Whirlwind_M1.txt"
//#define input_file "input.txt"
//#define output_file "output.txt"

//定义矩阵的高度、宽度
#define MAXHEIGHT 32
#define MAXWIDTH 32

//点的属性（种类）
#define is_LEAF 0
#define Splited 1
#define To_Be_Splited 2
#define Extended 3


//extension_round  扩展轮数

//如下为存下的矩阵状态
bool original_matrix[MAXHEIGHT][MAXWIDTH]={false} ;
//bitset<MAXWIDTH> ori_point[MAXHEIGHT] ;

unordered_map< bitset<MAXWIDTH>, int> Hash_to_point ;
unordered_map< int, bitset<MAXWIDTH>> Hash_to_bitset ;


//=====================
//图论部分
//原图边产生自程序的两个地方：①extension  ②用叶子结点与没有满足的点连边


//原图边
struct EDGE{
    int from, to, nxt ;
}Edges[(MAXHEIGHT*MAXWIDTH<<1)+1] ;
int head[MAXHEIGHT*MAXWIDTH+1] , idx_edge ;
int addedge(int from, int to) {
    Edges[++idx_edge] = {from,to,head[from]} ;
    head[from] = idx_edge ;
    return idx_edge ;
}

//反图边
EDGE Transpose_Graph_Edges[(MAXHEIGHT*MAXWIDTH<<1)+1] ;
int head_in_Transpose_Graph[MAXHEIGHT*MAXWIDTH+1] , idx_edge_in_Transpose_Graph ;
int add_Transpose_Graph_edge(int from, int to) {
    Transpose_Graph_Edges[++idx_edge_in_Transpose_Graph] = {from,to,head_in_Transpose_Graph[from]} ;
    head_in_Transpose_Graph[from] = idx_edge_in_Transpose_Graph ;
    return idx_edge_in_Transpose_Graph ;
}


//点
struct NODE {//该结构体存储点的属性
    int kind ;
    bool is_original ;  
    int size_of_original_line , *whichline ;
    ~ NODE() {
        delete[] whichline ;
    }
}Nodes[MAXHEIGHT*MAXWIDTH+1] ;
int idx_node ;
int addnode(bitset<MAXWIDTH> bs, int kind, bool is_original, int which_line) {
    //is_original为false时，默认whichline=-1
    //该函数只有在创建未产生过的结点时才能使用
    idx_node++ ;
    Nodes[idx_node].kind = kind ;
    Nodes[idx_node].is_original = is_original ;

    if(is_original) {
        Nodes[idx_node].whichline = new int[MAXHEIGHT+1] ;
        Nodes[idx_node].size_of_original_line = 1 ;
        Nodes[idx_node].whichline[1] = which_line ;
    }else {
        Nodes[idx_node].size_of_original_line = 0 ;
    }
    
    Hash_to_bitset[idx_node] = bs ; 
    Hash_to_point[bs] = idx_node ;


    return idx_node ;
}
void setnode(int index_of_node, int kind) {
    Nodes[index_of_node].kind = kind ;
}


//=====================


/******************** */
//如下为 输入输出 模块
void input_matrix() {
    int x=0,y=0 ;
    char c = getchar() ;
    while(!(x == MAXHEIGHT)){
        if(c=='0' || c=='1') {
            if(c=='0')  original_matrix[x][y] = false ;
            else        original_matrix[x][y] = true ;
            y++ ;
            if(y==MAXWIDTH) y=0 , x++ ;
        }  
        c = getchar() ;
    }
}

void show_matrix() {
    cout << "Original Matrix:" << endl ;
    for(int i=0 ; i<MAXHEIGHT ; i++) {
        for(int j=0 ; j<MAXWIDTH ; j++)
            cout << original_matrix[i][j] << ' ' ;
        cout << endl ;
    }
}

/******************** */
int to_be_splited_point[MAXHEIGHT+1],   sizeof_to_be_splited_point ;
int extended_point[MAXWIDTH*(MAXHEIGHT+2)+1],        sizeof_extended_point ; 

/******************** */
//如下为 具体执行 模块
void initialization() {
    for(int idx=0; idx<MAXWIDTH; idx++) {
        bitset<MAXWIDTH> bs(0) ;
        bs[idx] = 1 ;
        addnode(bs, is_LEAF, false, -1) ;
    }//创建x_0 ~ x_{MAXWIDTH-1}

    for(int line=0; line<MAXHEIGHT; line++) {
        bitset<MAXWIDTH> bs(0) ;
        for(int column=0; column<MAXWIDTH; column++) {
            if(original_matrix[line][column])
                bs[column] = true ;
        }
        if(bs.count() <= 1) continue ;
        if(Hash_to_point.find(bs)!=Hash_to_point.end()) {
            int index_of_point = Hash_to_point[bs] ;

            //注意!!!：如下两行代码是在无条件相信该点已有初始行的情况下进行的，需要保证此时该点已有初始行，否则会调用野指针
            Nodes[index_of_point].size_of_original_line++ ;
            Nodes[index_of_point].whichline[Nodes[index_of_point].size_of_original_line] = line ;
        }else {
            to_be_splited_point[++sizeof_to_be_splited_point] = 
                    addnode(bs, To_Be_Splited, true, line) ;
        }
        
    }//创建入度为0的初始点
}

bitset<MAXWIDTH> intersection[MAXHEIGHT * MAXHEIGHT + 1] ;    int sizeof_intersection ;
vector<int> intersection_in_each_to_be_splited[MAXHEIGHT+1] ;
bitset<MAXHEIGHT+1> to_be_splited_in_each_intersection[MAXHEIGHT * MAXHEIGHT + 1] ;
array<int,4> status_of_intersection[MAXHEIGHT * MAXHEIGHT + 1] ;
set<array<int,4>,greater<array<int,4>>> rnk ;//{value, useful_length, number_of_use, index_of_intersection}


bool compare_extended(int idx1, int idx2) {
    return Hash_to_bitset[idx1].count() < Hash_to_bitset[idx2].count() ;
}

void turn_extended_point_into_to_be_splited_point() {
    sort(extended_point+1, extended_point+sizeof_extended_point+1 , compare_extended) ;
    while(sizeof_extended_point > 0 && sizeof_to_be_splited_point<MAXHEIGHT) {
        setnode(extended_point[sizeof_extended_point], To_Be_Splited);
        to_be_splited_point[++sizeof_to_be_splited_point] = extended_point[sizeof_extended_point] ;
        sizeof_extended_point-- ;
    }//extended_point(MAXHEIGHT) -> to_be_splited_point
}
void clear_rubbish() {
    while(sizeof_to_be_splited_point) {
        to_be_splited_point[sizeof_to_be_splited_point] = 0 ;
        intersection_in_each_to_be_splited[sizeof_to_be_splited_point].clear() ;
        sizeof_to_be_splited_point-- ;
    }    
    while(sizeof_intersection) {
        intersection[sizeof_intersection] = 0 ;
        to_be_splited_in_each_intersection[sizeof_intersection] = 0 ;
        status_of_intersection[sizeof_intersection] = {0,0,0,0} ;
        sizeof_intersection-- ;
    }
    rnk.clear() ;
}

void extension() {
    unordered_map<bitset<MAXWIDTH>,int>intersections ; 
    for(int i=1 ; i<=sizeof_to_be_splited_point ; i++) {
        for(int j=1 ; j<i ; j++) {
            bitset<MAXWIDTH>intersec = Hash_to_bitset[to_be_splited_point[i]] & Hash_to_bitset[to_be_splited_point[j]] ;
            if(intersec.count() <= 1)   continue ;
            if(intersections.find(intersec) == intersections.end()) {
                intersection[++sizeof_intersection] = intersec ;
                intersections.insert({intersec, sizeof_intersection}) ;
                intersection_in_each_to_be_splited[i].push_back(sizeof_intersection) ;
                intersection_in_each_to_be_splited[j].push_back(sizeof_intersection) ;
                to_be_splited_in_each_intersection[sizeof_intersection] = 0 ;
                to_be_splited_in_each_intersection[sizeof_intersection][i] = to_be_splited_in_each_intersection[sizeof_intersection][j] = 1 ;
            }else {
                int idx = intersections[intersec] ;
                intersection_in_each_to_be_splited[i].push_back(idx) ;
                intersection_in_each_to_be_splited[j].push_back(idx) ;
                to_be_splited_in_each_intersection[idx][i] = to_be_splited_in_each_intersection[idx][j] = 1 ;
            }
        }
    }
    for(int i=1 ; i<=sizeof_intersection ; i++) {
        int useful_length = intersection[i].count() ;
        int number_of_use = to_be_splited_in_each_intersection[i].count() ;
        int value = useful_length * (number_of_use ) ;
        status_of_intersection[i] = {useful_length, value, number_of_use, i} ;
        rnk.insert(status_of_intersection[i]) ;
    }
    while(!rnk.empty()) {
        array<int,4>info = *rnk.begin() ;   rnk.erase(rnk.begin()) ;
        if(info[2] <= 1)  continue ;
        
        bitset<MAXWIDTH>bs = intersection[info[3]] ;
        //将 intersection 扩展成为 extended_point 
        if(Hash_to_point.find(bs) == Hash_to_point.end()) {
            extended_point[++sizeof_extended_point] = 
                    addnode(bs, Extended, false, -1) ;
        }else {
            if(Nodes[Hash_to_point[bs]].kind == To_Be_Splited ) {
                setnode(Hash_to_point[bs], Extended) ;
                extended_point[++sizeof_extended_point] = Hash_to_point[bs] ;
            }
            //如果是 extended point, splited point 则跳过扩展阶段
        }

        //将交集的点变为splited,更改估价, 可能产生 extended
        //注意：这里会进行建边
        for(int i=1 ; i<=sizeof_to_be_splited_point ; i++) {
            if( to_be_splited_in_each_intersection[info[3]][i] == 0
                || Hash_to_bitset[to_be_splited_point[i]] == bs)    continue ;
            
            setnode(to_be_splited_point[i], Splited) ;
            //to_be_splited point 与 extended point 建边
            if(to_be_splited_point[i] != Hash_to_point[bs])
                addedge(to_be_splited_point[i], Hash_to_point[bs]) ;

            bitset<MAXWIDTH> fr = Hash_to_bitset[to_be_splited_point[i]] ^ bs ;
            if(fr.count() >= 2) {
                int index_of_fr ;
                if(Hash_to_point.find(fr) == Hash_to_point.end()) {
                    index_of_fr = addnode(fr, Extended, false, -1) ;
                    extended_point[++sizeof_extended_point] = index_of_fr ;
                }
                else    index_of_fr = Hash_to_point[fr] ;
                //to_be_splited point 与 extended 建边   
                addedge(to_be_splited_point[i], index_of_fr) ;
                
            }

            //更改估价
            for(const int &idx_of_intersection:intersection_in_each_to_be_splited[i]) {
                array<int,4> info = status_of_intersection[idx_of_intersection] ;
                to_be_splited_in_each_intersection[info[3]][i] = 0 ;
                if(rnk.find(info) != rnk.end()) {
                    rnk.erase(rnk.find(info)) ;
                    info[2]-- ;
                    info[1] = info[0] * (info[2]) ;
                    status_of_intersection[idx_of_intersection] = info ;
                    rnk.insert(info) ;
                }
            }


        }
    }

    //将未被拆分的 to_be_splited 直接转换成 extended
    for(int i=1 ; i<=sizeof_to_be_splited_point ; i++) {
        int idx = to_be_splited_point[i] ;
        if(Nodes[idx].kind == To_Be_Splited) {
            setnode(idx, Extended) ;
            extended_point[++sizeof_extended_point] = idx ;
        }
    }


    //尾部处理：回收相关资源(只保留 extended_point  )
    clear_rubbish() ;
}


void final_connection_in_the_graph() {
    for(int i=MAXWIDTH+1 ; i<=idx_node ; i++) {
        bitset<MAXWIDTH>bs = Hash_to_bitset[i] ;
        for(int e=head[i] ; e ; e=Edges[e].nxt) {
            int to_point = Edges[e].to ;
            bs ^= Hash_to_bitset[to_point] ;
        }
        while(bs._Find_first()!=MAXWIDTH) {
            int loc = bs._Find_first() ;
            bitset<MAXWIDTH> bs_to_point(0) ;
            bs_to_point[loc] = 1 ;
            int to_point = Hash_to_point[bs_to_point] ;
            
            addedge(i,to_point) ;
            bs[loc] = 0 ;
        }
    }
}

void build_transpose_graph() {
    for(int from=1 ; from<=idx_node ; from++) {//遍历原图中的出点（反图的入点）
        for(int e=head[from] ; e ; e=Edges[e].nxt) {//原图上的边
            int to = Edges[e].to ;
            add_Transpose_Graph_edge(to, from) ;
        }
    }
}
/******************** */

int indgree[MAXHEIGHT*MAXWIDTH+1] ;

int x_of_node[MAXHEIGHT*MAXWIDTH+1] , idx_x ;

array<int,4> result[MAXHEIGHT*MAXWIDTH+1] ;
int sizeof_result ;

queue<int> node_queue ;


void add_result(int to, int from, bool show_y) {
    if(x_of_node[to] == 0) {
        x_of_node[to] = x_of_node[from] ;
    }else {
        result[++sizeof_result] = {++idx_x, x_of_node[to], x_of_node[from], (show_y ? to : 0) } ;
        x_of_node[to] = idx_x ;
    }
}
void get_result_through_topu() {
    //先设定 x_0 ~ x_{MAXWIDTH-1} 对应的点 的 x_of_node 为 1 ~ MAXWIDTH
    for(int i=0 ; i<MAXWIDTH ; i++) {
        bitset<MAXWIDTH> bs(0) ;
        bs[i] = 1 ;
        x_of_node[Hash_to_point[bs]] = ++idx_x ;
    }

    //如下遍历是在反图上进行的
    for(int from=1 ; from<=idx_node ; from++) {
        for(int e=head_in_Transpose_Graph[from] ; e ; e=Transpose_Graph_Edges[e].nxt) {
            int to = Transpose_Graph_Edges[e].to ;
            indgree[to]++ ;
        }
    }
    for(int i=1 ; i<=idx_node ; i++) {
        if(indgree[i] == 0)
            node_queue.push(i) ;
    }
    while(!node_queue.empty()) {
        int from = node_queue.front() ; node_queue.pop() ;
        for(int e=head_in_Transpose_Graph[from] ; e ; e=Transpose_Graph_Edges[e].nxt) {
            int to = Transpose_Graph_Edges[e].to ;

            indgree[to]-- ;
            if(indgree[to] == 0)    node_queue.push(to) ;

            add_result(to, from, (indgree[to] == 0) ) ;
            
            
        }    
    }
}

void print_answer() {
    for(int i=1 ; i<=sizeof_result ; i++) {
        array<int,4> info = result[i] ;
        printf("%4d %4d %4d\n" , info[0]-1, info[1]-1, info[2]-1) ;
    }
}
void show_answer() {
    cout << "Xor Count = " << sizeof_result << endl ;
    for(int i=1 ; i<=sizeof_result ; i++) {
        array<int,4> info = result[i] ;
        printf("x[%d] = x[%d] ^ x[%d]  " , info[0]-1 , info[1]-1 ,  info[2]-1) ;
        if(info[3] && Nodes[info[3]].is_original) {
            for(int j=1 ; j<=Nodes[info[3]].size_of_original_line ; j++) {
                printf("  y[%d]", Nodes[info[3]].whichline[j]) ;
            }
        }

        putchar('\n') ;
    }
}


signed main() {
    freopen(input_file, "r", stdin) ;
    freopen(output_file, "w", stdout) ;

    //按照标准格式读入矩阵
    input_matrix() ;
    //展示读到的矩阵
    show_matrix() ;

    //初始化，创建 x_0 ~ x_{MAXWIDTH-1} 和 入度为0的点（矩阵的行）
    initialization() ;


    // 指定轮数，使得结果按照贪心策略进行收敛
    for(int extension_round=1 ; extension_round<=MAXHEIGHT ; extension_round++) {
        extension() ;

        if(sizeof_extended_point == 0)   break ;
        turn_extended_point_into_to_be_splited_point() ;
    }


    //收敛轮数结束， 将图中点上没有流出的有效位 与 x_0 ~ x_{MAXWIDTH-1} 相连
    final_connection_in_the_graph() ;


    //构建反图
    build_transpose_graph() ;

    //按照反图拓扑序保存结果
    get_result_through_topu() ;



    //输出结果
    //print_answer() ;
    show_answer() ;


    //system("pause") ;
    return 0 ;

}
