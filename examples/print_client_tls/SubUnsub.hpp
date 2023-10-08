//#include <bits/stdc++.h>
#ifndef SUBUNSUB_H
#define SUBUNSUB_H
#include <cstddef>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <ByteBuffer.hpp>   
using namespace std;
enum BinRespTypes {
        CONNECTION_TYPE= 1,
        THROTTLING_TYPE= 2,
        ACK_TYPE= 3,
        SUBSCRIBE_TYPE= 4,
        UNSUBSCRIBE_TYPE= 5,
        DATA_TYPE= 6,
        CHPAUSE_TYPE= 7,
        CHRESUME_TYPE= 8,
        SNAPSHOT= 9,
        OPC_SUBSCRIBE= 10
};
class SubscribeRequest {
 public:
        std::string getScripByteBuffer(std::string c, std::string a) {
        std::vector<string> scripArray;
        boost::split(scripArray, c, boost::is_any_of("&"));
        uint16_t scripsCount = scripArray.size();
        uint16_t dataLen = 0;
        for (auto index = 0; index < scripsCount; ++index) {
            scripArray[index] = a + "|" + scripArray[index];
            dataLen += scripArray[index].length() + 1;
        }
        ByteBuffer bytes; // = new Uint8Array(dataLen + 2);
        auto pos = 0;
        bytes.addShort(scripsCount);
        for (auto index = 0; index < scripsCount; index++) {
            auto currScrip = scripArray[index];
            auto scripLen = currScrip.length();
            uint8_t sz =  scripLen & 255;
            bytes.put(sz);
            //this is pontless ? might just put scripArray[index]
            for (auto strIndex = 0; strIndex < scripLen; strIndex++) {
                bytes.put(currScrip[strIndex]);
            }
        }
        return bytes.tostr();
        }

        std::string prepareSubsUnSubsRequest(std::string c, uint8_t d, std::string e, uint8_t a) {
//           if (!isScripOK(c)) { // only checking number of subs
//             return
//           }
             uint8_t two = 2;
             uint8_t one = 1;
             uint16_t s_two = 2;
             uint16_t s_one = 1;
            std::string dataArr = getScripByteBuffer(c, e);
             uint16_t dataSize = dataArr.length() + 11;
            ByteBuffer buffer;  // new ByteData(dataArr.length + 11); ;
            buffer.addShort(dataSize);
            buffer.put(d); // 4 for subscription 
            buffer.put(two); 
            buffer.put(one);
            buffer.addShort(dataArr.length());
            buffer.put(dataArr);
            buffer.put(two);
            buffer.addShort(s_one);
            buffer.put(a);
            return buffer.tostr();
        }
        std::string parseData(ByteBuffer e) {
            auto pos = 0;
            uint16_t dataSize ;
            auto packetsCount = e.getLong(2);
            cout<<"packets.length: " <<packetsCount;
            return e.tostr();
        }

};
#endif