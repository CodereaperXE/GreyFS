#include "network.hpp"
#include "support.hpp"

int id=0;

int ports[]={8080,8081};

json grWrite={
    {"write_gr", 
        {
            {"id",-1},
            {"request_type","write_gr"},
            {"time",-1},
            {"source_address",-1},
            {"destination_address",-1}
        }
    },
    {"write_gr_iso_ack",
        {
            {"id",-1},
            {"request_type","write_gr_iso_ack"},
            {"time",-1},
            {"source_address",-1},
            {"destination_address",-1}
        }
    },
    {"write_gr_update", 
        {
            {"id",-1},
            {"request_type","write_gr_update"},
            {"data",""},
            {"time",-1},
            {"source_address",-1},
            {"destination_address",-1}
        }
    },
    {"write_gr_update_ack",
        {
            {"id",-1},
            {"request_type","write_gr_update_ack"},
            {"time",-1},
            {"source_address",-1},
            {"destination_address",-1}
        }
    }
};

//test bench reader and writer

class Reader{
    public:
    int port;
    std::string gr;
    std::unique_ptr<Network::Server> svr;
    Reader(){
        port = 8080;
        svr = std::make_unique<Network::Server>(port);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout<<"handle write"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        handleWrite();
    }

    void handleWrite(){
        
            int dstport;
            //incomming write request
            json data = svr->AcceptReq();
            std::cout<<"accepted "<<data.dump()<<std::endl;
            if(data["request_type"]=="write_gr"){
                svr->SendResponse(json{{"success",1}});
                dstport = data["source_address"];
                //ack isolation from gr
                json res = GrWriteAck(port,dstport);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                Network::Client c("127.0.0.1",dstport);
                c.SendData(res);
                std::cout<<"reader: incomming read request handled\n";
            }

            //incomming update
            data = svr->AcceptReq();
            if(data["request_type"]=="write_gr_update"){
                svr->SendResponse(json{{"success",1}});
                dstport = data["source_address"];
                gr = data["data"];
                //send update complete ack
                json res = GrUpdateAck(port,dstport);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                Network::Client c("127.0.0.1",dstport);
                c.SendData(res);
                std::cout<<"reader: incomming update request handled\n";
                std::cout<<"reader: gm data is " << gr <<"\n";
            }

        
    }
    json GrWrite(){
        json request = grWrite;
        request["write_gr"]["id"]=id++;
        request["write_gr"]["source_address"]=8081;
        request["write_gr"]["destination_address"]=8080;
        return request["write_gr"];
    }
    
    json GrWriteAck(int sport,int dport){
        json request = grWrite;
        request["write_gr_iso_ack"]["id"]=id++;
        request["write_gr_iso_ack"]["source_address"]=sport;
        request["write_gr_iso_ack"]["destination_address"]=dport;
        return request["write_gr_iso_ack"];
    }
    
    json GrUpdate(int sport, int dport,std::string data){
        json request = grWrite;
        request["write_gr_update"]["id"]=id++;
        request["write_gr_update"]["source_address"]=sport;
        request["write_gr_update"]["destination_address"]=dport;
        request["write_gr_update"]["data"]=data;
        return request["write_gr_update"];
    }

    json GrUpdateAck(int sport, int dport){
        json request = grWrite;
        request["write_gr_update_ack"]["id"]=id++;
        request["write_gr_update_ack"]["source_address"]=sport;
        request["write_gr_update_ack"]["destination_address"]=dport;
        return request["write_gr_update_ack"];
    }

};

class Writer{
    public:
    int port;
    std::string gr;
    std::unique_ptr<Network::Server> svr;
    Writer(){
        gr = "hello world";
        port = 8081;
        svr = std::make_unique<Network::Server>(port);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        handleRead();
    }

    void handleRead(){
            //send gr write request
            json data = GrWrite();
            Network::Client c("127.0.0.1",8080);
            std::cout<<data.dump()<<std::endl;
            std::cout<<c.SendData(data).dump();
            std::cout<<"writer: write request sent\n";
            
            int dstport;
            //incomming isolation ack
            data = svr->AcceptReq();
            if(data["request_type"]=="write_gr_iso_ack"){
                svr->SendResponse(json{{"success",1}});
                dstport = data["source_address"];
                //send data
                json res = GrUpdate(port,dstport,gr);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                Network::Client c("127.0.0.1",dstport);
                std::cout<<c.SendData(res).dump()<<std::endl;
                std::cout<<"writer: incomming isolation ack recv\n";
            }

            //incomming update ack
            data = svr->AcceptReq();
            if(data["request_type"]=="write_gr_update_ack"){
                svr->SendResponse(json{{"success",1}});
                dstport = data["source_address"];
                std::cout<<"writer: "<<"update confirmed\n"; 
            }

        
    }

