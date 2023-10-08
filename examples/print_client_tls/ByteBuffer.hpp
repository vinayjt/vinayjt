#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include<unordered_map>
 #include <array> 
 #include "dataWriter.hpp" 
extern std::mutex m_mutex;
extern std::condition_variable cv;
extern bool ready; 
extern bool niftyUnreal;
using namespace std;
class ByteBuffer : std::stringbuf
{
    enum ResponseTypes {
        SNAP= 83,
        UPDATE= 85
    };
    enum index {
        ltp=0,
        qty= 1,
        volume=2
    };
    unordered_map<string, vector<double>> stockmap;
    static const int sizeArray = 50;
public:
    unordered_map<string, array<float, sizeArray>> priceMap;
    array<float, sizeArray> priceArray;
    unordered_map<string, array<long long, sizeArray>> volMap;
    array<long long, sizeArray> volArray;
    unordered_map<string, array<long, sizeArray>> buyMap;
    array<long, sizeArray> buyArray;
    unordered_map<string, array<long, sizeArray>> sellMap;
    array<long, sizeArray> sellArray;
    array<string, sizeArray> topicArray;
    template <typename T>
    size_t get( T &out)
    {
        union coercion { T value; char data[ sizeof ( T ) ]; };

        coercion c;

        size_t s= xsgetn( c.data, sizeof(T));

        out= c.value;

        return s;
    }

    template <typename T>
    size_t put( T &in)
    {   
        union coercion { T value; char data[ sizeof ( T ) ]; };

        coercion c;

        c.value=in;

        return xsputn( c.data, sizeof(T));
    }

    template <>
    size_t put(std::string &in)
    {   
        return xsputn(in.c_str(), in.length());
    }

    size_t put(const std::string *in)
    {   
        return xsputn(in->c_str(), in->length());
    }

    std::string tostr() {
      return str();
    }


    size_t get( uint8_t *out, size_t count) {
        return xsgetn((char *)out, count);
    }

    u_long getLong(size_t count) {
        char out[count];
        xsgetn(out, count);
//        std::cout<<"long bitset "<<std::bitset<8>(out[0])<<std::endl;
        u_long val = 0;
        int j = count-1;
        for(size_t i = 0; i < count; ++i,--j) {
            unsigned char jj = out[j];
//            std::cout<<"long bitset "<<std::bitset<8>(jj)<<std::endl;
            val += jj << (i * 8);
//            std::cout<<"long =  "<<std::dec<<val<<std::endl;
        }
//        std::cout<<"long val "<<std::dec<<val<<std::endl;
        return  val;
    }

    std::string getString(int size) {
        char out[size];
        xsgetn(out, size);
        std::string s(out);
        return s;
    }

    size_t put( uint8_t *out, size_t count)
    {
        return xsputn((char *)out, count);
    }

