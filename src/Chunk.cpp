#include "Chunk.hpp"
#include "WavData.hpp"
#include <assert.h>

#define MAX_CHUNK_SIZE 0xffffffff

Chunk::Chunk(const std::string &name)
    : size_(0), actualSize_(0), undefined_(false), variableSize_(false) {
  assert(name.size() == 4);
  name_ = name;
}

Chunk::Chunk(const std::string &name, const std::vector<Field> &fields)
    : Chunk(name) {
  for (auto it = fields.begin(); it != fields.end(); it++) {
    addField(*it);
  }
}

Chunk::~Chunk(void) {}

void Chunk::addField(const struct Field f) {
  if (isVariable())
    throw std::string("Last field is variable. Cannot add anymore fields.");
  std::shared_ptr<Field> f_p = std::make_shared<Field>(f);
  fields_.push_back(f_p);
  fieldMap_[f.name] = f_p;
  long overflow = size_;
  assert(overflow + f.nBytes < MAX_CHUNK_SIZE - 4 - 4);
  size_ += (f.nBytes);
  if (f.nBytes == 0)
    makeVariable();
}

std::shared_ptr<Chunk::Field> Chunk::getField(const std::string &fieldName) {
  return fieldMap_[fieldName];
}

std::vector<std::shared_ptr<Chunk::Field>> Chunk::getAllFields(void) const {
  return fields_;
}

std::string Chunk::getChunkName(void) const { return name_; }

bool Chunk::checkSize(const unsigned int size) const { return (size == size_); }

void Chunk::print(void) const { std::cout << *this; }

std::ostream &operator<<(std::ostream &os, const Chunk &ck) {
  if (ck.getChunkName() == "data")
    return os;
  os << "---------- " << ck.getChunkName() << " chunk ----------\n";
  auto fields = ck.getAllFields();
  for (auto it = fields.begin(); it != fields.end(); it++) {
    os << (*it)->name << " : ";
    switch ((*it)->type) {
    case F_BYTE_ARRAY: {
      for (int i = 0; i < (*it)->val.size(); i++) {
        os << WavData::toType<unsigned int>(std::string(&((*it)->val[i]), 1))
           << '-';
      }
      break;
    }
    case F_INT: {
      os << WavData::toType<int>((*it)->val);
      break;
    }
    case F_UINT: {
      os << WavData::toType<unsigned int>((*it)->val);
      break;
    }
    case F_FLOAT: {
      os << WavData::toType<float>((*it)->val);
      break;
    }
    case F_SHORT: {
      os << WavData::toType<short>((*it)->val);
      break;
    }
    default: { os << (*it)->val; }
    }
    os << '\n';
  }
  return os;
}

void Chunk::resetChunk(void) {
  actualSize_ = 0;
  for (auto it = fields_.begin(); it != fields_.end(); it++) {
    (*it)->val = std::string("");
  }
  if (variableSize_) {
    fields_[fields_.size() - 1]->nBytes = 0;
  }
}

unsigned int Chunk::getSize(void) const { return size_; }

unsigned int Chunk::getActualSize(void) const { return actualSize_; }

bool Chunk::isUndefined(void) const { return undefined_; }

void Chunk::makeUndefined(void) { undefined_ = true; }

bool Chunk::isVariable(void) const { return variableSize_; }

void Chunk::makeVariable(void) { variableSize_ = true; }

void Chunk::addToActualSize(const unsigned int size) { actualSize_ += size; }
