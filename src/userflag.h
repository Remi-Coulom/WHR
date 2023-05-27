////////////////////////////////////////////////////////////////////////////
//
// userflag.h
//
// CUserFlag class declaration
//
// Remi Coulom
//
// September, 1996
//
////////////////////////////////////////////////////////////////////////////
#ifndef USERFLAG_H
#define USERFLAG_H

#ifndef CDECL
#define CDECL
#endif

#include <vector>
#include <cstddef>

class CUserFlag // flag
{
 private: //////////////////////////////////////////////////////////////////
  static std::vector<int> vState;
  static void CDECL InterruptSignalHandler(int sig);

  const std::size_t Index;

 public: ///////////////////////////////////////////////////////////////////
  CUserFlag();

  int IsSet() const;
  void Reset();
  void Set();

  static void DeActivate();
  static void Activate();
};

#endif
