#include<iostream>
#include<vector>

using namespace std;

void shift_zeroes(vector<int> &input)
{
    int l=0;
    for(int r=0;r<input.size();r++)
    {
        if(input[r]!=0)
            {
                swap(input[l],input[r]);
                l++;
            }
    }
}
int main()
{
    vector<int> test = {0,1,0,2,3};
    shift_zeroes(test);
    for(auto &x:test) cout<<x<<" ";
    return 0;
}