    void addShort(int endPos) {
       uint16_t len = endPos ; 
//       std::cout<<" len " << len<<std::endl;
       unsigned char y = ((len >> 8) & 255);
       char ch = y;
//       std::cout<<" ch1 "<<std::bitset<8>(ch)<<std::endl;
       xsputn(&ch,1);  
       y = (len & 255);
       ch = y;
//       std::cout<<" ch2 "<<std::bitset<8>(ch)<<std::endl;
       xsputn(&ch,1);  
    }
    void addInt(int endPos) {
       uint32_t len = endPos ; 
//       std::cout<<" len int " << len<<std::endl;
       unsigned char y = ((len >> 24) & 255);
       char ch = y;
//       std::cout<<" ch4 "<<std::bitset<8>(ch)<<std::endl;
       xsputn(&ch,1);  
       y = ((len >> 16) & 255);
       ch = y;
//       std::cout<<" ch3 "<<std::bitset<8>(ch)<<std::endl;
       xsputn(&ch,1);  
       y = ((len >> 8) & 255);
       ch = y;
 //      std::cout<<" ch2 "<<std::bitset<8>(ch)<<std::endl;
       xsputn(&ch,1);  
       y = (len & 255);
       ch = y;
 //      std::cout<<" ch1 "<<std::bitset<8>(ch)<<std::endl;
       xsputn(&ch,1);  
    }
    u_long parseAck() { 
        //there are two sets
        std::cout<<"fcount "<< getLong(1)<<std::endl;;                // fid id . not sure what is the usage
        std::cout<<"fid "<< getLong(1)<<std::endl;;                // fid id . not sure what is the usage
//        getLong(1);                // fid id . not sure what is the usage
        auto val = getLong(2);     // length of status field id . not sure what is the usage
        auto sval = getString(val); // length of status field id . not sure what is the usage
        std::cout<<"status "<<sval.substr(0,val)<<std::endl;

        val = getLong(1);                // fid id . not sure what is the usage
        std::cout<<"fid "<<val<<std::endl;
        val = getLong(2);     // length of status field id . not sure what is the usage
        std::cout<<"length "<<val<<std::endl;
        auto rval = getLong(val);   // length of ack field id . not sure what is the usage
        return rval;
    }
    u_long parseMessageNumber() { 
       return getLong(4);
    }
    std::string getMessageBuf(int ack) {
        ByteBuffer myBuf ;
        uint8_t ackType = 3;
        uint8_t one = 1;
        uint8_t four = 4;
        uint32_t messageNum = ack;
        myBuf.addShort(11);
        myBuf.put(ackType);
        myBuf.put(one);
        myBuf.put(one);
        myBuf.addShort(4);
        myBuf.addInt(messageNum);
        return myBuf.tostr();
    }
    std::string parseData(boost::lockfree::spsc_queue<std::string>* q) {
                auto g = getLong(2);
                while(g--) {
                   auto f = getLong(2); //  should be scrip name
                   auto responseType =  getLong(1); //  1 bit response type
                   if(responseType == SNAP) {
                       auto topicId = getLong(4);
                       auto len = getLong(1);
                       auto topicName = getString(len);
//                       stockmap[topicName] = 
                       auto fcount =  getLong(1);
                       vector<double> v;
                       cout<<"SNAP count and all "<<std::dec<<fcount<<" "<<topicId<<" "<<topicName.substr(0,len)<<endl;
                       while(fcount--) {
                            auto fvalue = getLong(4);
                            v.push_back(fvalue);
                            cout<<"fvalue = "<<std::dec<<fvalue<<endl;
                       }
                       fcount =  getLong(1);
                       unordered_map<long,std::string> m;
                       while(fcount--) {
                            auto fid = getLong(1);
                            auto len = getLong(1);
                            auto str = getString(len);
                            m[fid] = str;
                            cout<<"len = "<<len<<"fid  = "<<fid<<" str = "<<str.substr(0,len)<<endl;
                       }
                   }

                   if(responseType == UPDATE) {
                       auto topicId = getLong(4);
                       auto fcount = getLong(1);
//                       stockmap[topicName] = 
                       vector<double> v;
                       cout<<"UPDATE count and all "<<std::dec<<fcount<<" "<<topicId<<endl;
                       string str = "";
                       while(fcount--) {
                            auto fvalue = getLong(4);
//                            v.push_back(fvalue);
                            if(topicId ==0 && (fcount==17 ||fcount==13 || fcount ==12 )) {
                                std::cout << std::setprecision(2) << std::fixed;
                                str += to_string((double)fvalue/100);
                                str +=",";
                                cout<<"fvalue = "<<std::dec<<str<< " fcount = " << fcount<< endl;
                                if(fcount==12) {
                                    q->push(str);
                                    str ="";
                                    ready = true;
                                    cv.notify_one();
                                }
                            }
                            if(topicId ==1 &&fcount==5 ) {
                                std::cout << std::setprecision(2) << std::fixed;
                                str = to_string((double)fvalue/100);
                                cout<<"fvalue = "<<std::dec<<str<<endl;
                                q->push(str);
                                str ="";
                                ready = true;
                                cv.notify_one();
                            }
                       }
                   }

                }
             return "hello";
    }
std::string parseDataUnreal(boost::lockfree::spsc_queue<std::string>* q) {
                auto g = getLong(2);
                while(g--) {
                   auto f = getLong(2); //  should be scrip name
                   auto responseType =  getLong(1); //  1 bit response type
                   if(responseType == SNAP) {
                       auto topicId = getLong(4);
                       auto len = getLong(1);
                       auto topicName = getString(len);
//                       stockmap[topicName] = 
                       auto fcount =  getLong(1);
                       vector<double> v;
                       cout<<"SNAP count and all "<<std::dec<<fcount<<" "<<topicId<<" "<<topicName.substr(0,len)<<endl;
                       topicId = topicId%50;
                       cout<<"adjusted = "<<topicId<<" "<<topicName.substr(0,len)<<endl;
                       string strToWrite;
                       while(fcount--) {
                            auto fvalue = getLong(4);
                            getFvalueSnap(fcount,fvalue,strToWrite,topicId);
                            v.push_back(fvalue);
//                            cout<<"fvalue = "<<std::dec<<fvalue<<endl;
                       }
                       fcount =  getLong(1);
                       unordered_map<long,std::string> m;
                       while(fcount--) {
                            auto fid = getLong(1);
                            auto len = getLong(1);
                            auto str = getString(len);
                            m[fid] = str;
                            if(fid == 54) {
                                string scripName = str.substr(0,len-3) ;
                                string s = str.substr(0,len-3) + ",";
                                strToWrite = s + strToWrite;
                                cout<<"data = "<<strToWrite<<endl;
                                dataWriter::writeToFile(strToWrite);
                                cout<<"data topicID = "<<topicId<<endl;
                                topicArray[topicId] = scripName;
//                                priceMap[scripName] = priceArray;
//                                volMap[scripName] = volArray;
//                                sellMap[scripName] = sellArray;
//                               buyMap[scripName] = buyArray;
                            }
                            cout<<"len = "<<len<<"fid  = "<<fid<<" str = "<<str.substr(0,len-1)<<endl;
                       }
                   }

                   if(responseType == UPDATE) {
                       auto topicId = getLong(4);
                       auto fcount = getLong(1);
                       auto now = std::chrono::system_clock::now();
                       auto in_time_t = std::chrono::system_clock::to_time_t(now);
                       std::stringstream ss;
                       ss << std::put_time(std::localtime(&in_time_t), "%X");
//                       stockmap[topicName] = 
                       vector<double> v;
                       cout<<"UPDATE count and all "<<std::dec<<fcount<<" "<<topicId<<endl;
                       string str = "" + ss.str();
                        str += ",";
                       str +=  to_string(topicId);
                        str += ",";
                       while(fcount--) {
                            auto fvalue = getLong(4);
//                            v.push_back(fvalue);
                                 // 17 == Vol 
                                 // 16 == Ltp 
                                 // 14 ==  Sell Qty 
                                 // 13 ==  Buy Qty 

                            if((fcount==17 ||fcount==16 || fcount ==14 || fcount ==13 )) {
                                std::cout << std::setprecision(2) << std::fixed;
                                if(fcount == 16) {
                                    str += to_string((double)fvalue/100); 
                                } else {
                                    str += to_string(fvalue);
                                }
                                str +=",";
                                if(fcount == 13) {
                                    cout<<"fvalue = "<<std::dec<<str<< " fcount = " << fcount<< endl;
                                }
                            }
                       }
                   }

                }
             return "hello";
    }

