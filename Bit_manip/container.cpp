#include<iostream>
#include<vector>

using namespace std;

int Largest_container(vector<int> input)
{
    int l=0,r=input.size()-1;
    int max_water=0;
    while(l<r)
    {
        int water=min(input[l],input[r]) * abs(r-l);
        max_water=max(water,max_water);
        if(input[l]<input[r])
            l++;
        else if(input[l]>input[r])
            r--;
        else
            {
                l++; r--;
            }
    }

    return max_water;
}

int main()
{
    vector<int> test= {2,7,8,3,7,6};
    cout<<Largest_container(test);
    return 0;
}