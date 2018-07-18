#include "WavData.hpp"
#include <iostream>

#define WAVE_FORMAT_PCM 0x0001

WavData::WavData(void) : riffSize_(4) {
  // RIFF
  Chunk riff("RIFF");
  Chunk::Field field;
  field.name = "WAVEID";
  field.nBytes = 4;
  field.val = "WAVE";
  riff.addField(field);
  addChunk(riff);

  // fmt
  std::vector<std::string> fmtNames = {
      "FormatTag",   "Channels",      "SamplesPerSec", "AvgBytesPerSec",
      "BlockAlign",  "BitsPerSample", "Size",          "ValidBitsPerSample",
      "ChannelMask", "SubFormat[16]"};

  std::vector<unsigned int> fmtSizes = {2, 2, 4, 4, 2, 2, 2, 2, 4, 16};

  std::vector<F_TYPE> fmtTypes = {F_INT, F_INT, F_INT, F_INT, F_INT,
                                  F_INT, F_INT, F_INT, F_INT, F_BYTE_ARRAY};

  // Only fmt has variable amounts of fields (not to be confused with variable
  // size)
  Chunk fmtChunk("fmt ");
  for (int i = 0; i < fmtNames.size(); i++) {
    Chunk::Field f;
    f.name = fmtNames[i];
    f.nBytes = fmtSizes[i];
    f.type = fmtTypes[i];
    fmtChunk.addField(f);
  }
  addChunk(fmtChunk);

  // bext
  std::vector<std::string> bextNames = {"Description",
                                        "Originator",
                                        "OriginatorReference",
                                        "OriginationDate",
                                        "OriginationTime",
                                        "TimeReferenceLow",
                                        "TimeReferenceHigh",
                                        "Version",
                                        "UMID[64]",
                                        "LoudnessValue",
                                        "LoudnessRange",
                                        "MaxTruePeakLevel",
                                        "MaxMomentaryLoudness",
                                        "MaxShortTermLoudness",
                                        "Reserved",
                                        "CodingHistory"};
  std::vector<unsigned int> bextSizes = {256, 32, 32, 10, 8, 4, 4,   2,
                                         64,  2,  2,  2,  2, 2, 180, 0};
  std::vector<F_TYPE> bextTypes = {F_STRING,     F_STRING, F_STRING, F_STRING,
                                   F_STRING,     F_UINT,   F_UINT,   F_INT,
                                   F_BYTE_ARRAY, F_SHORT,  F_SHORT,  F_SHORT,
                                   F_SHORT,      F_SHORT,  F_NDEF,   F_NDEF};

  Chunk bextChunk("bext");
  for (int i = 0; i < bextNames.size(); i++) {
    Chunk::Field f;
    f.name = bextNames[i];
    f.nBytes = bextSizes[i];
    f.type = bextTypes[i];
    bextChunk.addField(f);
  }
  bextChunk.makeVariable();
  addChunk(bextChunk);

  // fact
  Chunk factChunk("fact");
  Chunk::Field sampleL;
  sampleL.name = "SampleLength";
  sampleL.nBytes = 4;
  sampleL.type = F_INT;
  factChunk.addField(sampleL);
  addChunk(factChunk);

  // cart
  Chunk cartChunk("cart");
  std::vector<std::string> cartNames = {"Version",
                                        "Title",
                                        "Artist",
                                        "CutID",
                                        "ClientID",
                                        "Category",
                                        "Classification",
                                        "OutCue",
                                        "StartDate",
                                        "StartTime",
                                        "EndDate",
                                        "EndTime",
                                        "ProducerAppID",
                                        "ProducerAppVersion",
                                        "UserDef",
                                        "LevelReference",
                                        "PostTimer",
                                        "Reserved",
                                        "URL",
                                        "TagText"};
  std::vector<unsigned int> cartSizes = {4,  64, 64, 64,  64,   64, 64,
                                         64, 10, 8,  10,  8,    64, 64,
                                         64, 4,  8,  276, 1024, 0};

  for (int i = 0; i < cartNames.size(); i++) {
    Chunk::Field f;
    f.name = cartNames[i];
    f.nBytes = cartSizes[i];
    f.type = F_STRING;
    cartChunk.addField(f);
  }
  cartChunk.makeVariable();
  addChunk(cartChunk);

  Chunk dataChunk("data");
  Chunk::Field dataField;
  dataField.name = "data";
  dataField.nBytes = 0;
  dataField.type = F_NDEF;
  dataChunk.addField(dataField);
  dataChunk.makeVariable();
  addChunk(dataChunk);
}

WavData::WavData(std::vector<Chunk> chunks) : WavData() {
  for (auto it = chunks.begin(); it != chunks.end(); it++)
    addChunk(*it);
}

WavData::~WavData(void) {}

std::shared_ptr<Chunk> WavData::getChunk(const std::string &name) {
  return chunks_[name];
}

std::map<std::string, std::shared_ptr<Chunk>>
WavData::getAllChunks(void) const {
  return chunks_;
}

void WavData::addChunk(const Chunk &chunk) {
  if (exists(chunk.getChunkName()))
    return;
  std::shared_ptr<Chunk> ch = std::make_shared<Chunk>(chunk);
  chunks_[chunk.getChunkName()] = ch;
}

