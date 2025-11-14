#include<iostream>
#include<vector>
#include <bits/stdc++.h>

using namespace std;


vector<vector<int>> pair_sum_sorted(vector<int> input, int start, int target)
{
    int l = start;
    int r = input.size()-1;
    vector<vector<int>> pairs;
    //vector<int> pair;
    while(l<=r)
    {
        int sum = input[l] + input[r];
        if(sum == target){
            //pair.push_back(input[l]);
            //pair.push_back(input[r]);
            pairs.push_back({input[l],input[r]});
            l++;
            while(l<r && input[l]==input[l-1])
                l++;
        }
        else if(sum > target)
            r--;
        else
            {
                l++;
            }
             
    }
    return pairs; 
}

vector<vector<int>> triple_sum(vector<int> input)
{
    vector<vector<int>> triplets;
    vector<vector<int>> pairs;
    sort(input.begin(), input.end());
    for(int s=0;s<input.size();s++)
    {   
        if(input[s]>0) break;
        if(s>0 && input[s]==input[s-1]) continue;
        pairs = pair_sum_sorted(input,s+1,-input[s]);
        for(auto &x:pairs)
        {
            //vector<int> triplet;
            //triplet.push_back(input[s]);
            //triplet.insert(triplet.end(),x.begin(),x.end());
            triplets.push_back({input[s],x[0],x[1]});
        }

    }
    return triplets;

}
int main()
{
    vector<int> test1 = {0,-1,2,-3,1};
    vector<vector<int>> output = triple_sum(test1);
    
    for(auto &x:output)
    {
        cout<<"[";
        for(size_t i=0;i<x.size();i++)
        {
            cout<<x[i]<<" ";
            if(i!=x.size()-1) cout<<",";
        }
        cout<<"]";

    }
     
    return 0;
}