#include <cstdio>
#include <cstring>
#include <iostream>

using namespace std;

class dataWriter {

public:
static void writeToFile(string& str) {
     auto now = std::chrono::system_clock::now();
     auto in_time_t = std::chrono::system_clock::to_time_t(now);
     std::stringstream ss;
     ss << std::put_time(std::localtime(&in_time_t), "%d%m%y");
     std::stringstream ss_time;
     ss_time << std::put_time(std::localtime(&in_time_t), "%H%M,");
     std::string fname = "";
     fname =  ss.str();
     fname += "voldata";
     cout<<"Writing to " <<fname<<" length = " <<str.size()<<endl;
     str = ss_time.str() + str;
    FILE* fp = fopen(fname.c_str(), "a");
    if (fp)
	{
		for(int i=0; i<str.size(); i++)  {
			putc(str[i],fp); 
        }
        putc('\n',fp);
	}
      cout<<"done writing"<<endl;
      fclose(fp);
}
};
