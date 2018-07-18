#include "WavData.hpp"
#include <iostream>

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Usage : wavFileTest [INPUT] [OUTPUT]";
    return -1;
  }

  WavData *wav = new WavData();

  // Reading, printing and changing a value
  wav->read(argv[1]);
  auto allChunks = wav->getAllChunks();
  for (auto it = allChunks.begin(); it != allChunks.end(); it++) {
    std::cout << *(it->second);
  }
  auto bextChunk = wav->getChunk("bext");
  short loudness = 25;
  bextChunk->getField("LoudnessValue")->val = WavData::toByte<short>(loudness);

  // Adding a custom field to a custom Chunk
  Chunk myChunk("abcd");
  Chunk::Field myField;
  myField.name = "myField";
  // If nBytes < val.size() and the chunk is not a variabled sized chunk, an
  // error is raised If nBytes > val.size(), val string is appended with '\0'
  // until the size is met
  myField.nBytes = 4;
  myField.type = F_INT;
  myField.val = WavData::toByte<int>(1234);
  myChunk.addField(myField);
  myChunk.makeVariable();
  wav->addChunk(myChunk);

  // Changing an existing variable size field
  wav->getChunk("cart")->getField("TagText")->val = std::string("blahblah");

  // Writing all defined chunks and dropping the undefined ones
  wav->write(argv[2], DROP_UNDEFINED_CHUNKS);

  // // Reading what we changed so far
  wav->read(argv[2]);

  delete wav;
  return 0;
}
