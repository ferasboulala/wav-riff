#include "WavData.hpp"
#include <iostream>

int main(int argc, char** argv){
    if (argc < 2){
        std::cerr << "Usage : wavFileTest [INPUT] [OUTPUT]";
        return -1;
    }

    WavData* wav = new WavData();

    // Reading and chaning a value
    wav->read(argv[1]);
    auto bextChunk = wav->getChunk("bext");
    short loudness = 25;
    bextChunk->getField("LoudnessValue")->val = WavData::toByte<short>(loudness);

    // Adding a custom field to a custom Chunk
    Chunk myChunk("abcd");
    Chunk::Field myField;
    myField.name = "myField";
    myField.nBytes = 4; // nBytes must be >= val.size()
    myField.type = F_INT;
    myField.val = WavData::toByte<int>(1234);
    myChunk.addField(myField);
    myChunk.makeVariable();
    wav->addChunk(myChunk);

    // Changing an existing variable field
    wav->getChunk("cart")->getField("TagText")->val = std::string("blahblah");

    // Writing all defined chunks and dropping the undefined ones
    wav->write(argv[2], DROP_UNDEFINED_TRUNKS);

    // Reseting the current wav object and reading what we changed so far
    wav->resetData();
    wav->read(argv[2], PRINT_CHUNKS);

    delete wav;
    return 0;
}
