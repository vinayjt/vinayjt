/*
 * Copyright (c) 2016, Peter Thorson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebSocket++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <cstddef>
#include <algorithm>
#include <vector>
#include <future>

#include <iostream>
#include <fstream>
#include <boost/thread.hpp>   
#include <boost/lockfree/spsc_queue.hpp>
#include <rapidjson/document.h>
#include <ByteBuffer.hpp>   
#include <SubUnsub.hpp>   
#include <publisher.cpp>   
#include "prepareSubString.hpp"   

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using namespace::std;
client c;
static int counter =0;
int ack =0;
bool niftyUnreal = false;
boost::lockfree::spsc_queue<std::string> q{100};
std::mutex m_mutex;
std::condition_variable cv;
bool ready = false;
void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
    websocketpp::lib::error_code ec;
//    std::cout << msg->get_payload() << std::endl;
    std::cout << "M" << std::endl;
    ByteBuffer buf;
    std::string payload = msg->get_payload();
    buf.put(payload);
    int packSize= buf.getLong(2);
    int connectionType= buf.getLong(1);
//    cout<<"packet size "<<packSize<<endl;
//    cout<<"connect type  "<<connectionType<<endl;
    if(connectionType == CONNECTION_TYPE){
       ack = buf.parseAck();
       cout<<"ack = "<<ack<<endl;
    }
    if(connectionType == DATA_TYPE){
        ++counter;
        auto messageNumber = buf.parseMessageNumber();
 //           cout<<"ack  counter messagenumber "<<ack<<":"<<counter<<":"<<messageNumber<<endl;
        if(counter == ack) {
            auto messageNumberBuf = buf.getMessageBuf(messageNumber);
//            cout<<"sending ack  "<<std::hex<<messageNumberBuf<<endl;
            c.send(hdl, messageNumberBuf, websocketpp::frame::opcode::binary, ec);                    
            if(ec) {
               std::cout << "> Error sending message: " << ec.message() << std::endl;
            }
            counter=0 ;
        }
        if(niftyUnreal) {
         std::string theData = buf.parseDataUnreal(&q); 
         buf.storeSums();
        } else {
         std::string theData = buf.parseData(&q); 
        }
//        q.push(theData);
    }
}

/// Verify that one of the subject alternative names matches the given hostname
bool verify_subject_alternative_name(const char * hostname, X509 * cert) {
    STACK_OF(GENERAL_NAME) * san_names = NULL;
    
    san_names = (STACK_OF(GENERAL_NAME) *) X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (san_names == NULL) {
        return false;
    }
    
    int san_names_count = sk_GENERAL_NAME_num(san_names);
    
    bool result = false;
    
    for (int i = 0; i < san_names_count; i++) {
        const GENERAL_NAME * current_name = sk_GENERAL_NAME_value(san_names, i);
        
        if (current_name->type != GEN_DNS) {
            continue;
        }
        
        char const * dns_name = (char const *) ASN1_STRING_get0_data(current_name->d.dNSName);
        
        // Make sure there isn't an embedded NUL character in the DNS name
        if (ASN1_STRING_length(current_name->d.dNSName) != strlen(dns_name)) {
            break;
        }
        // Compare expected hostname with the CN
        result = (strcasecmp(hostname, dns_name) == 0);
    }
    sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
    
    return result;
}

/// Verify that the certificate common name matches the given hostname
bool verify_common_name(char const * hostname, X509 * cert) {
    // Find the position of the CN field in the Subject field of the certificate
    int common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert), NID_commonName, -1);
    if (common_name_loc < 0) {
        return false;
    }
    
    // Extract the CN field
    X509_NAME_ENTRY * common_name_entry = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
    if (common_name_entry == NULL) {
        return false;
    }
    
    // Convert the CN field to a C string
    ASN1_STRING * common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
    if (common_name_asn1 == NULL) {
        return false;
    }
    
    char const * common_name_str = (char const *) ASN1_STRING_get0_data(common_name_asn1);
    
    // Make sure there isn't an embedded NUL character in the CN
    if (ASN1_STRING_length(common_name_asn1) != strlen(common_name_str)) {
        return false;
    }
    
    // Compare expected hostname with the CN
    return (strcasecmp(hostname, common_name_str) == 0);
}

/**
 * This code is derived from examples and documentation found ato00po
 * http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/example/cpp03/ssl/client.cpp
 * and
 * https://github.com/iSECPartners/ssl-conservatory
 */
