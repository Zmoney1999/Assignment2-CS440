/******
Class: CS 440
Assignment: Assignment 2
Names: Jake Gehrke & Zach Chand
Onid: gehrkej@oregonstate.edu & chandz@oregonstate.edu
******/

#include "classes.h"
#include <cmath>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *const argv[]) {

  // Create the EmployeeRelation file from Employee.csv
  StorageBufferManager manager("EmployeeRelation");
  manager.clearEmployeeRelations(); // Clear the file before loading
  manager.createFromFile("Employee.csv");

  // Loop to lookup IDs until user is ready to quit
  bool done = false;
  while (!done) {
    // Prompt user for ID
    cout << "Enter an employee ID (-1 to quit): ";
    int id;
    cin >> id;
    // Check if user wants to quit
    if (id == -1) {
      done = true;
      cout << "\nProgram is now exiting" << endl;
      continue;
    }
    // Lookup the employee
    else {
      Record employee = manager.findRecordById(id);

      // check if employee is empty
      if (employee.id == -1) {
        cout << "Employee with id " << id << " not found" << endl;
      } else {
        cout << "***************************" << endl;
        employee.print();
        cout << "***************************" << endl;
      }
    }
  }

  return 0;
}
