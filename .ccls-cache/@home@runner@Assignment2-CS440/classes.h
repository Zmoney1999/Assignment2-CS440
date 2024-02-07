/******
Class: CS 440
Assignment: Assignment 2
Names: Jake Gehrke & Zach Chand
Onid: gehrkej@oregonstate.edu & chandz@oregonstate.edu
******/

#include <bitset>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

class Record {
public:
  int id, manager_id;
  std::string bio, name;

  Record(vector<std::string> fields) {
    id = stoi(fields[0]);
    name = fields[1];
    bio = fields[2];
    manager_id = stoi(fields[3]);
  }

  void print() {
    cout << "\tID: " << id << "\n";
    cout << "\tNAME: " << name << "\n";
    cout << "\tBIO: " << bio << "\n";
    cout << "\tMANAGER_ID: " << manager_id << "\n";
  }
};
class Block {
public:
  char *buffer;
  int currentSize; // Current size of data in the buffer

  Block(int size) : currentSize(0) { buffer = new char[size]; }

  ~Block() { delete[] buffer; }

  /// Clear block
  void clearBlock() {
    currentSize = 0; // Reset the current size, effectively clearing the block
  }
};

class StorageBufferManager {

public:
  StorageBufferManager() {}

  ~StorageBufferManager() {
    delete currentBlock; // De allocate the memory that was resereved for the
                         // Block
  }

private:
  const int PAGE_SIZE = 4096;

  const int MAX_RECORD_SIZE = 716;

  int numRecords = 0;
  int numBlocks = 0;

  Block *currentBlock = NULL;

  // Create slot directory
  vector<int> slotDirectory;

  /*
  *** SLOT DIRECTORY ***

  [0] - OFFSET WHERE THE FREE DATA STARTS
  [1] - NUMBER OF RECORDS IN PAGE
  [N] - OFFSETS OF THE RECORDS
  */

  const int BLOCK_SIZE = PAGE_SIZE * 3; // initialize the  block size allowed in
                                        // main memory according to the question

  // Declare a block

  // Constructor that will initilize the block with the given size BLOCK_SIZE

  // This function allocates a block on the heap in main memory based on the
  // block size
  int getPosition() {
    ifstream file("EmployeeRelations.dat", ifstream::ate | ifstream::binary);
    if (!file.is_open()) {
      cerr << "Error getting position number" << endl;
      return 0;
    }
    streamsize size = file.tellg();
    file.close();

    int size_int = static_cast<int>(size);

    return size_int;
  }

  Block *allocateBlock() {
    // Allocate a block with the block size
    Block *newBlock = new Block(BLOCK_SIZE);
    return newBlock;
  }

