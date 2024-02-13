#ifndef _EXCEPTION_HPP
#define _EXCEPTION_HPP

#include <exception>
#include <string>

class CustomException: public std::exception {
private :

  const char* message;


public : 

  CustomException(const char* msg) : message(msg) {

  }

  const char* what() const noexcept override {
    return message;
  }

};

#endif