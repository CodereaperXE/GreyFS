#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;
using json = nlohmann::json;


int StoreShards(char shardBuffer[],size_t shardSize, std::string &shardPath){
    std::ofstream outputShard(shardPath,std::ios::binary);
    outputShard.write(shardBuffer,shardSize);
    return 1;
}

// int CreateShards(std::string &filePath){
//     size_t shardBufferSize =1024;
//     char shardBuffer[shardBufferSize];
   

//     std::ifstream file(filePath,std::ios::binary);

    
//     int count =0;
//     while(file.read(shardBuffer,shardBufferSize) || file.gcount() > 0) {
//         std::string path = "shard" + std::to_string(count) + ".dat";
//         StoreShards(shardBuffer,file.gcount(),path);
//         count++;
//     }
//     return 1;
// }


int GetFileSize(fs::path filePath){
    int size = fs::file_size(filePath);
    return size;
}

json ReadJson(fs::path filePath){
    std::ifstream file(filePath, std::ios::binary);
    json object;
    file >> object;
    file.close();
    return object;
}

int WriteJson(fs::path filePath, json metadata) {
    std::ofstream file(filePath, std::ios::binary);
    file << metadata.dump();
    file.close();
    return 1;
}

json CreateShards(fs::path &filePath, fs::path &dstPath){
    /*
    metadata {
                filename:something,
                size:something,
                time:something
                hash: something,
                shard_count: something
                shards:{
                    shard_id: something,
                    shard_name: something,(filename+shard_id)
                    size: something,
                    hash: something,
                    location: something;
                }
            }
    */

    json metadata;   
    metadata["filename"] = filePath;
    metadata["size"] = GetFileSize(filePath);
    metadata["time"] = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    metadata["hash"] = "null";
    metadata["shards"]={};
    

    size_t shardBufferSize =8192;
    char shardBuffer[shardBufferSize];
    std::ifstream file(filePath,std::ios::binary);
    
    int count =0;
    while(file.read(shardBuffer,shardBufferSize) || file.gcount() > 0) {
        std::string shardPath = std::string(dstPath / (std::string(filePath.filename())+"_shard" + std::to_string(count) + ".dat")); //srcpath/shard{number}.dat
        std::string shardName = fs::path(shardPath).filename();
        StoreShards(shardBuffer,file.gcount(),shardPath);
        json shard = {
            {"shard_id", count},
            {"shard_name", shardName},
            {"size", GetFileSize(fs::path(shardPath))},
            {"hash", "??"},
            {"location", "??"}
        };
        count++;

        metadata["shards"] += shard;
        metadata["shard_count"] = count;
    }

    // std::cout<<metadata.dump();

   return metadata;
   
}


int MergeShards(json metadata,fs::path &path, fs::path &shardPath){
    fs::path fileName = metadata["filename"];
    size_t size = metadata["size"];
    auto time = metadata["time"];
    int shardCount = metadata["shard_count"];
    //hash and location implementation
    //open file

    std::ofstream file(path / fileName,std::ios::binary | std::ios::app);

   
    //find shard
    auto FindShard = [&metadata](int shardId) -> json {
        for(json &shardData:metadata["shards"]){
            if(shardData["shard_id"]==shardId) return shardData;
        }
        return json();
    };

    //merge shards
    
    for(int i=0;i<shardCount;i++){
        json shardMetadata = FindShard(i);
        std::cout<<shardMetadata.dump();
        std::string sharddst = std::string(shardPath / std::string(shardMetadata["shard_name"]));
        std::ifstream shard(sharddst,std::ios::binary);
        
        size_t bufferSize = 8192;
        char buffer[bufferSize];
        while(shard.read(buffer, bufferSize) || shard.gcount() > 0) {
            file.write(buffer,shard.gcount());
        }
        shard.close();
    }
    file.close();
return 1;
}



int main(){
    fs::path filePath = "test.png";
    fs::path shardPath = "shardFolder";
    json m = CreateShards(filePath,shardPath);
    fs::path meta("metadata.json");

    WriteJson(meta, m);

    std::remove("test.png");
    std::this_thread::sleep_for(std::chrono::seconds(15));

    fs::path temp = fs::path(".");
    MergeShards(m,temp,shardPath);
    return 0;
}
