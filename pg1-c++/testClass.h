
#ifndef TestClass_H
#define TestClass_H

#include <iostream>

class TestClass {

  public:
  std::string name;

  TestClass(std::string aName = "unnamed");
  virtual ~TestClass();

  private:

};

#endif