bool verify_certificate(const char * hostname, bool preverified, boost::asio::ssl::verify_context& ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // Retrieve the depth of the current cert in the chain. 0 indicates the
    // actual server cert, upon which we will perform extra validation
    // (specifically, ensuring that the hostname matches. For other certs we
    // will use the 'preverified' flag from Asio, which incorporates a number of
    // non-implementation specific OpenSSL checking, such as the formatting of
    // certs and the trusted status based on the CA certs we imported earlier.
    int depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());

    // if we are on the final cert and everything else checks out, ensure that
    // the hostname is present on the list of SANs or the common name (CN).
    if (depth == 0 && preverified) {
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        
        if (verify_subject_alternative_name(hostname, cert)) {
            return true;
        } else if (verify_common_name(hostname, cert)) {
            return true;
        } else {
            return false;
        }
    }

    return preverified;
}

/// TLS Initialization handler
/**
 * WebSocket++ core and the Asio Transport do not handle TLS context creation
 * and setup. This callback is provided so that the end user can set up their
 * TLS context using whatever settings make sense for their application.
 *
 * As Asio and OpenSSL do not provide great documentation for the very common
 * case of connect and actually perform basic verification of server certs this
 * example includes a basic implementation (using Asio and OpenSSL) of the
 * following reasonable default settings and verification steps:
 *
 * - Disable SSLv2 and SSLv3
 * - Load trusted CA certificates and verify the server cert is trusted.
 * - Verify that the hostname matches either the common name or one of the
 *   subject alternative names on the certificate.
 *
 * This is not meant to be an exhaustive reference implimentation of a perfect
 * TLS client, but rather a reasonable starting point for building a secure
 * TLS encrypted WebSocket client.
 *
 * If any TLS, Asio, or OpenSSL experts feel that these settings are poor
 * defaults or there are critically missing steps please open a GitHub issue
 * or drop a line on the project mailing list.
 *
 * Note the bundled CA cert ca-chain.cert.pem is the CA cert that signed the
 * cert bundled with echo_server_tls. You can use print_client_tls with this
 * CA cert to connect to echo_server_tls as long as you use /etc/hosts or
 * something equivilent to spoof one of the names on that cert 
 * (websocketpp.org, for example).
 */
context_ptr on_tls_init(const char * hostname, websocketpp::connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    boost::system::error_code ec;

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);

        std::cout << "options done" << std::endl;

//        ctx->set_verify_mode(boost::asio::ssl::verify_peer);
          ctx->set_verify_mode(boost::asio::ssl::verify_none);
        std::cout << "verify done" << std::endl;
        ctx->set_verify_callback(bind(&verify_certificate, hostname, ::_1, ::_2));
        std::cout << "verify callback done" << std::endl;

        // Here we load the CA certificates of all CA's that this client trusts.
//        ctx->load_verify_file("ca-chain.cert.pem");
        std::cout << "verify file done" << std::endl;
       // ec = ctx->get_transport_ec();
        if(ec) {
	   std::cerr<<"Init tls failed,reason:"<< ec.message()<<std::endl;
        } 
    } catch (std::exception& e) {
        std::cout << "in exception" << std::endl;
        std::cout << e.what() << std::endl;
    }
    return ctx;
}

const void runSocket(client* c) 
{
    std::cout << "run Socket "<< std::endl ;
    c->run();
}

rapidjson::Document readJsonFile(istream& file) {
   // ifstream file("example.json");
  
    // Read the entire file into a string
    string json((istreambuf_iterator<char>(file)),
                istreambuf_iterator<char>());
  
    // Create a Document object 
      // to hold the JSON data
    rapidjson::Document doc;
  
    // Parse the JSON data
    //cout<<"the json string "<<json.c_str()<<endl;
    doc.Parse(json.c_str());
  
    // Check for parse errors
    if (doc.HasParseError()) {
        cerr << "Error parsing JSON: "
             << doc.GetParseError() << endl;
        return doc;
    }
  
    // Now you can use the Document object to access the
    // JSON data
    return doc;
}
std::string  prepareConnectionRequest2(string a, string c) {
        string src = "JS_API";
        uint8_t connectionType = 1; 
        uint8_t numberOfElements = 3; 
        uint8_t ElementSeq_1 = 1; 
        uint8_t ElementSeq_2 = 2; 
        uint8_t ElementSeq_3 = 3; 
        uint16_t srcLen = src.length();
        uint16_t jwtLen = a.length();
        uint16_t  redisLen = c.length();
        ByteBuffer buffer;
//        let buffer = new ByteData(srcLen + jwtLen + redisLen + 13);
        uint16_t size= srcLen + jwtLen + redisLen + 11;
//        buffer.markStartOfMsg();
        buffer.addShort(size);
//        buffer.put(size); // Size 2 byte Type = 1
        buffer.put(connectionType);    // Connection Type = 1
        buffer.put(numberOfElements);   // 3 element , should be 1 byte
        buffer.put(ElementSeq_1); // first element 
        buffer.addShort(jwtLen);
        buffer.put(a);
        buffer.put(ElementSeq_2);
        buffer.addShort(redisLen);
        buffer.put(c);
        buffer.put(ElementSeq_3);
        buffer.addShort(srcLen);
        buffer.put(src);
        return buffer.tostr();
}

