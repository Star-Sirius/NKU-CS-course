#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<queue>
#include "源.h"

//.h Part
#include<iostream>
#define N 255
using namespace std;

class BitMap
{
private:
    vector<int> _bit;
public:
    //位图的内存大小和数据范围有关
    BitMap(int range)
        :_bit(range / 32 + 1)
    {}

    void set(int num)
    {
        //计算数组中的下标
        int idx = num / 32;
        //计算num在对应下标整数中的下标位置
        int bitIdx = num % 32;
        //将对应的比特位置1
        _bit[idx] |= 1 << bitIdx;
    }

    bool find(int num)
    {
        int idx = num / 32;
        int bitIdx = num % 32;
        return (_bit[idx] >> bitIdx) & 1;
    }
    //将num位重置为0
    void reset(int num)
    {
        int idx = num / 32;
        int bitIdx = num % 32;
        _bit[idx] &= ~(1 << bitIdx);
    }
    void show()
    {
        for (int i = 0; i < _bit.size(); i++) {
            int temp = _bit[i];
            for (int j = 0; j < 32; j++) {
                cout << (temp & 1); // 输出当前数的最低位
                temp >>= 1; // 右移一位，继续处理下一位
            }
            cout << " "; // 每32位输出一个空格
        }
        cout << endl;
    }//从bit[0]开始显示，直到bit[range*32-1]

    int lp() //返回最高位
    {
        for (int i = N; i >= 0; i--)
        {
            if (find(i))
                return i;
        }//全为0时返回-1
    }
    void Xor(BitMap *t)
    {
        for (int i = 0; i < _bit.size(); i++)
        {
            _bit[i] = _bit[i] ^ t->_bit[i];
        }
    }
};





//.cpp Part
using namespace std;
void Eliminate(BitMap** BeiXiao_bitMap, BitMap** XiaoYuan_bitMap,
                                        int Bei_xiaoYuan_Init_vecSize,int XiaoYuan_Init_VecSize)
{

    vector<BitMap*> Result_Map_LpFrist(0);//放置结果,按lp()从大到小
    
    for (int i = 0; i < Bei_xiaoYuan_Init_vecSize; i++){  
        int temp = BeiXiao_bitMap[i]->lp();//使用temp减少高度调用，但是也要注意在异或之后更改temp
        for (int j = 0; j < XiaoYuan_Init_VecSize; j++) {//要求消元子有序即可
            if (temp > XiaoYuan_bitMap[j]->lp())
                break;
            if (temp == XiaoYuan_bitMap[j]->lp())
            {
                BeiXiao_bitMap[i]->Xor(XiaoYuan_bitMap[j]);
                temp = BeiXiao_bitMap[i]->lp();
            }
            if (Result_Map_LpFrist.size() != 0)
            {
                for (int k = 0; k < Result_Map_LpFrist.size(); k++)
                {
                    if (temp > Result_Map_LpFrist[k]->lp())
                        break;
                    if (temp == Result_Map_LpFrist[k]->lp() )
                    {
                        BeiXiao_bitMap[i]->Xor(Result_Map_LpFrist[k]);
                        temp= BeiXiao_bitMap[i]->lp();
                    }
                }
            }
        }//消元结束，该结果成为新的消元子，并同时成为结果的一部分
         //需要注意的是，新的消元子升格后，会影响之后的消元结果
        if(Result_Map_LpFrist.size()==0)
            Result_Map_LpFrist.push_back(BeiXiao_bitMap[i]);
        else
        {
            for (int t = 0; t < Result_Map_LpFrist.size(); t++)
            {
                if (temp > Result_Map_LpFrist[t]->lp()) {
                    auto it = Result_Map_LpFrist.begin() + t;
                    Result_Map_LpFrist.insert(it, BeiXiao_bitMap[i]);
                    break;
                }
                else if (temp <= Result_Map_LpFrist[Result_Map_LpFrist.size() - 1]->lp())
                {//错误二：判断里的等于一定要加，当全被消完时，temp==末尾的lp()，如果不加等号属于将被消元行丢失
                    Result_Map_LpFrist.push_back(BeiXiao_bitMap[i]);
                    break;
                }
            }
        }
    }//报错一：——向量下标越界——结果为for循环控制条件整错了
     
    for (int i = 0; i < Result_Map_LpFrist.size(); i++) {
        cout << i <<"  "<< Result_Map_LpFrist[i]->lp() << endl;
        //Result_Map_LpFrist[i]->show();
    }
        


}

