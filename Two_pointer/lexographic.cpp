#include<iostream>
#include<string>
#include <algorithm>
#include <bits/stdc++.h>

using namespace std;


string next_lexicographical_seq(string input)
{

    int pivot = input.size()-2;

    while(pivot>0 && input[pivot] >= input[pivot+1])
        pivot--;
    
    if(pivot==-1) {
        std::reverse(input.begin(),input.end());
        return input;
    }
        
    int rightmost_successor = input.size()-1;
    while(input[rightmost_successor]<=input[pivot])
        rightmost_successor--;

    swap(input[pivot], input[rightmost_successor]);
    reverse(input.begin()+pivot+1,input.end());
    return input;
}

int main()
{
    string test="abcedda";
    cout<<next_lexicographical_seq(test);
    return 0;
}