    json GrWrite(){
        json request = grWrite;
        request["write_gr"]["id"]=id++;
        request["write_gr"]["source_address"]=8081;
        request["write_gr"]["destination_address"]=8080;
        return request["write_gr"];
    }
    
    json GrWriteAck(int sport,int dport){
        json request = grWrite;
        request["write_gr_iso_ack"]["id"]=id++;
        request["write_gr_iso_ack"]["source_address"]=sport;
        request["write_gr_iso_ack"]["destination_address"]=dport;
        return request["write_gr_iso_ack"];
    }
    
    json GrUpdate(int sport, int dport,std::string data){
        json request = grWrite;
        request["write_gr_update"]["id"]=id++;
        request["write_gr_update"]["source_address"]=sport;
        request["write_gr_update"]["destination_address"]=dport;
        request["write_gr_update"]["data"]=data;

        return request["write_gr_update"];
    }

    json GrUpdateAck(int sport, int dport){
        json request = grWrite;
        request["write_gr_update_ack"]["id"]=id++;
        request["write_gr_update_ack"]["source_address"]=sport;
        request["write_gr_update_ack"]["destination_address"]=dport;
        return request["write_gr_update_ack"];
    }

};

void runreader() { Reader r; while(1){std::this_thread::sleep_for(std::chrono::milliseconds(100));}};
// void runwriter() { Writer w; while(1){std::this_thread::sleep_for(std::chrono::milliseconds(100));}};


int main(){
    std::thread t1(runreader);
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    // std::thread t2(runwriter);
    Writer w;
    while(1){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


}





// int var =0;
// void run(){
//     Network::Server server(8081);
//     while(!var){
//         server.AcceptReq();
//         server.SendResponse(json{{"hello from server",1}});
//     }
// }

// int main(){
//     std::thread t(run);
//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     var=1;
//     Network::Client client("127.0.0.1",8081);
//     std::cout<<client.SendData(json{{"hello from client",1}}).dump();
//     t.join();
    
// }
//structure of request
/*  5/1/2025 (global resource is a object that is stored locally in every nodes but is treated as a singular shared object among all nodes)

    source node sends write request to all nodes
    all other nodes complete reading from global resource while source node waits (gr is isolated in local node copies)
    other nodes send isolation ack from (global resource) to source node (source nodes waits)
    once ack is recv source node writes to (global resource in source node) takes place by source node
    source node broadcasts update (other nodes update their gr)
    and source node waits for other nodes to ack update (source node waits for ack from other nodes)
*/
/* write gr (global resource) request (writer)
    {
    id:-1,
    request_type:write_gr,
    time:time
    source_address:address,
    destination_address:address
    }

    isolation response ack  (reader)

    {
    id:-1,
    request_type:write_gr_iso_ack,
    time:time,
    source_address:address,
    destination_address:address
    }

    source update gr post (writer)

    {
    id:-1
    request_type:write_gr_update,
    time:time,
    data:data,
    source_address:address,
    destination_address:address
    }

    update ack (reader)

    {
    id:-1,
    request_type:write_gr_update_ack,
    time:time,
    source_address:address,
    destination_address:address
    }
    

*/

// json grWrite={
//     {"write_gr", 
//         {
//             {"id",-1},
//             {"request_type","write_gr"},
//             {"time",-1},
//             {"source_address",-1},
//             {"destination_address",-1}
//         }
//     },
//     {"write_gr_iso_ack",
//         {
//             {"id",-1},
//             {"request_type","write_gr_iso_ack"},
//             {"time",-1},
//             {"source_address",-1},
//             {"destination_address",-1}
//         }
//     },
//     {"write_gr_update", 
//         {
//             {"id",-1},
//             {"request_type","write_gr_update"},
//             {"data",""},
//             {"time",-1},
//             {"source_address",-1},
//             {"destination_address",-1}
//         }
//     },
//     {"write_gr_update_ack",
//         {
//             {"id",-1},
//             {"request_type","write_gr_update_ack"},
//             {"time",-1},
//             {"source_address",-1},
//             {"destination_address",-1}
//         }
//     }
// };
