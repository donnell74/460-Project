#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
using namespace std;

string result = "";

void convbase(int number)
{
  int remainder;
  
  if(number <=2)
    {
      cout << number;
      return;
    }

  remainder = number%4;
  convbase(number >> 2);
  result += to_string(remainder);
}

int main(int argc, char ** argv)
{
  int number = atoi(argv[1]);

  if(number < 0)
    {
      cout<< "Must be positive integer" <<endl;
    }

  else
    {
      convbase(number);
    }

  cout<<result<<endl;
  exit(EXIT_SUCCESS);
}