  // Insert new record
  void insertRecord(Record record) {

    stringstream ss;

    // prepare a variable length record using the "Fields Delimited by Special
    // Symbols" approach
    ss << 4 << record.id << "$" << record.name << "$" << record.bio << "$"
       << record.manager_id << "$";
    string serializedRecord = ss.str();

    // Get the size of the serialized record in bytes
    size_t slotSizeInBytes = serializedRecord.size() * sizeof(char);

    // Check to see if the record is larger then the allowed size

    if (serializedRecord.size() > MAX_RECORD_SIZE) {

      cerr << "Record size exceeds maximum allowed size" << endl;
      return;
    }

    /// If the block doesnt exist allocate the block
    if (numRecords == 0) {
      // allocate block
      currentBlock = allocateBlock();

      // Initilize slot directory
      slotDirectory.push_back(0); // free memory or the length of the block
      slotDirectory.push_back(0); // number of records in page
    }

    // Get the size in bytes of the slotDirectory
    size_t slotDirectorySize = slotDirectory.size() * sizeof(int);

    // See if the current buffer can fit the new serializedRecord while
    // accounting for the slotDirectory
    if ((currentBlock->currentSize + slotSizeInBytes) +
            serializedRecord.size() <=
        BLOCK_SIZE) {
      // Get a pointer to the end of the buffer
      char *bufferEnd = currentBlock->buffer + currentBlock->currentSize;

      // Add record to buffer
      memcpy(bufferEnd, serializedRecord.data(), serializedRecord.size());

      // Update the size of the current block
      currentBlock->currentSize += serializedRecord.size();

      // Update the number of records in the block
      numRecords++;

      // Update where free space is in the slot directory
      slotDirectory[0] = currentBlock->currentSize;

      // Update the number of record in the slot directory
      slotDirectory[1] = numRecords;

      // Insert the offset of the current record at the end of the slot
      // directory
      slotDirectory.push_back(serializedRecord.size());

    }

    // If the current block cannot fit the new serializedRecord, Write the block
    // and clear the current block to be refilled
    else {

      // Write the block to the file
      writeFile(currentBlock->buffer, currentBlock->currentSize);

      // Clear the current block

      currentBlock->clearBlock();
      // Reset the number of records in a block
      numRecords = 0;

      // Reset the slot directory
      slotDirectory.clear();

      // Initilize slot directory
      slotDirectory.push_back(0); // free memory
      slotDirectory.push_back(0); // number of records in page

      // Add current record to the new block
      char *bufferEnd = currentBlock->buffer + currentBlock->currentSize;
      memcpy(bufferEnd, serializedRecord.data(), serializedRecord.size());

      // Increase size of the block
      currentBlock->currentSize += serializedRecord.size();

      // Icrease number of records in the block
      numRecords++;

      // Update where free space is in the slot directory
      slotDirectory[0] = currentBlock->currentSize;

      // Update the number of record in the slot directory
      slotDirectory[1] = numRecords;

      // Insert the offset of the current record at the end of the slot
      // directory
      slotDirectory.push_back(serializedRecord.size());
    }
  }

public:
  StorageBufferManager(string NewFileName) {

    // initialize your variables
    numRecords = 0;
    numBlocks = 0;

    // Create your EmployeeRelation file
  }
  // Clears the file so that the program can run multiple times
  void clearEmployeeRelations() {
    ifstream file;
    file.open("EmployeeRelations.dat");
    if (file) {
      remove("EmployeeRelations.dat");
    }
  }

  // Writes a page of records to the data file with a slot directory footer
  void writeFile(const char *buffer, size_t size) {

    FILE *fp =
        fopen("EmployeeRelations.dat", "ab"); // added b to write in binary mode
    if (fp != nullptr) {
      fseek(fp, 0, SEEK_END);                 // Move to the end of the file
      fwrite(buffer, sizeof(char), size, fp); // Write Buffer to file

      // Write the slot directory to the file starting from the end of the
      // vector
      for (int idx = slotDirectory.size() - 1; idx >= 0; idx--) {
        int currentValue = slotDirectory[idx];
        fwrite(&currentValue, sizeof(int), 1, fp);
      }

      fputc('\n', fp); // Write a new line character to the file

      fclose(fp);
    } else {
      cerr << "Failed to open file for writing" << endl;
    }
  }

  // Read csv file (Employee.csv) and add records to the (EmployeeRelation)
  void createFromFile(string csvFName) {

    FILE *file_;
    file_ = fopen(csvFName.c_str(), "r");
    // Catch error if file is not found
    if (file_ == nullptr) {
      cerr << "Failed to open file" << endl;
      return;
    }
    char line[MAX_RECORD_SIZE];
    char *token;
    vector<std::string> fields;

    // parse the line for each field and insertRecord into buffer
    while (fgets(line, MAX_RECORD_SIZE, file_)) {
      fields.clear();
      token = strtok(line, ",");

      // read the line and split it into tokens then store in fields vector
      while (token != NULL) {
        fields.push_back(token);
        token = strtok(NULL, ",");
      }

      // Create record object
      Record newRecord(fields);

      // Insert the record
      insertRecord(newRecord);
    }

    // Write the rest of the buffer to the file
    writeFile(currentBlock->buffer, currentBlock->currentSize);

    // Close the file
    fclose(file_);
  }

