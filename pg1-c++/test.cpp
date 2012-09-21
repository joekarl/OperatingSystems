
#include "testClass.h"

int main(int argc, char ** argv)
{
  TestClass t("t");
  TestClass t3;

  TestClass *t2 = new TestClass("test");

  delete t2;

  return 0;
}
