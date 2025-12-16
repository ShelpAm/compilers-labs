#pragma once
#include <string>

// Symbols:
// *
// +
// .
// (
// )
// a-zA-Z
// |

// *
// ax*b
// can be converted to
// 1 -a> 2 -x> 2 -b> 3

// ax*b*c
// 1 -a> 2 -x> 2 -eps> 3 -b> 3 -eps> 4 -c> 5

// +
// ax+b
// can be converted to
// axx*b

// .
// a.b
// a -charset> b

// INVALID case:
// *
// x*+

class RegularExpression {
    friend class NondeterminsticFiniteAutomaton;

  public:
    RegularExpression(std::string re) : re_(std::move(re)) {}

  private:
    std::string re_;
};