  // Given an ID, find the relevant record and print it
  Record findRecordById(int id) {
    // print the id the user is looking for
    cout << "Searching for employee ID: " << id << endl;

    ifstream file("EmployeeRelations.dat", ios::binary);

    if (!file.is_open()) {
      cerr << "Failed to open file" << endl;
    }

    // Define your newline delimiter (replace with your specific value)
    const char newlineDelimiter = '\n';

    // Define a saveLocation & startPage for reading the file
    int saveLocation = 0;
    int startPage = 0;

    // loop thrugh the file
    while (!file.eof()) {

      // Move file pointer to saveLocation
      file.seekg(saveLocation);

      // seek the next new line
      char currentByte;
      while (file.read(&currentByte, sizeof(char)) &&
             currentByte != newlineDelimiter) {
      }

      // Check if we reached the end of the file
      if (file.eof()) {
        continue;
      }

      // Store where the file pointer is in a varible
      saveLocation = file.tellg();

      // Move the file pointer back to read the int before the newline, moving
      // past the newline as well
      file.seekg(-sizeof(int) - 1, ios::cur);

      // Read the int before the newline aka the size of the page
      int sizeOfPage;

      // Read in the size of records from slot directory
      file.read(reinterpret_cast<char *>(&sizeOfPage), sizeof(int));

      // Move the file pointer back to get the number of records while
      // accounting for the previously read int
      file.seekg(-sizeof(int) * 2, ios::cur);

      // Read in the number of records from slot directory
      file.read(reinterpret_cast<char *>(&numRecords), sizeof(int));

      // clear slot directory
      slotDirectory.clear();

      // Add in OFFSET WHERE THE FREE DATA STARTS
      slotDirectory.push_back(sizeOfPage);

      // Add in NUMBER OF RECORDS IN PAGE
      slotDirectory.push_back(numRecords);

      // intialize offset int
      int offset = 0;

      // Loop to fill slot directory
      for (int i = 0; i < numRecords; i++) {

        // Move the file pointer back to get the next offset while accounting
        // for last read in int
        file.seekg(-sizeof(int) * 2, ios::cur);

        // Read in the next offset
        file.read(reinterpret_cast<char *>(&offset), sizeof(int));

        // Put offset into slot directory
        slotDirectory.push_back(offset);
      }

      // Move file pointer back to the start of the page
      file.seekg(startPage);

      // Update start page for the next run
      startPage = saveLocation;

      // allocate a block to store the data
      currentBlock = allocateBlock();

      // Read the file data into the buffer according to the value in the slot
      // directory
      file.read(currentBlock->buffer, slotDirectory[0]);

      // An int to track where in the buffer we are while parsing
      int bufferLocation = 0;

      // Varibles to store records
      int numFields = 0;
      vector<std::string> fields;

      // Parse the buffer Record by Record Looking for the ID
      while (bufferLocation < slotDirectory[0]) {

        // Get the number of fields (converts to int value)
        numFields = (currentBlock->buffer[bufferLocation]) - '0';

        // increase the buffer location
        bufferLocation++;

        // Get the fields for Record
        for (int i = 0; i < numFields; i++) {

          // Store the field here
          stringstream ss;
          ss.str("");
          ss.clear();

          // Loop until a '$' is found
          while (currentBlock->buffer[bufferLocation] != '$') {
            ss << currentBlock->buffer[bufferLocation];

            bufferLocation++;
          }

          // insert the field
          fields.push_back(ss.str());

          // increase the buffer location past the '$'
          bufferLocation++;
        }

        // Create a record object
        Record newRecord(fields);


        // Check if Id in new record is the ID we are looking for
        if (newRecord.id == id) {
          return newRecord;
        }

        // Clear fields
        fields.clear();
      }
    }

    file.close();

    // return an empty record
    vector<string> fakeFields = {"-1", "John Doe", "Some bio",
                                 "456"}; // Fake Employee
    return Record(fakeFields);           // return an Fake record
  }
};