int main()
{
    ifstream ifs;
    ifs.open("C:\\Users\\李星谊\\Desktop\\SpecialGuassElimi\\测试样例2 矩阵列数254，非零消元子106，被消元行53\\被消元行.txt");
    if (!ifs.is_open())
    {
        cout << "Cannot open the file" << endl;
        return 0;
    }

    // Define a vector of vectors
    vector<vector<int>> Bei_xiaoYuan_Init_vec;//被消元行

    char line[1024] = { 0 };

    // Loop over all lines in the file
    while (ifs.getline(line, sizeof(line)))
    {
        // Define a temporary vector to store the row
        vector<int> Bei_xiaoYuan_singleRow;

        // Extract the numbers from the line
        stringstream ss(line);
        int num;
        while (ss >> num)
        {
            Bei_xiaoYuan_singleRow.push_back(num);
        }

        // Add the row to the array of vectors
        Bei_xiaoYuan_Init_vec.push_back(Bei_xiaoYuan_singleRow);
    }
    
    // Print the array of vectors
    /*for (auto row : Bei_xiaoYuan_Init_vec)
    {
        for (auto num : row)
        {
            cout << num << " ";
        }
        cout << endl;
    }*/
    ifs.close();
    cout << endl;//成功读取被消元行

    ifs.open("C:\\Users\\李星谊\\Desktop\\SpecialGuassElimi\\测试样例2 矩阵列数254，非零消元子106，被消元行53\\消元子.txt");
    if (!ifs.is_open())
    {
        cout << "Cannot open the file" << endl;
        return 0;
    }

    // Define a vector of vectors
    vector<vector<int>> XiaoYuan_Init_Vec;//消元子
    
    char line2[1024] = { 0 };//有可能随着数组增长它爆掉了

    // Loop over all lines in the file
    while(ifs.getline(line2, sizeof(line2)))
    {
        // Define a temporary vector to store the row
        vector<int> XiaoYuan_singleRow;

        // Extract the numbers from the line
        stringstream ss(line2);
        int num2;
        while (ss >> num2)
        {
            XiaoYuan_singleRow.push_back(num2);
        }

        // Add the row to the array of vectors
        XiaoYuan_Init_Vec.push_back(XiaoYuan_singleRow);
    }
    /*for (auto row : XiaoYuan_Init_Vec)
    {
        for (auto num : row)
        {
            cout << num << " ";
        }
        cout << endl;
    }*/
    ifs.close();//成功读取消元子



    //创建位图：

     BitMap** BeiXiao_bitMap = new BitMap * [Bei_xiaoYuan_Init_vec.size()]; // 创建被消元行位图
    for (int i = 0; i < Bei_xiaoYuan_Init_vec.size(); ++i) {
        BeiXiao_bitMap[i] = new BitMap(N);
    }
     BitMap** XiaoYuan_bitMap = new BitMap * [XiaoYuan_Init_Vec.size()]; // 创建消元子位图
    for (int i = 0; i < XiaoYuan_Init_Vec.size(); ++i) {
        XiaoYuan_bitMap[i] = new BitMap(N); 
    }
    
    for (int i = 0; i < Bei_xiaoYuan_Init_vec.size(); i++)
        for (int j = 0; j < Bei_xiaoYuan_Init_vec[i].size(); j++)
            BeiXiao_bitMap[i]->set(Bei_xiaoYuan_Init_vec[i][j]);

    for (int i = 0; i < XiaoYuan_Init_Vec.size(); i++)
        for (int j = 0; j < XiaoYuan_Init_Vec[i].size(); j++)
            XiaoYuan_bitMap[i]->set(XiaoYuan_Init_Vec[i][j]);



    /*for (int i = 0; i < Bei_xiaoYuan_Init_vec.size(); ++i)
        BeiXiao_bitMap[i]->show();
    
    for (int i = 0; i < XiaoYuan_Init_Vec.size(); ++i)
        XiaoYuan_bitMap[i]->show();*/


    cout << "被消元行的个数为： " << Bei_xiaoYuan_Init_vec.size() << endl;
    cout << "消元子的个数为： " << XiaoYuan_Init_Vec.size() << endl;

    cout << endl << endl;

    Eliminate(BeiXiao_bitMap, XiaoYuan_bitMap, Bei_xiaoYuan_Init_vec.size(), XiaoYuan_Init_Vec.size());



    return 0;
}