   void getFvalueSnap(int fcount, long fvalue, string & str , const long topicId) {
        // 17 == Vol 
        // 16 == Ltp 
        // 14 ==  Sell Qty 
        // 13 ==  Buy Qty 
                            if((fcount==20 ||fcount==19 || fcount ==17 || fcount ==16 )) {
                                std::cout << std::setprecision(2) << std::fixed;
                                if(fcount == 19) {
                                    str += to_string((double)fvalue/100); 
                                    std::fill_n(priceArray.begin() + topicId,1,fvalue/100);
                                } else {
                                    str += to_string(fvalue);
                                }
                                str +=",";
                                if(fcount == 20) {
                                    std::fill_n(volArray.begin() + topicId,1,fvalue);
                                }
                                if(fcount == 17) {
                                    std::fill_n(buyArray.begin() + topicId,1,fvalue);
                                }
                                if(fcount == 16) {
                                    std::fill_n(sellArray.begin() + topicId,1,fvalue);
                                    cout<<"fvalue = "<<std::dec<<str<< " fcount = " << fcount<< endl;
                                }
                            }
 
   }

   long getSummationVol() {
    //          long total =   accumulate(volArray.begin(), volArray.end(),0); 
             long total = 0;
             for(int i=0; i< volArray.size();++i ) {
                total += volArray[i]*priceArray[i];
             }
             cout<<"total Vol " << total << endl;
             return total;
   }
   long getSummationBuyValue() {
             long total = 0;
             for(int i=0; i< buyArray.size();++i ) {
                total += buyArray[i]*priceArray[i];
             }
             cout<<"total Buy " << total << endl;
             return total;
   }
   long getSummationSellValue() {
             long total = 0;
             for(int i=0; i< sellArray.size();++i ) {
                total += sellArray[i]*priceArray[i];
             }
             cout<<"total Sell " << total << endl;
             return total;
   }
   float getSummationMyIndex() { 
        long total =   accumulate(volArray.begin(), volArray.end(),0); 
        float index = static_cast<float> (getSummationVol()) /static_cast<float> (total);
        cout<<"index value " << index << endl;
        return index ;
   }

   void storeSums() {
        string strToWrite = "SUM,"; 
        strToWrite +=  to_string(getSummationVol());
        strToWrite +=  ",";
        strToWrite +=  to_string(getSummationMyIndex());
        strToWrite +=  ",";
        strToWrite +=  to_string(getSummationBuyValue());
        strToWrite +=  ",";
        strToWrite +=  to_string(getSummationSellValue());
        cout<<strToWrite<<endl;
        dataWriter::writeToFile(strToWrite);
   }


};
#endif