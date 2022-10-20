/**
*******************************************************************************
* @file	compiler_defines.cpp
* @author	Samuel Martel
* @brief   This file's content is really meant to reduce program size.
* Created on: May 12, 2021
*******************************************************************************
*/
#include "../processes/application.hpp"

#include <malloc.h>
#include <new>


void* operator new(std::size_t size);
void* operator new[](std::size_t size);

void operator delete(void* ptr);
void operator delete[](void* ptr);
void operator delete(void* ptr, size_t);
void operator delete[](void* ptr, size_t);

/* Optionally you can override the 'nothrow' versions as well.
  This is useful if you want to catch failed allocs with your
  own debug code, or keep track of heap usage for example,
  rather than just eliminate exceptions.
*/

void* operator new(std::size_t size, const std::nothrow_t&) noexcept;
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept;

void  operator delete(void* ptr, const std::nothrow_t&);
void operator delete[](void* ptr, const std::nothrow_t&);

extern "C" void __cxa_pure_virtual(void);
