#ifndef PUBLISHER_H
#define PUBLISHER_H
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "iostream"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include <fstream>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/thread.hpp>   
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <SubUnsub.hpp>   
//extern std::mutex m_mutex;
//extern std::condition_variable cv;
//extern bool ready;

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

extern client c;
//#define RAPIDJSON_HAS_STDSTRING 1
class publisher {

   static  zmq::socket_t socket_tt;

   public:
    void static  connectTo(std::string topic, boost::lockfree::spsc_queue<std::string>* q ) {
      zmq::context_t ctx;
      zmq::socket_t sock1(ctx, zmq::socket_type::pub);
//     sock1.set( zmq::sockopt::ZMQ_LINGER, 0 );   
      sock1.bind("tcp://127.0.0.1:*");
      std::string fname = "/var/tmp/connect.json";
     // std::ofstream jsonfile(fname);
      const std::string last_endpoint =
      sock1.get(zmq::sockopt::last_endpoint);
      std::cout << "Connecting to "
              << last_endpoint << std::endl;
                // Create the JSON document
      FILE* fp = fopen(fname.c_str(), "w");
      char buffer[65536]; 
      rapidjson::Document d;
      d.SetObject();
      rapidjson::Value key(topic.c_str(),d.GetAllocator());
      rapidjson::Value value(last_endpoint.c_str(),d.GetAllocator());
      d.AddMember(key, value, d.GetAllocator());
      rapidjson::FileWriteStream os(fp, buffer, sizeof(buffer));
      rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
      d.Accept(writer);
      fclose(fp);
      //send a hello message only for debuging
      sock1.send(zmq::str_buffer("hello"), zmq::send_flags::sndmore);
      sock1.send(zmq::str_buffer("how are you"));
      while (true) {
         std::unique_lock<std::mutex> lck{m_mutex};
         while (!ready) cv.wait(lck);
         ready = false;
         std::string s;
           while (q->pop(s)) {
               std::cout<<"sending to socket"<<std::endl;
               zmq::message_t msg(s);
               sock1.send(zmq::str_buffer("nifty"), zmq::send_flags::sndmore);
               sock1.send(std::move(msg));
           }
           // if()
      }
      std::cout << "sent hello "<<std::endl;
   } 

    void static  publish(zmq::socket_ref sock1,std::string topic, const  std::string message) {
      sock1.send(zmq::str_buffer("nifty"), zmq::send_flags::sndmore);
               zmq::message_t msg(message);
      sock1.send((msg));
   }

   void static snapShots(std::string scrip_index, uint8_t snapShotType9, std::string scrip, uint8_t channel , client::connection_ptr con ) {
      cout<<" Snap Shot Async  "<<endl;
      bool keepRunning = true;
      websocketpp::lib::error_code ec;
      while(keepRunning) {
       auto now = std::chrono::system_clock::now();
       auto in_time_t = std::chrono::system_clock::to_time_t(now);
      std::stringstream ss;
      ss << std::put_time(std::localtime(&in_time_t), "%H%M");
      string time = ss.str();
      string timeMinutes = time.substr(2,2);
      cout<<" time Min " << timeMinutes << " of " <<time<<endl;
      int timeInt = stoi(timeMinutes) ;
//      if(timeInt%5 == 0) {
         cout<<" snapping  " << timeMinutes << " of " <<time<<endl;
        SubscribeRequest s;
        std::string bufsub = s.prepareSubsUnSubsRequest(scrip_index, snapShotType9, scrip, channel); // scrip and line 406
        string scrip_nifty = "nse_cm|Nifty 50";
        const string index="if";
//        bufsub = s.prepareSubsUnSubsRequest(scrip_nifty, snapShotType9, index, channel); // scrip and line 406
        c.send(con->get_handle(), bufsub, websocketpp::frame::opcode::binary, ec);
//      }
      boost::this_thread::sleep( boost::posix_time::seconds(60) );
   }
   }
};
#endif