void print_bytes(std::ostream& out, const char *title, const char *data, size_t dataLen, bool format = true) {
    out << title << std::endl;
    out << std::setfill('0');
    for(size_t i = 0; i < dataLen; ++i) {
        out << std::dec << std::setw(2) << std::bitset<8>(data[i]);
//        out << std::dec<<std::setw(2)<< int(data[i]);
        if (format) {
            out << (((i + 1) % 16 == 0) ? "\n" : " ");
        }
    }
    out << std::endl;
}

int main(int argc, char* argv[]) {
//    client c;
    cout<<"start setup" << endl;
    ifstream jsonfile("./neo_s.json");
    rapidjson::Document d;
    d=readJsonFile(jsonfile);

    std::string hostname = "lhsm.kotaksecurities.com";
    std::string port = "443";

    std::string uri = "wss://" + hostname; 
   //  "wss://lhsi.kotaksecurities.com/realtime?sId="+handshakeServerId; order socket data address
    if(argc > 1 && std::strcmp(argv[1], "niftyUnreal" ) == 0 ) {
            cout<<"seting up niftyunreal" << endl;
            niftyUnreal = true;
    } 
     

    try {
       publisher p;
       auto ft  = std::async(std::launch::async, publisher::connectTo, "nifty" , &q);
     //   p.connectTo("nifty");
        // Set logging to be pretty verbose (everything except message payloads)
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        c.set_error_channels(websocketpp::log::elevel::all);

        // Initialize ASIO
        c.init_asio();

        // Register our message handler
        c.set_message_handler(&on_message);
        c.set_tls_init_handler(bind(&on_tls_init, hostname.c_str(), ::_1));

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return 0;
        }

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        c.connect(con);

        c.get_alog().write(websocketpp::log::alevel::app, "Connecting to " + uri);

        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        boost::thread t1(runSocket, &c);
        string buf;
        if(d.HasMember("data")) {
          rapidjson::Value *config_node = &(d["data"]);
//          cout<<(*config_node)["token"].GetString() ;
//         cout<<(*config_node)["sid"].GetString() ;
          buf = prepareConnectionRequest2((*config_node)["token"].GetString(),(*config_node)["sid"].GetString());
//          cout<<"buf =["<<std::hex<<buf<<"]"<<endl;
//          print_bytes(cout,"Buffer" ,buf.c_str(),buf.length()) ;
           }  
        boost::this_thread::sleep( boost::posix_time::seconds(1) );
        c.send(con->get_handle(), buf, websocketpp::frame::opcode::binary, ec);
        boost::this_thread::sleep( boost::posix_time::seconds(2) );
        // subscriber 
        const string index="if";
        const string scrip="sf";
        uint8_t subsType4 = 4;
        uint8_t snapShotType9 = 9;
        int channel = 1;
        SubscribeRequest s;
//        string bufsub = s.prepareSubsUnSubsRequest(scrip_index, subsType4, index, channel); // index=if and  line 405
        if(niftyUnreal) {
            cout<<"setting unreal"<<endl;
            // TCS  INFOSYS TataMOTORS  NTPC  
            string scrip_index = "nse_cm|11536&nse_cm|1594&nse_cm|3456&nse_cm|11630";
            scrip_index += "&nse_cm|16675"; // Bajaj finserver
            scrip_index += "&nse_cm|526"; // Bharat petroleum 
            prepareSubString p ;
            scrip_index = p.prepareSubscription();
            auto ft  = std::async(std::launch::async, publisher::snapShots, scrip_index, snapShotType9, scrip, channel, con);
            string bufsub = s.prepareSubsUnSubsRequest(scrip_index, snapShotType9, scrip, channel); // scrip and line 406
            c.send(con->get_handle(), bufsub, websocketpp::frame::opcode::binary, ec);
        }
        else {
            string scrip_index = "nse_fo|35048";
            string bufsub = s.prepareSubsUnSubsRequest(scrip_index, subsType4, scrip, channel); // scrip and line 406
            c.send(con->get_handle(), bufsub, websocketpp::frame::opcode::binary, ec);

            scrip_index = "nse_cm|Nifty 50";
            bufsub = s.prepareSubsUnSubsRequest(scrip_index, subsType4, index, channel); // scrip and line 406
            c.send(con->get_handle(), bufsub, websocketpp::frame::opcode::binary, ec);
            if (ec) {
             std::cout << "> Error sending message: " << ec.message() << std::endl;
             return 0;
            }   
        } 
//        print_bytes(cout,"SubRequest" ,bufsub.c_str(),bufsub.length()) ;
//        cout<<"SubRequest =["<<std::hex<<bufsub<<"]"<<endl;
        //subscriber ends
        t1.join();
//        c.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }

}
