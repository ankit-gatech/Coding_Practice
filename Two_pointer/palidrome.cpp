#include<iostream>
#include<string>

using namespace std;

bool ispalindrome(string s)
{
    int l=0,r=s.size()-1;
    while(l<r)
    {
        while(l<r && !(isalnum(s[l])))
            l++;
        while(l<r && !(isalnum(s[r])))
            r--;
        if(s[l]!=s[r]) return false;
        l++; r--;
    }
    return true;
}

int main()
{

    string s="a dog! a panic in a pagoda.";
    if(ispalindrome(s))
    {
        cout<<"True";
    }
    else
    {
        cout<<"False";
    }
    return 0;
}