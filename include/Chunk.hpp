#ifndef CHUNK_HPP_
#define CHUNK_HPP_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define ID_SIZE 4
#define CK_SIZE_BYTES 4

// Field types
typedef int F_TYPE;
enum FieldType {
  F_BYTE_ARRAY,
  F_UINT,
  F_INT,
  F_SHORT,
  F_FLOAT,
  F_STRING,
  F_DATE = F_STRING,
  F_NDEF
};

class Chunk {
  friend class WavData;

public:
  /**
   * @brief Field of a chunk.
   * @member F_TYPE type of the field for printing purposes
   * @member unsigned int number of bytes to read and write
   * @member std::string name to access that field at runtime
   * @member std::string value in a byte array
   */
  struct Field {
    // Defaults
    Field() : type(F_NDEF), nBytes(0), name("\0") {}

    F_TYPE type;
    unsigned int nBytes;
    std::string name;
    std::string val;
  };

  /**
   * @brief Name constructor
   * @param std::string
   */
  Chunk(const std::string &name);

  /**
   * @breif Name and field vector constructor (order matters)
   * @param std::string
   * @param std::vector<Chunk::Field> all fields of that chunk
   */
  Chunk(const std::string &name, const std::vector<Field> &fields);

  /**
   * @brief Destructor
   */
  ~Chunk(void);

  /**
   * @brief Push_back a field into the defined fields (order matters)
   * @param Chunk::Field
   */
  void addField(const Field f);

  /**
   * @brief Get a pointer to a field by its name
   * @param std::string
   * @return std::shared_ptr<Chunk::Field>
   */
  std::shared_ptr<Field> getField(const std::string &fieldName);

  /**
   * @brief Get a vector of all fields (order matters)
   * @return std::vector<std::shared_ptr<Chunk::Field>>
   */
  std::vector<std::shared_ptr<Field>> getAllFields(void) const;

  /**
   * @brief Get the name of the current chunk
   * @return std::string
   */
  std::string getChunkName(void) const;

  /**
   * @brief Prints the current chunk to std::cout output stream
   */
  void print(void) const;

  /**
   * @brief Output stream << overloading
   * @usage os << ChunkObject;
   */
  friend std::ostream &operator<<(std::ostream &os, const Chunk &ck);

  /**
   * @brief Gets the expected size of a chunk as it is defined
   * It is the sum of the sizes of all fields
   * @return unsigned int
   */
  unsigned int getSize(void) const;

  /**
   * @brief Gets the actual size of the chunk. Useful only after
   * WavData::write() because it is set to 0 when initiliazed
   * @return unsigned int
   */
  unsigned int getActualSize(void) const;

  /**
   * @brief Checks if the current Chunk is undefined. To define a
   * new chunk, add it and read the file again.
   * @return bool
   */
  bool isUndefined(void) const;

  /**
   * @brief If a Chunk has a variable size field, call this function.
   * The variable field is always the last field in the vector so make
   * sure it is.
   */
  void makeVariable(void);

protected:
  bool checkSize(const unsigned int size) const;
  bool isVariable(void) const;
  void makeUndefined(void);
  void addToActualSize(const unsigned int size);

private:
  // Expected chunk size and actual size with variable fields if they exist
  unsigned int size_, actualSize_;
  std::string name_;
  std::vector<std::shared_ptr<Field>> fields_;
  std::map<std::string, std::shared_ptr<Field>> fieldMap_;
  bool undefined_, variableSize_;
};

#endif // CHUNK_HPP_