#pragma once

#include <new>


int& newCallCount();


void* operator new  ( std::size_t count );
void* operator new[]( std::size_t count );


//void* operator new  ( std::size_t count, const std::nothrow_t& tag);
//void* operator new[]( std::size_t count, const std::nothrow_t& tag);

