
#include "testClass.h"
#include <iostream>


TestClass::TestClass(std::string aName):name(aName) {
  std::cout << "constructing testClass with name: " << name << std::endl;
}

TestClass::~TestClass() {
  std::cout << "destructing testClass with name: " << name << std::endl;
}
