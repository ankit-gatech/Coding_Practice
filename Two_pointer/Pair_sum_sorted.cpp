#include<iostream>
#include<vector>

using namespace std;


vector<int> pair_sum_sorted(vector<int> input, int target)
{
    int l = 0;
    int r = input.size()-1;
    while(l<=r)
    {
        if((input[l] + input[r])< target)
            l++;
        else if((input[l] + input[r]) > target)
            r--;
        else
            {
                return {l,r};
            }
             
    }
    return {-1}; 
}

int main()
{
    vector<int> test1 = {-5,-2,3,4,6};
    int target_test1 = 7;
    vector<int> res_test1 = pair_sum_sorted(test1,target_test1);
    for(auto& x:res_test1) cout<<x<<" ";
    return 0;
}