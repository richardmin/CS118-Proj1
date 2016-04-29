#include <iostream>
#include <string.h>
using namespace std;
int main()
{
	char buf[] = "GET / HTTP/1.0\r\n";

	int rn_found = 0;
  bool r_found = false;
  bool header_split_found = false;
  while(1)
  {
    for(uint i = 0; i < strlen(buf); i++)
    {
      if(buf[i] == '\r')
      {
        if(r_found)
          rn_found = 0;

        r_found = true;

      }
      else if(buf[i] == '\n')
      {

        if(r_found)
          rn_found++;
        else
          rn_found = 0;

        r_found = false;
      }
      else
      {
        r_found = false;
        rn_found = 0;
      }

      if(rn_found >= 2)
      {
        std::cout << "found end!" << std::endl;
        break;
      }

    }

    if(rn_found >= 2)
    {
      break;
    }
    else
    {
     std::cout << "failed :(" << std::endl;
   }
 }
 std::cout << "we made it" << std::endl;
}