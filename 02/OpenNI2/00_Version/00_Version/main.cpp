#include <iostream>

#include <OpenNI.h>

int main(int argc, const char * argv[])
{
  openni::Version version = openni::OpenNI::getVersion();
  std::cout << "OpenNI version : "
            << version.major << "."
            << version.minor << "."
            << version.maintenance << "."
            << version.build << std::endl;
  
  return 0;
}

