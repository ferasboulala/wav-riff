#ifndef WAVDATA_HPP_
#define WAVDATA_HPP_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <assert.h>
#include "Chunk.hpp"

#define DROP_UNDEFINED_TRUNKS false
#define PRINT_CHUNKS true

class WavData {
public:
    /**
     * @brief Default constructor. RIFF, fmt+fact, bext and cart chunks
     * defined by default.
    */
    WavData(void);

    /**
     * @brief Chunk vector constructor.
     * @param std::vector<Chunk>
    */
    WavData(std::vector<Chunk> chunks);

    /**
     * @brief Destructor
    */
    ~WavData(void);

    /**
     * @brief Get a pointer to a Chunk object
     * @param std::string name of the chunk
     * @return std::shared_ptr<Chunk>
    */
    std::shared_ptr<Chunk> getChunk(const std::string& name);

    /**
     * @brief Get a hash table of all the Chunk objects
     * @return std::map<std::string,std::shared_ptr<Chunk>>
    */
    std::map<std::string, std::shared_ptr<Chunk>> getAllChunks(void) const;

    /**
     * @brief Add a chunk to the defined chunks
     * @param Chunk
    */
    void addChunk(const Chunk& chunk);

    /**
     * @brief Remove a chunk from the defined chunks
     * @param std::string name of the chunk
    */
    void removeChunk(const std::string& name);

    /**
     * @brief Reads all defined chunks from a binary file and prints them out
     * Chunks that are read but not defined are store as undefined chunks.
     * @param std::string filename
     * @param bool print or not
    */
    void read(const std::string& fn, bool print = false);
    
    /**
     * @brief Writes all defined and undefined chunks to a binary file
     * @param std::string filename
     * @param bool to drop or not drop undefined trunks when writing
    */
    void write(const std::string& fn, bool writeUndefinedChunks = true);

    /**
     * @brief Converts a numerical type to a string (byte array, up to 8 bytes long)
     * @param T numerical value
     * @return std::string 
    */
    template<typename T>
    static std::string toByte(const T v){
        assert(sizeof(T) <= sizeof(long int));
        union {T in; long int out;} data;
        data.in = v;
        std::string s;
        for (unsigned int i = 0; i < sizeof(T); i++){
            unsigned char c = data.out >> (8*i);
            s.push_back(c);
        }
        return s;
    }

    /**
     * @brief Converts a string to a numerical type (up to 8 bytes long)
     * @param std::string
     * @return T numerical value
    */
    template<typename T>
    static T toType(const std::string& s){
        assert(sizeof(T) <= sizeof(long int));
        assert(s.size() <= sizeof(T));
        union {long int in; T out;} data;
        data.in = 0;
        for (unsigned int i = 0; i < s.size(); i++){
            unsigned char c = s[i];
            data.in |= c << 8*i;
        }
        return data.out;
    }

    /**
     * @brief Checks if a chunk is already defined to avoid seg faults
     * @param std::string name of the chunk
     * @return bool
    */
    bool exists(const std::string& name);

    /**
     * @brief Resets values of all fields of all Chunks except RIFF.
     * Call this after read() if you want to read() again because variable fields
     * must be set back to 0 in side (@ref Chunk.hpp)
    */
    void resetData(void);

private:
    void readBytes(const unsigned int nBytes, std::string& data);
    void writeBytes(const std::string& data);
    void writeChunk(const std::string& name);
    void saveUndefinedChunk(const std::string& chunkId);
    
    std::map<std::string, std::shared_ptr<Chunk>> chunks_;

    int riffSize_;
    std::string data_;

    std::ifstream r_;
    std::ofstream w_;
};

#endif //WAVDATA_HPP_