void WavData::readChunk(std::string &readData) {
  // If the chunk is not defined and is not considered junk, it is saved as
  // undefined
  if (!exists(readData)) {
    if (readData != "JUNK") {
      saveUndefinedChunk(readData);
    } else {
      readBytes(CK_SIZE_BYTES, readData);
      r_.ignore(toType<unsigned int>(readData));
    }
    return;
  }
  auto ck = chunks_[readData];
  readBytes(CK_SIZE_BYTES, readData);
  unsigned int ckSize = toType<unsigned int>(readData);

  // If the expected chunk size is bigger than what is read, it is ignored
  // Only fmt is processed because it guarantees backward compatibility
  // Alternatively, an error could be raised
  if (ck->getSize() > ckSize && ck->getChunkName() != "fmt ") {
    r_.ignore(ckSize);
    return;
  }

  // Saving the amount of expected bytes into the proper fields
  auto fields = ck->getAllFields();
  unsigned int i = 0;     // field index
  unsigned int count = 0; // amount of bytes read
  while (count < ckSize) {
    unsigned int size = fields[i]->nBytes;
    if (count + size > ckSize)
      break;
    // Variable chunks get the rest in their last field (like TagText)
    if (size == 0) {
      if (ck->isVariable()) {
        fields[i]->nBytes = ckSize - count;
        readBytes(fields[i]->nBytes, fields[i]->val);
        count += fields[i++]->nBytes;
        break; // There should be no field in that chunk after a variable one
      } else {
        throw std::string("A defined field is variable but the chunk is not\n");
      }
    }
    count += size;
    readBytes(size, fields[i++]->val);
  }
}

void WavData::read(const std::string &fn) {
  resetData();
  r_.open(fn, std::ifstream::binary);
  assert(r_.is_open());

  // RIFF check
  auto riff = chunks_["RIFF"];
  std::string data;
  readBytes(ID_SIZE, data);
  if (data.compare(0, ID_SIZE, "RIFF"))
    throw std::string("Not a RIFF compliant file format\n");
  readBytes(CK_SIZE_BYTES, data);
  unsigned int ckSize = toType<unsigned int>(data);
  readBytes(4, data);
  if (data.compare(0, ID_SIZE, "WAVE"))
    throw std::string("Not an adequate wav file format\n");

  // Reading chunks
  while (!r_.eof()) {
    readBytes(ID_SIZE, data);
    readChunk(data);
  }
  r_.close();
}

void WavData::write(const std::string &fn, bool writeUndefinedChunks) {
  assert(exists("RIFF") && exists("fmt ") && exists("data") && exists("fact"));
  w_.open(fn, std::ios::binary);
  assert(w_.is_open());
  // For every chunk, update the RIFF size
  for (auto it = chunks_.begin(); it != chunks_.end(); it++) {
    auto fields = it->second->getAllFields();
    // For every field of every chunk, update the chunk size first
    for (auto f = fields.begin(); f != fields.end(); f++) {
      // If the user has updated a variable field but not its size
      if ((*f)->nBytes == 0 && (*f)->val.size() != 0)
        (*f)->nBytes = (*f)->val.size();
      it->second->addToActualSize((*f)->nBytes);
    }
    // Then update the RIFF size
    riffSize_ += it->second->getActualSize() + 4 + 4;
  }
  // RIFF and fmt first
  writeChunk("RIFF");
  writeChunk("fmt ");
  // non-PCM data must have a fact chunk
  if (toType<int>(chunks_["fmt "]->getField("FormatTag")->val) !=
      WAVE_FORMAT_PCM)
    writeChunk("fact");
  for (auto it = chunks_.begin(); it != chunks_.end(); it++) {
    if (it->first == "RIFF" || it->first == "fmt " || it->first == "fact")
      continue;
    else if (writeUndefinedChunks || !chunks_[it->first]->isUndefined())
      writeChunk(it->first);
  }
  w_.close();
}

void WavData::writeBytes(const std::string &data) {
  w_.write((data.c_str()), data.size());
}

void WavData::writeChunk(const std::string &name) {
  auto chunk = chunks_[name];
  writeBytes(chunk->getChunkName());
  writeBytes(toByte<unsigned int>(chunk->getActualSize()));
  auto fields = chunk->getAllFields();
  for (auto it = fields.begin(); it != fields.end(); it++) {
    // If the string's size is not the same size, empty values are appended
    int diff = (*it)->val.size() - (*it)->nBytes;
    if (diff < 0) {
      for (int i = 0; i < -diff; i++)
        (*it)->val.push_back('\0');
    }
    // If the string's size is greater, it does not respect the defined size
    else if (diff > 0)
      throw std::string(
          "Size of the field\'s value is greater than the defined size\n");
    writeBytes((*it)->val);
  }
}

void WavData::readBytes(const unsigned int nBytes, std::string &data) {
  char *c = new char[nBytes];
  r_.read(c, nBytes);
  data.assign(c, nBytes);
  delete c;
}

void WavData::saveUndefinedChunk(const std::string &chunkId) {
  std::string data;
  Chunk c(chunkId);
  readBytes(CK_SIZE_BYTES, data);
  Chunk::Field f;
  f.nBytes = toType<unsigned int>(data);
  if (f.nBytes == 0)
    return; // Useless if empty
  f.name = "ndef";
  f.type = F_NDEF;
  readBytes(f.nBytes, f.val);
  c.addField(f);
  c.makeUndefined();
  addChunk(c);
}

void WavData::removeChunk(const std::string &name) {
  // Must have chunks that should not be removed
  assert(name != "RIFF" && name != "fmt" && name != "fact" && name != "data");
  assert(exists(name));
  chunks_.erase(name);
}

bool WavData::exists(const std::string &name) {
  return chunks_.find(name) != chunks_.end();
}

void WavData::resetData(void) {
  for (auto it = chunks_.begin(); it != chunks_.end(); it++) {
    if (it->first == "RIFF")
      continue;
    it->second->resetChunk();
  }
}
